#ifndef TEPHRA_CONFIG_HPP_INCLUDED
#define TEPHRA_CONFIG_HPP_INCLUDED

#include <captal_foundation/config.hpp>

namespace tph
{

using namespace cpt::foundation;

template<typename VulkanObject, typename... Args>
VulkanObject underlying_cast(const Args&...) noexcept
{
    //Declare a compatible type:
    //    template<typename VulkanObject, typename... Args>
    //    friend VulkanObject underlying_cast(const Args&...) noexcept;

    static_assert(!std::is_same<VulkanObject, VulkanObject>::value, "tph::underlying_cast called with incompatible arguments.");
}

using bool32_t = std::uint32_t;

struct viewport
{
    float x{};
    float y{};
    float width{};
    float height{};
    float min_depth{};
    float max_depth{};
};

struct scissor
{
    std::int32_t x{};
    std::int32_t y{};
    std::uint32_t width{};
    std::uint32_t height{};
};

}

#endif
