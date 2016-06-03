

uniform mat3 lightRot;
uniform vec3 lightTra;
uniform vec3 lightDir;
uniform vec4 cutPlane;
uniform sampler2D shadowMap;


varying vec2 cor; // corner pos, [-1,+1]^2
varying float rad; // radius
varying vec3 pos;


// inverse of the octMap
vec3 octaMapI( vec2 uv ){
	vec3 res;
        res.z = 1.0 - dot( abs(uv) , vec2(1.0));
	vec2 signs = sign( uv );
        res.xy = (res.z>=0.0) ? uv.yx : (signs.x * signs.y)*(signs -  uv) ;
	return normalize(res);
}


void main(){
	vec3 norm = octaMapI( cor );
        vec3 p = pos + norm * rad;


	float diffuse = dot( norm , lightDir );

        p = lightRot*(p+lightTra);
        p = ( p + 1.0) / 2.0 ; // TODO: avoid this op

        p.z = (diffuse>=0.0)? 1.0-p.z : p.z;
        p.x = 0.5 * (p.x + ((diffuse>=0.0)? 0.0 : 1.0) );

        float depth = texture( shadowMap, p.xy ).x;

        if ( depth<p.z ) discard;
        gl_FragColor.xyz = vec3( abs( diffuse ) );


}
