#ifndef CAPTAL_ALGORITHM_HPP_INCLUDED
#define CAPTAL_ALGORITHM_HPP_INCLUDED

#include "config.hpp"

#include <string_view>
#include <vector>
#include <algorithm>

#include <glm/vec2.hpp>

namespace cpt
{

template<typename InputIt>
constexpr std::size_t unique_count(InputIt first, InputIt last)
{
    if(first == last)
        return 0;

    std::size_t result{1};
    InputIt temp{first};

    while(++first != last)
    {
        if(!(*temp == *first))
        {
            temp = first;
            ++result;
        }
    }

    return result;
}


template<typename InputIt, typename Predicate>
constexpr std::size_t unique_count(InputIt first, InputIt last, Predicate pred)
{
    if(first == last)
        return 0;

    std::size_t result{1};
    InputIt temp{first};

    while(++first != last)
    {
        if(!pred(*temp, *first))
        {
            temp = first;
            ++result;
        }
    }

    return result;
}

constexpr bool bounding_box_query(const glm::vec2& point, const glm::vec2& box_position, const glm::vec2& box_size) noexcept
{
    return point.x >= box_position.x && point.x < box_position.x + box_size.x && point.y >= box_position.y && point.y < box_position.y + box_size.y;
}

template<typename T, typename U, typename V, typename W, typename X, typename Y>
constexpr bool bounding_box_query(T&& point_x, U&& point_y, V&& box_x, W&& box_y, X&& box_width, Y&& box_height) noexcept
{
    return bounding_box_query(glm::vec2{point_x, point_y}, glm::vec2{box_x, box_y}, glm::vec2{box_width, box_height});
}

template<typename CharT, typename Traits>
std::vector<std::basic_string_view<CharT, Traits>> split(std::basic_string_view<CharT, Traits> string, CharT delimiter)
{
    std::vector<std::basic_string_view<CharT, Traits>> substrings{};
    substrings.reserve(std::count(std::begin(string), std::end(string), delimiter));

    std::size_t position{};
    std::size_t last{};
    while(position != std::u32string_view::npos)
    {
        position = string.find(delimiter, last);
        substrings.push_back(string.substr(last, position - last));
        last = position + 1;
    }

    return substrings;
}

template<typename CharT, typename Traits>
std::vector<std::basic_string_view<CharT, Traits>> split(std::basic_string_view<CharT, Traits> string, std::basic_string_view<CharT, Traits> delimiter)
{
    std::vector<std::basic_string_view<CharT, Traits>> substrings{};

    std::size_t position{};
    std::size_t last{};
    while(position != std::u32string_view::npos)
    {
        position = string.find(delimiter, last);
        substrings.push_back(string.substr(last, position - last));
        last = position + std::size(delimiter);
    }

    return substrings;
}

constexpr std::uint8_t bswap(std::uint8_t value) noexcept
{
    return value;
}

constexpr std::uint16_t bswap(std::uint16_t value) noexcept
{
    return (value << 8) | (value >> 8);
}

constexpr std::uint32_t bswap(std::uint32_t value) noexcept
{
    value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0x00FF00FF);

    return (value << 16) | (value >> 16);
}

constexpr std::uint64_t bswap(std::uint64_t value) noexcept
{
    value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
    value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
    value = ((value & 0x00FF00FF00FF00FFull) << 8)  | ((value & 0xFF00FF00FF00FF00ull) >> 8);

    return value;
}

template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum> && std::is_unsigned_v<std::underlying_type_t<Enum>>>>
constexpr Enum bswap(Enum value) noexcept
{
    return static_cast<Enum>(bswap(static_cast<std::underlying_type_t<Enum>>(value)));
}

}

#endif
