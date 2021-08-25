#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VS_OUT{
    vec2 TexCoords;
    vec3 Normal;
} vs_in[];

uniform mat4 ProjectMat;
uniform int VoxelSize;

out GS_OUT{
    vec3 geoVoxelPos;
    vec3 geoNormal;
    vec2 geoTexCoords;
}gs_out;

vec4 projection(vec4 pos, int axis){
    vec4 temp = pos;
    // temp.z = -pos.z;
    if (axis == 0){
        //return pos.zyxw;
        return vec4(-pos.z, pos.yxw);
    }else if (axis == 1){
        // return pos.xzyw;
        return vec4(pos.x, -pos.z, pos.yw);
    }else if (axis == 2){
        // return pos;
        return vec4(pos.xy, -pos.z, pos.w);
    }else{
        
    }
}

void main()
{
    // x : 0, y = 1, z = 2
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec3 norm = abs(cross(p1 - p0, p2 - p0));
    int major_axis = 0;
    if (norm.y > norm.x){
        major_axis = 1;
    }
    if(norm.z > norm.y && norm.z > norm.x){
        major_axis = 2;
    }

    vec4 temp0 = ProjectMat * gl_in[0].gl_Position;
    temp0.z = -temp0.z;
    gs_out.geoVoxelPos = ((temp0 + 1.0f) * 0.5f * VoxelSize).xyz;
    //gs_out.geoVoxelPos.z = -gs_out.geoVoxelPos.z;
    gs_out.geoNormal = vs_in[0].Normal;
    gs_out.geoTexCoords = vs_in[0].TexCoords;
    gl_Position = ProjectMat * projection(gl_in[0].gl_Position, major_axis);
    EmitVertex();

    vec4 temp1 = ProjectMat * gl_in[1].gl_Position;
    temp1.z = -temp1.z;
    gs_out.geoVoxelPos = ((temp1 + 1.0f) * 0.5f * VoxelSize).xyz;
    //gs_out.geoVoxelPos.z = -gs_out.geoVoxelPos.z;
    gs_out.geoNormal = vs_in[1].Normal;
    gs_out.geoTexCoords = vs_in[1].TexCoords;
    gl_Position = ProjectMat * projection(gl_in[1].gl_Position, major_axis);
    EmitVertex();

    vec4 temp2 = ProjectMat * gl_in[2].gl_Position;
    temp2.z = -temp2.z;
    gs_out.geoVoxelPos = ((temp2 + 1.0f) * 0.5f * VoxelSize).xyz;
    //gs_out.geoVoxelPos.z = -gs_out.geoVoxelPos.z;
    gs_out.geoNormal = vs_in[2].Normal;
    gs_out.geoTexCoords = vs_in[2].TexCoords;
    gl_Position = ProjectMat * projection(gl_in[2].gl_Position, major_axis);
    EmitVertex();

    EndPrimitive();
}  