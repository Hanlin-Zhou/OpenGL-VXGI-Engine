#version 430 core

layout(binding = 0, rgba8) writeonly uniform image3D tex3D;

uniform vec3 lightPos;
uniform float far_plane;

uniform sampler2D texture_opacity1;
uniform sampler2D texture_diffuse1;

uniform mat4 ProjectMat;
uniform int VoxelSize;

in GS_OUT{
    vec2 gTexCoords;
    vec3 gNormal;
    vec3 gPos;
} gs_out;


void main()
{
    float opacity = texture(texture_opacity1, gs_out.gTexCoords).r;
    float lightDistance = length(gs_out.gPos - lightPos);
    vec4 temp = ProjectMat * vec4(gs_out.gPos, 1.0);
    temp.z = -temp.z;
    vec3 VoxelPos = ((temp + 1.0f) * 0.5f * VoxelSize).xyz;
    float falloff = 1000.0/(4.0 * 3.1415 * lightDistance * lightDistance);
    vec3 lightDir = normalize(lightPos - gs_out.gPos);
    float diff = max(dot(lightDir, gs_out.gNormal), 0.0);
    float diffuse = diff * falloff;
    vec3 color = texture(texture_diffuse1, gs_out.gTexCoords).rgb * diffuse;
    imageStore(tex3D, ivec3(VoxelPos), vec4(color, 1.0));
    
}  
