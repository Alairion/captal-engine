#ifndef CAPTAL_PHYSICS_HPP_INCLUDED
#define CAPTAL_PHYSICS_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <variant>
#include <span>

#include <captal_foundation/math.hpp>

struct cpSpace;
struct cpBody;
struct cpShape;
struct cpArbiter;
struct cpCollisionHandler;
struct cpConstraint;

namespace cpt
{

class physical_body;
class physical_shape;

using collision_type_t = std::uint64_t;
using group_t = std::uint64_t;
using collision_id_t = std::uint64_t;

inline constexpr group_t no_group{};
inline constexpr group_t all_groups{std::numeric_limits<group_t>::max()};
inline constexpr collision_id_t no_collision_id{};
inline constexpr collision_id_t all_collision_ids{std::numeric_limits<collision_id_t>::max()};

class CAPTAL_API physical_collision_arbiter
{
public:
    explicit physical_collision_arbiter(cpArbiter* arbiter) noexcept
    :m_arbiter{arbiter}
    {

    }

    ~physical_collision_arbiter() = default;
    physical_collision_arbiter(const physical_collision_arbiter&) = delete;
    physical_collision_arbiter& operator=(const physical_collision_arbiter&) = delete;
    physical_collision_arbiter(physical_collision_arbiter&&) noexcept = default;
    physical_collision_arbiter& operator=(physical_collision_arbiter&&) noexcept = default;

    void set_restitution(float restitution) noexcept;
    void set_friction(float friction) noexcept;
    void set_surface_velocity(vec2f surface_velocity) noexcept;
    void set_user_data(void* user_data) noexcept;

    std::pair<physical_shape&, physical_shape&> shapes() const noexcept;
    std::pair<physical_body&, physical_body&> bodies() const noexcept;
    vec2f normal() const noexcept;

    std::size_t contact_count() const noexcept;
    std::pair<vec2f, vec2f> points(std::size_t contact_index) const noexcept;
    float depth(std::size_t contact_index) const noexcept;

    bool is_first_contact() const noexcept;
    bool is_removal() const noexcept;

    cpArbiter* handle() noexcept
    {
        return m_arbiter;
    }

    const cpArbiter* handle() const noexcept
    {
        return m_arbiter;
    }

private:
    cpArbiter* m_arbiter{};
};

class CAPTAL_API physical_world
{
public:
    using collision_begin_callback_type      = std::function<bool(physical_world& world, physical_body& first, physical_body& second, physical_collision_arbiter arbiter, void* userdata)>;
    using collision_pre_solve_callback_type  = std::function<bool(physical_world& world, physical_body& first, physical_body& second, physical_collision_arbiter arbiter, void* userdata)>;
    using collision_post_solve_callback_type = std::function<void(physical_world& world, physical_body& first, physical_body& second, physical_collision_arbiter arbiter, void* userdata)>;
    using collision_end_callback_type        = std::function<void(physical_world& world, physical_body& first, physical_body& second, physical_collision_arbiter arbiter, void* userdata)>;

public:
    struct collision_handler
    {
        collision_begin_callback_type collision_begin{};
        collision_pre_solve_callback_type collision_pre_solve{};
        collision_post_solve_callback_type collision_post_solve{};
        collision_end_callback_type collision_end{};
        void* userdata{};
    };

public:
    struct point_hit
    {
        physical_shape& shape;
        vec2f position{};
        float distance{};
        vec2f gradient{};
    };

    struct region_hit
    {
        physical_shape& shape;
    };

    struct ray_hit
    {
        physical_shape& shape;
        vec2f position{};
        vec2f normal{};
        float distance{};
    };

public:
    using point_query_callback_type = std::function<void(point_hit hit)>;
    using region_query_callback_type = std::function<void(region_hit hit)>;
    using ray_callback_type = std::function<void(ray_hit hit)>;

public:
    physical_world();
    ~physical_world();
    physical_world(const physical_world&) = delete;
    physical_world& operator=(const physical_world&) = delete;
    physical_world(physical_world&&) noexcept;
    physical_world& operator=(physical_world&&) noexcept;

