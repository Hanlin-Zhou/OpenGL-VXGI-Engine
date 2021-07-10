#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform samplerCube depthCubemap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float far_plane;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_opacity1;


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
    vec3 color = pow(texture(texture_diffuse1, fs_in.TexCoords).rgb, vec3(gamma));
    float op = texture(texture_diffuse1, fs_in.TexCoords).a;
    // color = vec3(1.0, 1.0, 1.0);
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float falloff = 1000.0/(4.0 * 3.1415 * length(lightPos - fs_in.FragPos) * length(lightPos - fs_in.FragPos));
    float diff = max(dot(lightDir, normal), 0.0);
    float diffuse = diff * falloff;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float ks = texture(texture_specular1, fs_in.TexCoords).r;
    // float ks = 0.01;
    float spec;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0);
    float specular = spec * ks * falloff;
    float shadow = ShadowCalculation(fs_in.FragPos);
    vec3 lighting = shadow * (diffuse + specular) * color;    
    // FragColor = vec4(lighting, 1.0);
    float opacity = texture(texture_opacity1, fs_in.TexCoords).r;
    if(op < 0.1)
        discard;
    FragColor = vec4(lighting, 1);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}  