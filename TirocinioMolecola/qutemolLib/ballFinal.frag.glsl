uniform float patchSize;

#if USE_AMBIENT_LIGHT
  uniform sampler2D  irradianceMap;
  uniform float aoMul, aoSum;
#elif USE_FIXED_AMBIENT
  uniform float aoSum;
#endif

#if USE_CUT_PLANE
uniform vec4 cutPlane;
uniform vec4 lightoutCut;
#endif

#if USE_FOG
uniform vec3 fogColor;
uniform vec2 fogAmount;
#endif

#if USE_SHADOW_MAP
uniform mat4x4 shadowMatrix;
uniform sampler2DShadow shadowMap;
#endif

#if USE_SHADOW_ATTENUATION
uniform float shadowAttenuation;
#endif

varying vec2 cor; // corner pos, [-1,+1]^2
flat varying float rad; // radius
flat varying vec3 col;
flat varying vec2 uvVar;

#if USE_FLAT_HIGHLIGHTS
uniform float shineThreshold;
#endif

#if USE_COLORED_LIGHTS
uniform vec3 lightColorOpposite;
uniform vec3 lightColorDirect;
uniform vec3 lightColorAmbient;
#elif USE_TWO_WAY_LIGHT
uniform float oppositeLight;
#endif

#if USE_DIRECT_LIGHT
uniform vec3 lightDir;
#endif

#if USE_SPECULAR
uniform float shine, gloss;
uniform vec3 halfwayDir;
uniform vec3 oppositeHalfwayDir;
uniform float oppositeShine;
#endif

#if USE_DIRECT_LIGHT_BONUS
uniform float directLightBonus;
#endif


#if USE_BORDERS
uniform float borderSize;
uniform float borderColorMul;
flat varying float radiusWithBorder;
uniform float borderVariance;
#endif


#define ao lightout.x
#define diffuse lightout.y
#define spec lightout.z
#define nl lightout.w



// parametrizes: (unit sphere surface) -> [-1,+1]^2
vec2 octMap( vec3 pos ){

  pos /= dot( abs(pos), vec3(1) );
  return (pos.z>=0) ? pos.yx :  (1 -  abs(pos).xy) * sign(pos.yx);

/*
  // another way
  vec3 signs = sign( pos );
  pos /= dot( pos, signs );

  return (pos.z>=0) ? pos.yx : (signs.x * signs.y)*(signs.xy -  pos.xy) ;
*/


/*

  // yet another way
  pos /= dot( abs(pos), vec3(2) );
  vec3 s = sign( pos );
  
  vec3 tmp =  vec3(1,-1,-1)*s.z + vec3(1);
  tmp.yz *= s.y*s.x;

  pos.z = 1;
  return vec2( dot( tmp, pos), dot( tmp, pos.yxz) );
*/


}

// inverse of the octMap
vec3 octaMapI( vec2 uv ){
  vec3 res;
  vec2 signs = sign( uv );
  res.z = 1 - dot( uv , signs );
  res.xy = (res.z>=0) ? uv.yx : (signs.x * signs.y)*(signs -  uv) ;
  return normalize(res);
}

vec3 procText( vec2 uv ){
  //uv = (uv*0.5+vec2(0.5,0.5)) ;
  //uv = 0.1*uv + 0.9*frac( 1024 * abs(uv) );

  return vec3( uv.x, 0.5, uv.y  );
}



