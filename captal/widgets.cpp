#include <iostream>

#include <captal/engine.hpp>
#include <captal/widgets.hpp>
#include <captal/renderable.hpp>
/*
static_assert(cpt::tuple_like<std::tuple<int, float, double>>);
static_assert(cpt::tuple_like<std::array<int, 42>>);
static_assert(cpt::tuple_like<std::pair<int, float>>);
static_assert(!cpt::tuple_like<int>);
static_assert(!cpt::tuple_like<std::string>);

template<typename... Children>
struct widget : cpt::basic_widget
{
    std::tuple<Children...> children{};
};

struct simple_widget : cpt::basic_widget
{

};

static_assert(cpt::widget<widget<>>);
static_assert(!cpt::widget<std::string>);

static_assert(cpt::widget_tuple<std::tuple<widget<>>>);
static_assert(!cpt::widget_tuple<std::tuple<std::string>>);

static_assert(cpt::parent_widget<widget<simple_widget>>);
static_assert(!cpt::parent_widget<widget<std::string>>);
*/
static_assert(cpt::renderable<cpt::basic_renderable>);

using namespace cpt::enum_operations;

static void run()
{
    auto window{cpt::make_window("Captal widgets", 640, 480, apr::window_options::resizable | apr::window_options::extended_client_area)};
    auto other{cpt::make_window("Captal other", 640, 480, apr::window_options::borderless)};

    constexpr cpt::video_mode mode
    {
        .image_count = 3,
        .present_mode = tph::present_mode::mailbox,
        .sample_count = tph::sample_count::msaa_x4
    };

    auto target{cpt::make_render_window(window, mode)};
    target->set_clear_color(cpt::color{0.0f, 0.0f, 0.0f, 0.0f});

    auto other_target{cpt::make_render_window(other, mode)};
    target->set_clear_color(cpt::color{1.0f, 1.0f, 1.0f, 1.0f});

    cpt::view view{target, cpt::render_technique_info{.multisample{tph::sample_count::msaa_x4}}};
    view.fit(window);

    cpt::sprite sprite{40, 40, cpt::colors::dodgerblue};
    sprite.move_to(cpt::vec3f{0.0f, 0.0f, 0.0f});

    auto transfer_info{cpt::engine::instance().begin_transfer()};

    view.upload(transfer_info);
    sprite.upload(transfer_info);

    cpt::engine::instance().submit_transfers();

    cpt::engine::instance().frame_per_second_update_signal().connect([&target](std::uint32_t fps)
    {
        std::cout << fps << " : " << (target->status() == cpt::render_window_status::ok) << std::endl;
    });

    window->on_close().connect([](cpt::window& window, const apr::window_event&){window.close();});

    window->on_key_pressed().connect([](cpt::window& window, const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::f1)
        {
            window.switch_to_fullscreen();
        }
        else if(event.scan == apr::scancode::f2)
        {
            window.switch_to_windowed_fullscreen();
        }
        else if(event.scan == apr::scancode::f3)
        {
            window.switch_to_windowed();
        }
    });

    window->change_hit_test_function([](std::int32_t, std::int32_t y) -> apr::hit_test_result
    {
        if(y > 0 && y < 20)
        {
            return apr::hit_test_result::drag;
        }

        return apr::hit_test_result::normal;
    });

    other->on_close().connect([](cpt::window& window, const apr::window_event&){window.close();});

    other->on_key_pressed().connect([](cpt::window& window, const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::f1)
        {
            window.switch_to_fullscreen();
        }
        else if(event.scan == apr::scancode::f2)
        {
            window.switch_to_windowed_fullscreen();
        }
        else if(event.scan == apr::scancode::f3)
        {
            window.switch_to_windowed();
        }
    });

    other->change_hit_test_function([](std::int32_t, std::int32_t y) -> apr::hit_test_result
    {
        if(y > 0 && y < 20)
        {
            return apr::hit_test_result::drag;
        }

        return apr::hit_test_result::normal;
    });

    while(cpt::engine::instance().run())
    {
        window->dispatch_events();
        other->dispatch_events();

        auto render_info{target->begin_render(cpt::begin_render_options::reset)};

        if(render_info)
        {
            view.bind(*render_info);
            sprite.draw(*render_info, view);
        }

        target->present();

        other_target->begin_render(cpt::begin_render_options::none);
        other_target->present();
    }
}

int main()
{
    const cpt::system_parameters system
    {
        .extensions = apr::application_extension::extended_client_area
    };

    const cpt::audio_parameters audio
    {
        .channel_count = 2,
        .frequency = 44100
    };

    const cpt::graphics_parameters graphics
    {
        .options = tph::renderer_options::small_memory_heaps,
    };

    try
    {
        cpt::engine engine{"captal_test", cpt::version{0, 1, 0}, system, audio, graphics};
        engine.set_framerate_limit(100);

        run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
