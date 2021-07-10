#version 330 core
in vec4 FragPos;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform float far_plane;

uniform sampler2D texture_diffuse1;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    lightDistance = lightDistance / far_plane;
    gl_FragDepth = lightDistance;
}  
