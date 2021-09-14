#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
uniform vec3 viewPos;
// uniform bool HDR;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gTangent;
uniform sampler2D Shadow;
uniform sampler3D Radiance3D;

uniform vec3 lightPos;

// uniform sampler2D SSAO;
// uniform sampler2D skybox;

uniform float VoxelCellSize; 
uniform float MaxCoord; 
uniform int VoxelSize;
uniform mat4 ProjectMat;

uniform float offsetFactor;
uniform float SpecularAperture;
uniform float DiffuseAperture;
uniform float OcculsionAperture;
uniform float stepSize;

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


vec4 ConeTracing(vec3 origin, vec3 direction, float aperture){
    float t = offsetFactor * VoxelCellSize;
    vec3 acc = vec3(0.0);
    float occlusion = 0.0;
    float max_t = 2.828426 * MaxCoord;
    float diameter = 2.0 * t * tan(aperture/ 2.0);
    vec3 currPos = origin + t * direction;

    while(occlusion < 1 && t < max_t){
        vec4 s = sampleVoxel(currPos, diameter);
        acc += (1.0 - occlusion) * s.a * s.xyz;
        occlusion += (1.0 - occlusion) * s.a;
        //step
        // t += diameter / 2.0;
        // diameter = t * 0.222;
        // t += diameter / 2.0;
        t += stepSize * diameter;
        diameter = 2 * t * tan(aperture/ 2.0);
        currPos = origin + t * direction;
    }
    return vec4(acc, 1.0);

}

float OcclusionConeTracing(vec3 pos, vec3 lightPos, vec3 normal, float aperture){
    float t = offsetFactor * VoxelCellSize;
    float occlusion = 0.0;
    vec3 lightDir = lightPos - pos;
    float max_t = length(lightDir);
    lightDir = normalize(lightDir);
    vec3 currPos = pos + t * lightDir;
    float diameter = 2.0 * t * tan(aperture/ 2.0);
    while(occlusion < 1.0 && t < max_t) {
        float s = sampleVoxel(currPos, diameter).a;
        occlusion += s;
         t += stepSize * diameter;
        diameter = 2 * t * tan(aperture/ 2.0);
        currPos = pos + t * lightDir;
    }
    return (1.0 - min(1.0, occlusion)) * max(dot(lightDir, normal), 0.0);

}

vec4 IndirectSpecularLighting(vec3 pos, vec3 traceDir){
   return ConeTracing(pos, traceDir, SpecularAperture);
}

vec4 IndirectDiffuseLighting(vec3 pos, vec3 normal, vec3 tangent, vec3 bitangent){
   vec3 dir1 = mix(normal, tangent, 0.666);
   vec3 dir2 = mix(normal, -tangent, 0.666);
   vec3 dir3 = mix(normal, bitangent, 0.666);
   vec3 dir4 = mix(normal, -bitangent, 0.666);
   vec3 dir5 = normal;
   vec4 color = ConeTracing(pos, mix(normal, tangent, 0.666), DiffuseAperture);
   color += ConeTracing(pos, mix(normal, -tangent, 0.666), DiffuseAperture);
   color += ConeTracing(pos, mix(normal, bitangent, 0.666), DiffuseAperture);
   color += ConeTracing(pos, mix(normal, -bitangent, 0.666), DiffuseAperture);
   color += ConeTracing(pos, normal, DiffuseAperture);
   color /= 5.0;
   return color;
}


vec4 DirectLighting(vec3 pos, vec3 viewDir, vec3 normal, vec3 lightPos){
    vec4 color = texture(gAlbedoSpec, TexCoords);
    vec3 PosToLight = lightPos - pos;
    float LightDist = length(PosToLight);
    vec3 lightDir = normalize(PosToLight);
    float diff = max(dot(lightDir, normal), 0.0);
    float falloff = 1000.0/(4.0 * 3.1415 * LightDist * LightDist);
    float ks = texture(gAlbedoSpec, TexCoords).a;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0) * ks;
    return falloff * (diff + spec) * color;

}

void main()
{           
    vec4 FragPos = texture(gPosition, TexCoords);
    float Valid = 1.0 - FragPos.a;
//    vec3 skyboxColor = texture(skybox, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 tangent = texture(gTangent, TexCoords).rgb;
    vec3 bitangent = normalize(cross(normal, tangent));
    vec3 viewDir = normalize(viewPos - FragPos.xyz);
    vec3 specularTrace = normalize(reflect(-viewDir, normal));
//    if (HDR){
//        lighting = lighting / (lighting + vec3(1.0));
//        float ssao = texture(SSAO, TexCoords).r;
//        lighting *= ssao;
//    }
//    FragColor = vec4(lighting, 1);
    vec4 IndirectSpecular = IndirectSpecularLighting(FragPos.xyz, specularTrace);
    vec4 IndirectDiffuse = IndirectDiffuseLighting(FragPos.xyz, normal, tangent, bitangent);
    vec4 Direct = DirectLighting(FragPos.xyz, viewDir, normal, lightPos);
    float occlu = OcclusionConeTracing(FragPos.xyz, lightPos, normal, OcculsionAperture);
    // FragColor = vec4(occlu, occlu, occlu, 1.0);
    FragColor = (occlu * Direct + 0.5 * IndirectSpecular) * Valid;
    // FragColor = IndirectDiffuse * Valid;
    // FragColor = sampleVoxel(FragPos.xyz, 1.0);
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}  