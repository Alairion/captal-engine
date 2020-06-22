#include "shader.hpp"

#include <fstream>
#include <iterator>
#include <cstring>

#include <captal_foundation/utility.hpp>

#include "renderer.hpp"

namespace tph
{

shader::shader(renderer& renderer, shader_stage stage, const std::filesystem::path& file)
:shader{renderer, stage, read_file<std::vector<std::uint32_t>>(file)}
{

}

shader::shader(renderer& renderer, shader_stage stage, std::span<const std::uint8_t> data)
:m_stage{stage}
{
    std::vector<std::uint32_t> code{};
    code.resize(std::size(data) / 4);

    std::memcpy(std::data(code), std::data(data), std::size(data));

    m_shader = vulkan::shader{underlying_cast<VkDevice>(renderer), std::size(code) * 4, std::data(code)};
}

shader::shader(renderer& renderer, shader_stage stage, std::span<const std::uint32_t> spirv)
:m_stage{stage}
,m_shader{underlying_cast<VkDevice>(renderer), std::size(spirv) * sizeof(std::uint32_t), std::data(spirv)}
{

}

shader::shader(renderer& renderer, shader_stage stage, std::istream& stream)
:m_stage{stage}
{
    stream.seekg(0, std::ios_base::end);
    const std::size_t file_size{static_cast<std::size_t>(stream.tellg())};

    std::vector<std::uint32_t> code{};
    code.resize(file_size / 4);

    stream.seekg(0, std::ios_base::beg);
    stream.read(reinterpret_cast<char*>(std::data(code)), file_size);

    m_shader = vulkan::shader{underlying_cast<VkDevice>(renderer), std::size(code) * 4, std::data(code)};
}

}
