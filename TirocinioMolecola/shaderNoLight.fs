#version 330


// Interpolated values from the vertex shaders
in vec3 fragmentColor;
in vec3 PositionWorld;
in vec3 NormalCamera;
in vec3 EyeDirectionCamera;
in vec3 LightDirectionCamera;


//output data
out vec3 color;

void main(){


    color =  fragmentColor;
}

