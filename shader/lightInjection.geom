#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

in VS_OUT{
    vec2 vTexCoords;
    vec3 vNormal;
    vec3 vPos;
} vs_out[];

out GS_OUT{
    vec2 gTexCoords;
    vec3 gNormal;
    vec3 gPos;
} gs_out;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            gs_out.gTexCoords = vs_out[i].vTexCoords;
            gs_out.gNormal = vs_out[i].vNormal;
            gs_out.gPos = vs_out[i].vPos;
            vec4 FragPos = gl_in[i].gl_Position;

            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  