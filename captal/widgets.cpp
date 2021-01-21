#include <iostream>

#include <captal/engine.hpp>
#include <captal/widgets.hpp>

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

    cpt::engine engine{"captal_test", cpt::version{0, 1, 0}, system, audio, graphics};


}
