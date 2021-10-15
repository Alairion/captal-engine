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

#ifndef CAPTAL_RENDER_TARGET_HPP_INCLUDED
#define CAPTAL_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <concepts>

#include <tephra/render_target.hpp>

#include "signal.hpp"
#include "texture.hpp"
#include "window.hpp"

namespace cpt
{

using frame_time_t = std::chrono::duration<std::uint64_t, std::nano>;
using frame_presented_signal = cpt::signal<>;
using frame_time_signal = cpt::signal<frame_time_t>;

struct frame_render_info
{
    tph::command_buffer& buffer;
    frame_presented_signal& signal;
    asynchronous_resource_keeper& keeper;
    optional_ref<frame_time_signal> time_signal{};
};

enum class begin_render_options : std::uint32_t
{
    none = 0x00,
    timed = 0x01,
    reset = 0x02
};

class CAPTAL_API render_target
{
public:
    render_target() = default;
    explicit render_target(const tph::render_pass_info& info);

    virtual ~render_target() = default;

    virtual std::optional<frame_render_info> begin_render(begin_render_options options) = 0;
    virtual void present() = 0;
    virtual void wait() = 0;

    tph::render_pass& get_render_pass() noexcept
    {
        return m_render_pass;
    }

    const tph::render_pass& get_render_pass() const noexcept
    {
        return m_render_pass;
    }

protected:
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept = default;
    render_target& operator=(render_target&&) noexcept = default;

private:
    tph::render_pass m_render_pass{};
};

using render_target_ptr = std::shared_ptr<render_target>;
using render_target_weak_ptr = std::weak_ptr<render_target>;

}

template<> struct cpt::enable_enum_operations<cpt::begin_render_options> {static constexpr bool value{true};};

#endif
