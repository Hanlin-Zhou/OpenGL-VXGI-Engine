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

uniform bool diffuse;
uniform bool specular;
uniform bool normal;
uniform bool height;
uniform bool opacity;

vec3 poissonDisk[16] = vec3[]( 
   vec3( -0.94201624, -0.39906216, -0.606647), 
   vec3( 0.94558609, -0.76890725, 0.932066), 
   vec3( -0.094184101, -0.92938870, -0.798486), 
   vec3( 0.34495938, 0.29387760, 0.161229), 
   vec3( -0.91588581, 0.45771432, 0.680715), 
   vec3( -0.81544232, -0.87912464, -0.895474), 
   vec3( -0.38277543, 0.27676845, 0.260598), 
   vec3( 0.97484398, 0.75648379, 0.222541), 
   vec3( 0.44323325, -0.97511554, -0.379254), 
   vec3( 0.53742981, -0.47373420, 0.279855), 
   vec3( -0.26496911, -0.41893023, 0.312845), 
   vec3( 0.79197514, 0.19090188, 0.0382092), 
   vec3( -0.24188840, 0.99706507, -0.890805), 
   vec3( -0.81409955, 0.91437590, 0.890835), 
   vec3( 0.19984126, 0.78641367, 0.451979), 
   vec3( 0.14383161, -0.14100790, -0.449232) 
);

vec3 offsetDisk[6] = vec3[](
    vec3(1,0,0),
    vec3(-1,0,0),
    vec3(0,1,0),
    vec3(0,-1,0),
    vec3(0,0,1),
    vec3(0,0,-1)

);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float ShadowCalculation(vec3 fragPos)
{
    int nSample = 16;
    vec3 fragToLight = fragPos - lightPos; 
    float currentDepth = length(fragToLight);
    float lsize = 0.02;
    float shadow = 1.0;
    float sum = 0.0;
    float count = 0;
    for (int i = 0; i < nSample; i++){
        int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
        float closestDepth = texture(depthCubemap, fragToLight + poissonDisk[index] * lsize).r;
        closestDepth *= far_plane;
        if (closestDepth < currentDepth - 0.05) {
            // shadow -= 0.25;
            sum += closestDepth;
            count += 1;
        }
    }
    if (count < 1){
        return 1.0;
    }else{
        float avg = sum / count; 
        float pen_ratio = lsize * (currentDepth - avg) / avg;
        
        float pen_count = 0;
        for (int i = 0; i < nSample; i++){
            int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
            float closestDepth = texture(depthCubemap, fragToLight + poissonDisk[index] * pen_ratio).r;
            closestDepth *= far_plane;
            if (closestDepth > currentDepth - 0.05) {
                pen_count += 1;   
            }
        }
        return pen_count/nSample;
    }
}

void main()
{           
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // vec3 color = vec3(1.0, 1.0, 1.0);
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float ks = texture(texture_specular1, fs_in.TexCoords).r;
    // float ks = 0.01;
    float spec;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 10.0);
    float specular = spec * ks;
    float shadow = ShadowCalculation(fs_in.FragPos);
    vec3 lighting = shadow * (diffuse + specular) * color;    
    // FragColor = vec4(lighting, 1.0);
    FragColor = vec4(lighting, 1.0);
}  