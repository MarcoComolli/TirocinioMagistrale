varying vec2 uv;
uniform sampler2D colMap;
uniform sampler2D depthMap;
uniform vec2 texelSize;
uniform vec3 dofQuadric;

float blurAmount(float z) {
	//return (x<0.999)?0:1;
	//z = (z-1+0.003)*500.0;
	//return 1-clamp(z,0.0,1.0);
	//return clamp( dot( vec3(1,z,z*z) , dofQuadric ), 0.0, 1.0);
	z += dofQuadric.x;
	z = abs(z)*dofQuadric.y + dofQuadric.z;
	return clamp(z,0.0,1.0);
}

/*const vec2 offsets[] = vec2[](
   SAMPLES_VERBATIM
);*/

#if CARE_FOR_ALPHA
vec4 cumulateSample( vec4 pix ){
    return vec4( pix.rgb *pix.a, pix.a );
}
#else
#define cumulateSample( X ) X
#endif

void main(){
        float dof0 = blurAmount( texture( depthMap, uv ).r );
        vec2 ts = texelSize * dof0 ;
        vec4 res = cumulateSample( texture( colMap, uv ) );
        //res += texture( colMap, uv );
        res += cumulateSample( texture( colMap, uv+ts*vec2(-1.4, 0.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 0.0, 1.4) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 1.4, 0.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 0.0,-1.4) ) );

        res += cumulateSample( texture( colMap, uv+ts*vec2(-1.0, 1.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 1.0, 1.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2(-1.0,-1.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2(+1.0,-1.0) ) );

        res += cumulateSample( texture( colMap, uv+ts*vec2(-0.7, 0.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 0.0,+0.7) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2(+0.7, 0.0) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 0.0,-0.7) ) );

        res += cumulateSample( texture( colMap, uv+ts*vec2(-0.5, 0.5) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2( 0.5, 0.5) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2(-0.5,-0.5) ) );
        res += cumulateSample( texture( colMap, uv+ts*vec2(+0.5,-0.5) ) );
#if CARE_FOR_ALPHA
        gl_FragColor.rgb = res.rgb/res.a;
        gl_FragColor.a = res.a/17.0;
#else
        gl_FragColor = res/17.0;
#endif
	/*vec4 res = texture( colMap, uv );
	res.x =  (texture( depthMap, uv ).r < dofQuadric.x)?0:1;
	gl_FragColor = res;*/
}
