#include "physics.hpp"

#include <stdexcept>

#include <chipmunk/chipmunk.h>

namespace cpt
{

static inline cpVect tocp(const glm::vec2& vec) noexcept
{
    return cpVect{static_cast<cpFloat>(vec.x), static_cast<cpFloat>(vec.y)};
}

static inline cpFloat tocp(float value) noexcept
{
    return static_cast<cpFloat>(value);
}

static inline glm::vec2 fromcp(const cpVect& vec) noexcept
{
    return glm::vec2{static_cast<float>(vec.x), static_cast<float>(vec.y)};
}

static inline float fromcp(cpFloat value) noexcept
{
    return static_cast<float>(value);
}

void physical_collision_arbiter::set_restitution(float restitution) noexcept
{
    cpArbiterSetRestitution(m_arbiter, restitution);
}

void physical_collision_arbiter::set_friction(float friction) noexcept
{
    cpArbiterSetFriction(m_arbiter, friction);
}

void physical_collision_arbiter::set_surface_velocity(const glm::vec2& surface_velocity) noexcept
{
    cpArbiterSetSurfaceVelocity(m_arbiter, tocp(surface_velocity));
}

void physical_collision_arbiter::set_user_data(void* user_data) noexcept
{
    cpArbiterSetUserData(m_arbiter, user_data);
}

std::pair<physical_shape&, physical_shape&> physical_collision_arbiter::shapes() const noexcept
{
    cpShape* first{};
    cpShape* second{};
    cpArbiterGetShapes(m_arbiter, &first, &second);

    return {*reinterpret_cast<physical_shape*>(cpShapeGetUserData(first)), *reinterpret_cast<physical_shape*>(cpShapeGetUserData(second))};
}

std::pair<physical_body&, physical_body&> physical_collision_arbiter::bodies() const noexcept
{
    cpBody* first{};
    cpBody* second{};
    cpArbiterGetBodies(m_arbiter, &first, &second);

    return {*reinterpret_cast<physical_body*>(cpBodyGetUserData(first)), *reinterpret_cast<physical_body*>(cpBodyGetUserData(second))};
}

std::size_t physical_collision_arbiter::contact_count() const noexcept
{
    return static_cast<std::size_t>(cpArbiterGetCount(m_arbiter));
}

glm::vec2 physical_collision_arbiter::normal() const noexcept
{
    return fromcp(cpArbiterGetNormal(m_arbiter));
}

std::pair<glm::vec2, glm::vec2> physical_collision_arbiter::points(std::size_t contact_index) const noexcept
{
    return std::make_pair(fromcp(cpArbiterGetPointA(m_arbiter, contact_index)), fromcp(cpArbiterGetPointB(m_arbiter, contact_index)));
}

float physical_collision_arbiter::depth(std::size_t contact_index) const noexcept
{
    return fromcp(cpArbiterGetDepth(m_arbiter, contact_index));
}

bool physical_collision_arbiter::is_first_contact() const noexcept
{
    return static_cast<bool>(cpArbiterIsFirstContact(m_arbiter));
}

bool physical_collision_arbiter::is_removal() const noexcept
{
    return static_cast<bool>(cpArbiterIsRemoval(m_arbiter));
}

physical_world::physical_world()
:m_world{cpSpaceNew()}
{
    if(!m_world)
        throw std::runtime_error{"Can not allocate physical world."};

    cpSpaceSetUserData(m_world, this);
}

physical_world::~physical_world()
{
    cpSpaceFree(m_world);
}

void physical_world::add_collision(std::uint32_t first_type, std::uint32_t second_type, collision_handler handler)
{
    add_callback(cpSpaceAddCollisionHandler(m_world, first_type, second_type), std::move(handler));
}

void physical_world::add_wildcard(std::uint32_t type, collision_handler handler)
{
    add_callback(cpSpaceAddWildcardHandler(m_world, type), std::move(handler));
}

void physical_world::region_query(float x, float y, float width, float height, std::uint64_t group, std::uint32_t id, std::uint32_t mask, region_query_callback_type callback)
{
    const cpShapeFilter filter{cpShapeFilterNew(static_cast<cpGroup>(group), id, mask)};

    const auto native_callback = [](cpShape* shape, void* data)
    {
        const region_query_callback_type& callback{*reinterpret_cast<const region_query_callback_type*>(data)};
        physical_body& body{*reinterpret_cast<physical_body*>(cpShapeGetUserData(shape))};

        callback(body);
    };

    cpSpaceBBQuery(m_world, cpBBNew(tocp(x), tocp(y), tocp(x + width), tocp(y + height)), filter, native_callback, &callback);
}

void physical_world::raycast(const glm::vec2& from, const glm::vec2& to, float thickness, std::uint64_t group, std::uint32_t id, std::uint32_t mask, raycast_callback_type callback)
{
    const cpShapeFilter filter{cpShapeFilterNew(static_cast<cpGroup>(group), id, mask)};

    const auto native_callback = [](cpShape* shape, cpVect point, cpVect normal, cpFloat alpha, void* data)
    {
        const raycast_callback_type& callback{*reinterpret_cast<const raycast_callback_type*>(data)};
        physical_body& body{*reinterpret_cast<physical_body*>(cpShapeGetUserData(shape))};
        const raycast_hit info{body, fromcp(point), fromcp(normal), fromcp(alpha)};

        callback(info);
    };

    cpSpaceSegmentQuery(m_world, tocp(from), tocp(to), thickness, filter, native_callback, &callback);
}

std::optional<physical_world::raycast_hit> physical_world::raycast_first(const glm::vec2& from, const glm::vec2& to, float thickness, std::uint64_t group, std::uint32_t id, std::uint32_t mask)
{
    const cpShapeFilter filter{cpShapeFilterNew(static_cast<cpGroup>(group), id, mask)};

    cpSegmentQueryInfo info{};
    if(cpSpaceSegmentQueryFirst(m_world, tocp(from), tocp(to), thickness, filter, &info))
    {
        physical_body& body{*reinterpret_cast<physical_body*>(cpShapeGetUserData(info.shape))};
        return raycast_hit{body, fromcp(info.point), fromcp(info.normal), fromcp(info.alpha)};
    }

    return std::nullopt;
}

void physical_world::update(float time)
{
    m_time += time;
    const std::uint32_t steps{std::min(static_cast<std::uint32_t>(m_time / m_step), m_max_steps)};

    for(std::uint32_t i{}; i < steps; ++i)
    {
        cpSpaceStep(m_world, m_step);
        m_time -= m_step;
    }
}

void physical_world::set_gravity(const glm::vec2& gravity) noexcept
{
    cpSpaceSetGravity(m_world, tocp(gravity));
}

void physical_world::set_damping(float damping) noexcept
{
    cpSpaceSetDamping(m_world, tocp(damping));
}

void physical_world::set_idle_threshold(float idle_threshold) noexcept
{
    cpSpaceSetIdleSpeedThreshold(m_world, tocp(idle_threshold));
}

void physical_world::set_sleep_threshold(float sleep_threshold) noexcept
{
    cpSpaceSetSleepTimeThreshold(m_world, tocp(sleep_threshold));
}

void physical_world::set_collision_slop(float collision_slop) noexcept
{
    cpSpaceSetCollisionSlop(m_world, tocp(collision_slop));
}

void physical_world::set_collision_bias(float collision_bias) noexcept
{
    cpSpaceSetCollisionBias(m_world, tocp(collision_bias));
}

void physical_world::set_collision_persistence(std::uint32_t collision_persistance) noexcept
{
    cpSpaceSetCollisionPersistence(m_world, static_cast<cpTimestamp>(collision_persistance));
}

void physical_world::set_iteration_count(std::uint32_t count) noexcept
{
    cpSpaceSetIterations(m_world, static_cast<int>(count));
}

glm::vec2 physical_world::gravity() const noexcept
{
    return fromcp(cpSpaceGetGravity(m_world));
}

float physical_world::damping() const noexcept
{
    return fromcp(cpSpaceGetDamping(m_world));
}

float physical_world::idle_threshold() const noexcept
{
    return fromcp(cpSpaceGetIdleSpeedThreshold(m_world));
}

float physical_world::sleep_threshold() const noexcept
{
    return fromcp(cpSpaceGetSleepTimeThreshold(m_world));
}

float physical_world::collision_slop() const noexcept
{
    return fromcp(cpSpaceGetCollisionSlop(m_world));
}

float physical_world::collision_bias() const noexcept
{
    return fromcp(cpSpaceGetCollisionBias(m_world));
}

std::uint32_t physical_world::collision_persistence() const noexcept
{
    return static_cast<std::uint32_t>(cpSpaceGetCollisionPersistence(m_world));
}

void physical_world::add_callback(cpCollisionHandler *cphandler, collision_handler handler)
{
    auto it{m_callbacks.find(cphandler)};
    if(it == std::end(m_callbacks))
    {
        it = m_callbacks.emplace(std::make_pair(cphandler, std::make_unique<collision_handler>(std::move(handler)))).first;
    }
    else
    {
        it->second = std::make_unique<collision_handler>(std::move(handler));
    }

    collision_handler& new_handler{*it->second};
    cphandler->userData = &new_handler;

    if(new_handler.collision_begin)
    {
        cphandler->beginFunc = [](cpArbiter* cparbiter, cpSpace* cpspace, void* userdata) -> cpBool
        {
            cpBody* cpfirst{};
            cpBody* cpsecond{};
            cpArbiterGetBodies(cparbiter, &cpfirst, &cpsecond);

            physical_world& world{*reinterpret_cast<physical_world*>(cpSpaceGetUserData(cpspace))};
            physical_body& first{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpfirst))};
            physical_body& second{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpsecond))};
            physical_collision_arbiter arbiter{cparbiter};
            collision_handler& handler{*reinterpret_cast<collision_handler*>(userdata)};

            return static_cast<cpBool>(handler.collision_begin(world, first, second, std::move(arbiter), handler.userdata));
        };
    }
    else
    {
        cphandler->beginFunc = [](cpArbiter*, cpSpace*, void*) -> cpBool
        {
            return cpTrue;
        };
    }

    if(new_handler.collision_pre_solve)
    {
        cphandler->preSolveFunc = [](cpArbiter* cparbiter, cpSpace* cpspace, void* userdata) -> cpBool
        {
            cpBody* cpfirst{};
            cpBody* cpsecond{};
            cpArbiterGetBodies(cparbiter, &cpfirst, &cpsecond);

            physical_world& world{*reinterpret_cast<physical_world*>(cpSpaceGetUserData(cpspace))};
            physical_body& first{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpfirst))};
            physical_body& second{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpsecond))};
            physical_collision_arbiter arbiter{cparbiter};
            collision_handler& handler{*reinterpret_cast<collision_handler*>(userdata)};

            return static_cast<cpBool>(handler.collision_pre_solve(world, first, second, std::move(arbiter), handler.userdata));
        };
    }
    else
    {
        cphandler->preSolveFunc = [](cpArbiter*, cpSpace*, void*) -> cpBool
        {
            return cpTrue;
        };
    }

    if(new_handler.collision_post_solve)
    {
        cphandler->postSolveFunc = [](cpArbiter* cparbiter, cpSpace* cpspace, void* userdata) -> void
        {
            cpBody* cpfirst{};
            cpBody* cpsecond{};
            cpArbiterGetBodies(cparbiter, &cpfirst, &cpsecond);

            physical_world& world{*reinterpret_cast<physical_world*>(cpSpaceGetUserData(cpspace))};
            physical_body& first{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpfirst))};
            physical_body& second{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpsecond))};
            physical_collision_arbiter arbiter{cparbiter};
            collision_handler& handler{*reinterpret_cast<collision_handler*>(userdata)};

            handler.collision_post_solve(world, first, second, std::move(arbiter), handler.userdata);
        };
    }
    else
    {
        cphandler->postSolveFunc = [](cpArbiter*, cpSpace*, void*) -> void
        {

        };
    }

    if(new_handler.collision_end)
    {
        cphandler->separateFunc = [](cpArbiter* cparbiter, cpSpace* cpspace, void* userdata) -> void
        {
            cpBody* cpfirst{};
            cpBody* cpsecond{};
            cpArbiterGetBodies(cparbiter, &cpfirst, &cpsecond);

            physical_world& world{*reinterpret_cast<physical_world*>(cpSpaceGetUserData(cpspace))};
            physical_body& first{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpfirst))};
            physical_body& second{*reinterpret_cast<physical_body*>(cpBodyGetUserData(cpsecond))};
            physical_collision_arbiter arbiter{cparbiter};
            collision_handler& handler{*reinterpret_cast<collision_handler*>(userdata)};

            handler.collision_end(world, first, second, std::move(arbiter), handler.userdata);
        };
    }
    else
    {
        cphandler->separateFunc = [](cpArbiter*, cpSpace*, void*) -> void
        {

        };
    }
}

