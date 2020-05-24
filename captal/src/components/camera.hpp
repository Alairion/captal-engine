#ifndef CAPTAL_COMPONENTS_CAMERA_HPP_INCLUDED
#define CAPTAL_COMPONENTS_CAMERA_HPP_INCLUDED

#include "../config.hpp"

#include "../view.hpp"

namespace cpt
{

namespace components
{

class camera
{
public:
    using value_type = view;

public:
    camera() = default;

    camera(view_ptr attachment)
    :m_attachment{std::move(attachment)}
    {

    }

    ~camera() = default;
    camera(const camera&) = default;
    camera& operator=(const camera&) = default;
    camera(camera&&) noexcept = default;
    camera& operator=(camera&&) noexcept = default;

    void attach(view_ptr attachment) noexcept
    {
        m_attachment = std::move(attachment);
    }

    void detach() noexcept
    {
        m_attachment = nullptr;
    }

    const view_ptr& attachment() const noexcept
    {
        return m_attachment;
    }

    view* operator->() const noexcept
    {
        return m_attachment.get();
    }

private:
    view_ptr m_attachment{};
};

}

}

#endif
