#version 330 core

uniform vec3 uCol;
out vec4 FragColor;

void main() {
	FragColor = vec4(uCol, 1.0f);
}