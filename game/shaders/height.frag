#version 450

layout(set = 0, binding = 2) uniform sampler2D texture_sampler;
layout(set = 0, binding = 4) uniform sampler2D height_sampler;

layout(location = 0) in vec3 frag_position;
layout(location = 1) in vec4 frag_color;
layout(location = 2) in vec2 frag_texture_coord;

layout(location = 0) out vec4 out_color;

void main() 
{
	vec4 texture_color = texture(texture_sampler, frag_texture_coord);
	vec4 texture_height = texture(height_sampler, frag_texture_coord);
	
    out_color = vec4(texture_height.rg, frag_position.z, frag_color.a * texture_color.a);
}