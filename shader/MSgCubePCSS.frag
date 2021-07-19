#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform samplerCube depthCubemap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float far_plane;
uniform bool HDR;
uniform int MSAA_Sample;

uniform sampler2DMS gPosition;
uniform sampler2DMS gNormal;
uniform sampler2DMS gAlbedoSpec;

uniform sampler2D SSAO;


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
    float gamma = 2.2;
    //    vec3 FragPos = vec3(0.0);
    //    vec3 color = vec3(0.0);
    //    vec3 normal = vec3(0.0);
    //    float ks = 0.0;
    ivec2 MScoord = ivec2(TexCoords * textureSize(gPosition));
    vec3 sumLight = vec3(0.0);
    for (int i = 0; i < MSAA_Sample; i++){
        //    for (int i = 0; i < MSAA_Sample; i++){
        //        FragPos += texelFetch(gPosition, MScoord, i).rgb;
        //        color += texelFetch(gAlbedoSpec, MScoord, i).rgb;
        //        normal += texelFetch(gNormal, MScoord, i).rgb;
        //        ks += texelFetch(gAlbedoSpec, MScoord, i).a;
        //    }
        //    FragPos /= float(MSAA_Sample);
        //    color /= float(MSAA_Sample);
        //    normal /= float(MSAA_Sample);
        //    ks /= float(MSAA_Sample);
        // vec3 FragPos = texture(gPosition, TexCoords).rgb;
        // vec3 color = pow(texture(texture_diffuse1, fs_in.TexCoords).rgb, vec3(gamma));
        // vec3 color = texture(gAlbedoSpec, TexCoords).rgb;
        // color = vec3(1.0, 1.0, 1.0);
        // vec3 normal = texture(gNormal, TexCoords).rgb;
        vec3 FragPos = texelFetch(gPosition, MScoord, i).rgb;
        vec3 color = texelFetch(gAlbedoSpec, MScoord, i).rgb;
        vec3 normal = texelFetch(gNormal, MScoord, i).rgb;
        float ks = texelFetch(gAlbedoSpec, MScoord, i).a;
        vec3 lightColor = vec3(1.0);
        vec3 lightDir = normalize(lightPos - FragPos);
        float falloff = 1000.0/(4.0 * 3.1415 * length(lightPos - FragPos) * length(lightPos - FragPos));
        float diff = max(dot(lightDir, normal), 0.0);
        float diffuse = diff * falloff;
        vec3 viewDir = normalize(viewPos - FragPos);
        // float ks = texture(gAlbedoSpec, TexCoords).a;
        float spec;
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0);
        float specular = spec * ks * falloff;
        float shadow = ShadowCalculation(FragPos);
        vec3 lighting = shadow * (diffuse + specular) * color;
        sumLight += lighting;
    }
    // FragColor = vec4(lighting, 1.0);
    sumLight /= float(MSAA_Sample);
    float ssao = texture(SSAO, TexCoords).r;
    sumLight *= ssao;
    if (HDR){
        sumLight = sumLight / (sumLight + vec3(1.0));
    }
    FragColor = vec4(sumLight, 1);
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}  