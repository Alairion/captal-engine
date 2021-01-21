#version 450

layout(set = 1, binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec2 frag_texture_coord;

layout(location = 0) out vec4 out_color;

void main()
{
	const vec4 texture_color = texture(texture_sampler, frag_texture_coord);
	
	out_color = frag_color * texture_color;
}