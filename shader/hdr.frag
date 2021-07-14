#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{             
    float gamma = 2.2;
    vec3 color = pow(texture(depthMap, TexCoords).rgb, vec3(gamma));
    vec3 mapped = color / (color + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);
}