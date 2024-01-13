#version 330 core

in vec2 chTex;
in vec3 chCol;

out vec4 outCol;

uniform vec3 color;
uniform float uAlpha;
uniform sampler2D uTex;
uniform bool useTexture;

void main()
{
    if (useTexture) {
        vec4 texColor = texture(uTex, chTex);
        outCol = texColor * vec4(1.0, 1.0, 1.0, 1.0 - uAlpha);
    } else {
        outCol = vec4(chCol.rgb + color, 1.0 - uAlpha);
    }
}
