#ifndef CAPTAL_COMPONENTS_CONTROLLER_HPP_INCLUDED
#define CAPTAL_COMPONENTS_CONTROLLER_HPP_INCLUDED

#include "../config.hpp"

#include <vector>

#include "../physics.hpp"

namespace cpt
{

namespace components
{

struct controller : private std::vector<physical_constraint>
{
    using parent_type = std::vector<physical_constraint>;

public:
    using value_type = parent_type::value_type;
    using size_type = parent_type::size_type;
    using difference_type = parent_type::difference_type;
    using pointer = parent_type::pointer;
    using const_pointer = parent_type::const_pointer;
    using reference = parent_type::reference;
    using const_reference = parent_type::const_reference;
    using iterator = parent_type::iterator;
    using const_iterator = parent_type::const_iterator;
    using reverse_iterator = parent_type::reverse_iterator;
    using const_reverse_iterator = parent_type::const_reverse_iterator;

public:
    controller() = default;

    explicit controller(cpt::physical_world& world)
    :m_attachment{std::in_place, world, physical_body_type::kinematic}
    {

    }

    ~controller() = default;
    controller(const controller&) = delete;
    controller& operator=(const controller&) = delete;
    controller(controller&&) noexcept = default;
    controller& operator=(controller&&) noexcept = default;

    void attach(cpt::physical_world& world)
    {
        clear();

        m_attachment.emplace(world, physical_body_type::kinematic);
    }

    template<typename DisambiguationTag, typename... Args>
    reference attach_constraint(DisambiguationTag tag, physical_body& body, Args&&... args)
    {
        return emplace_back(tag, m_attachment.value(), body, std::forward<Args>(args)...);
    }

    void detach()
    {
        clear();
        m_attachment.reset();
    }

    void detach_constraint(const_iterator position)
    {
        erase(position);
    }

    void detach_constraints() noexcept
    {
        clear();
    }

    physical_body& attachment() noexcept
    {
        return m_attachment.value();
    }

    const physical_body& attachment() const noexcept
    {
        return m_attachment.value();
    }

    bool has_attachment() const noexcept
    {
        return m_attachment.has_value();
    }

    void swap(controller& other) noexcept
    {
        parent_type::swap(other);
        m_attachment.swap(other.m_attachment);
    }

    explicit operator bool() const noexcept
    {
        return m_attachment.has_value();
    }

    physical_body& operator*() noexcept
    {
        return m_attachment.value();
    }

    const physical_body& operator*() const noexcept
    {
        return m_attachment.value();
    }

    physical_body* operator->() noexcept
    {
        return &m_attachment.value();
    }

    const physical_body* operator->() const noexcept
    {
        return &m_attachment.value();
    }

    using parent_type::operator[];
    using parent_type::front;
    using parent_type::back;
    using parent_type::data;

    using parent_type::empty;
    using parent_type::size;
    using parent_type::max_size;
    using parent_type::reserve;
    using parent_type::capacity;
    using parent_type::shrink_to_fit;

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;

private:
    std::optional<cpt::physical_body> m_attachment{};
};

}

}

#endif
