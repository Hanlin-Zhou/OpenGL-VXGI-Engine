#version 430 core

layout(binding = 0, rgba8) writeonly uniform image3D tex3D;

uniform sampler2D texture_diffuse1;

in GS_OUT{
    vec3 geoVoxelPos;
    vec3 geoNormal;
    vec2 geoTexCoords;
}gs_out;


void main()
{
    vec3 color = texture(texture_diffuse1, gs_out.geoTexCoords).rgb;
    imageStore(tex3D, ivec3(gs_out.geoVoxelPos), vec4(color, 1.0));
}  
