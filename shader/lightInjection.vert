#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoords;

out VS_OUT{
    vec2 vTexCoords;
    vec3 vNormal;
    vec3 vPos;
} vs_out;

void main()
{
    vs_out.vTexCoords = aTexCoords;
    vs_out.vNormal = aNormal;
    vs_out.vPos = aPos;
    gl_Position = vec4(aPos, 1.0);
}  