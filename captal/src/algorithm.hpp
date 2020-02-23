#ifndef CAPTAL_ALGORITHM_HPP_INCLUDED
#define CAPTAL_ALGORITHM_HPP_INCLUDED

#include "config.hpp"

#include <glm/vec2.hpp>

namespace cpt
{

template<typename InputIt>
std::size_t unique_count(InputIt first, InputIt last)
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
std::size_t unique_count(InputIt first, InputIt last, Predicate pred)
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

inline bool bounding_box_query(const glm::vec2& point, const glm::vec2& box_position, const glm::vec2& box_size) noexcept
{
    return point.x >= box_position.x && point.x < box_position.x + box_size.x && point.y >= box_position.y && point.y < box_position.y + box_size.y;
}

template<typename T, typename U, typename V, typename W, typename X, typename Y>
bool bounding_box_query(T&& point_x, U&& point_y, V&& box_x, W&& box_y, X&& box_width, Y&& box_height) noexcept
{
    return bounding_box_query(glm::vec2{point_x, point_y}, glm::vec2{box_x, box_y}, glm::vec2{box_width, box_height});
}

}

#endif
