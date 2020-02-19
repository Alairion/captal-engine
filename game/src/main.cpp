#include <iostream>

#include <captal/engine.hpp>

#include "config.hpp"

int main()
{
    try
    {
        const cpt::audio_parameters audio{2, 44100};
        const cpt::graphics_parameters graphics{tph::renderer_options::tiny_memory_heaps};

        cpt::engine engine{"my_project", mpr::game_version, audio, graphics};
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
