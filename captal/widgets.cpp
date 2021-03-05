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

static void run()
{
    auto window{cpt::make_render_window("Captal widgets", cpt::video_mode{640, 480}, apr::window_options::resizable)};
    window->set_clear_color(cpt::colors::white);

    cpt::view view{window};
    view.fit(window);

    cpt::sprite sprite{40, 40, cpt::colors::dodgerblue};
    sprite.move_to(cpt::vec3f{300.0f, 220.0f, 0.0f});

    auto transfer_info{cpt::engine::instance().begin_transfer()};

    view.upload(transfer_info);
    sprite.upload(transfer_info);

    cpt::engine::instance().submit_transfers();

    while(cpt::engine::instance().run())
    {
        window->update();

        if(window->is_rendering_enable())
        {
            auto render_info{window->begin_render()};

            view.bind(render_info);
            sprite.draw(render_info, view);

            window->present();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }
}

int main()
{
    const cpt::system_parameters system{};

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

        run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
