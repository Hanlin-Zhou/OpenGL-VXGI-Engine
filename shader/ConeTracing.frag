#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
uniform vec3 viewPos;
// uniform bool HDR;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D Shadow;
uniform sampler3D Radiance3D;

uniform vec3 lightPos;

// uniform sampler2D SSAO;
// uniform sampler2D skybox;

uniform float VoxelCellSize; 
uniform float MaxCoord; 
uniform int VoxelSize;
uniform mat4 ProjectMat;

vec3 convWorldPosToVoxelPos(vec3 pos){
     vec4 temp = ProjectMat * vec4(pos, 1.0);
     temp.z = -temp.z;
     return clamp(((temp + 1.0f) * 0.5f).xyz, vec3(0.0), vec3(1.0));
}

vec4 sampleVoxel(vec3 pos, float diameter){
    float MipLevel = log2(max(diameter, VoxelCellSize) / VoxelCellSize);
    vec3 VoxelPos = convWorldPosToVoxelPos(pos);
    return textureLod(Radiance3D, VoxelPos, min(MipLevel, 6));
}


vec4 ConeTracing(vec3 origin, vec3 direction){
    float t = 3 * VoxelCellSize;
    vec3 acc = vec3(0.0);
    float occlusion = 0.0;
    float max_t = 2.828426 * MaxCoord;
    float diameter = t * 0.2;
    vec3 currPos = origin + t * direction;

    while(occlusion < 1 && t < max_t){
        vec4 s = sampleVoxel(currPos, diameter);
        acc += (1.0 - occlusion) * s.a * s.xyz;
        occlusion += (1.0 - occlusion) * s.a;
        //step
        t += diameter / 2.0;
        diameter = t * 0.222;
        t += diameter / 2.0;
        currPos = origin + t * direction;
    }
    return vec4(acc, 1.0);

}
//vec4 IndirectLighting(){
//    
//}
//
//
//vec4 DirectLighting(){
//    
//}

void main()
{           
    vec4 FragPos = texture(gPosition, TexCoords);
    float Valid = 1.0 - FragPos.a;
//    float invalid = texture(gPosition, TexCoords).a;
//    vec3 skyboxColor = texture(skybox, TexCoords).rgb;
//    vec3 color = texture(gAlbedoSpec, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
//    float ks = texture(gAlbedoSpec, TexCoords).a;
//    vec3 lightColor = vec3(1.0);
//    vec3 lightDir = normalize(lightPos - FragPos);
//    float falloff = 1000.0/(4.0 * 3.1415 * length(lightPos - FragPos) * length(lightPos - FragPos));
//    float diff = max(dot(lightDir, normal), 0.0);
//    float diffuse = diff * falloff;
    vec3 viewDir = normalize(viewPos - FragPos.xyz);
//    float spec;
//    vec3 halfwayDir = normalize(lightDir + viewDir);  
//    spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0);
//    float specular = spec * ks * falloff;
//    float shadow = texture(Shadow, TexCoords).r;
//    vec3 lighting = shadow * (diffuse + specular) * color;
    vec3 specularTrace = normalize(reflect(-viewDir, normal));
//    if (invalid == 1.0){
//        lighting = skyboxColor;
//    }
//    if (HDR){
//        lighting = lighting / (lighting + vec3(1.0));
//        float ssao = texture(SSAO, TexCoords).r;
//        lighting *= ssao;
//    }
//    FragColor = vec4(lighting, 1);
    FragColor = ConeTracing(FragPos.xyz, specularTrace) * Valid;
    // FragColor = sampleVoxel(FragPos.xyz, 1.0);
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}  