#ifndef CAPTAL_FOUNDATION_MATH_HPP_INCLUDED
#define CAPTAL_FOUNDATION_MATH_HPP_INCLUDED

#include <array>
#include <cstdint>
#include <concepts>
#include <ranges>
#include <cmath>

namespace cpt
{

inline namespace foundation
{

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<arithmetic T, std::size_t Size>
struct vec;

template<arithmetic T>
struct vec<T, 2> : private std::array<T, 2>
{
    using parent_type = std::array<T, 2>;

public:
    using value_type = typename parent_type::value_type;
    using size_type = typename parent_type::size_type;
    using difference_type = typename parent_type::difference_type;
    using reference = typename parent_type::reference;
    using const_reference = typename parent_type::const_reference;
    using pointer = typename parent_type::pointer;
    using const_pointer = typename parent_type::const_pointer;
    using iterator = typename parent_type::iterator;
    using const_iterator = typename parent_type::const_iterator;
    using reverse_iterator  = typename parent_type::reverse_iterator;
    using const_reverse_iterator = typename parent_type::const_reverse_iterator;

public:
    constexpr vec() noexcept = default;

    constexpr explicit vec(value_type value) noexcept
    :parent_type{value, value}
    {

    }

    constexpr explicit vec(value_type x, value_type y) noexcept
    :parent_type{x, y}
    {

    }

    template<std::size_t Size>
    constexpr explicit vec(const vec<value_type, Size>& other) noexcept requires (Size > 2) //Conversion from vec3 or vec4
    :parent_type{other[0], other[1]}
    {

    }

    constexpr vec(const vec&) noexcept = default;
    constexpr vec& operator=(const vec&) noexcept = default;
    constexpr vec(vec&&) noexcept = default;
    constexpr vec& operator=(vec&&) noexcept = default;

    constexpr reference x() noexcept
    {
        return operator[](0);
    }

    constexpr const_reference x() const noexcept
    {
        return operator[](0);
    }

    constexpr reference y() noexcept
    {
        return operator[](1);
    }

    constexpr const_reference y() const noexcept
    {
        return operator[](1);
    }

    using parent_type::operator[];
    using parent_type::empty;
    using parent_type::data;
    using parent_type::size;
    using parent_type::max_size;

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;
};

template<arithmetic T>
struct vec<T, 3> : private std::array<T, 3>
{
    using parent_type = std::array<T, 3>;

public:
    using value_type = typename parent_type::value_type;
    using size_type = typename parent_type::size_type;
    using difference_type = typename parent_type::difference_type;
    using reference = typename parent_type::reference;
    using const_reference = typename parent_type::const_reference;
    using pointer = typename parent_type::pointer;
    using const_pointer = typename parent_type::const_pointer;
    using iterator = typename parent_type::iterator;
    using const_iterator = typename parent_type::const_iterator;
    using reverse_iterator  = typename parent_type::reverse_iterator;
    using const_reverse_iterator = typename parent_type::const_reverse_iterator;

public:
    constexpr vec() noexcept = default;

    constexpr explicit vec(value_type value) noexcept
    :parent_type{value, value, value}
    {

    }

    constexpr explicit vec(value_type x, value_type y, value_type z) noexcept
    :parent_type{x, y, z}
    {

    }

    constexpr explicit vec(value_type value, const vec<value_type, 2>& other) noexcept
    :parent_type{value, other[0], other[1]}
    {

    }

    constexpr explicit vec(const vec<value_type, 2>& other, value_type value) noexcept
    :parent_type{other[0], other[1], value}
    {

    }

    constexpr explicit vec(const vec<value_type, 4>& other) noexcept
    :parent_type{other[0], other[1], other[2]}
    {

    }

    constexpr vec(const vec&) noexcept = default;
    constexpr vec& operator=(const vec&) noexcept = default;
    constexpr vec(vec&&) noexcept = default;
    constexpr vec& operator=(vec&&) noexcept = default;

    constexpr reference x() noexcept
    {
        return operator[](0);
    }

    constexpr const_reference x() const noexcept
    {
        return operator[](0);
    }

    constexpr reference y() noexcept
    {
        return operator[](1);
    }

    constexpr const_reference y() const noexcept
    {
        return operator[](1);
    }

    constexpr reference z() noexcept
    {
        return operator[](2);
    }

    constexpr const_reference z() const noexcept
    {
        return operator[](2);
    }

    using parent_type::operator[];
    using parent_type::empty;
    using parent_type::data;
    using parent_type::size;
    using parent_type::max_size;

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;
};

template<arithmetic T>
struct vec<T, 4> : private std::array<T, 4>
{
    using parent_type = std::array<T, 4>;

public:
    using value_type = typename parent_type::value_type;
    using size_type = typename parent_type::size_type;
    using difference_type = typename parent_type::difference_type;
    using reference = typename parent_type::reference;
    using const_reference = typename parent_type::const_reference;
    using pointer = typename parent_type::pointer;
    using const_pointer = typename parent_type::const_pointer;
    using iterator = typename parent_type::iterator;
    using const_iterator = typename parent_type::const_iterator;
    using reverse_iterator  = typename parent_type::reverse_iterator;
    using const_reverse_iterator = typename parent_type::const_reverse_iterator;

public:
    constexpr vec() noexcept = default;

    constexpr explicit vec(value_type value) noexcept
    :parent_type{value, value, value}
    {

    }

    constexpr explicit vec(value_type x, value_type y, value_type z, value_type w) noexcept
    :parent_type{x, y, z, w}
    {

    }

    constexpr explicit vec(value_type value, const vec<value_type, 3>& other) noexcept
    :parent_type{value, other[0], other[1], other[2]}
    {

    }

    constexpr explicit vec(const vec<value_type, 3>& other, value_type value) noexcept
    :parent_type{other[0], other[1], other[2], value}
    {

    }

    constexpr vec(const vec&) noexcept = default;
    constexpr vec& operator=(const vec&) noexcept = default;
    constexpr vec(vec&&) noexcept = default;
    constexpr vec& operator=(vec&&) noexcept = default;

    constexpr reference x() noexcept
    {
        return operator[](0);
    }

    constexpr const_reference x() const noexcept
    {
        return operator[](0);
    }

    constexpr reference y() noexcept
    {
        return operator[](1);
    }

    constexpr const_reference y() const noexcept
    {
        return operator[](1);
    }

    constexpr reference z() noexcept
    {
        return operator[](2);
    }

    constexpr const_reference z() const noexcept
    {
        return operator[](2);
    }

    constexpr reference w() noexcept
    {
        return operator[](3);
    }

    constexpr const_reference w() const noexcept
    {
        return operator[](3);
    }

    using parent_type::operator[];
    using parent_type::empty;
    using parent_type::data;
    using parent_type::size;
    using parent_type::max_size;

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;
};

template<typename T>
using vec2 = vec<T, 2>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;
using vec2i = vec2<std::int32_t>;
using vec2u = vec2<std::uint32_t>;

template<typename T>
using vec3 = vec<T, 3>;
using vec3f = vec3<float>;
using vec3d = vec3<double>;
using vec3i = vec3<std::int32_t>;
using vec3u = vec3<std::uint32_t>;

template<typename T>
using vec4 = vec<T, 4>;
using vec4f = vec4<float>;
using vec4d = vec4<double>;
using vec4i = vec4<std::int32_t>;
using vec4u = vec4<std::uint32_t>;

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size> operator+(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    vec<T, Size> output{};

    for(std::size_t i{}; i < Size; ++i)
    {
        output[i] = left[i] + right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size>& operator+=(vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    left = left + right;

    return left;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size> operator-(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    vec<T, Size> output{};

    for(std::size_t i{}; i < Size; ++i)
    {
        output[i] = left[i] - right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size>& operator-=(vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    left = left - right;

    return left;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size> operator*(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    vec<T, Size> output{};

    for(std::size_t i{}; i < Size; ++i)
    {
        output[i] = left[i] * right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size>& operator*=(vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    left = left * right;

    return left;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size> operator/(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    vec<T, Size> output{};

    for(std::size_t i{}; i < Size; ++i)
    {
        output[i] = left[i] / right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Size>
constexpr vec<T, Size>& operator/=(vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    left = left / right;

    return left;
}

struct identity_t{};
inline constexpr identity_t identity{};

template<arithmetic T, std::size_t Rows, std::size_t Cols>
struct mat;

template<arithmetic T, std::size_t Cols>
struct mat<T, 2, Cols> : private std::array<vec<T, Cols>, 2>
{
    using parent_type = std::array<vec<T, Cols>, 2>;

public:
    using arithmetic_type = T;
    using value_type = typename parent_type::value_type;
    using row_type = value_type;
    using size_type = typename parent_type::size_type;
    using difference_type = typename parent_type::difference_type;
    using reference = typename parent_type::reference;
    using const_reference = typename parent_type::const_reference;
    using pointer = typename parent_type::pointer;
    using const_pointer = typename parent_type::const_pointer;
    using iterator = typename parent_type::iterator;
    using const_iterator = typename parent_type::const_iterator;
    using reverse_iterator  = typename parent_type::reverse_iterator;
    using const_reverse_iterator = typename parent_type::const_reverse_iterator;

public:
    constexpr mat() noexcept = default;

    constexpr explicit mat(identity_t) noexcept requires (Cols == 2)
    :parent_type{{static_cast<arithmetic_type>(1), arithmetic_type{}},
                 {arithmetic_type{}, static_cast<arithmetic_type>(1)}}
    {

    }

    constexpr explicit mat(const value_type& x, const value_type& y) noexcept
    :parent_type{x, y}
    {

    }

    constexpr mat(const mat&) noexcept = default;
    constexpr mat& operator=(const mat&) noexcept = default;
    constexpr mat(mat&&) noexcept = default;
    constexpr mat& operator=(mat&&) noexcept = default;

    constexpr reference x() noexcept
    {
        return operator[](0);
    }

    constexpr const_reference x() const noexcept
    {
        return operator[](0);
    }

    constexpr reference y() noexcept
    {
        return operator[](1);
    }

    constexpr const_reference y() const noexcept
    {
        return operator[](1);
    }

    using parent_type::operator[];
    using parent_type::empty;
    using parent_type::data;
    using parent_type::size;
    using parent_type::max_size;

    constexpr size_type rows() const noexcept
    {
        return 2;
    }

    constexpr size_type cols() const noexcept
    {
        return Cols;
    }

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;
};

template<arithmetic T, std::size_t Cols>
struct mat<T, 3, Cols> : private std::array<vec<T, Cols>, 3>
{
    using parent_type = std::array<vec<T, Cols>, 3>;

public:
    using arithmetic_type = T;
    using value_type = typename parent_type::value_type;
    using row_type = value_type;
    using size_type = typename parent_type::size_type;
    using difference_type = typename parent_type::difference_type;
    using reference = typename parent_type::reference;
    using const_reference = typename parent_type::const_reference;
    using pointer = typename parent_type::pointer;
    using const_pointer = typename parent_type::const_pointer;
    using iterator = typename parent_type::iterator;
    using const_iterator = typename parent_type::const_iterator;
    using reverse_iterator  = typename parent_type::reverse_iterator;
    using const_reverse_iterator = typename parent_type::const_reverse_iterator;

public:
    constexpr mat() noexcept = default;

    constexpr explicit mat(identity_t) noexcept requires (Cols == 3)
    :parent_type{{static_cast<arithmetic_type>(1), arithmetic_type{}, arithmetic_type{}},
                 {arithmetic_type{}, static_cast<arithmetic_type>(1), arithmetic_type{}},
                 {arithmetic_type{}, arithmetic_type{}}, static_cast<arithmetic_type>(1)}
    {

    }

    constexpr explicit mat(const value_type& x, const value_type& y, const value_type& z) noexcept
    :parent_type{x, y, z}
    {

    }

    constexpr mat(const mat&) noexcept = default;
    constexpr mat& operator=(const mat&) noexcept = default;
    constexpr mat(mat&&) noexcept = default;
    constexpr mat& operator=(mat&&) noexcept = default;

    constexpr reference x() noexcept
    {
        return operator[](0);
    }

    constexpr const_reference x() const noexcept
    {
        return operator[](0);
    }

    constexpr reference y() noexcept
    {
        return operator[](1);
    }

    constexpr const_reference y() const noexcept
    {
        return operator[](1);
    }

    using parent_type::operator[];
    using parent_type::empty;
    using parent_type::data;
    using parent_type::size;
    using parent_type::max_size;

    constexpr size_type rows() const noexcept
    {
        return 2;
    }

    constexpr size_type cols() const noexcept
    {
        return Cols;
    }

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;
};

}

}

#endif
