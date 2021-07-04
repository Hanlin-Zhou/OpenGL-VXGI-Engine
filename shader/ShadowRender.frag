#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D depthMap;

uniform vec3 lightPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = 0.0;
    if (closestDepth < currentDepth - 0.01) {
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
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
    // vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    vec3 lighting = (1.0 - shadow) * diffuse * color;
    FragColor = vec4(lighting, 1.0);
    // FragColor = texture(depthMap, fs_in.TexCoords);
}