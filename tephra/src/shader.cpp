#include "shader.hpp"

#include <fstream>
#include <iterator>

#include "renderer.hpp"

namespace tph
{

shader::shader(renderer& renderer, shader_stage stage, const std::filesystem::path& file)
:m_stage{stage}
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + file.string() + "\"."};

    const std::string spirv{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};

    m_shader = vulkan::shader{underlying_cast<VkDevice>(renderer), std::size(spirv), reinterpret_cast<const std::uint8_t*>(std::data(spirv))};
}

shader::shader(renderer& renderer, shader_stage stage, std::string_view spirv)
:m_stage{stage}
,m_shader{underlying_cast<VkDevice>(renderer), std::size(spirv), reinterpret_cast<const std::uint8_t*>(std::data(spirv))}
{

}

shader::shader(renderer& renderer, shader_stage stage, std::istream& stream)
:m_stage{stage}
{
    const std::string spirv{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

    m_shader = vulkan::shader{underlying_cast<VkDevice>(renderer), std::size(spirv), reinterpret_cast<const std::uint8_t*>(std::data(spirv))};
}

}
