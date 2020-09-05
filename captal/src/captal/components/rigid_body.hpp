#ifndef CAPTAL_COMPONENTS_RIGID_BODY_HPP_INCLUDED
#define CAPTAL_COMPONENTS_RIGID_BODY_HPP_INCLUDED

#include "../config.hpp"

#include <vector>

#include "../physics.hpp"

#include "attachment.hpp"

namespace cpt::components
{

class rigid_body : public impl::basic_attachement<cpt::physical_body>
{
public:
    using iterator = std::vector<physical_shape>::iterator;
    using const_iterator = std::vector<physical_shape>::const_iterator;
    using reverse_iterator = std::vector<physical_shape>::reverse_iterator;
    using const_reverse_iterator = std::vector<physical_shape>::const_reverse_iterator;

public:
    template<typename... Args> requires std::constructible_from<physical_shape, Args...>
    physical_shape& attach(Args&&... args)
    {
        return m_shapes.emplace_back(attachment(), std::forward<Args>(args)...);
    }

    void detach(const_iterator position)
    {
        m_shapes.erase(position);
    }

    void reserve(std::size_t count)
    {
        m_shapes.reserve(count);
    }

    void shrink_to_fit()
    {
        m_shapes.shrink_to_fit();
    }

    std::size_t capacity() const noexcept
    {
        return m_shapes.capacity();
    }

    std::size_t size() const noexcept
    {
        return std::size(m_shapes);
    }

    bool empty() const noexcept
    {
        return std::empty(m_shapes);
    }

    physical_shape& operator[](std::size_t index) noexcept
    {
        return m_shapes[index];
    }

    const physical_shape& operator[](std::size_t index) const noexcept
    {
        return m_shapes[index];
    }

    iterator               begin()         noexcept {return std::begin(m_shapes);}
    const_iterator         begin()   const noexcept {return std::begin(m_shapes);}
    const_iterator         cbegin()  const noexcept {return std::cbegin(m_shapes);}
    iterator               end()           noexcept {return std::end(m_shapes);}
    const_iterator         end()     const noexcept {return std::end(m_shapes);}
    const_iterator         cend()    const noexcept {return std::cend(m_shapes);}
    reverse_iterator       rbegin()        noexcept {return std::rbegin(m_shapes);}
    const_reverse_iterator rbegin()  const noexcept {return std::rbegin(m_shapes);}
    const_reverse_iterator crbegin() const noexcept {return std::crbegin(m_shapes);}
    reverse_iterator       rend()          noexcept {return std::rend(m_shapes);}
    const_reverse_iterator rend()    const noexcept {return std::rend(m_shapes);}
    const_reverse_iterator crend()   const noexcept {return std::crend(m_shapes);}

private:
    std::vector<physical_shape> m_shapes{};
};

}

#endif
