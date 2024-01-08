#version 330 core

layout (location = 0) in vec3 aPosition;

uniform vec2 uTranslation;
uniform mat4 uM;
uniform mat4 uP;
uniform mat4 uV;
void main()
{
    gl_Position = uP * uV * uM * vec4(aPosition.x + uTranslation.x, aPosition.y, aPosition.z + uTranslation.y, 1.0);
}
