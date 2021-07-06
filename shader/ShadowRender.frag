#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec4 FPLS[6];
} fs_in;

uniform sampler2D depthMaps[6];
uniform sampler2D depthMap;
uniform vec3 lightPos;
uniform bool soft_shadow;

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

float ShadowSoftCalculation()
{
    float shadow_vote = 0.0;
    float diff = 0.0;
    for (int i = 0; i < 6; i++){
        vec3 projCoords = fs_in.FPLS[i].xyz / fs_in.FPLS[i].w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(depthMaps[i], projCoords.xy).r;
        float currentDepth = projCoords.z;
        if (closestDepth < currentDepth - 0.01) {
            shadow_vote += 1;
            diff += currentDepth - closestDepth;
        }
    }

    // perform perspective divide
    vec3 m_projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    m_projCoords = m_projCoords * 0.5 + 0.5;
    float m_closestDepth = texture(depthMap, m_projCoords.xy).r; 
    float m_currentDepth = m_projCoords.z;
    if (m_closestDepth < m_currentDepth - 0.01) {
        shadow_vote += 1;
        diff += m_currentDepth - m_closestDepth;
    }
    if (diff > 0.1) {
        // Not artifact. Apply Shadow
        return shadow_vote / 7.0;
    }else{
        return 0.0;
    }
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
    float shadow;
    if (soft_shadow){
        shadow = ShadowSoftCalculation();
    } else{
        shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    }  
    // vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    vec3 lighting = (1.0 - shadow) * diffuse * color;
    FragColor = vec4(lighting, 1.0);
    // FragColor = texture(depthMap, fs_in.TexCoords);
}