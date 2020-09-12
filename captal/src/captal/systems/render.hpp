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
            render_target& target{camera->target()};
            render_technique_ptr technique{camera->render_technique()};
            auto&& [buffer, signal] = target.begin_render();

            camera->upload();

            tph::cmd::set_viewport(buffer, camera->viewport());
            tph::cmd::set_scissor(buffer, camera->scissor());
            tph::cmd::bind_pipeline(buffer, technique->pipeline());

            for(auto&& range : camera->render_technique()->ranges())
            {
                tph::cmd::push_constants(buffer, technique->pipeline_layout(), range.stages, range.offset, range.size, std::data(camera->push_constant_buffer()) + range.offset / 4u);
            }

            std::vector<asynchronous_resource_ptr> to_keep_alive{};
            to_keep_alive.reserve(world.size<Drawable>() * 2 + 2);

            to_keep_alive.emplace_back(camera->resource());
            to_keep_alive.emplace_back(std::move(technique));

            world.view<Drawable>().each([&to_keep_alive, &camera, &buffer = buffer](Drawable& drawable)
            {
                if(drawable)
                {
                    cpt::renderable& renderable{drawable.renderable()};

                    if(!renderable.hidden())
                    {
                        renderable.set_view(*camera);
                        renderable.upload();
                        renderable.draw(buffer);

                        to_keep_alive.emplace_back(renderable.set());
                        to_keep_alive.emplace_back(renderable.resource());
                    }
                }
            });

            signal.connect([to_keep_alive = std::move(to_keep_alive)]()
            {

            });
        }
    };

    world.view<const components::node, Drawable>().each(drawable_update);
    world.view<const components::node, components::camera>().each(camera_update);
    world.view<components::camera>().each(draw);
}

}

#endif
