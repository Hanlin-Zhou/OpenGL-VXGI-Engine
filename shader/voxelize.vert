#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT{
    vec3 Pos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

void main()
{
    vs_out.Pos = aPos;
    vs_out.Normal = aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}  