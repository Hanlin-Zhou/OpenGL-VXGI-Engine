#version 430 core
#extension GL_ARB_shader_image_load_store : require

layout(binding = 0, r32ui) uniform volatile coherent uimage3D Albedo3D;
layout(binding = 1, r32ui) uniform volatile coherent uimage3D Normal3D;

uniform sampler2D texture_diffuse1;

in GS_OUT{
    vec3 geoVoxelPos;
    vec3 geoNormal;
    vec2 geoTexCoords;
}gs_out;

vec4 convRGBA8ToVec4(uint val){
    return vec4(float((val & 0x000000FF)), float ((val & 0x0000FF00) >> 8U), float (( val & 0x00FF0000) >> 16U), float ((val & 0xFF000000) >> 24U));
}

uint convVec4ToRGBA8(vec4 val){
    return (uint(val.w) & 0x000000FF) << 24U | (uint(val.z) &0x000000FF) << 16U | (uint(val.y) & 0x000000FF) << 8U | (uint(val.x) & 0x000000FF);
}


vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

void imageAtomicRGBA8Avg(layout(r32ui) coherent volatile uimage3D img, ivec3 coords, vec4 val){
    val.rgb *= 255.0f;
    uint newVal = convVec4ToRGBA8(val);
    uint prevStoredVal = 0;
    uint curStoredVal;
    uint numIterations = 0;
    while ((curStoredVal = imageAtomicCompSwap(img, coords, prevStoredVal, newVal)) != prevStoredVal 
            && numIterations < 255) {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.xyz = (rval.xyz * rval.w);
        vec4 curValF = rval + val;
        curValF.xyz /= (curValF.w);
        newVal = convVec4ToRGBA8(curValF);
        ++numIterations;
    }
}

void main()
{
    vec4 color = texture(texture_diffuse1, gs_out.geoTexCoords);
    // imageStore(Albedo3D, ivec3(gs_out.geoVoxelPos), color);
    // imageStore(Normal3D, ivec3(gs_out.geoVoxelPos), vec4(gs_out.geoNormal, 1.0));
    ivec3 pos = ivec3(gs_out.geoVoxelPos);
    imageAtomicRGBA8Avg(Albedo3D, ivec3(gs_out.geoVoxelPos), color);
    imageAtomicRGBA8Avg(Normal3D, ivec3(gs_out.geoVoxelPos), vec4(EncodeNormal(gs_out.geoNormal), 1.0));
}  
