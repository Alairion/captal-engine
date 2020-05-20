#version 450

layout(set = 0, binding = 0) uniform view_uniform
{
	vec4 position;
    mat4 view;
    mat4 proj;
} view;
layout(set = 0, binding = 1) uniform model_uniform
{
    mat4 model;
} model;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texture_coord;

layout(location = 0) out vec4 frag_position;
layout(location = 1) out vec4 frag_color;
layout(location = 2) out vec2 frag_texture_coord;
layout(location = 3) out vec3 view_position;
layout(location = 4) out vec2 frag_local_position;

void main() 
{
    gl_Position = view.proj * view.view * model.model * vec4(position, 1.0);
	
	frag_position = model.model * vec4(position, 1.0);
    frag_color = color;
    frag_texture_coord = texture_coord;
	
	view_position = vec3(view.position);
	frag_local_position = (vec2(gl_Position) + 1.0f) / 2.0f;
}