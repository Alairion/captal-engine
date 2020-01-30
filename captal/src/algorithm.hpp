#ifndef CAPTAL_ALGORITHM_HPP_INCLUDED
#define CAPTAL_ALGORITHM_HPP_INCLUDED

#include "config.hpp"

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


}

#endif