    void add_collision(collision_type_t first_type, collision_type_t second_type, collision_handler handler);
    void add_wildcard(collision_type_t type, collision_handler handler);

    void point_query(vec2f point, float max_distance, group_t group, collision_id_t id, collision_id_t mask, point_query_callback_type callback);
    void region_query(float x, float y, float width, float height, group_t group, collision_id_t id, collision_id_t mask, region_query_callback_type callback);
    void ray_query(vec2f from, vec2f to, float thickness, group_t group, collision_id_t id, collision_id_t mask, ray_callback_type callback);

    std::vector<point_hit> point_query(vec2f point, float max_distance, group_t group, collision_id_t id, collision_id_t mask);
    std::vector<region_hit> region_query(float x, float y, float width, float height, group_t group, collision_id_t id, collision_id_t mask);
    std::vector<ray_hit> ray_query(vec2f from, vec2f to, float thickness, group_t group, collision_id_t id, collision_id_t mask);

    std::optional<point_hit> point_query_nearest(vec2f point, float max_distance, group_t group, collision_id_t id, collision_id_t mask);
    std::optional<ray_hit> ray_query_first(vec2f from, vec2f to, float thickness, group_t group, collision_id_t id, collision_id_t mask);

    void update(float time);

    void set_gravity(vec2f gravity) noexcept;
    void set_damping(float damping) noexcept;
    void set_idle_threshold(float idle_threshold) noexcept;
    void set_sleep_threshold(float speed_threshold) noexcept;
    void set_collision_slop(float collision_slop) noexcept;
    void set_collision_bias(float collision_bias) noexcept;
    void set_collision_persistence(std::uint64_t collision_persistance) noexcept;
    void set_iteration_count(std::uint32_t count) noexcept;

    void set_step(float step) noexcept
    {
        m_step = step;
    }

    void set_max_steps(std::uint32_t max_steps) noexcept
    {
        m_max_steps = max_steps;
    }

    vec2f gravity() const noexcept;
    float damping() const noexcept;
    float idle_threshold() const noexcept;
    float sleep_threshold() const noexcept;
    float collision_slop() const noexcept;
    float collision_bias() const noexcept;
    std::uint64_t collision_persistence() const noexcept;

    float step() const noexcept
    {
        return m_step;
    }

    std::uint32_t max_steps() const noexcept
    {
        return m_max_steps;
    }

    cpSpace* handle() noexcept
    {
        return m_world;
    }

    const cpSpace* handle() const noexcept
    {
        return m_world;
    }

private:
    void add_callback(cpCollisionHandler* cphandler, collision_handler handler);

private:
    cpSpace* m_world{};
    std::unordered_map<cpCollisionHandler*, std::unique_ptr<collision_handler>> m_callbacks{};
    float m_step{0.001f};
    std::uint32_t m_max_steps{std::numeric_limits<std::uint32_t>::max()};
    float m_time{};
};

struct bounding_box
{
    vec2f top_left{};
    vec2f bottom_right{};
};

class CAPTAL_API physical_shape
{
public:
    physical_shape() = default;
    explicit physical_shape(physical_body& body, float radius, vec2f offset = vec2f{});
    explicit physical_shape(physical_body& body, vec2f first, vec2f second, float thickness = 0.0f);
    explicit physical_shape(physical_body& body, std::span<const vec2f> points, float radius = 0.0f);
    explicit physical_shape(physical_body& body, float width, float height, float radius = 0.0f);

    ~physical_shape();
    physical_shape(const physical_shape&) = delete;
    physical_shape& operator=(const physical_shape&) = delete;
    physical_shape(physical_shape&&) noexcept;
    physical_shape& operator=(physical_shape&&) noexcept;

