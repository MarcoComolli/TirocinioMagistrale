varying vec2 uv;
uniform vec2 textureSize;

void main(){
    uv = gl_Vertex.xy * textureSize;
    gl_Position = gl_Vertex*2.0-1.0;
}
