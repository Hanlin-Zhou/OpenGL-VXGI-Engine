#version 430 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2DMS gNormal;
uniform sampler2D ds_gNormal;

void main()
{    
    vec3 avg_normal = texture(ds_gNormal, TexCoords).xyz;

    ivec2 MScoord = ivec2(TexCoords * textureSize(gNormal));
    vec3 ms_normal_0 = texelFetch(gNormal, MScoord, 0).rgb;
    vec3 ms_normal_1 = texelFetch(gNormal, MScoord, 1).rgb;
    vec3 ms_normal_2 = texelFetch(gNormal, MScoord, 2).rgb;
    vec3 ms_normal_3 = texelFetch(gNormal, MScoord, 3).rgb;

    float difference = distance(avg_normal, ms_normal_0) + distance(avg_normal, ms_normal_1) +
                    distance(avg_normal, ms_normal_2) + distance(avg_normal, ms_normal_3);
	float V_coverage = 1.0 - step(1.0, texture(ds_gNormal, TexCoords).a);
	float V_diffNormal = step(0.1, difference);
    FragColor = V_coverage * V_diffNormal;
}  