    void set_sensor(bool enable) noexcept;
    void set_elasticity(float elasticity) noexcept;
    void set_friction(float friction) noexcept;
    void set_surface_velocity(vec2f surface_velocity) noexcept;
    void set_collision_type(collision_type_t type) noexcept;
    void set_filter(group_t group, collision_id_t categories, collision_id_t collision_mask) noexcept;

    void set_user_data(void* userdata) noexcept
    {
        m_userdata = userdata;
    }

    void deactivate() noexcept;

    physical_world& world() const noexcept;
    physical_body& body() const noexcept;
    bool is_sensor() const noexcept;
    float elasticity() const noexcept;
    float friction() const noexcept;
    vec2f surface_velocity() const noexcept;
    collision_type_t collision_type() const noexcept;
    group_t group() const noexcept;
    collision_id_t categories() const noexcept;
    collision_id_t collision_mask() const noexcept;
    cpt::bounding_box bounding_box() const noexcept;

    cpShape* handle() noexcept
    {
        return m_shape;
    }

    const cpShape* handle() const noexcept
    {
        return m_shape;
    }

    void* user_data() const noexcept
    {
        return m_userdata;
    }

    bool active() const noexcept
    {
        return m_active;
    }

private:
    cpShape* m_shape{};
    void* m_userdata{};
    bool m_active{};
};

enum class physical_body_type : std::uint32_t
{
    dynamic = 0,
    steady = 1,
    kinematic = 2
};

CAPTAL_API float circle_moment(float mass, float radius, vec2f origin = vec2f{}, float inner_radius = 0.0f) noexcept;
CAPTAL_API float segment_moment(float mass, vec2f first, vec2f second, float thickness = 0.0f) noexcept;
CAPTAL_API float polygon_moment(float mass, std::span<const vec2f> points, vec2f offset = vec2f{}, float radius = 0.0f) noexcept;
CAPTAL_API float square_moment(float mass, float width, float height) noexcept;

inline constexpr float no_rotation{std::numeric_limits<float>::infinity()};

class CAPTAL_API physical_body
{
    friend class physical_shape;

public:
    physical_body() = default;
    explicit physical_body(physical_world& world, physical_body_type type, float mass = 1.0f, float moment = no_rotation);

    ~physical_body();
    physical_body(const physical_body&) = delete;
    physical_body& operator=(const physical_body&) = delete;
    physical_body(physical_body&&) noexcept;
    physical_body& operator=(physical_body&&) noexcept;

    void apply_force(vec2f force, vec2f point) noexcept;
    void apply_local_force(vec2f force, vec2f point) noexcept;
    void apply_impulse(vec2f impulse, vec2f point) noexcept;
    void apply_local_impulse(vec2f impulse, vec2f point) noexcept;
    void add_torque(float torque) noexcept;

    void set_angular_velocity(float velocity) noexcept;
    void set_mass(float mass) noexcept;
    void set_mass_center(vec2f center) noexcept;
    void set_moment_of_inertia(float moment) noexcept;
    void set_position(vec2f position) noexcept;
    void set_rotation(float rotation) noexcept;
    void set_velocity(vec2f velocity) noexcept;

    void sleep() noexcept;
    void wake_up() noexcept;

    vec2f world_to_local(vec2f vec) noexcept;
    vec2f local_to_world(vec2f vec) noexcept;

    void set_user_data(void* userdata) noexcept
    {
        m_userdata = userdata;
    }

    physical_world& world() const noexcept;
    float angular_damping() const noexcept;
    float angular_velocity() const noexcept;
    float mass() const noexcept;
    vec2f mass_center() const noexcept;
    float moment_of_inertia() const noexcept;
    vec2f position() const noexcept;
    float rotation() const noexcept;
    vec2f velocity() const noexcept;
    bool sleeping() const noexcept;
    physical_body_type type() const noexcept;

    void* user_data() const noexcept
    {
        return m_userdata;
    }

    cpBody* handle() noexcept
    {
        return m_body;
    }

