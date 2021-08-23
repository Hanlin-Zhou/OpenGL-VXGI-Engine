#version 430 core

layout(rgba8) uniform image3D texture3D;


in GS_OUT{
    vec3 geoVoxelPos;
    vec3 geoNormal;
    vec2 geoTexCoords;
}gs_out;


void main()
{
    
    imageStore(texture3D, ivec3(gs_out.geoVoxelPos), vec4(1.0));
}  