physical_shape::physical_shape(physical_body_ptr body, float radius, const glm::vec2& offset)
:m_body{std::move(body)}
,m_shape{cpCircleShapeNew(m_body->handle(), tocp(radius), tocp(offset))}
{
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};

    cpSpaceAddShape(m_body->world()->handle(), m_shape);

    cpSpaceSetUserData(m_body->world()->handle(), this);
    m_body->register_shape(this);
}

physical_shape::physical_shape(physical_body_ptr body, const glm::vec2& first, const glm::vec2& second, float thickness)
:m_body{std::move(body)}
,m_shape{cpSegmentShapeNew(m_body->handle(), tocp(first), tocp(second), tocp(thickness))}
{
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};

    cpSpaceAddShape(m_body->world()->handle(), m_shape);

    cpSpaceSetUserData(m_body->world()->handle(), this);
    m_body->register_shape(this);
}

physical_shape::physical_shape(physical_body_ptr body, const std::vector<glm::vec2>& vertices, float radius)
:m_body{std::move(body)}
{
    std::vector<cpVect> native_vertices{};
    native_vertices.reserve(std::size(vertices));
    for(auto&& vertex : vertices)
        native_vertices.push_back(tocp(vertex));

    m_shape = cpPolyShapeNew(m_body->handle(), static_cast<int>(std::size(vertices)), std::data(native_vertices), cpTransformIdentity, tocp(radius));
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};

    cpSpaceAddShape(m_body->world()->handle(), m_shape);

    cpSpaceSetUserData(m_body->world()->handle(), this);
    m_body->register_shape(this);
}

