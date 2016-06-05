#version 330

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 vertexPositionModel;
layout(location = 1) in vec3 vertexColor;


out vec3 shaded_col;


uniform vec3 lineColor;
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main()
{

    //lineColor = vec3(0.0,0.0,0.0);
    gl_Position =  MVP* vec4(vertexPositionModel, 1.0);
    vec3 view_pos = (V*M*vec4(vertexPositionModel, 1.0)).xyz;
    float fog = 0.5-(view_pos.z/100.0);
    fog = clamp(fog,0.0,1.0);
    vec3 bgCol = vec3(0.17, 0.20, 0.20);
    shaded_col = mix(bgCol, lineColor, fog);
}


