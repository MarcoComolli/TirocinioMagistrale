
uniform vec4 cutPlane;

varying vec2 cor; // corner pos, [-1,+1]^2


void main(){

        vec3 spherePos;
	spherePos.xy = cor;
	spherePos.z = 1.0 - dot(cor,cor);
        if ( spherePos.z < 0.0 ) discard;

#if USE_CUT_PLANE
        if ( dot(gl_FragCoord , cutPlane) < 0.0 ) discard;
#endif

}
