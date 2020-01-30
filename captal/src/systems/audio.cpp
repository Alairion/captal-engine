#include "audio.hpp"

#include "../engine.hpp"
#include "../sound.hpp"

#include "../components/node.hpp"
#include "../components/listener.hpp"
#include "../components/audio_emiter.hpp"

namespace cpt
{

namespace systems
{

static void update_listener(entt::registry& world)
{
    world.view<components::listener, components::node>().each([](entt::entity entity [[maybe_unused]], const components::listener& listener [[maybe_unused]], components::node& node)
    {
        if(node.is_updated())
        {
            engine::instance().audio_mixer().move_listener(node.position().x, node.position().y, node.position().z);
            //engine::instance().audio_mixer().set_listener_direction();
        }
    });
}

static void update_emiters(entt::registry& world)
{
    world.view<components::audio_emiter, components::node>().each([](entt::entity entity [[maybe_unused]], const components::audio_emiter& emiter, components::node& node)
    {
        if(node.is_updated())
        {
            if(emiter.attachment())
            {
                emiter.attachment()->move(node.position().x, node.position().y, node.position().z);
            }
        }
    });
}

void audio(entt::registry& world)
{
    update_listener(world);
    update_emiters(world);
}

}

}
