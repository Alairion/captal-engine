#include "shader.hpp"

#include <fstream>
#include <iterator>

#include "renderer.hpp"

namespace tph
{

shader::shader(renderer& renderer, shader_stage stage, std::string_view file, load_from_file_t)
:m_stage{stage}
{
    std::ifstream ifs{std::string{file}, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + std::string{file} + "\"."};

    const std::string spirv{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};

    m_shader = vulkan::shader{underlying_cast<VkDevice>(renderer), std::size(spirv), reinterpret_cast<const std::uint8_t*>(std::data(spirv))};
}

shader::shader(renderer& renderer, shader_stage stage, std::string_view spirv, load_from_memory_t)
:m_stage{stage}
,m_shader{underlying_cast<VkDevice>(renderer), std::size(spirv), reinterpret_cast<const std::uint8_t*>(std::data(spirv))}
{

}

}
