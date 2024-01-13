#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec2 chTex;

uniform mat4 uM;
uniform mat4 uP;
uniform mat4 uV;


void main()
{
    gl_Position = uP * uV * uM * vec4(inPos.x, inPos.y, inPos.z, 1.0);
    chTex = vec2((inPos.x + 1.0) * 0.5, (inPos.z + 1.0) * 0.5);
}