#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uMatModel;
uniform mat4 uMatMVP;

out vec3 vPosition;

void main()
{
    vPosition = vec3(uMatModel * vec4(aPosition, 1.0));
    gl_Position = uMatMVP * vec4(aPosition, 1.0);
}
