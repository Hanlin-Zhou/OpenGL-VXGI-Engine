#version 430 core

layout(binding = 0, r8) writeonly uniform image2DMS shadowImage;

in vec2 TexCoords;

uniform samplerCube depthCubemap;

uniform vec3 lightPos;
uniform float far_plane;

uniform sampler2DMS gPosition;
uniform int MSAA_Sample;

vec3 offsetDisk[6] = vec3[](
    vec3(1,0,0),
    vec3(-1,0,0),
    vec3(0,1,0),
    vec3(0,-1,0),
    vec3(0,0,1),
    vec3(0,0,-1)

);

float ShadowCalculation(vec3 fragPos)
{
    float light_size = 0.05;
    float bias = 0.2;
    vec3 fragToLight = fragPos - lightPos; 
    float currentDepth = length(fragToLight);
    float nSample = 4;
    float blocker_search_radius = 0.5 * light_size * currentDepth;
    float incre = blocker_search_radius * 2 / nSample;
    float shadow = 1.0;
    float sum = 0.0;
    float blocker_count = 0;
    for (float i = -blocker_search_radius; i < blocker_search_radius; i += incre){
        for (float w = -blocker_search_radius; w < blocker_search_radius; w += incre){
            for (float q = -blocker_search_radius; q < blocker_search_radius; q += incre){
                float closestDepth = texture(depthCubemap, fragToLight + vec3(i,w,q)).r;
                closestDepth *= far_plane;
                if (closestDepth < currentDepth - bias) {
                    sum += closestDepth;
                    blocker_count += 1;
                }
           }
        }
    }
    if (blocker_count < 1){
        return 1.0;
    }else{
        float avg = sum / blocker_count; 
        float pen_ratio =  (currentDepth - avg) / avg;
        float pen_radius = light_size * pen_ratio;
        float pen_incre = pen_radius * 2 / nSample;
        float pen_count = 0;
        for (float i = -pen_radius; i < pen_radius; i += pen_incre){
            for (float w = -pen_radius; w < pen_radius; w += pen_incre){
                for (float q = -pen_radius; q < pen_radius; q += pen_incre){
                    float closestDepth = texture(depthCubemap, fragToLight + vec3(i,w,q)).r;
                    closestDepth *= far_plane;
                    if (closestDepth >= currentDepth - bias) {
                        pen_count += 1;
                    }
                }
            }
        }
        return pen_count/(nSample * nSample * nSample);
    }
}

void main()
{           
    ivec2 MScoord = ivec2(TexCoords * textureSize(gPosition));
    for (int i = 0; i < MSAA_Sample; i++){
        vec3 FragPos = texelFetch(gPosition, MScoord, i).rgb;
        float shadow = ShadowCalculation(FragPos);
        imageStore(shadowImage, MScoord, i, vec4(shadow));
    }
}  