physical_shape::physical_shape(physical_body_ptr body, float width, float height, float radius)
:m_body{std::move(body)}
,m_shape{cpBoxShapeNew(m_body->handle(), tocp(width), tocp(height), tocp(radius))}
{
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};

    cpSpaceAddShape(m_body->world()->handle(), m_shape);

    cpSpaceSetUserData(m_body->world()->handle(), this);
    m_body->register_shape(this);
}

physical_shape::~physical_shape()
{
    m_body->unregister_shape(this);
    cpSpaceRemoveShape(m_body->world()->handle(), m_shape);
    cpShapeFree(m_shape);
}

void physical_shape::set_sensor(bool enable) noexcept
{
    cpShapeSetSensor(m_shape, static_cast<cpBool>(enable));
}

void physical_shape::set_elasticity(float elasticity) noexcept
{
    cpShapeSetElasticity(m_shape, tocp(elasticity));
}

void physical_shape::set_friction(float friction) noexcept
{
    cpShapeSetFriction(m_shape, tocp(friction));
}

void physical_shape::set_surface_velocity(const glm::vec2& surface_velocity) noexcept
{
    cpShapeSetSurfaceVelocity(m_shape, tocp(surface_velocity));
}

void physical_shape::set_collision_type(std::uint64_t type) noexcept
{
    cpShapeSetCollisionType(m_shape, static_cast<cpCollisionType>(type));
}

