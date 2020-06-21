#include "render.hpp"

#include <tephra/commands.hpp>

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
    world.view<components::node, components::drawable>().each([](components::node& node, components::drawable& drawable)
    {
        if(node.is_updated())
        {
            assert(drawable.attachment() && "Invalid attachment");

            drawable.attachment()->move_to(node.position());
            drawable.attachment()->set_origin(node.origin());
            drawable.attachment()->set_rotation(node.rotation());
            drawable.attachment()->set_scale(node.scale());
        }
    });

    world.view<components::node, components::camera>().each([](components::node& node, components::camera& camera)
    {
        if(node.is_updated())
        {
            assert(camera.attachment() && "Invalid attachment");

            camera.attachment()->move_to(node.position());
            camera.attachment()->set_origin(node.origin());
            camera.attachment()->set_rotation(node.rotation());
            camera.attachment()->set_scale(node.scale());
        }
    });
}

static void draw(entt::registry& world)
{
    world.view<components::camera>().each([&](components::camera& camera)
    {
        assert(camera.attachment() && "Invalid attachment");

        if(camera.attachment()->target().is_rendering_enable())
        {
            const view_ptr& view{camera.attachment()};
            const render_technique_ptr& technique{view->render_technique()};
            render_target& target{view->target()};
            auto&& [buffer, signal] = target.begin_render();

            view->upload();

            tph::cmd::set_viewport(buffer, view->viewport());
            tph::cmd::set_scissor(buffer, view->scissor());
            tph::cmd::bind_pipeline(buffer, technique->pipeline());

            for(auto&& range : view->render_technique()->ranges())
            {
                tph::cmd::push_constants(buffer, technique->pipeline_layout(), range.stages, range.offset, range.size, std::data(view->render_technique()->push_constant_buffer()) + range.offset / 4u);
            }

            std::vector<std::shared_ptr<asynchronous_resource>> to_keep_alive{};
            to_keep_alive.reserve(world.size<components::drawable>() * 2 + 2);

            to_keep_alive.emplace_back(view);
            to_keep_alive.emplace_back(technique);

            world.view<components::drawable>().each([&, &buffer = buffer](entt::entity entity [[maybe_unused]], const components::drawable& drawable)
            {
                assert(drawable.attachment() && "Invalid attachment");

                renderable& renderable{*drawable.attachment()};

                if(!renderable.hidden())
                {
                    renderable.set_view(view);
                    renderable.upload();
                    renderable.draw(buffer);

                    to_keep_alive.emplace_back(drawable.attachment());
                    to_keep_alive.emplace_back(renderable.set());
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
