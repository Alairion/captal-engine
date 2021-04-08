#ifndef CAPTAL_FOUNDATION_OPTIONAl_REF_HPP_INCLUDED
#define CAPTAL_FOUNDATION_OPTIONAl_REF_HPP_INCLUDED

#include <cstdint>
#include <type_traits>
#include <optional>

namespace cpt
{

inline namespace foundation
{

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

    constexpr optional_ref(std::optional<value_type>& ref)
    :m_ptr{ref ? &(*ref) : nullptr}
    {

    }

    constexpr optional_ref(const std::optional<value_type>& ref)
    :m_ptr{ref ? &(*ref) : nullptr}
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
}

}

#endif
