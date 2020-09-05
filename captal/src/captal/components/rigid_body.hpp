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
    template<typename... Args>
    physical_shape& attach_shape(Args&&... args)
    {
        return m_shapes.emplace_back(attachment(), std::forward<Args>(args)...);
    }

    void detach(std::size_t index)
    {
        m_shapes.erase(std::begin(m_shapes) + index);
    }

    physical_shape& shape(std::size_t index) noexcept
    {
        return m_shapes[index];
    }

    const physical_shape& shape(std::size_t index) const noexcept
    {
        return m_shapes[index];
    }

private:
    std::vector<physical_shape> m_shapes{};
};

}

#endif
