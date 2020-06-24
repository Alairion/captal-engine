#ifndef CAPTAL_AUDIO_HPP_INCLUDED
#define CAPTAL_AUDIO_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../engine.hpp"

#include "../components/node.hpp"
#include "../components/listener.hpp"
#include "../components/audio_emiter.hpp"

namespace cpt::systems
{

inline void audio(entt::registry& world)
{
    const auto update_listener = [](components::node& node)
    {
        if(node.is_updated())
        {
            engine::instance().audio_mixer().move_listener_to(node.position());
        }
    };

    const auto update_emiters = [](const components::audio_emiter& emiter, components::node& node)
    {
        if(node.is_updated())
        {
            assert(emiter.attachment() && "Invalid attachment");

            emiter.attachment()->move_to(node.position());
        }
    };

    world.view<components::listener, components::node>().each(update_listener);
    world.view<components::audio_emiter, components::node>().each(update_emiters);
}

}

#endif
