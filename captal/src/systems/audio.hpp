#ifndef CAPTAL_AUDIO_HPP_INCLUDED
#define CAPTAL_AUDIO_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../sound.hpp"

#include "../components/node.hpp"
#include "../components/listener.hpp"
#include "../components/audio_emiter.hpp"

namespace cpt::systems
{

namespace impl
{

CAPTAL_API void move_listener(components::node& node);

}

inline void audio(entt::registry& world)
{
    world.view<components::listener, components::node>().each([](components::node& node)
    {
        if(node.is_updated())
        {
            impl::move_listener(node);
        }
    });

    world.view<components::audio_emiter, components::node>().each([](const components::audio_emiter& emiter, components::node& node)
    {
        if(node.is_updated())
        {
            assert(emiter.attachment() && "Invalid attachment");

            emiter.attachment()->move_to(node.position());
        }
    });
}

}

#endif
