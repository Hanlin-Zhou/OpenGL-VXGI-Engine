#version 430 core
layout (location = 0) out vec3 gViewPos;
  
in vec2 TexCoords;

uniform sampler2D ds_gPosition;
uniform mat4 view;

void main(){
	vec3 pos = texture(ds_gPosition, TexCoords).xyz; 
	vec4 viewpos = view * vec4(pos, 1.0);
	gViewPos = viewpos.xyz / viewpos.w;
}