void physical_shape::set_filter(std::uint64_t group, std::uint32_t categories, std::uint32_t collision_mask) noexcept
{
    cpShapeSetFilter(m_shape, cpShapeFilterNew(static_cast<cpGroup>(group), categories, collision_mask));
}

bool physical_shape::is_sensor() const noexcept
{
    return static_cast<bool>(cpShapeGetSensor(m_shape));
}

float physical_shape::elasticity() const noexcept
{
    return fromcp(cpShapeGetElasticity(m_shape));
}

float physical_shape::friction() const noexcept
{
    return fromcp(cpShapeGetFriction(m_shape));
}

glm::vec2 physical_shape::surface_velocity() const noexcept
{
    return fromcp(cpShapeGetSurfaceVelocity(m_shape));
}

std::uint64_t physical_shape::collision_type() const noexcept
{
    return static_cast<std::uint64_t>(cpShapeGetCollisionType(m_shape));
}

std::uint64_t physical_shape::group() const noexcept
{
    return static_cast<std::uint64_t>(cpShapeGetFilter(m_shape).group);
}

std::uint32_t physical_shape::categories() const noexcept
{
    return static_cast<std::uint32_t>(cpShapeGetFilter(m_shape).categories);
}

std::uint32_t physical_shape::collision_mask() const noexcept
{
    return static_cast<std::uint32_t>(cpShapeGetFilter(m_shape).mask);
}

