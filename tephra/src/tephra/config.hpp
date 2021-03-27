#ifndef TEPHRA_CONFIG_HPP_INCLUDED
#define TEPHRA_CONFIG_HPP_INCLUDED

#include <captal_foundation/base.hpp>
#include <captal_foundation/version.hpp>
#include <captal_foundation/enum_operations.hpp>
#include <captal_foundation/optional_ref.hpp>

#ifdef _WIN32
    #if defined(TEPHRA_SHARED_BUILD)
        #define TEPHRA_API __declspec(dllexport)
    #elif !defined(TEPHRA_STATIC_BUILD)
        #define TEPHRA_API __declspec(dllimport)
    #else
        #define TEPHRA_API
    #endif
#else
    #define TEPHRA_API
#endif

namespace tph
{

using namespace cpt::foundation;

template<typename VulkanObject, typename... Args>
VulkanObject underlying_cast(const Args&...) noexcept
{
    //Declare a compatible type:
    //    template<typename VulkanObject, typename... Args>
    //    friend VulkanObject underlying_cast(const Args&...) noexcept;

    static_assert(!std::is_same<VulkanObject, VulkanObject>::value,
        "tph::underlying_cast called with incompatible arguments. (A common mistake is calling a valid underlying_cast, but without including the header where it is declared.)");
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
