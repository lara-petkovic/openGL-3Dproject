#version 330 core

struct Material {
    vec3 kA;
    vec3 kD;
    vec3 kS;
    float shine;
};

in vec3 chFragPos;
in vec3 chNor;

out vec4 outCol;

uniform vec3 color;
uniform float uAlpha;
uniform sampler2D uTex;

uniform Material uMaterial;
uniform vec3 uViewPos;

void main()
{
    vec3 resA = vec3(0.1);

    vec3 normal = normalize(chNor);
    
    vec3 lightDirection = normalize(vec3(0.0, 1.8, 0.0)); // Mesecina
    
    float nD = max(dot(normal, lightDirection), 0.0);
    vec3 resD = vec3(0.5) * (nD * uMaterial.kD);

    vec3 viewDirection = normalize(uViewPos - chFragPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float s = pow(max(dot(viewDirection, reflectionDirection), 0.0), uMaterial.shine);
    vec3 resS = vec3(0.2) * (s * uMaterial.kS); // Specular term (adjust intensity as needed)

    vec3 finalColor = resA + resD + resS;

    outCol = vec4(color * finalColor, 1.0 - uAlpha);
}