    const cpBody* handle() const noexcept
    {
        return m_body;
    }

private:
    void unregister() noexcept;

private:
    cpBody* m_body{};
    void* m_userdata{};
};

enum class physical_constraint_type : std::uint32_t
{
    pin_joint = 0,
    slide_joint = 1,
    pivot_joint = 2,
    groove_joint = 3,
    damped_spring = 4,
    damped_rotary_spring = 5,
    rotary_limit_joint = 6,
    ratchet_joint = 7,
    gear_joint = 8,
    motor_joint = 9
};

struct pin_joint_t{};
inline constexpr pin_joint_t pin_joint{};
struct slide_joint_t{};
inline constexpr slide_joint_t slide_joint{};
struct pivot_joint_t{};
inline constexpr pivot_joint_t pivot_joint{};
struct groove_joint_t{};
inline constexpr groove_joint_t groove_joint{};
struct damped_spring_t{};
inline constexpr damped_spring_t damped_spring{};
struct damped_rotary_spring_t{};
inline constexpr damped_rotary_spring_t damped_rotary_spring{};
struct rotary_limit_joint_t{};
inline constexpr rotary_limit_joint_t rotary_limit_joint{};
struct ratchet_joint_t{};
inline constexpr ratchet_joint_t ratchet_joint{};
struct gear_joint_t{};
inline constexpr gear_joint_t gear_joint{};
struct motor_joint_t{};
inline constexpr motor_joint_t motor_joint{};

class CAPTAL_API physical_constraint
{
public:
    physical_constraint() = default;
    explicit physical_constraint(pin_joint_t, physical_body& first, physical_body& second, vec2f first_anchor, vec2f second_anchor);
    explicit physical_constraint(slide_joint_t, physical_body& first, physical_body& second, vec2f first_anchor, vec2f second_anchor, float min, float max);
    explicit physical_constraint(pivot_joint_t, physical_body& first, physical_body& second, vec2f pivot);
    explicit physical_constraint(pivot_joint_t, physical_body& first, physical_body& second, vec2f first_anchor, vec2f second_anchor);
    explicit physical_constraint(groove_joint_t, physical_body& first, physical_body& second, vec2f first_groove, vec2f second_groove, vec2f anchor);
    explicit physical_constraint(damped_spring_t, physical_body& first, physical_body& second, vec2f first_anchor, vec2f second_anchor, float rest_length, float stiffness, float damping);
    explicit physical_constraint(damped_rotary_spring_t, physical_body& first, physical_body& second, float rest_angle, float stiffness, float damping);
    explicit physical_constraint(rotary_limit_joint_t, physical_body& first, physical_body& second, float min, float max);
    explicit physical_constraint(ratchet_joint_t, physical_body& first, physical_body& second, float phase, float ratchet);
    explicit physical_constraint(gear_joint_t, physical_body& first, physical_body& second, float phase, float ratio);
    explicit physical_constraint(motor_joint_t, physical_body& first, physical_body& second, float rate);

    ~physical_constraint();
    physical_constraint(const physical_constraint&) = delete;
    physical_constraint& operator=(const physical_constraint&) = delete;
    physical_constraint(physical_constraint&&) noexcept;
    physical_constraint& operator=(physical_constraint&&) noexcept;

    void set_max_force(float force) noexcept;
    void set_error_bias(float bias) noexcept;
    void set_max_bias(float bias) noexcept;
    void set_collide_bodies(bool enable) noexcept;

    void set_user_data(void* userdata) noexcept
    {
        m_userdata = userdata;
    }

    void deactivate() noexcept;

    physical_world& world() const noexcept;
    physical_body& first_body() const noexcept;
    physical_body& second_body() const noexcept;
    std::pair<physical_body&, physical_body&> bodies() const noexcept;
    float max_force() const noexcept;
    float error_bias() const noexcept;
    float max_bias() const noexcept;
    bool collide_bodies() const noexcept;

    cpConstraint* handle() noexcept
    {
        return m_constraint;
    }

