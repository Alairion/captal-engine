#include "uniform_buffer.hpp"

#include <tephra/commands.hpp>

#include "engine.hpp"

namespace cpt
{

static std::uint64_t compute_size(const std::vector<buffer_part>& parts) noexcept
{
    const auto align_up = [](std::uint64_t offset, std::uint64_t alignment) noexcept -> std::uint64_t
    {
        return (offset + alignment - 1) & ~(alignment - 1);
    };

    const std::uint64_t uniform_alignment{engine::instance().graphics_device().limits().min_uniform_buffer_alignment};
    std::uint64_t total_size{};

    for(auto&& part : parts)
    {
        if(part.type == buffer_part_type::uniform)
        {
            total_size = align_up(total_size, uniform_alignment) + part.size;
        }
        else
        {
            total_size += part.size;
        }
    }

    return align_up(total_size, uniform_alignment);
}

static tph::buffer_usage compute_usage(const std::vector<buffer_part>& parts) noexcept
{
    tph::buffer_usage output{};

    for(auto&& part : parts)
    {
        if(part.type == buffer_part_type::uniform)
        {
            output |= tph::buffer_usage::uniform;
        }
        else if(part.type == buffer_part_type::index)
        {
            output |= tph::buffer_usage::index;
        }
        else if(part.type == buffer_part_type::vertex)
        {
            output |= tph::buffer_usage::vertex;
        }
    }

    return output;
}

uniform_buffer::uniform_buffer(std::vector<buffer_part> parts)
:m_size{compute_size(parts)}
,m_parts{std::move(parts)}
,m_device_buffer{engine::instance().renderer(), m_size, compute_usage(m_parts) | tph::buffer_usage::device_only | tph::buffer_usage::transfer_destination}
{
    m_data.resize(m_size);
}

void uniform_buffer::upload()
{
    std::size_t staging_index{std::numeric_limits<std::size_t>::max()};
    for(std::size_t i{}; i < std::size(m_stagings); ++i)
    {
        if(m_stagings[i].available)
        {
            staging_index = i;
            break;
        }
    }

    if(staging_index == std::numeric_limits<std::size_t>::max())
    {
        m_stagings.emplace_back(staging_buffer{tph::buffer{engine::instance().renderer(), m_size, tph::buffer_usage::transfer_source | tph::buffer_usage::staging}});
        staging_index = std::size(m_stagings) - 1;
    }

    staging_buffer& buffer{m_stagings[staging_index]};
    buffer.available = false;

    std::memcpy(buffer.buffer.map(), std::data(m_data), m_size);
    buffer.buffer.unmap();

    auto&& [command_buffer, signal] = engine::instance().begin_transfer();
    tph::cmd::copy(command_buffer, buffer.buffer, m_device_buffer);

    buffer.connection = signal.connect([this, staging_index]()
    {
        m_stagings[staging_index].available = true;
    });
}

std::uint64_t uniform_buffer::uniform_alignement()
{
    return engine::instance().graphics_device().limits().min_uniform_buffer_alignment;
}

}
