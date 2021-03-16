#version 450

layout(row_major, set = 0, binding = 0) uniform view_uniform
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texture_coord;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 frag_texture_coord;

void main() 
{
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(position, 0.0, 1.0);
	
    frag_color = color;
    frag_texture_coord = texture_coord;
}