    const cpConstraint* handle() const noexcept
    {
        return m_constraint;
    }

    physical_constraint_type type() const noexcept
    {
        return m_type;
    }

    void* user_data() const noexcept
    {
        return m_userdata;
    }

    bool active() const noexcept
    {
        return m_active;
    }

public:
    void set_pin_joint_first_anchor(vec2f anchor) noexcept;
    void set_pin_joint_second_anchor(vec2f anchor) noexcept;
    void set_pin_joint_distance(float distance) noexcept;
    vec2f pin_joint_first_anchor() const noexcept;
    vec2f pin_joint_second_anchor() const noexcept;
    float pin_joint_distance() const noexcept;

public:
    void set_slide_joint_first_anchor(vec2f anchor) noexcept;
    void set_slide_joint_second_anchor(vec2f anchor) noexcept;
    void set_slide_joint_min(float min) noexcept;
    void set_slide_joint_max(float max) noexcept;
    vec2f slide_joint_first_anchor() const noexcept;
    vec2f slide_joint_second_anchor() const noexcept;
    float slide_joint_min() const noexcept;
    float slide_joint_max() const noexcept;

public:
    void set_pivot_joint_first_anchor(vec2f anchor) noexcept;
    void set_pivot_joint_second_anchor(vec2f anchor) noexcept;
    vec2f pivot_joint_first_anchor() const noexcept;
    vec2f pivot_joint_second_anchor() const noexcept;

public:
    void set_groove_joint_first_groove(vec2f groove) noexcept;
    void set_groove_joint_second_groove(vec2f groove) noexcept;
    void set_groove_joint_anchor(vec2f anchor) noexcept;
    vec2f groove_joint_first_groove() const noexcept;
    vec2f groove_joint_second_groove() const noexcept;
    vec2f groove_joint_anchor() const noexcept;

public:
    void set_damped_spring_first_anchor(vec2f anchor) noexcept;
    void set_damped_spring_second_anchor(vec2f anchor) noexcept;
    void set_damped_spring_rest_length(float rest_length) noexcept;
    void set_damped_spring_stiffness(float stiffness) noexcept;
    void set_damped_spring_damping(float damping) noexcept;
    vec2f damped_spring_first_anchor() const noexcept;
    vec2f damped_spring_second_anchor() const noexcept;
    float damped_spring_rest_length() const noexcept;
    float damped_spring_stiffness() const noexcept;
    float damped_spring_damping() const noexcept;

public:
    void set_damped_rotary_spring_rest_angle(float rest_angle) noexcept;
    void set_damped_rotary_spring_stiffness(float stiffness) noexcept;
    void set_damped_rotary_spring_damping(float damping) noexcept;
    float damped_rotary_spring_rest_angle() const noexcept;
    float damped_rotary_spring_stiffness() const noexcept;
    float damped_rotary_spring_damping() const noexcept;

public:
    void set_rotary_limit_joint_min(float min) noexcept;
    void set_rotary_limit_joint_max(float max) noexcept;
    float rotary_limit_joint_min() const noexcept;
    float rotary_limit_joint_max() const noexcept;

public:
    void set_ratchet_joint_angle(float angle) noexcept;
    void set_ratchet_joint_phase(float phase) noexcept;
    void set_ratchet_joint_ratchet(float ratchet) noexcept;
    float ratchet_joint_angle() const noexcept;
    float ratchet_joint_phase() const noexcept;
    float ratchet_joint_ratchet() const noexcept;

public:
    void set_gear_joint_phase(float phase) noexcept;
    void set_gear_joint_ratio(float ratio) noexcept;
    float gear_joint_phase() const noexcept;
    float gear_joint_ratio() const noexcept;

public:
    void set_motor_joint_rate(float rate) noexcept;
    float motor_joint_rate() const noexcept;

private:
    cpConstraint* m_constraint{};
    physical_constraint_type m_type{};
    void* m_userdata{};
    bool m_active{};
};

}

#endif