physical_body::physical_body(physical_world_ptr world, physical_body_type type, float mass, float inertia)
:m_world{std::move(world)}
{
    if(type == physical_body_type::dynamic)
    {
        m_body = cpBodyNew(tocp(mass), tocp(inertia));
    }
    else if(type == physical_body_type::steady)
    {
        m_body = cpBodyNewStatic();
    }
    else if(type == physical_body_type::kinematic)
    {
        m_body = cpBodyNewKinematic();
    }

    if(!m_body)
        throw std::runtime_error{"Can not allocate physical body."};

    cpBodySetUserData(m_body, this);
    cpSpaceAddBody(m_world->handle(), m_body);
}

physical_body::~physical_body()
{
    cpSpaceRemoveBody(m_world->handle(), m_body);
    cpBodyFree(m_body);
}

void physical_body::apply_force(const glm::vec2& force, const glm::vec2& point) noexcept
{
    cpBodyApplyForceAtWorldPoint(m_body, tocp(force), tocp(point));
}

void physical_body::apply_local_force(const glm::vec2& force, const glm::vec2& point) noexcept
{
    cpBodyApplyForceAtLocalPoint(m_body, tocp(force), tocp(point));
}

void physical_body::apply_impulse(const glm::vec2& impulse, const glm::vec2& point) noexcept
{
    cpBodyApplyImpulseAtWorldPoint(m_body, tocp(impulse), tocp(point));
}

void physical_body::apply_local_impulse(const glm::vec2& impulse, const glm::vec2& point) noexcept
{
    cpBodyApplyImpulseAtLocalPoint(m_body, tocp(impulse), tocp(point));
}

void physical_body::add_torque(float torque) noexcept
{
    cpBodySetTorque(m_body, cpBodyGetTorque(m_body) + tocp(torque));
}

void physical_body::set_angular_velocity(float velocity) noexcept
{
    cpBodySetAngularVelocity(m_body, tocp(velocity));
}

void physical_body::set_mass(float mass) noexcept
{
    cpBodySetMass(m_body, tocp(mass));
}

void physical_body::set_mass_center(const glm::vec2& center) noexcept
{
    cpBodySetCenterOfGravity(m_body, tocp(center));
}

void physical_body::set_moment_of_inertia(float moment) noexcept
{
    cpBodySetMoment(m_body, tocp(moment));
}

void physical_body::set_position(const glm::vec2& position) noexcept
{
    cpBodySetPosition(m_body, tocp(position));
    cpSpaceReindexShapesForBody(m_world->handle(), m_body);
}

void physical_body::set_rotation(float rotation) noexcept
{
    cpBodySetAngle(m_body, tocp(rotation));
}

void physical_body::set_velocity(const glm::vec2& velocity) noexcept
{
    cpBodySetVelocity(m_body, tocp(velocity));
}

void physical_body::sleep() noexcept
{
    cpBodySleep(m_body);
}

void physical_body::wake_up() noexcept
{
    cpBodyActivate(m_body);
}

