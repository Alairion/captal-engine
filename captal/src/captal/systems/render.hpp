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

#ifndef CAPTAL_RENDER_HPP_INCLUDED
#define CAPTAL_RENDER_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include <tephra/commands.hpp>

#include "../components/node.hpp"
#include "../components/drawable.hpp"
#include "../components/camera.hpp"

#include "../engine.hpp"
#include "../view.hpp"
#include "../render_window.hpp"
#include "../renderable.hpp"

namespace cpt::systems
{

template<components::drawable_specialization Drawable = components::drawable>
void prepare_render(entt::registry& world)
{
    const auto drawable_update = [](const components::node& node, Drawable& drawable)
    {
        if(drawable && node.is_updated())
        {
            drawable.apply([&node](auto& renderable)
            {
                renderable.move_to(node.position());
                renderable.set_origin(node.origin());
                renderable.set_rotation(node.rotation());
                renderable.set_scale(node.scale());
            });
        }
    };

    const auto camera_update = [](const components::node& node, components::camera& camera)
    {
        if(camera && node.is_updated())
        {
            camera->move_to(node.position());
            camera->set_origin(node.origin());
            camera->set_rotation(node.rotation());
            camera->set_scale(node.scale());
        }
    };

    world.view<const components::node, Drawable>().each(drawable_update);
    world.view<const components::node, components::camera>().each(camera_update);
}

template<components::drawable_specialization Drawable = components::drawable>
void render(entt::registry& world, cpt::begin_render_options options = cpt::begin_render_options::none)
{
    prepare_render<Drawable>(world);

    world.view<components::camera>().each([&world, options](components::camera& camera)
    {
        if(camera)
        {
            auto render  {camera->target().begin_render(options)};
            auto transfer{engine::instance().begin_transfer()};

            if(render)
            {
                camera->upload(transfer);
                camera->bind(*render);

                world.view<Drawable>().each([&camera, &transfer, &render](Drawable& drawable)
                {
                    if(drawable)
                    {
                        drawable.apply([&camera, &transfer, &render](auto& renderable)
                        {
                            if(!renderable.hidden())
                            {
                                renderable.upload(transfer);

                                if(render)
                                {
                                    renderable.draw(*render, *camera);
                                }
                            }
                        });
                    }
                });
            }
            else
            {
                camera->upload(transfer);

                world.view<Drawable>().each([&camera, &transfer](Drawable& drawable)
                {
                    if(drawable)
                    {
                        drawable.apply([&camera, &transfer](auto& renderable)
                        {
                            if(!renderable.hidden())
                            {
                                renderable.upload(transfer);
                            }
                        });
                    }
                });
            }

        }
    });
}

}

#endif
