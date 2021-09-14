#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Tangent;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 proj;
uniform mat4 view;

void main()
{    
    //vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPos = aPos;
    // vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.Normal = aNormal;
    vs_out.TexCoords = aTexCoords;
    vec3 Ntangent = normalize(aTangent);
    vec3 B = normalize(cross(normalize(aNormal), Ntangent));
    
    vs_out.TBN = mat3(B, Ntangent, normalize(aNormal));
    vs_out.Tangent = B;
    gl_Position = proj * view * vec4(aPos, 1.0);
}