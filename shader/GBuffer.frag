#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gTangent;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Tangent;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform int MSAA_Sample;
in int gl_SampleMaskIn[1];

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_opacity1;
uniform sampler2D texture_normal1;

void main()
{    
    float div = pow(2, MSAA_Sample) - 1.0;
    float opa = texture(texture_opacity1, fs_in.TexCoords).r;
    if (opa < 0.1){
        discard;
    }
    gPosition = fs_in.FragPos;
    vec3 norm = texture(texture_normal1, fs_in.TexCoords).rgb;
    if (length(norm) == 0.0){
        gNormal = vec4(fs_in.Normal, float(gl_SampleMaskIn[0])/ div);
    }else{
        norm.xy = norm.xy * 2.0 - 1.0;
        gNormal = vec4(normalize(fs_in.TBN * norm), float(gl_SampleMaskIn[0])/ div);
    }
    gTangent = fs_in.Tangent;
    gAlbedoSpec.rgb = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    gAlbedoSpec.a = texture(texture_specular1, fs_in.TexCoords).r;


}  