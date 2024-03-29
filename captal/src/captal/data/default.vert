#version 450

layout(row_major, set = 0, binding = 0) uniform view_uniform
{
    mat4 view;
    mat4 proj;
} view;

layout(row_major, set = 1, binding = 0) uniform model_uniform
{
    mat4 model;
} model;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texture_coord;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 frag_texture_coord;

void main() 
{
    gl_Position = view.proj * view.view * model.model * vec4(position, 1.0);
	
    frag_color = color;
    frag_texture_coord = texture_coord;
}