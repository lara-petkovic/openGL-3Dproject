#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inCol;
layout(location = 2) in vec2 inTex;
layout(location = 3) in vec3 inNor;

uniform vec3 uTranslation;

out vec3 chCol;
out vec2 chTex;
out vec3 chNor;

uniform mat4 uM;
uniform mat4 uP;
uniform mat4 uV;

void main()
{
	gl_Position = uP * uV * uM * vec4(inPos + vec3(uTranslation.x, uTranslation.y, uTranslation.z), 1.0);
	chTex = vec2((inPos.x + 1.0) * 0.5, (inPos.z + 1.0) * 0.5);
	chCol = inCol;
	chNor = mat3(transpose(inverse(uM))) * inNor;
}