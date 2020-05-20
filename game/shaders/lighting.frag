#version 450

struct directional_light
{
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct point_light
{
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float radius;
};

layout(set = 0, binding = 2) uniform sampler2D texture_sampler;
layout(set = 0, binding = 3) uniform sampler2D normal_sampler;
layout(set = 0, binding = 4) uniform sampler2D height_sampler;
layout(set = 0, binding = 5) uniform sampler2D specular_sampler;
layout(set = 0, binding = 6) uniform sampler2D emission_sampler;
layout(set = 0, binding = 7) uniform lights_uniform
{
    uint directional_light_count;
    directional_light directional_lights[2];
    uint point_light_count;
    point_light point_lights[32];
} lights;

layout(location = 0) in vec4 frag_position;
layout(location = 1) in vec4 frag_color;
layout(location = 2) in vec2 frag_texture_coord;
layout(location = 3) in vec3 view_position;
layout(location = 4) in vec2 frag_local_position;

layout(location = 0) out vec4 out_color;

float directional_light_shadow(vec2 texture_coord, vec3 step) 
{
	const vec4 data = texture(height_sampler, texture_coord);
    const float height = data.b + data.r;
	
	vec3 current_pixel = vec3(texture_coord, height);
    for(uint i = 0; i < 255; ++i) 
	{
		current_pixel += step;
		
		const vec4 other_data = texture(height_sampler, current_pixel.xy);
		const float other_height = other_data.b + other_data.r;
		const float other_depth = other_data.g + (1.0f / 256.0f);
		const float other_base = other_height - other_depth;

		if(other_base <= current_pixel.z && current_pixel.z <= other_height) 
		{
			return 1.0f;
		}
	}
	
    return 0.0f;
}

vec3 compute_directional_light(uint light_index, vec3 texture_diffuse, vec3 texture_normal, vec3 texture_specular)
{
    const directional_light light = lights.directional_lights[light_index];

	const vec3 ambient = vec3(light.ambient) * texture_diffuse;
	
	const vec3 light_direction = normalize(-vec3(light.direction));
	const float diffuse_strength = max(dot(texture_normal, light_direction), 0.0f);
	const vec3 diffuse = vec3(light.diffuse) * (diffuse_strength * texture_diffuse);
	
	const vec3 view_direction = normalize(view_position - vec3(frag_position));
	const vec3 halfway = normalize(light_direction + view_direction);
	const float specular_stength = pow(max(dot(view_direction, halfway), 0.0f), 128.0f);
	const vec3 specular = vec3(light.specular) * (specular_stength * texture_specular);
    
	const vec2 height_map_size = vec2(textureSize(height_sampler, 0));
    const vec3 height_map_step = vec3(1.0f / height_map_size.x, 1.0f / height_map_size.y, 1.0f / 256.0f);
    const vec3 step = light_direction * height_map_step;
	const float shadow = directional_light_shadow(frag_local_position, step);
	
    return ambient + (diffuse + specular) * (1.0f - shadow);
}

float point_light_shadow(vec2 texture_coord, vec3 step)
{
	const vec4 data = texture(height_sampler, texture_coord);
    const float height = data.b + data.r;
	
	vec3 current_pixel = vec3(texture_coord, height);
    for(uint i = 0; i < 128; ++i) 
	{
		current_pixel += step;
		
		const vec4 other_data = texture(height_sampler, current_pixel.xy);
		const float other_height = other_data.b + other_data.r;
		const float other_depth = other_data.g + (1.0f / 256.0f);
		const float other_base = other_height - other_depth;

		if(other_base <= current_pixel.z && current_pixel.z <= other_height) 
		{
			return 1.0f;
		}
	}
	
    return 0.0f;
}

vec3 compute_point_light(uint light_index, vec3 texture_diffuse, vec3 texture_normal, vec3 texture_specular)
{
    const point_light light = lights.point_lights[light_index];

    const float distance = max(length(light.position - frag_position) - light.radius, 0.0f);
    const float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    if(attenuation < 0.01)
        return vec3(0.0f);

	const vec3 ambient = vec3(light.ambient) * texture_diffuse;
	
	const vec3 light_direction = normalize(vec3(light.position - frag_position));
	const float diffuse_strength = max(dot(texture_normal, light_direction), 0.0f);
	const vec3 diffuse = vec3(light.diffuse) * (diffuse_strength * texture_diffuse);
    
	const vec3 view_direction = normalize(view_position - vec3(frag_position));
	const vec3 halfway = normalize(light_direction + view_direction);
	const float specular_stength = pow(max(dot(view_direction, halfway), 0.0f), 128.0f);
	const vec3 specular = vec3(light.specular) * (specular_stength * texture_specular);
    
	const vec2 height_map_size = vec2(textureSize(height_sampler, 0));
    const vec3 height_map_step = vec3(1.0f / height_map_size.x, 1.0f / height_map_size.y, 1.0f / 256.0f);
    const vec3 step = light_direction * height_map_step;
    const float shadow = point_light_shadow(frag_local_position, step);
    
    return (ambient + (diffuse + specular) * (1.0f - shadow)) * attenuation;
}

vec3 aces(vec3 color)
{
    const mat3 aces_input_matrix  = mat3(0.59719f, 0.35458f, 0.04823f, 0.07600f, 0.90834f, 0.01566f, 0.02840f, 0.13383f, 0.83777f);
    const mat3 aces_output_matrix = mat3( 1.60475f, -0.53108f, -0.07367f, -0.10208f, 1.10813f, -0.00605f, -0.00327f, -0.07276f, 1.07602f);

    color = aces_input_matrix * color;
    
    const vec3 a = color * (color + 0.0245786f) - 0.000090537f;
    const vec3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
    color = a / b;
    
    return aces_output_matrix * color;
}

void main() 
{
	const vec4 texture_color = texture(texture_sampler, frag_texture_coord);
    const vec4 texture_emission = texture(emission_sampler, frag_texture_coord) * frag_color;
    
    out_color = vec4(0.0f, 0.0f, 0.0f, frag_color.a * texture_color.a * texture_emission.a);
   
    if(out_color.a != 0.0f)
    {
        const vec3 diffuse = texture_color.rgb * frag_color.rgb;
        const vec3 normal = normalize(texture(normal_sampler, frag_texture_coord).rgb * 2.0f - 1.0f);
        const vec3 specular = texture(specular_sampler, frag_texture_coord).rgb;
        
        for(uint i = 0; i < lights.directional_light_count; ++i)
        {
            out_color.rgb += compute_directional_light(i, diffuse, normal, specular);
        }
            
        for(uint i = 0; i < lights.point_light_count; ++i)
        {
            out_color.rgb += compute_point_light(i, diffuse, normal, specular);
        }
            
        out_color.rgb += texture_emission.rgb;
        out_color.rgb = aces(out_color.rgb);
    }
}