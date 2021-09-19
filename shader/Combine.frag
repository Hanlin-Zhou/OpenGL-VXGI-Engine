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

uniform vec3 PointLightPos;
uniform float PointLightStrength;

uniform sampler2D SSAO;
uniform sampler2D skybox;



void main()
{      
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    float invalid = texture(gPosition, TexCoords).a;
    vec3 skyboxColor = texture(skybox, TexCoords).rgb;
    vec3 color = texture(gAlbedoSpec, TexCoords).rgb;
    float ks = texture(gAlbedoSpec, TexCoords).a;
    float shadow = texture(Shadow, TexCoords).r;

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
    FragColor = vec4(lighting, 1);
}