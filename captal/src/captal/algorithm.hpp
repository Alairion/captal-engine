#ifndef CAPTAL_ALGORITHM_HPP_INCLUDED
#define CAPTAL_ALGORITHM_HPP_INCLUDED

#include "config.hpp"

#include <string_view>
#include <vector>
#include <algorithm>
#include <iterator>
#include <concepts>

#include <captal_foundation/math.hpp>

namespace cpt
{

template<std::input_iterator InputIt>
constexpr std::size_t unique_count(InputIt first, InputIt last)
{
    if(first == last)
    {
        return 0;
    }

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


template<std::input_iterator InputIt, std::predicate Predicate>
constexpr std::size_t unique_count(InputIt first, InputIt last, Predicate pred)
{
    if(first == last)
    {
        return 0;
    }

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

template<arithmetic T>
constexpr bool bounding_box_query(const vec2<T>& point, const vec2<T>& box_position, const vec2<T>& box_size) noexcept
{
    return point.x >= box_position.x && point.x < box_position.x + box_size.x && point.y >= box_position.y && point.y < box_position.y + box_size.y;
}

template<typename CharT, typename Traits>
std::vector<std::basic_string_view<CharT, Traits>> split(std::basic_string_view<CharT, Traits> string, CharT delimiter)
{
    std::vector<std::basic_string_view<CharT, Traits>> substrings{};
    substrings.reserve(std::count(std::begin(string), std::end(string), delimiter));

    std::size_t position{};
    std::size_t last{};
    while(position != std::basic_string_view<CharT, Traits>::npos)
    {
        position = string.find(delimiter, last);
        substrings.emplace_back(string.substr(last, position - last));
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
    while(position != std::basic_string_view<CharT, Traits>::npos)
    {
        position = string.find(delimiter, last);
        substrings.emplace_back(string.substr(last, position - last));
        last = position + std::size(delimiter);
    }

    return substrings;
}

}

#endif
