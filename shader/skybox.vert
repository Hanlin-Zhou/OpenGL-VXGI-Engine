#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 proj;
uniform mat4 view;

out vec3 localPos;

void main()
{
    localPos = aPos;

    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = proj * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos;
}