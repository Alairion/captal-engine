#ifndef CAPTAL_RENDER_HPP_INCLUDED
#define CAPTAL_RENDER_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include <tephra/commands.hpp>

#include "../components/node.hpp"
#include "../components/drawable.hpp"
#include "../components/camera.hpp"

#include "../view.hpp"
#include "../render_window.hpp"
#include "../renderable.hpp"

namespace cpt::systems
{

template<components::drawable_specialization Drawable = components::drawable>
void render(entt::registry& world)
{
    const auto drawable_update = [](const components::node& node, Drawable& drawable)
    {
        if(drawable && node.is_updated())
        {
            cpt::renderable& renderable{drawable.renderable()};

            renderable.move_to(node.position());
            renderable.set_origin(node.origin());
            renderable.set_rotation(node.rotation());
            renderable.set_scale(node.scale());
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

    const auto draw = [&world](components::camera& camera)
    {
        if(camera && camera->target().is_rendering_enable())
        {
            auto&& [buffer, signal, keeper] = camera->target().begin_render();

            camera->bind(buffer);
            camera->upload();

            const std::size_t camera_binding_count{std::size(camera->bindings()) + 1}; //one more for view/projection matrices uniform
            const std::size_t camera_resource_count{camera_binding_count + 1}; //one more for the render technique
            const std::size_t drawable_binding_count{std::size(camera->render_technique()->bindings()) - camera_binding_count};
            const std::size_t drawable_resource_count{world.size<Drawable>() * (drawable_binding_count + 2)};

            keeper.reserve(camera_resource_count + drawable_resource_count);

            keeper.keep(camera->render_technique());
            keeper.keep(camera->resource());
            for(const auto& [index, binding] : camera->bindings())
            {
                keeper.keep(get_binding_resource(binding));
            }

            world.view<Drawable>().each([&keeper = keeper, &camera, &buffer = buffer](Drawable& drawable)
            {
                if(drawable)
                {
                    cpt::renderable& renderable{drawable.renderable()};

                    if(!renderable.hidden())
                    {
                        renderable.set_view(*camera);
                        renderable.upload();
                        renderable.draw(buffer);

                        keeper.keep(renderable.set());
                        keeper.keep(renderable.resource());
                        keeper.keep(renderable.texture());
                        for(const auto& [index, binding] : renderable.bindings())
                        {
                            keeper.keep(get_binding_resource(binding));
                        }
                    }
                }
            });
        }
    };

    world.view<const components::node, Drawable>().each(drawable_update);
    world.view<const components::node, components::camera>().each(camera_update);
    world.view<components::camera>().each(draw);
}

}

#endif
