#include "render.hpp"

#include <tephra/commands.hpp>

#include <entt/entity/observer.hpp>

#include "../components/node.hpp"
#include "../components/drawable.hpp"
#include "../components/camera.hpp"

#include "../view.hpp"
#include "../render_window.hpp"
#include "../renderable.hpp"

namespace cpt
{

namespace systems
{

static void update_nodes(entt::registry& world)
{
    world.view<components::node, components::drawable>().each([](entt::entity entity [[maybe_unused]], components::node& node, components::drawable& drawable)
    {
        if(node.is_updated())
        {
            if(drawable.attachment())
            {
                drawable.attachment()->move_to(node.position());
                drawable.attachment()->set_origin(node.origin());
                drawable.attachment()->set_rotation(node.rotation());
                drawable.attachment()->set_scale(node.scale());
            }
        }
    });

    world.view<components::node, components::camera>().each([](entt::entity entity [[maybe_unused]], components::node& node, components::camera& camera)
    {
        if(node.is_updated())
        {
            if(camera.attachment())
            {
                camera.attachment()->move_to(node.position());
                camera.attachment()->set_origin(node.origin());
                camera.attachment()->set_rotation(node.rotation());
                camera.attachment()->set_scale(node.scale());
            }
        }
    });
}

static void draw(entt::registry& world)
{
    world.view<components::camera>().each([&](entt::entity entity [[maybe_unused]], components::camera& camera)
    {
        if(camera.attachment() && camera.attachment()->target().is_rendering_enable())
        {
            const view_ptr& view{camera.attachment()};
            const render_technique_ptr& technique{view->render_technique()};
            render_target& target{view->target()};
            auto&& [buffer, signal] = target.begin_render();

            view->upload();

            tph::cmd::set_viewport(buffer, view->viewport());
            tph::cmd::set_scissor(buffer, view->scissor());
            tph::cmd::bind_pipeline(buffer, technique->pipeline());

            std::vector<std::shared_ptr<asynchronous_resource>> to_keep_alive{};
            to_keep_alive.reserve(world.size<components::drawable>() + 1);
            to_keep_alive.push_back(view);

            world.view<components::drawable>().each([&, &buffer = buffer](entt::entity entity [[maybe_unused]], const components::drawable& drawable)
            {
                if(drawable.attachment())
                {
                    const renderable_ptr& renderable{drawable.attachment()};

                    if(!renderable->hidden())
                    {
                        renderable->set_view(view);
                        renderable->upload();
                        renderable->draw(buffer);

                        to_keep_alive.push_back(renderable);
                    }
                }
            });

            signal.connect([to_keep_alive = std::move(to_keep_alive)]()
            {

            });
        }
    });
}

void render(entt::registry& world)
{
    update_nodes(world);
    draw(world);
}

}

}
