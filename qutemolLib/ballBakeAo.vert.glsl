uniform float patchSize;
uniform float keepInsideFactor;

attribute vec3 position;
attribute float radius;
attribute vec3 color;
attribute vec2 uv; // patchCenter

varying vec2 cor; // corner point
varying float rad;
varying vec3 pos;



void main()
{
  pos = position;
  cor = gl_Vertex.xy*keepInsideFactor; // to stay inside 1/2 texel of the patch
  rad = radius/4096.0;

  gl_Position.xy = (uv/16384.0 + gl_Vertex.xy*patchSize);
  gl_Position.xy = gl_Position.xy*2.0 - vec2(1.0);
  gl_Position.z = 0.5;
  gl_Position.w = 1.0;
}



