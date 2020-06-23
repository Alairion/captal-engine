#include "audio.hpp"

#include "../engine.hpp"

namespace cpt::systems::impl
{

void move_listener(components::node& node)
{
    engine::instance().audio_mixer().move_listener_to(node.position());
}

}
