#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
uniform vec3 viewPos;
uniform bool HDR;
uniform int MSAA_Sample;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D Shadow;

uniform vec3 lightPos;

uniform sampler2D SSAO;
uniform sampler2D skybox;



void main()
{           
    float gamma = 2.2;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    float invalid = texture(gPosition, TexCoords).a;
    vec3 skyboxColor = texture(skybox, TexCoords).rgb;
    vec3 color = texture(gAlbedoSpec, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    float ks = texture(gAlbedoSpec, TexCoords).a;
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
    float shadow = texture(Shadow, TexCoords).r;
    vec3 lighting = shadow * (diffuse + specular) * color;
    if (invalid == 1.0){
        lighting = skyboxColor;
    }
    if (HDR){
        lighting = lighting / (lighting + vec3(1.0));
        float ssao = texture(SSAO, TexCoords).r;
        lighting *= ssao;
    }
    FragColor = vec4(lighting, 1);
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}  