glm::vec2 physical_body::world_to_local(const glm::vec2& vec) noexcept
{
    return fromcp(cpBodyWorldToLocal(m_body, tocp(vec)));
}

glm::vec2 physical_body::local_to_world(const glm::vec2& vec) noexcept
{
    return fromcp(cpBodyLocalToWorld(m_body, tocp(vec)));
}

cpt::bounding_box physical_body::bounding_box() const noexcept
{
    if(std::empty(m_shapes))
        return cpt::bounding_box{};

    cpBB bb{};
    for(auto shape : m_shapes)
        bb = cpBBMerge(bb, cpShapeGetBB(shape->handle()));

    return cpt::bounding_box{fromcp(bb.l), fromcp(bb.b), fromcp(bb.r - bb.l), fromcp(bb.t - bb.b)};
}

float physical_body::angular_velocity() const noexcept
{
    return fromcp(cpBodyGetAngularVelocity(m_body));
}

float physical_body::mass() const noexcept
{
    return fromcp(cpBodyGetMass(m_body));
}

glm::vec2 physical_body::mass_center() const noexcept
{
    return fromcp(cpBodyGetCenterOfGravity(m_body));
}

float physical_body::moment_of_inertia() const noexcept
{
    return fromcp(cpBodyGetMoment(m_body));
}

glm::vec2 physical_body::position() const noexcept
{
    return fromcp(cpBodyGetPosition(m_body));
}

float physical_body::rotation() const noexcept
{
    return fromcp(cpBodyGetAngle(m_body));
}

glm::vec2 physical_body::velocity() const noexcept
{
    return fromcp(cpBodyGetVelocity(m_body));
}

bool physical_body::sleeping() const noexcept
{
    return static_cast<bool>(cpBodyIsSleeping(m_body));
}

physical_body_type physical_body::type() const noexcept
{
    switch(cpBodyGetType(m_body))
    {
        case CP_BODY_TYPE_DYNAMIC: return physical_body_type::dynamic;
        case CP_BODY_TYPE_STATIC: return physical_body_type::steady;
        case CP_BODY_TYPE_KINEMATIC: return physical_body_type::kinematic;
    }

    std::terminate();
}

physical_constraint::physical_constraint(pin_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, const glm::vec2& first_anchor, const glm::vec2& second_anchor)
:m_constaint{cpPinJointNew(first->handle(), second->handle(), tocp(first_anchor), tocp(second_anchor))}
,m_type{physical_constraint_type::pin_joint}
{
    if(!m_constaint)
        throw std::runtime_error{"Can not create physical constaint."};
}

physical_constraint::physical_constraint(slide_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, const glm::vec2& first_anchor, const glm::vec2& second_anchor, float min, float max)
:m_constaint{cpSlideJointNew(first->handle(), second->handle(), tocp(first_anchor), tocp(second_anchor), fromcp(min), fromcp(max))}
,m_type{physical_constraint_type::pin_joint}
{

}

physical_constraint::physical_constraint(pivot_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, const glm::vec2& pivot)
{

}

physical_constraint::physical_constraint(pivot_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, const glm::vec2& first_anchor, const glm::vec2& second_anchor)
{

}

physical_constraint::physical_constraint(groove_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, const glm::vec2& first_groove, const glm::vec2& second_groove, const glm::vec2& anchor)
{

}

physical_constraint::physical_constraint(damped_spring_t, const physical_body_ptr& first, const physical_body_ptr& second, const glm::vec2& first_anchor, const glm::vec2& second_anchor, float rest_length, float stiffness, float damping)
{

}

physical_constraint::physical_constraint(damped_rotary_spring_t, const physical_body_ptr& first, const physical_body_ptr& second, float rest_angle, float stiffness, float damping)
{

}

physical_constraint::physical_constraint(rotary_limit_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, float min, float max)
{

}

physical_constraint::physical_constraint(ratchet_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, float phase, float ratchet)
{

}

physical_constraint::physical_constraint(gear_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, float phase, float ratio)
{

}

physical_constraint::physical_constraint(motor_joint_t, const physical_body_ptr& first, const physical_body_ptr& second, float rate)
{

}

physical_constraint::~physical_constraint()
{

}


}
