#ifndef CAPTAL_PUSH_CONSTANT_BUFFER_HPP_INCLUDED
#define CAPTAL_PUSH_CONSTANT_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <array>
#include <vector>
#include <span>
#include <variant>
#include <cassert>

#include <tephra/pipeline.hpp>
#include <tephra/commands.hpp>

namespace cpt
{

class push_constants_buffer
{
public:
    push_constants_buffer() = default;
    ~push_constants_buffer() = default;
    push_constants_buffer(const push_constants_buffer&) = delete;
    push_constants_buffer& operator=(const push_constants_buffer&) = delete;
    push_constants_buffer(push_constants_buffer&& other) noexcept = default;
    push_constants_buffer& operator=(push_constants_buffer&& other) noexcept = default;

    template<typename T>
    void set(tph::shader_stage stages, std::uint32_t offset, T&& value)
    {
        static_assert(alignof(T) <= 4, "cpt::push_constants_buffer::set called with a T having an alignement > 4, which may lead to UB.");
        assert(offset % 4 == 0 && "cpt::push_constants_buffer::set called with invalid offset (offset must be a multiple of 4)");

        const auto index{assure(stages, offset, sizeof(T) / 4)};

        *std::launder(reinterpret_cast<T*>(std::data(m_data) + index)) = std::forward<T>(value);
    }

    template<typename T>
    const T& get(tph::shader_stage stages, std::uint32_t offset) const
    {
        static_assert(alignof(T) <= 4, "cpt::push_constants_buffer::get_push_constant called with a T having an alignement > 4, which may lead to UB.");

        const auto key{make_key(stages, offset)};
        const auto it {m_offsets.find(key)};

        assert(it != std::end(m_offsets) && "cpt::push_constants_buffer::get object doesn't exist at specified offset and stages.");

        return *std::launder(reinterpret_cast<const T*>(std::data(m_data) + it->second));
    }

    bool has(tph::shader_stage stages, std::uint32_t offset) const
    {
        return m_offsets.find(make_key(stages, offset)) != std::end(m_offsets);
    }

    void push(tph::command_buffer& buffer, tph::pipeline_layout& layout, std::span<const tph::push_constant_range> ranges) const
    {
        for(auto&& range : ranges)
        {
            const auto key{make_key(range.stages, range.offset)};
            const auto it {m_offsets.find(key)};

            assert(it != std::end(m_offsets) && "cpt::push_constants_buffer::push object doesn't exist at specified offset and stages, which results in UB.");

            tph::cmd::push_constants(buffer, layout, range.stages, range.offset, range.size, std::data(m_data) + it->second);
        }
    }

private:
    std::size_t assure(tph::shader_stage stages, std::uint32_t offset, std::size_t size)
    {
        const auto key{make_key(stages, offset)};
        const auto it {m_offsets.find(key)};

        if(it == std::end(m_offsets))
        {
            const auto index{std::size(m_data)};

            m_offsets.emplace(key, index);
            m_data.resize(index + size);

            return index;
        }

        return it->second;
    }

    static std::uint64_t make_key(tph::shader_stage stages, std::uint32_t offset) noexcept
    {
        return static_cast<std::uint64_t>(stages) << 32 | offset;
    }

private:
    std::vector<std::uint32_t> m_data{};
    std::unordered_map<std::uint64_t, std::size_t> m_offsets{};
};

}

#endif
