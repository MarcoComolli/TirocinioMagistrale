uniform mat3 lightRot;
uniform vec3 lightTra;
uniform float lightScale;

attribute vec3 posit;
attribute float radius;

varying vec2 cor; // which corner of the impostor, in [-1,+1]^2

void main()
{

	cor = gl_Vertex.xy;

	gl_Position.xyz = lightRot * (posit + lightTra);

        gl_Position.xy += cor * ( radius/4096.0 * lightScale );
        gl_Position.w = 1.0;

}
