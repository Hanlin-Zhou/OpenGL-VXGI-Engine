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
uniform sampler2D ds_gMSAA;

uniform vec3 PointLightPos;
uniform float PointLightStrength;

uniform sampler2D SSAO;
uniform sampler2D skybox;



void main()
{           
    ivec2 MScoord = ivec2(TexCoords * textureSize(gPosition));
    vec3 sumLight = vec3(0.0);
    int run = 1;
    if (texture(ds_gMSAA, TexCoords).r == 1.0){
        run  = 4;
    }
    int count = 0;
    for (int i = 0; i < run; i++){
        vec3 FragPos = texelFetch(gPosition, MScoord, i).rgb;
        float invalid = texelFetch(gPosition, MScoord, i).a;
        vec3 skyboxColor = texture(skybox, TexCoords).rgb;
        vec3 color = texelFetch(gAlbedoSpec, MScoord, i).rgb;
        vec3 normal = texelFetch(gNormal, MScoord, i).rgb;
        float ks = texelFetch(gAlbedoSpec, MScoord, i).a;
        float shadow = texelFetch(Shadow, MScoord, i).r;

        vec3 fragToLight = PointLightPos - FragPos;
        float lightDist = length(fragToLight);
        vec3 lightDir = normalize(fragToLight);
        float falloff = PointLightStrength/(4.0 * 3.1415 * lightDist * lightDist);
        float diffuse = max(dot(lightDir, normal), 0.0) * (1.0 - ks);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float specular = pow(max(dot(normal, halfwayDir), 0.0), 10.0) * ks;
        vec3 lighting = shadow * falloff * (diffuse + specular) * color;
        // skybox
        lighting = (1.0 - invalid) * lighting + invalid * skyboxColor;
        // tonemap
        lighting = lighting / (lighting + vec3(1.0));
        sumLight += clamp(lighting, 0.0, 1.0);
    }
    sumLight /= float(run);
    FragColor = vec4(sumLight, 1);

//    if (texture(ds_gMSAA, TexCoords).r == 1.0){
//        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//    }
}  