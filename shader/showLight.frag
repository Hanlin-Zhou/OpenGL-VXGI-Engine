#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;
void main()
{           
    vec3 color = vec3(0.5,1.0,0.5);
    
    FragColor = vec4(color, 1.0);
}  