#include "uniform_buffer.hpp"

#include <tephra/commands.hpp>

#include "engine.hpp"

namespace cpt
{

uniform_buffer::uniform_buffer(std::span<const buffer_part> parts)
:m_parts{compute_part_info(parts)}
,m_buffer{engine::instance().uniform_pool().allocate(m_parts.back().offset + m_parts.back().size, engine::instance().graphics_device().limits().min_uniform_buffer_alignment)}
{

}

void uniform_buffer::upload()
{
    m_buffer.upload();
}

void uniform_buffer::upload(std::size_t index)
{
    m_buffer.upload(m_parts[index].offset, m_parts[index].size);
}

std::vector<uniform_buffer::buffer_part_info> uniform_buffer::compute_part_info(std::span<const buffer_part> parts)
{
    const std::uint64_t uniform_alignment{engine::instance().graphics_device().limits().min_uniform_buffer_alignment};

    std::vector<uniform_buffer::buffer_part_info> output{};

    std::uint64_t offset{};
    for(auto&& part : parts)
    {
        output.emplace_back(offset, part.size);

        if(part.type == buffer_part_type::uniform)
        {
            offset = align_up(offset, uniform_alignment) + part.size;
        }
        else
        {
            offset += part.size;
        }
    }

    return output;
}

}
