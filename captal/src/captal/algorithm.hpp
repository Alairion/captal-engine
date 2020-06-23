#ifndef CAPTAL_ALGORITHM_HPP_INCLUDED
#define CAPTAL_ALGORITHM_HPP_INCLUDED

#include "config.hpp"

#include <string_view>
#include <vector>
#include <algorithm>
#include <iterator>
#include <concepts>

#include <glm/vec2.hpp>

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