void main(){

  vec3 spherePos;
  spherePos.xy = cor;
#if USE_BORDERS
  spherePos.z = dot(cor,cor);
  if ( spherePos.z>=radiusWithBorder ) discard;
  spherePos.z = 1.0 - spherePos.z;
  spherePos.z = (spherePos.z<0)?spherePos.z*borderVariance*10:sqrt(spherePos.z);
#else
  spherePos.z = 1.0 - dot(cor,cor);
  if ( spherePos.z<0.0f ) discard;
  spherePos.z = sqrt(spherePos.z);
#endif


  gl_FragDepth =  gl_FragCoord.z - spherePos.z*rad;

  vec4 lightout; // x = ambient; y = abs(diffuse); z = shine; w = n*l;

  #if USE_TRIVIAL_LIGHT
  #if USE_DIRECT_LIGHT
  lightout.zw = vec2(spherePos.z) * vec2(1.0,lightDir.z);
  #else
  lightout.z = spherePos.z;
  #endif
  #endif

  #if USE_DIRECT_LIGHT
    #if USE_TRIVIAL_LIGHT
    #else
    nl = dot( spherePos, lightDir );
    #endif
  #endif


#if USE_SPECULAR
   #if USE_TRIVIAL_LIGHT
   #else
   spec = dot( spherePos, halfwayDir );
   #endif
   #if USE_TWO_WAY_LIGHT
      float spec2 = dot( spherePos, oppositeHalfwayDir );
      float currShine  = (spec>spec2)?shine:oppositeShine;
      spec = (spec>spec2)?spec:spec2;
   #else
      #define currShine shine
   #endif
   #if USE_FLAT_HIGHLIGHTS
      spec = ( spec > shineThreshold)? currShine: 0;
   #else
      spec = pow( abs(spec), gloss) * currShine;
   #endif
#endif

#if USE_AMBIENT_LIGHT
  vec2 tpos = uvVar + patchSize*octMap( spherePos*gl_NormalMatrix );
  ao = texture( irradianceMap, tpos ).r;
  //#if SKIP_INVISIBLE if ( ao == 0) discard; #endif
  ao = ao * aoMul + aoSum;
#elif USE_FIXED_AMBIENT
  ao = aoSum;
#endif

#if USE_DIRECT_LIGHT_BONUS
  #define PLUS_directLightBonus   +directLightBonus
  #define MINUS_directLightBonus  -directLightBonus
#else
  #define PLUS_directLightBonus
  #define MINUS_directLightBonus
#endif


#if USE_SEM_MODE
    nl = lightDir.z - nl;
    /*nl *= nl;*/
#endif

#if USE_COLORED_LIGHTS
  diffuse = abs(nl) PLUS_directLightBonus;
#else
  #if USE_TWO_WAY_LIGHT
     diffuse = (nl<0)
       ? oppositeLight*(nl MINUS_directLightBonus)
       : (nl PLUS_directLightBonus);
  #else
     diffuse = (nl<0) ?
            #if (USE_DIRECT_LIGHT_BONUS && USE_SHADOW_ATTENUATION)
                directLightBonus
            #else
                 0
            #endif
               : (nl PLUS_directLightBonus);
  #endif
#endif


#if USE_FOG
  float fog = fogAmount.x * gl_FragDepth + fogAmount.y;
#endif

#if USE_CUT_PLANE
  float dist = dot( vec4(gl_FragCoord.xyz,1) , cutPlane) ;
  float delta = (spherePos.z*rad)*cutPlane.z;
  #if USE_BORDERS
  delta = (spherePos.z<0)?0:delta;
  #endif
  if ( dist < -abs(delta) ) discard;
  lightout = (dist > delta)?lightout:lightoutCut;
  //gl_FragDepth =  (dist > delta)? gl_FragDepth:0.0;
#endif

#if USE_SHADOW_MAP
  vec4 realFragCoord = vec4(gl_FragCoord.xyz,1);
  //realFragCoord.w = gl_FragCoord.w;
  realFragCoord.z -= spherePos.z*rad;
  vec4 shadowMapPos = shadowMatrix * realFragCoord;
  #if USE_TWO_WAY_LIGHT
      // pick the left or right side of the shadowmap (according to which of the two oppsite lights is visible)
      shadowMapPos.z = (nl>0)? shadowMapPos.z : shadowMapPos.w-shadowMapPos.z ;
      shadowMapPos.x += (nl>0)? 0.0 : shadowMapPos.w;
      shadowMapPos.x *= 0.5 ;
  #endif
  float shadowed = shadow2DProj(shadowMap,shadowMapPos).r;

  #if USE_SHADOW_ATTENUATION
  shadowed = mix(shadowed, 1, shadowAttenuation );
  #endif

  lightout.yzw *= shadowed; // remove any direct light components if shadowed;
#endif

#if USE_SEM_MODE
 // fake sem rendering
 gl_FragColor.rgb =  col* (
 #if USE_AMBIENT_LIGHT | USE_FIXED_AMBIENT
  ao
 #else
  0
 #endif
  *diffuse
 );
#elif USE_COLORED_LIGHTS
  vec3 currColDiffuse = (nl>=0) ? lightColorDirect: lightColorOpposite;
  gl_FragColor.rgb =  col* ( 0
  #if USE_AMBIENT_LIGHT | USE_FIXED_AMBIENT
    + ao * lightColorAmbient
  #endif
  #if USE_DIRECT_LIGHT
    + diffuse * currColDiffuse
  #endif
  );
  #if USE_SPECULAR
  gl_FragColor.rgb += currColDiffuse * spec;
  #endif
#else
  gl_FragColor.rgb =  col* (
  #if USE_AMBIENT_LIGHT | USE_FIXED_AMBIENT
   + ao
  #endif
  #if USE_DIRECT_LIGHT
   + diffuse
  #endif
  );
  #if USE_SPECULAR
  gl_FragColor.rgb = mix( gl_FragColor.rgb, vec3(1) , spec);
  #endif
#endif

#if USE_BORDERS
  gl_FragColor.rgb = (spherePos.z>0)? gl_FragColor.rgb : col* borderColorMul;
#endif

#if USE_FOG
  gl_FragColor.rgb = mix(gl_FragColor.rgb, fogColor, fog);
#endif

#if CARE_FOR_ALPHA
  gl_FragColor.a = 1.0;
#endif


#if USE_SHADOW_MAP
  //gl_FragColor.xy = fract((shadowMapPos.xy / shadowMapPos.w)/512.0*6.0);
  //gl_FragColor.xyz = vec3(shadowMapPos.z);
  //gl_FragColor.xyz =  vec3( texture(shadowMap,shadowMapPos.xy/512.0).r );
#endif


}
