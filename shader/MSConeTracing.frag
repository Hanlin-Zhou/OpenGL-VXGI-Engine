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

uniform vec3 PointLightPos;
uniform float PointLightStrength;

uniform vec3 DirectionalLightDirection;
uniform float DirectionalLightStrength;

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
        t += stepSize * diameter;
        diameter = 2 * t * tan(aperture/ 2.0);
        currPos = origin + t * direction;
    }
    return vec4(acc, 1.0);
}

float OcclusionConeTracing(vec3 pos, vec3 direction, vec3 normal, float aperture, float mt){
    float t = OcclusionOffsetFactor * VoxelCellSize;
    pos += 1.0 * normal * VoxelCellSize;
    float occlusion = 0.0;
    vec3 lightDir = direction;
    float max_t = mt;
    lightDir = normalize(lightDir);
    vec3 currPos = pos + t * lightDir;
    float diameter = 2.0 * t * tan(aperture/ 2.0);
    while(occlusion < 1.0 && t < max_t) {
        float s = sampleVoxel(currPos, diameter).a;
        s = 1.0 - pow((1.0 - s), diameter / VoxelCellSize);
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


vec4 DirectLighting(vec3 color, float ks, vec3 pos, vec3 viewDir, vec3 normal, vec3 PlightPos, float P_occlu, float D_occlu){
    vec3 PosToLight = PlightPos - pos;
    float LightDist = length(PosToLight);
    vec3 lightDir = normalize(PosToLight);
    float p_diff = max(dot(lightDir, normal), 0.0) * (1.0 - ks);
    float p_falloff = PointLightStrength/(4.0 * 3.1415 * LightDist * LightDist);
    vec3 p_halfwayDir = normalize(lightDir + viewDir);  
    float p_spec = pow(max(dot(normal, p_halfwayDir), 0.0), 10.0) * ks;

    float d_diff = max(dot(DirectionalLightDirection, normal), 0.0) * (1.0 - ks);
    float d_falloff = DirectionalLightStrength;
    vec3 d_halfwayDir = normalize(DirectionalLightDirection + viewDir);  
    float d_spec = pow(max(dot(normal, d_halfwayDir), 0.0), 10.0) * ks;

    return (P_occlu * p_falloff * (p_diff + p_spec) +  D_occlu * d_falloff * (d_diff + d_spec))* vec4(color, 1.0);
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
        float point_occlu = OcclusionConeTracing(FragPos.xyz, PointLightPos - FragPos.xyz, normal, OcculsionAperture, length(PointLightPos - FragPos.xyz));
        float dir_occlu = OcclusionConeTracing(FragPos.xyz, DirectionalLightDirection, normal, OcculsionAperture, MaxCoord);
        vec4 Direct = DirectLighting(color.xyz, color.a, FragPos.xyz, viewDir, normal, PointLightPos, point_occlu, dir_occlu);
        vec4 Indirect = color.a * IndirectSpecular + (1.0 - color.a) * IndirectDiffuse;
        vec4 lighting = (Direct + vec4(color.xyz, 1.0) * Indirect) * Valid + (1.0 - Valid) * vec4(skyboxColor, 1.0);
        lighting.xyz = lighting.xyz / (lighting.xyz + vec3(1.0));
        sum += lighting.xyz;
    }
    FragColor = vec4(sum / float(run), 1.0);
    // FragColor = IndirectSpecular  * Valid;
}  