#version 330 core

struct Light {
    vec3 pos;
    vec3 kA;
    vec3 kD;
    vec3 kS;
};

struct Material {
    vec3 kA;
    vec3 kD;
    vec3 kS;
    float shine;
};

in vec2 chTex;
in vec3 chCol;
in vec3 chNor;

out vec4 outCol;

uniform vec3 color;
uniform float uAlpha;
uniform sampler2D uTex;
uniform bool useTexture;

uniform Light uLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

void main()
{
    vec3 resA = uLight.kA * uMaterial.kA;

    vec3 normal = normalize(chNor);
    vec3 lightDirection = normalize(uLight.pos - gl_FragCoord.xyz);
    float nD = max(dot(normal, lightDirection), 0.0);
    vec3 resD = uLight.kD * (nD * uMaterial.kD);

    vec3 viewDirection = normalize(uViewPos - gl_FragCoord.xyz);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float s = pow(max(dot(viewDirection, reflectionDirection), 0.0), uMaterial.shine);
    vec3 resS = uLight.kS * (s * uMaterial.kS);

    vec3 finalColor = resA + resD + resS;

    if (useTexture) {
        vec4 texColor = texture(uTex, chTex);
        outCol = texColor * vec4(finalColor, 1.0 - uAlpha);
    } else {
        outCol = vec4(chCol.rgb + color * finalColor, 1.0 - uAlpha);
    }
}