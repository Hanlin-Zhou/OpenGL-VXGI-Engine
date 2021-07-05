#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform samplerCube depthMap;

uniform vec3 lightPos;

uniform float far_plane;

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - lightPos; 
    float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= far_plane;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    if (closestDepth < currentDepth - 0.03) {
        shadow = 0.5;
    }

    return shadow;
}

void main()
{           
    vec3 color = vec3(1.0,1.0,1.0);
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    // ambient
    // vec3 ambient = 0.15 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    // vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    // float spec = 0.0;
    // vec3 halfwayDir = normalize(lightDir + viewDir);  
    // spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    // vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPos);                      
    // vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    vec3 lighting = (1.0 - shadow) * diffuse * color;
    
    FragColor = vec4(lighting, 1.0);
}  