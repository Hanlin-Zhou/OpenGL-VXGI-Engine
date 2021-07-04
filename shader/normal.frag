#version 430 core
out vec4 FragColor;

in vec3 normal;
in vec2 TexCoord;

uniform mat4 view;
uniform mat4 proj;

uniform sampler2D ourTexture;

void main()
{
    FragColor = vec4(normal,1);
}