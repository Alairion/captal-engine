#ifndef CAPTAL_DRAWABLE_HPP_INCLUDED
#define CAPTAL_DRAWABLE_HPP_INCLUDED

#include "../config.hpp"

#include "../renderable.hpp"

namespace cpt
{

namespace components
{

class CAPTAL_API drawable
{
public:
    using value_type = renderable;

public:
    drawable() = default;

    explicit drawable(renderable_ptr attachment)
    :m_attachment{std::move(attachment)}
    {

    }

    ~drawable() = default;
    drawable(const drawable&) = default;
    drawable& operator=(const drawable&) = default;
    drawable(drawable&&) noexcept = default;
    drawable& operator=(drawable&&) noexcept = default;

    void attach(renderable_ptr attachment) noexcept
    {
        m_attachment = std::move(attachment);
    }

    void detach() noexcept
    {
        m_attachment = nullptr;
    }

    const renderable_ptr& attachment() const noexcept
    {
        return m_attachment;
    }

    renderable* operator->() const noexcept
    {
        return m_attachment.get();
    }

private:
    renderable_ptr m_attachment{};
};

}

}

#endif
