#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec4 FPLS[6];
} vs_out;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;
uniform mat4 LSM[6];
uniform bool soft_shadow;

void main()
{    
    //vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPos = aPos;
    // vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.Normal = aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    if (soft_shadow){
        for (int i = 0; i < 6; i++){
            vs_out.FPLS[i] = LSM[i] * vec4(vs_out.FragPos, 1.0);
        }
    }
    gl_Position = proj * view * vec4(vs_out.FragPos, 1.0);
}