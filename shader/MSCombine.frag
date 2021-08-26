#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
uniform vec3 viewPos;
uniform bool HDR;
uniform int MSAA_Sample;

uniform sampler2DMS gPosition;
uniform sampler2DMS gNormal;
uniform sampler2DMS gAlbedoSpec;
uniform sampler2DMS Shadow;

uniform vec3 lightPos;

uniform sampler2D SSAO;
uniform sampler2D skybox;



void main()
{           
    ivec2 MScoord = ivec2(TexCoords * textureSize(gPosition));
    vec3 sumLight = vec3(0.0);
    
    int count = 0;
    for (int i = 0; i < MSAA_Sample; i++){
        vec3 FragPos = texelFetch(gPosition, MScoord, i).rgb;
        float invalid = texelFetch(gPosition, MScoord, i).a;
        vec3 skyboxColor = texture(skybox, TexCoords).rgb;
        vec3 color = texelFetch(gAlbedoSpec, MScoord, i).rgb;
        vec3 normal = texelFetch(gNormal, MScoord, i).rgb;
        float ks = texelFetch(gAlbedoSpec, MScoord, i).a;
        vec3 lightColor = vec3(1.0);
        vec3 lightDir = normalize(lightPos - FragPos);
        float falloff = 1000.0/(4.0 * 3.1415 * length(lightPos - FragPos) * length(lightPos - FragPos));
        float diff = max(dot(lightDir, normal), 0.0);
        float diffuse = diff * falloff;
        vec3 viewDir = normalize(viewPos - FragPos);
        float spec;
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0);
        float specular = spec * ks * falloff;
        float shadow = texelFetch(Shadow, MScoord, i).r;
        vec3 lighting = shadow * (diffuse + specular) * color;
        if (invalid == 1.0){
            sumLight += skyboxColor;
        }else{
            sumLight += lighting;
        }
    }
    // FragColor = vec4(lighting, 1.0);
    sumLight /= float(MSAA_Sample);
    if (HDR){
        sumLight = sumLight / (sumLight + vec3(1.0));
        float ssao = texture(SSAO, TexCoords).r;
        sumLight *= ssao;
    }
    FragColor = vec4(sumLight, 1);
}  