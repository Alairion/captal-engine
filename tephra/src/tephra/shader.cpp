//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "shader.hpp"

#include <fstream>
#include <iterator>
#include <cstring>

#include <captal_foundation/utility.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "device.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

shader::shader(device& dev, shader_stage stage, const std::filesystem::path& file)
:shader{dev, stage, read_file<std::vector<std::uint32_t>>(file)}
{

}

shader::shader(device& dev, shader_stage stage, std::span<const std::uint8_t> data)
:m_stage{stage}
{
    std::vector<std::uint32_t> code{};
    code.resize(std::size(data) / 4);

    std::memcpy(std::data(code), std::data(data), std::size(data));

    m_shader = vulkan::shader{dev.context(), std::size(code) * 4, std::data(code)};
}

shader::shader(device& dev, shader_stage stage, std::span<const std::uint32_t> spirv)
:m_shader{dev.context(), std::size(spirv) * sizeof(std::uint32_t), std::data(spirv)}
,m_stage{stage}
{

}

shader::shader(device& dev, shader_stage stage, std::istream& stream)
:m_stage{stage}
{
    stream.seekg(0, std::ios_base::end);
    const std::size_t file_size{static_cast<std::size_t>(stream.tellg())};

    std::vector<std::uint32_t> code{};
    code.resize(file_size / 4);

    stream.seekg(0, std::ios_base::beg);
    stream.read(reinterpret_cast<char*>(std::data(code)), file_size);

    m_shader = vulkan::shader{dev.context(), std::size(code) * 4, std::data(code)};
}

void set_object_name(device& dev, const shader& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkShaderModule>(object));
    info.pObjectName = std::data(name);

    vulkan::check(dev->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(dev), &info));
}

}
