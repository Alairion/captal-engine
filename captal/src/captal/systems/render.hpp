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

    const auto draw = [&world, options](components::camera& camera)
    {
        if(camera && camera->target().is_renderable())
        {
            auto render  {camera->target().begin_render(options)};
            auto transfer{engine::instance().begin_transfer()};

            camera->upload(transfer);

            if(render)
            {
                camera->bind(*render);
            }

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
    };

    world.view<components::camera>().each(draw);
}

}

#endif
