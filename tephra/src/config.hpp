#ifndef TEPHRA_CONFIG_HPP_INCLUDED
#define TEPHRA_CONFIG_HPP_INCLUDED

#include <cstdint>
#include <type_traits>
#include <cstring>

namespace tph
{

/*
Declare a compatible type:

template<typename VulkanObject, typename... Args>
friend VulkanObject underlying_cast(const Args&...) noexcept;
*/

template<typename VulkanObject, typename... Args>
VulkanObject underlying_cast(const Args&...) noexcept
{
    static_assert(!std::is_same<VulkanObject, VulkanObject>::value, "tph::underlying_cast called with incompatible arguments.");
}

struct alignas(std::uint64_t) version
{
    std::uint16_t major; //No default initializers because version must be trivial
    std::uint16_t minor;
    std::uint32_t patch;
};

inline std::uint64_t from_version(version value) noexcept
{
    std::uint64_t output{};
    std::memcpy(&output, &value, sizeof(version));

    return output;
}

inline version to_version(std::uint64_t value) noexcept
{
    version output{};
    std::memcpy(&output, &value, sizeof(std::uint64_t));

    return output;
}

constexpr bool operator==(version left, version right) noexcept
{
    return left.major == right.major && left.minor == right.minor && left.patch == right.patch;
}

constexpr bool operator!=(version left, version right) noexcept
{
    return !(left == right);
}

constexpr bool operator>(version left, version right) noexcept
{
    return left.major > right.major || (left.major == right.major && (left.minor > right.minor || (left.minor == right.minor && left.patch > right.patch)));
}

constexpr bool operator<(version left, version right) noexcept
{
    return left.major < right.major || (left.major == right.major && (left.minor < right.minor || (left.minor == right.minor && left.patch < right.patch)));
}

constexpr bool operator>=(version left, version right) noexcept
{
    return left > right || left == right;
}

constexpr bool operator<=(version left, version right) noexcept
{
    return left < right || left == right;
}

using bool32_t = std::uint32_t;

struct nullref_t{};
static constexpr nullref_t nullref{};

template<typename T>
class optional_ref
{
public:
    using value_type = T;

public:
    constexpr optional_ref() noexcept = default;

    constexpr optional_ref(nullref_t) noexcept
    :m_ptr{}
    {

    }

    constexpr optional_ref(value_type& ref) noexcept
    :m_ptr{&ref}
    {

    }

    template<class U>
    constexpr optional_ref(const optional_ref<U>& other) noexcept
    :m_ptr{other.m_ptr}
    {
        static_assert(std::is_convertible<U*, value_type*>::value, "Can no bind a reference on value_type from a reference on U");
    }

    optional_ref(const optional_ref& other) = default;
    optional_ref(optional_ref&& other) noexcept = default;

    optional_ref& operator=(const optional_ref& other) = delete; //References are immutable
    optional_ref& operator=(optional_ref&& other) noexcept = delete; //References are immutable

    constexpr explicit operator bool() const noexcept
    {
        return m_ptr;
    }

    constexpr operator value_type&() const noexcept
    {
        return *m_ptr;
    }

    constexpr value_type* operator&() const noexcept
    {
        return m_ptr;
    }

    constexpr value_type& operator*() const noexcept
    {
        return *m_ptr;
    }

    constexpr value_type* operator->() const noexcept
    {
        return m_ptr;
    }

    constexpr bool has_value() const noexcept
    {
        return m_ptr;
    }

    constexpr value_type& value() const noexcept
    {
        return *m_ptr;
    }

private:
    value_type* m_ptr{};
};

template<typename T>
constexpr optional_ref<T> ref(T& ref) noexcept
{
    return optional_ref<T>{ref};
}

template<typename T>
constexpr optional_ref<const T> cref(const T& ref) noexcept
{
    return optional_ref<const T>{ref};
}

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

inline namespace enum_operations
{

template<typename E>
struct enable_enum_operations
{
    static constexpr bool value{false};
};

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E operator&(E left, E right) noexcept
{
    return static_cast<E>(static_cast<std::underlying_type_t<E>>(left) & static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E& operator&=(E& left, E right) noexcept
{
    return reinterpret_cast<E&>(reinterpret_cast<std::underlying_type_t<E>&>(left) &= static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E operator|(E left, E right) noexcept
{
    return static_cast<E>(static_cast<std::underlying_type_t<E>>(left) | static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E& operator|=(E& left, E right) noexcept
{
    return reinterpret_cast<E&>(reinterpret_cast<std::underlying_type_t<E>&>(left) |= static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E operator~(E right) noexcept
{
    return static_cast<E>(~static_cast<std::underlying_type_t<E>>(right));
}

}

}

#endif
