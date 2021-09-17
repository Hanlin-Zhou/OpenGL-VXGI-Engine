#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
uniform vec3 viewPos;

uniform sampler2DMS gPosition;
uniform sampler2DMS gNormal;
uniform sampler2DMS gAlbedoSpec;
uniform sampler2DMS gTangent;
uniform sampler3D Radiance3D;
uniform sampler2D skybox;
uniform sampler2D ds_gNormal;

uniform vec3 lightPos;
uniform float lightStrength;
// uniform sampler2D SSAO;

uniform float VoxelCellSize; 
uniform float MaxCoord; 
uniform int VoxelSize;
uniform mat4 ProjectMat;

uniform float OcclusionOffsetFactor;
uniform float DiffuseOffsetFactor;
uniform float SpecularOffsetFactor;
uniform float SpecularAperture;
uniform float DiffuseAperture;
uniform float OcculsionAperture;
uniform float stepSize;
uniform float DiffuseConeAngleMix;

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


vec4 ConeTracing(vec3 origin, vec3 normal, vec3 direction, float aperture, float offset){
    float t = offset * VoxelCellSize;
    vec3 acc = vec3(0.0);
    float occlusion = 0.0;
    float max_t = 2.828426 * MaxCoord;
    float diameter = 2.0 * t * tan(aperture/ 2.0);
    origin += 1.0 * normal * VoxelCellSize;
    vec3 currPos = origin + t * direction;

    while(occlusion < 1 && t < max_t){
        vec4 s = sampleVoxel(currPos, diameter);
        s.a = 1.0 - pow((1.0 - s.a), diameter / VoxelCellSize);
        acc = occlusion * acc + (1.0 - occlusion) * s.a * s.xyz;
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
    float t = OcclusionOffsetFactor * VoxelCellSize;
    float occlusion = 0.0;
    vec3 lightDir = lightPos - pos;
    float max_t = length(lightDir);
    lightDir = normalize(lightDir);
    vec3 currPos = pos + t * lightDir;
    float diameter = 2.0 * t * tan(aperture/ 2.0);
    while(occlusion < 1.0 && t < max_t) {
        float s = sampleVoxel(currPos, diameter).a;
        occlusion += (1.0 - occlusion) * s;
        t += stepSize * diameter;
        diameter = 2 * t * tan(aperture/ 2.0);
        currPos = pos + t * lightDir;
    }
    return (1.0 - min(1.0, occlusion)) * max(dot(lightDir, normal), 0.0);

}

vec4 IndirectSpecularLighting(vec3 pos, vec3 normal, vec3 traceDir){
   return ConeTracing(pos, normal, traceDir, SpecularAperture, SpecularOffsetFactor);
}

vec4 IndirectDiffuseLighting(vec3 pos, vec3 normal, vec3 tangent, vec3 bitangent){
   // vec4 color = vec4(0.0);
   normal = normalize(normal);
   tangent = normalize(tangent);
   bitangent = normalize(bitangent);
   vec4 color = 0.5 * ConeTracing(pos, normal, mix(normal, tangent, DiffuseConeAngleMix), DiffuseAperture, DiffuseOffsetFactor);
   color += 0.5 * ConeTracing(pos, normal, mix(normal, -tangent, DiffuseConeAngleMix), DiffuseAperture, DiffuseOffsetFactor);
   color += 0.5 * ConeTracing(pos, normal, mix(normal, bitangent, DiffuseConeAngleMix), DiffuseAperture, DiffuseOffsetFactor);
   color += 0.5 * ConeTracing(pos, normal, mix(normal, -bitangent, DiffuseConeAngleMix), DiffuseAperture, DiffuseOffsetFactor);
   color += ConeTracing(pos, normal, normal, DiffuseAperture, DiffuseOffsetFactor);
   return vec4(color.xyz, 1.0);
}


vec4 DirectLighting(vec3 color, vec3 pos, vec3 viewDir, vec3 normal, vec3 lightPos, float ks){
    vec3 PosToLight = lightPos - pos;
    float LightDist = length(PosToLight);
    vec3 lightDir = normalize(PosToLight);
    float diff = max(dot(lightDir, normal), 0.0) * (1.0 - ks);
    float falloff = lightStrength/(4.0 * 3.1415 * LightDist * LightDist);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0) * ks;
    return falloff * (diff + spec) * vec4(color, 1.0);

}

void main()
{           
    ivec2 MScoord = ivec2(TexCoords * textureSize(gPosition));
    vec3 sum = vec3(0.0);
    int run = 1 + 3 * int(texture(ds_gNormal, TexCoords).r);

    for (int i = 0; i < run; i++){
        vec4 FragPos = texelFetch(gPosition, MScoord, i);
        vec4 color = texelFetch(gAlbedoSpec, MScoord, i);
        float Valid = 1.0 - FragPos.a;
        vec3 skyboxColor = texture(skybox, TexCoords).rgb;
        vec3 normal = texelFetch(gNormal, MScoord, i).rgb;
        vec3 tangent = texelFetch(gTangent, MScoord, i).rgb;

        vec3 bitangent = normalize(cross(normal, tangent));
        vec3 viewDir = normalize(viewPos - FragPos.xyz);
        vec3 specularTrace = normalize(reflect(-viewDir, normal));
        vec4 IndirectSpecular = IndirectSpecularLighting(FragPos.xyz, normal, specularTrace);
        vec4 IndirectDiffuse = IndirectDiffuseLighting(FragPos.xyz, normal, tangent, bitangent);
        vec4 Direct = DirectLighting(color.xyz, FragPos.xyz, viewDir, normal, lightPos, color.a);
        float occlu = OcclusionConeTracing(FragPos.xyz, lightPos, normal, OcculsionAperture);
        vec4 Indirect = color.a * IndirectSpecular + (1.0 - color.a) * IndirectDiffuse;
        vec4 lighting = (occlu * Direct + vec4(color.xyz, 1.0) * Indirect) * Valid + (1.0 - Valid) * vec4(skyboxColor, 1.0);
        lighting.xyz = lighting.xyz / (lighting.xyz + vec3(1.0));
        sum += lighting.xyz;
    }
    FragColor = vec4(sum / float(run), 1.0);
    // FragColor = IndirectSpecular  * Valid;
}  