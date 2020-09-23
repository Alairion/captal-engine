#ifndef CAPTAL_FOUNDATION_MATH_HPP_INCLUDED
#define CAPTAL_FOUNDATION_MATH_HPP_INCLUDED

#include <array>
#include <cstdint>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <cmath>

namespace cpt
{

//abs, pow, sqrt, sin, cos, tan

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

template<typename T, std::size_t Size>
constexpr bool operator==(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    return std::equal(std::begin(left), std::end(left), std::begin(right), std::end(right));
}

template<typename T, std::size_t Size>
constexpr auto operator<=>(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    return std::lexicographical_compare_three_way(std::begin(left), std::end(left), std::begin(right), std::end(right));
}

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
constexpr vec<T, Size> operator-(const vec<T, Size>& vector) noexcept
{
    vec<T, Size> output{};

    for(std::size_t i{}; i < Size; ++i)
    {
        output[i] = -vector[i];
    }

    return output;
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

template<arithmetic T, std::size_t Size>
constexpr T dot(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    T output{};

    for(std::size_t i{}; i < Size; ++i)
    {
        output += left[i] * right[i];
    }

    return output;
}

template<arithmetic T>
constexpr vec<T, 3> cross(const vec<T, 3>& left, const vec<T, 3>& right) noexcept
{
    return vec<T, 3>{left[1] * right[2] - left[2] * right[1],
                     left[2] * right[0] - left[0] * right[2],
                     left[0] * right[1] - left[1] * right[0]};
}

template<arithmetic T, std::size_t Size>
T length(const vec<T, Size>& vector) noexcept
{
    return static_cast<T>(std::sqrt(dot(vector, vector)));
}

template<arithmetic T, std::size_t Size>
vec<T, Size> normalize(const vec<T, Size>& vector) noexcept
{
    return vector / vec<T, Size>{length(vector)};
}

template<arithmetic T, std::size_t Size>
T distance(const vec<T, Size>& left, const vec<T, Size>& right) noexcept
{
    return length(left - right);
}

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
    :parent_type{value_type{static_cast<T>(1), T{}},
                 value_type{T{}, static_cast<T>(1)}}
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
    :parent_type{value_type{static_cast<T>(1), T{}, T{}},
                 value_type{T{}, static_cast<T>(1), T{}},
                 value_type{T{}, T{}, static_cast<T>(1)}}
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

    constexpr size_type rows() const noexcept
    {
        return 3;
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
struct mat<T, 4, Cols> : private std::array<vec<T, Cols>, 4>
{
    using parent_type = std::array<vec<T, Cols>, 4>;

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

    constexpr explicit mat(identity_t) noexcept requires (Cols == 4)
    :parent_type{value_type{static_cast<T>(1), T{}, T{}, T{}},
                 value_type{T{}, static_cast<T>(1), T{}, T{}},
                 value_type{T{}, T{}, static_cast<T>(1), T{}},
                 value_type{T{}, T{}, T{}, static_cast<T>(1)}}
    {

    }

    constexpr explicit mat(const value_type& x, const value_type& y, const value_type& z, const value_type& w) noexcept
    :parent_type{x, y, z, w}
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

    constexpr size_type rows() const noexcept
    {
        return 4;
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

template<typename T, std::size_t Rows, std::size_t Cols>
constexpr bool operator==(const mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    return std::equal(std::begin(left), std::end(left), std::begin(right), std::end(right));
}

template<typename T, std::size_t Rows, std::size_t Cols>
constexpr auto operator<=>(const mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    return std::lexicographical_compare_three_way(std::begin(left), std::end(left), std::begin(right), std::end(right));
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols> operator+(const mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    mat<T, Rows, Cols> output{};

    for(std::size_t i{}; i < Rows; ++i)
    {
        output[i] = left[i] + right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols>& operator+=(mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    left = left + right;

    return left;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols> operator-(const mat<T, Rows, Cols>& matrix) noexcept
{
    mat<T, Rows, Cols> output{};

    for(std::size_t i{}; i < Rows; ++i)
    {
        output[i] = -matrix[i];
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols> operator-(const mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    mat<T, Rows, Cols> output{};

    for(std::size_t i{}; i < Rows; ++i)
    {
        output[i] = left[i] - right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols>& operator-=(mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    left = left - right;

    return left;
}

template<arithmetic T, std::size_t Size1, std::size_t Size2, std::size_t Size3>
constexpr mat<T, Size1, Size3> operator*(const mat<T, Size1, Size2>& left, const mat<T, Size2, Size3>& right) noexcept
{
    mat<T, Size1, Size3> output{};

    for(std::size_t i{}; i < Size1; ++i)
    {
        for(std::size_t j{}; j < Size3; ++j)
        {
            for(std::size_t k{}; k < Size2; ++k)
            {
                output[i][j] += left[i][k] * right[k][j];
            }
        }
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr vec<T, Rows> operator*(const mat<T, Rows, Cols>& left, const vec<T, Cols>& right) noexcept
{
    vec<T, Rows> output{};

    for(std::size_t i{}; i < Rows; ++i)
    {
        for(std::size_t j{}; j < Cols; ++j)
        {
            output[i] += left[i][j] * right[j];
        }
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr vec<T, Cols> operator*(const vec<T, Rows>& left, const mat<T, Rows, Cols>& right) noexcept
{
    vec<T, Cols> output{};

    for(std::size_t j{}; j < Cols; ++j)
    {
        for(std::size_t i{}; i < Rows; ++i)
        {
            output[j] += left[i] * right[i][j];
        }
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols>& operator*=(mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    left = left * right;

    return left;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols> operator/(const mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    mat<T, Rows, Cols> output{};

    for(std::size_t i{}; i < Rows; ++i)
    {
        output[i] = left[i] / right[i];
    }

    return output;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Rows, Cols>& operator/=(mat<T, Rows, Cols>& left, const mat<T, Rows, Cols>& right) noexcept
{
    left = left / right;

    return left;
}

template<arithmetic T, std::size_t Rows, std::size_t Cols>
constexpr mat<T, Cols, Rows> transpose(const mat<T, Rows, Cols>& matrix) noexcept
{
    mat<T, Cols, Rows> output{};

    for(std::size_t x{}; x < Rows; ++x)
    {
        for(std::size_t y{}; y < Cols; ++y)
        {
            output[x][y] = matrix[y][x];
        }
    }

    return output;
}

template<arithmetic T>
constexpr T determinant(const mat<T, 2, 2>& matrix) noexcept
{
    return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
}

template<arithmetic T>
constexpr T determinant(const mat<T, 3, 3>& matrix) noexcept
{
    const std::array matrices
    {
        mat<T, 2, 2>
        {
            vec<T, 2>{matrix[1][1], matrix[1][2]},
            vec<T, 2>{matrix[2][1], matrix[2][2]}
        },
        mat<T, 2, 2>
        {
            vec<T, 2>{matrix[1][0], matrix[1][2]},
            vec<T, 2>{matrix[2][0], matrix[2][2]}
        },
        mat<T, 2, 2>
        {
            vec<T, 2>{matrix[1][0], matrix[1][1]},
            vec<T, 2>{matrix[2][0], matrix[2][1]}
        },
    };

    return matrix[0][0] * determinant(matrices[0]) - matrix[0][0] * determinant(matrices[1]) + matrix[0][0] * determinant(matrices[2]);
}

template<arithmetic T>
constexpr T determinant(const mat<T, 4, 4>& matrix) noexcept
{
    const std::array matrices
    {
        mat<T, 3, 3>
        {
            vec<T, 3>{matrix[1][1], matrix[1][2], matrix[1][3]},
            vec<T, 3>{matrix[2][1], matrix[2][2], matrix[2][3]},
            vec<T, 3>{matrix[3][1], matrix[3][2], matrix[3][3]}
        },
        mat<T, 3, 3>
        {
            vec<T, 3>{matrix[1][0], matrix[1][2], matrix[1][3]},
            vec<T, 3>{matrix[2][0], matrix[2][2], matrix[2][3]},
            vec<T, 3>{matrix[3][0], matrix[3][2], matrix[3][3]}
        },
        mat<T, 3, 3>
        {
            vec<T, 3>{matrix[1][0], matrix[1][1], matrix[1][3]},
            vec<T, 3>{matrix[2][0], matrix[2][1], matrix[2][3]},
            vec<T, 3>{matrix[3][0], matrix[3][1], matrix[3][3]}
        },
        mat<T, 3, 3>
        {
            vec<T, 3>{matrix[1][0], matrix[1][1], matrix[1][2]},
            vec<T, 3>{matrix[2][0], matrix[2][1], matrix[2][2]},
            vec<T, 3>{matrix[3][0], matrix[3][1], matrix[3][2]}
        },
    };

    return matrix[0][0] * determinant(matrices[0]) - matrix[0][1] * determinant(matrices[1]) + matrix[0][2] * determinant(matrices[2]) - matrix[0][3] * determinant(matrices[3]);
}

template<arithmetic T>
constexpr mat<T, 4, 4> translate(const vec<T, 3>& translation) noexcept
{
    mat<T, 4, 4> output{identity};

    output[0][3] = translation[0];
    output[1][3] = translation[1];
    output[2][3] = translation[2];

    return output;
}

template<arithmetic T>
mat<T, 4, 4> rotate(T angle, const vec<T, 3>& axis) noexcept
{
    const auto cos{std::cos(angle)};
    const auto sin{std::sin(angle)};
    const auto factor{static_cast<T>(1) - cos};
    const auto temp{vec<T, 3>{factor} * axis};

    mat<T, 4, 4> output{identity};

    output[0][0] = axis[0] * temp[0] + cos;
    output[0][1] = axis[1] * temp[0] - axis[2] * sin;
    output[0][2] = axis[2] * temp[0] + axis[1] * sin;

    output[1][0] = axis[0] * temp[1] + axis[2] * sin;
    output[1][1] = axis[1] * temp[1] + cos;
    output[1][2] = axis[2] * temp[1] - axis[0] * sin;

    output[2][0] = axis[0] * temp[2] - axis[1] * sin;
    output[2][1] = axis[1] * temp[2] + axis[0] * sin;
    output[2][2] = axis[2] * temp[2] + cos;

    return output;
}

template<arithmetic T>
constexpr mat<T, 4, 4> scale(const vec<T, 3>& scale) noexcept
{
    mat<T, 4, 4> output{identity};

    output[0][0] = scale[0];
    output[1][1] = scale[1];
    output[2][2] = scale[2];

    return output;
}

template<arithmetic T>
mat<T, 4, 4> model(const vec<T, 3>& translation, T angle, const vec<T, 3>& axis, const vec<T, 3>& factor)
{
    return translate(translation) * rotate(angle, axis) * scale(factor);
}

template<arithmetic T>
mat<T, 4, 4> model(const vec<T, 3>& translation, T angle, const vec<T, 3>& axis, const vec<T, 3>& factor, const vec<T, 3>& origin)
{
    return translate(-origin) * rotate(angle, axis) * translate(translation) * scale(factor);
}

template<arithmetic T>
using mat2 = mat<T, 2, 2>;
using mat2f = mat2<float>;
using mat2d = mat2<double>;
using mat2i = mat2<std::int32_t>;
using mat2u = mat2<std::uint32_t>;

template<arithmetic T>
using mat3 = mat<T, 3, 3>;
using mat3f = mat3<float>;
using mat3d = mat3<double>;
using mat3i = mat3<std::int32_t>;
using mat3u = mat3<std::uint32_t>;

template<arithmetic T>
using mat4 = mat<T, 4, 4>;
using mat4f = mat4<float>;
using mat4d = mat4<double>;
using mat4i = mat4<std::int32_t>;
using mat4u = mat4<std::uint32_t>;

namespace indices
{

inline constexpr std::size_t x{0};
inline constexpr std::size_t y{1};
inline constexpr std::size_t z{2};
inline constexpr std::size_t w{3};

}

}

}

#endif
