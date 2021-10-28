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

#ifndef CAPTAL_AUDIO_HPP_INCLUDED
#define CAPTAL_AUDIO_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../engine.hpp"

#include "../components/node.hpp"
#include "../components/listener.hpp"
#include "../components/audio_emitter.hpp"

namespace cpt::systems
{

inline void audio(entt::registry& world)
{
    const auto update_listener = [](const components::node& node)
    {
        if(node.is_updated())
        {
            engine::instance().listener().move_to(node.position());
        }
    };

    const auto update_emitters = [](components::audio_emitter& emitter, const components::node& node)
    {
        if(node.is_updated() && emitter.has_attachment())
        {
            emitter->move_to(node.position());
        }
    };

    world.view<components::listener, const components::node>().each(update_listener);
    world.view<components::audio_emitter, const components::node>().each(update_emitters);
}

}

#endif
