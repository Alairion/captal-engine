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

#include "render_window.hpp"

#include "engine.hpp"

namespace cpt
{

static tph::texture_format choose_surface_format(tph::surface& surface, video_mode& mode)
{
    const auto choose_format = [&surface]
    {
        const auto formats{surface.formats(engine::instance().graphics_device())};

        if(std::size(formats) == 1 && formats[0] == tph::texture_format::undefined)
        {
            return tph::texture_format::r8g8b8a8_srgb;
        }

        for(auto format : formats)
        {
            if(format == tph::texture_format::b8g8r8a8_srgb)
            {
                return format;
            }
            else if(format == tph::texture_format::r8g8b8a8_srgb)
            {
                return format;
            }
        }

        for(auto format : formats)
        {
            if(format == tph::texture_format::b8g8r8a8_unorm)
            {
                return format;
            }
            else if(format == tph::texture_format::b8g8r8a8_unorm)
            {
                return format;
            }
        }

        return formats[0];
    };

    if(mode.surface_format == tph::texture_format::undefined)
    {
        mode.surface_format = choose_format();
    }

    return mode.surface_format;
}

static tph::render_pass_info make_render_pass_info(tph::texture_format color_format, tph::sample_count sample_count, tph::texture_format depth_format)
{
    const bool has_multisampling{sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{depth_format != tph::texture_format::undefined};

    tph::render_pass_info output{};
    auto& subpass{output.subpasses.emplace_back()};

    auto& color_attachement{output.attachments.emplace_back()};
    color_attachement.format = color_format;
    color_attachement.sample_count = sample_count;
    color_attachement.load_op = tph::attachment_load_op::clear;

    if(has_multisampling)
    {
        color_attachement.store_op = tph::attachment_store_op::dont_care;
    }
    else
    {
        color_attachement.store_op = tph::attachment_store_op::store;
    }

    color_attachement.stencil_load_op = tph::attachment_load_op::clear;
    color_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
    color_attachement.initial_layout = tph::texture_layout::undefined;

    if(has_multisampling)
    {
        color_attachement.final_layout = tph::texture_layout::color_attachment_optimal;
    }
    else
    {
        color_attachement.final_layout = tph::texture_layout::present_source;
    }

    subpass.color_attachments.emplace_back(tph::attachment_reference{0, tph::texture_layout::color_attachment_optimal});

    if(has_depth_stencil)
    {
        auto& depth_attachement{output.attachments.emplace_back()};
        depth_attachement.format = depth_format;
        depth_attachement.sample_count = sample_count;
        depth_attachement.load_op = tph::attachment_load_op::clear;
        depth_attachement.store_op = tph::attachment_store_op::dont_care;
        depth_attachement.stencil_load_op = tph::attachment_load_op::clear;
        depth_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
        depth_attachement.initial_layout = tph::texture_layout::undefined;
        depth_attachement.final_layout = tph::texture_layout::depth_stencil_attachment_optimal;

        subpass.depth_attachment.emplace(tph::attachment_reference{1, tph::texture_layout::depth_stencil_attachment_optimal});
    }

    if(has_multisampling)
    {
        auto& resolve_attachement{output.attachments.emplace_back()};
        resolve_attachement.format = color_format;
        resolve_attachement.sample_count = tph::sample_count::msaa_x1;
        resolve_attachement.load_op = tph::attachment_load_op::clear;
        resolve_attachement.store_op = tph::attachment_store_op::store;
        resolve_attachement.stencil_load_op = tph::attachment_load_op::clear;
        resolve_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
        resolve_attachement.initial_layout = tph::texture_layout::undefined;
        resolve_attachement.final_layout = tph::texture_layout::present_source;

        if(has_depth_stencil)
        {
            subpass.resolve_attachments.emplace_back(tph::attachment_reference{2, tph::texture_layout::color_attachment_optimal});
        }
        else
        {
            subpass.resolve_attachments.emplace_back(tph::attachment_reference{1, tph::texture_layout::color_attachment_optimal});
        }
    }

    return output;
}

static std::optional<tph::swapchain> make_swapchain(const cpt::video_mode& mode, window& window, optional_ref<tph::swapchain> old)
{
    const auto capabilities{window.surface().capabilities(engine::instance().device())};

    tph::swapchain_info info{};

    if(capabilities.current_width == 0 || capabilities.current_height == 0)
    {
        return std::nullopt;
    }
    else if(capabilities.current_width == 0xFFFFFFFFu || capabilities.current_height == 0xFFFFFFFFu)
    {
        const auto [width, height] = window.atomic_surface_size();

        info.width  = std::clamp(width,  capabilities.min_width,  capabilities.max_width);
        info.height = std::clamp(height, capabilities.min_height, capabilities.max_height);
    }
    else
    {
        info.width  = capabilities.current_width;
        info.height = capabilities.current_height;
    }

    info.image_count  = mode.image_count;
    info.format       = mode.surface_format;
    info.usage        = mode.usage;
    info.composite    = mode.composite;
    info.transform    = capabilities.current_transform;
    info.present_mode = mode.present_mode;
    info.clipping     = mode.clipping;

    return tph::swapchain{engine::instance().device(), window.surface(), info, old};
}

static std::pair<tph::texture, tph::texture_view> make_msaa_texture(const tph::swapchain& swapchain, tph::texture_format surface_format, tph::sample_count sample_count)
{
    if(sample_count == tph::sample_count::msaa_x1)
    {
        return std::pair<tph::texture, tph::texture_view>{};
    }

    const tph::texture_info info{.format = surface_format, .usage = tph::texture_usage::color_attachment, .sample_count = sample_count};

    tph::texture texture{engine::instance().device(), swapchain.info().width, swapchain.info().height, info};
    tph::texture_view view{engine::instance().device(), texture};

    return std::make_pair(std::move(texture), std::move(view));
}

static std::pair<tph::texture, tph::texture_view> make_depth_texture(const tph::swapchain& swapchain, tph::texture_format depth_format, tph::sample_count sample_count)
{
    if(depth_format == tph::texture_format::undefined)
    {
        return std::pair<tph::texture, tph::texture_view>{};
    }

    const tph::texture_info info{.format = depth_format, .usage = tph::texture_usage::depth_stencil_attachment, .sample_count = sample_count};

    tph::texture texture{engine::instance().device(), swapchain.info().width, swapchain.info().height, info};
    tph::texture_view view{engine::instance().device(), texture};

    return std::make_pair(std::move(texture), std::move(view));
}

static std::vector<std::reference_wrapper<tph::texture_view>> make_attachments(video_mode mode, tph::texture_view& color, tph::texture_view& multisampling, tph::texture_view& depth)
{
    const bool has_multisampling{mode.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{mode.depth_format != tph::texture_format::undefined};

    std::vector<std::reference_wrapper<tph::texture_view>> output{};

    if(has_multisampling)
    {
        output.emplace_back(multisampling);

        if(has_depth_stencil)
        {
            output.emplace_back(depth);
        }

        output.emplace_back(color);
    }
    else
    {
        output.emplace_back(color);

        if(has_depth_stencil)
        {
            output.emplace_back(depth);
        }
    }

    return output;
}

render_window::render_window(window_ptr window, video_mode mode)
:render_target{make_render_pass_info(choose_surface_format(window->surface(), mode), mode.sample_count, mode.depth_format)}
,m_window{std::move(window)}
,m_mode{mode}
,m_pool{engine::instance().device(), tph::command_pool_options::reset}
{
    recreate();
}

render_window::~render_window()
{
    wait();
}

std::optional<frame_render_info> render_window::begin_render(begin_render_options options)
{
    if(m_fake_frame || m_status == render_window_status::surface_lost)
    {
        m_fake_frame = true;

        return std::nullopt;
    }

    if(!m_swapchain || std::empty(m_frames_data))
    {
        if(!recreate())
        {
            m_fake_frame = true;

            return std::nullopt;
        }
    }

    auto& data{m_frames_data[m_frame_index]};

    if(data.begin)
    {
        if(data.epoch == m_epoch)
        {
            return std::nullopt;
        }

        if(static_cast<bool>(options & begin_render_options::timed))
        {
            assert(data.timed && "cpt::render_window::begin_render must not be called with begin_render_options::timed flag if initial call was made without.");

            return frame_render_info{data.buffer, data.signal, data.keeper, data.time_signal};
        }
        else
        {
            return frame_render_info{data.buffer, data.signal, data.keeper};
        }
    }

    if(static_cast<bool>(options & begin_render_options::reset))
    {
        ++m_epoch;
    }

    if(!acquire(data))
    {
        return std::nullopt;
    }

    auto& framebuffer{m_framebuffers[m_swapchain->image_index()]};
    update_clear_values(framebuffer);

    tph::cmd::begin(data.buffer, tph::command_buffer_reset_options::none);

    if(static_cast<bool>(options & begin_render_options::timed))
    {
        data.timed = true;

        tph::cmd::reset_query_pool(data.buffer, data.query_pool, 0, 2);
        tph::cmd::write_timestamp(data.buffer, data.query_pool, 0, tph::pipeline_stage::top_of_pipe);

        tph::cmd::begin_render_pass(data.buffer, get_render_pass(), framebuffer);

        return frame_render_info{data.buffer, data.signal, data.keeper, data.time_signal};
    }
    else
    {
        tph::cmd::begin_render_pass(data.buffer, get_render_pass(), framebuffer);

        return frame_render_info{data.buffer, data.signal, data.keeper};
    }
}

void render_window::present()
{
    if(std::exchange(m_fake_frame, false))
    {
        return;
    }

    auto& data{m_frames_data[m_frame_index]};

    assert(data.begin && "cpt::render_window::present called without prior call to cpt:render_window::begin_render.");

    m_frame_index = (m_frame_index + 1) % m_swapchain->info().image_count;

    if(data.epoch != m_epoch)
    {
        tph::cmd::end_render_pass(data.buffer);

        if(data.timed)
        {
            tph::cmd::write_timestamp(data.buffer, data.query_pool, 1, tph::pipeline_stage::bottom_of_pipe);
        }

        tph::cmd::end(data.buffer);
        data.epoch = m_epoch;
    }

    data.begin = false;

    tph::submit_info submit_info{};
    submit_info.wait_semaphores.emplace_back(data.image_available);
    submit_info.wait_stages.emplace_back(tph::pipeline_stage::color_attachment_output);
    submit_info.command_buffers.emplace_back(data.buffer);
    submit_info.signal_semaphores.emplace_back(data.image_presentable);

    data.fence.reset();

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().device(), submit_info, data.fence);
    lock.unlock();

    data.submitted = true;

    const auto status{m_swapchain->present(data.image_presentable)};

    if(status == tph::swapchain_status::surface_lost)
    {
        m_status = render_window_status::surface_lost;
    }
    else if(status != tph::swapchain_status::valid)
    {
        recreate();
    }
}

void render_window::wait()
{
    for(auto& data : m_frames_data)
    {
        if(data.submitted)
        {
            reset_frame_data(data);
        }
    }
}

#ifdef CAPTAL_DEBUG
void render_window::set_name(std::string_view name)
{
    m_name = std::string{name};

    const bool has_multisampling{m_mode.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{m_mode.depth_format != tph::texture_format::undefined};

    tph::set_object_name(engine::instance().device(), get_render_pass(), m_name + " render pass");

    if(m_swapchain)
    {
        tph::set_object_name(engine::instance().device(), *m_swapchain, m_name + " swapchain");

        if(has_multisampling)
        {
            tph::set_object_name(engine::instance().device(), m_msaa_texture, m_name + " multisampling texture");
        }

        if(has_depth_stencil)
        {
            tph::set_object_name(engine::instance().device(), m_depth_texture, m_name + " depth texture");
        }

        for(std::uint32_t i{}; i < m_swapchain->info().image_count; ++i)
        {
            tph::set_object_name(engine::instance().device(), m_swapchain->textures()[i], m_name + " swapchain image #" + std::to_string(i));
            tph::set_object_name(engine::instance().device(), m_framebuffers[i],          m_name + " swapchain framebuffer #" + std::to_string(i));

            tph::set_object_name(engine::instance().device(), m_frames_data[i].buffer,            m_name + " frame #" + std::to_string(i) + " command buffer");
            tph::set_object_name(engine::instance().device(), m_frames_data[i].image_available,   m_name + " frame #" + std::to_string(i) + " available semaphore");
            tph::set_object_name(engine::instance().device(), m_frames_data[i].image_presentable, m_name + " frame #" + std::to_string(i) + " presentable semaphore");
            tph::set_object_name(engine::instance().device(), m_frames_data[i].fence,             m_name + " frame #" + std::to_string(i) + " fence");
            tph::set_object_name(engine::instance().device(), m_frames_data[i].query_pool,        m_name + " frame #" + std::to_string(i) + " query pool");
        }
    }
}
#endif

void render_window::setup_frame_data()
{
    m_frames_data.clear();
    m_frames_data.reserve(m_swapchain->info().image_count);

    for(std::uint32_t i{}; i < m_swapchain->info().image_count; ++i)
    {
        frame_data data{};
        data.buffer = tph::cmd::begin(m_pool, tph::command_buffer_level::primary);
        data.image_available = tph::semaphore{engine::instance().device()};
        data.image_presentable = tph::semaphore{engine::instance().device()};
        data.fence = tph::fence{engine::instance().device(), true};
        data.query_pool = tph::query_pool{engine::instance().device(), 2, tph::query_type::timestamp};

        m_frames_data.emplace_back(std::move(data));
    }
}

void render_window::setup_framebuffers()
{
    m_framebuffers.clear();
    m_framebuffers.reserve(m_swapchain->info().image_count);

    for(std::uint32_t i{}; i < m_swapchain->info().image_count; ++i)
    {
        const auto attachments{make_attachments(m_mode, m_swapchain->texture_views()[i], m_msaa_texture_view, m_depth_texture_view)};

        m_framebuffers.emplace_back(engine::instance().device(), get_render_pass(), attachments, m_swapchain->info().width, m_swapchain->info().height, 1);
    }
}

void render_window::update_clear_values(tph::framebuffer& framebuffer)
{
    const bool has_multisampling{m_mode.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{m_mode.depth_format != tph::texture_format::undefined};

    framebuffer.set_clear_value(0, m_clear_color);

    if(has_depth_stencil)
    {
        framebuffer.set_clear_value(1, m_clear_depth_stencil);

        if(has_multisampling)
        {
            framebuffer.set_clear_value(2, m_clear_color);
        }
    }
    else if(has_multisampling)
    {
        framebuffer.set_clear_value(1, m_clear_color);
    }
}

bool render_window::check_renderability()
{
    try
    {
        const auto capabilities{m_window->surface().capabilities(engine::instance().device())};

        if(capabilities.current_width != 0 && capabilities.current_height != 0)
        {
            m_status = render_window_status::ok;
            return true;
        }
        else
        {
            m_status = render_window_status::unrenderable;
            return false;
        }
    }
    catch(const tph::vulkan::error& e)
    {
        if(e.error_code() == VK_ERROR_SURFACE_LOST_KHR)
        {
            m_status = render_window_status::surface_lost;
            return false;
        }

        throw;
    }
}

void render_window::time_results(frame_data& data)
{
    std::array<std::uint64_t, 2> results;
    data.query_pool.results(0, 2, std::size(results) * sizeof(std::uint64_t), std::data(results), sizeof(std::uint64_t), tph::query_results::uint64 | tph::query_results::wait);

    const auto         period{static_cast<double>(cpt::engine::instance().graphics_device().limits().timestamp_period)};
    const frame_time_t time  {static_cast<std::uint64_t>(static_cast<double>(results[1] - results[0]) * period)};

    data.time_signal(time);
}

void render_window::flush_frame_data(frame_data& data)
{
    data.fence.wait();

    data.submitted = false;

    if(data.timed)
    {
        time_results(data);
    }

    data.signal();
}

void render_window::reset_frame_data(frame_data& data)
{
    data.fence.wait();

    data.submitted = false;

    if(data.timed)
    {
        time_results(data);

        data.timed = false;
        data.time_signal.disconnect_all();
    }

    data.signal();
    data.signal.disconnect_all();

    data.keeper.clear();
}

bool render_window::acquire(frame_data& data)
{
    auto status{m_swapchain->acquire(data.image_available, tph::nullref)};

    //We continue the normal path even if the swapchain is suboptimal
    //The present function will recreate it after presentation (if possible)
    while(status == tph::swapchain_status::out_of_date)
    {
        if(!recreate())
        {
            m_fake_frame = true;

            return false;
        }

        status = m_swapchain->acquire(data.image_available, tph::nullref);
    }

    if(status == tph::swapchain_status::surface_lost) //may happens on window closure
    {
        m_status = render_window_status::surface_lost;
        m_fake_frame = true;

        return false;
    }

    data.begin = true;

    if(data.epoch == m_epoch)
    {
        flush_frame_data(data);

        return false;
    }

    reset_frame_data(data);

    return true;
}

bool render_window::recreate()
{
    wait();

    ++m_epoch;

    try
    {
        m_swapchain = make_swapchain(m_mode, *m_window, m_swapchain);

        if(!m_swapchain)
        {
            m_status = render_window_status::unrenderable;
            return false;
        }
    }
    catch(const tph::vulkan::error& e)
    {
        if(e.error_code() == VK_ERROR_SURFACE_LOST_KHR)
        {
            m_status = render_window_status::surface_lost;
            return false;
        }

        throw;
    }

    auto [msaa_texture, msaa_view] = make_msaa_texture(*m_swapchain, m_mode.surface_format, m_mode.sample_count);
    m_msaa_texture = std::move(msaa_texture);
    m_msaa_texture_view = std::move(msaa_view);

    auto [depth_texture, depth_view] = make_depth_texture(*m_swapchain, m_mode.depth_format, m_mode.sample_count);
    m_depth_texture = std::move(depth_texture);
    m_depth_texture_view = std::move(depth_view);

    setup_framebuffers();

    if(std::size(m_frames_data) != m_swapchain->info().image_count)
    {
        setup_frame_data();
    }

    #ifdef CAPTAL_DEBUG
    if(!std::empty(m_name))
    {
        set_name(m_name);
    }
    #endif

    m_status = render_window_status::ok;

    return true;
}


}
