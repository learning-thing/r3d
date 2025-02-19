#version 330 core

/* === Attributes === */

layout(location = 0) in vec3 aPosition;

/* === Instanced attributes === */

layout(location = 10) in mat4 aInstanceModel;

/* === Uniforms === */

uniform mat4 uMatModel;
uniform mat4 uMatMVP;

/* === Varyings === */

out vec3 vPosition;

/* === Main function === */

void main()
{
    mat4 instanceModel = transpose(aInstanceModel);
    vPosition = vec3((uMatModel * instanceModel) * vec4(aPosition, 1.0));
    gl_Position = uMatMVP * (instanceModel * vec4(aPosition, 1.0));
}
