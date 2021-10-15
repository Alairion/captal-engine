//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#ifndef CAPTAL_ALGORITHM_HPP_INCLUDED
#define CAPTAL_ALGORITHM_HPP_INCLUDED

#include "config.hpp"

#include <string_view>
#include <vector>
#include <algorithm>
#include <iterator>
#include <concepts>
#include <variant>

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
    return point.x() >= box_position.x()
        && point.x() <  box_position.x() + box_size.x()
        && point.y() >= box_position.y()
        && point.y() <  box_position.y() + box_size.y();
}

template<typename CharT, typename Traits>
std::vector<std::basic_string_view<CharT, Traits>> split_once(std::basic_string_view<CharT, Traits> string, CharT delimiter)
{
    std::vector<std::basic_string_view<CharT, Traits>> substrings{};
    substrings.reserve(std::count(std::begin(string), std::end(string), delimiter) + 1);

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
std::vector<std::basic_string_view<CharT, Traits>> split_once(std::basic_string_view<CharT, Traits> string, std::basic_string_view<CharT, Traits> delimiter)
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

template<typename CharT, typename Delimiter, typename Traits = std::char_traits<CharT>>
class split_iterator
{
    static_assert(std::is_same_v<Delimiter, std::basic_string_view<CharT, Traits>> || std::is_same_v<Delimiter, CharT>, "Delimiter must be either a string view or a character");

public:
    using char_type = CharT;
    using traits_type = Traits;
    using string_view_type = std::basic_string_view<char_type, traits_type>;
    using delimiter_type = Delimiter;

    using iterator_category = std::input_iterator_tag;
    using value_type = std::pair<string_view_type, string_view_type>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

public:
    constexpr split_iterator() = default;

    constexpr split_iterator(string_view_type string, delimiter_type delimiter) noexcept
    :m_value{{}, string}
    ,m_delimiter{delimiter}
    ,m_not_end{true}
    {
        operator++();
    }

    ~split_iterator() = default;
    split_iterator(const split_iterator&) = default;
    split_iterator& operator=(const split_iterator&) = default;
    split_iterator(split_iterator&& other) noexcept = default;
    split_iterator& operator=(split_iterator&& other) noexcept = default;

    constexpr reference operator*() const noexcept
    {
        return m_value;
    }

    constexpr pointer operator->() const noexcept
    {
        return &m_value;
    }

    constexpr split_iterator& operator++()
    {
        if(std::empty(m_value.second))
        {
            m_value.first = string_view_type{};
            m_not_end = false;
        }
        else
        {
            const std::size_t position{m_value.second.find(m_delimiter)};

            if(position == string_view_type::npos)
            {
                m_value.first = m_value.second;
                m_value.second = string_view_type{};
            }
            else
            {
                m_value.first = m_value.second.substr(0, position);

                if constexpr(std::is_same_v<delimiter_type, char_type>)
                {
                    m_value.second = m_value.second.substr(position + 1);
                }
                else
                {
                    m_value.second = m_value.second.substr(position + std::size(m_delimiter));
                }
            }
        }

        return *this;
    }

    constexpr split_iterator operator++(int)
    {
        split_iterator temp{*this};
        operator++();

        return temp;
    }

    constexpr split_iterator begin() const noexcept
    {
        return *this;
    }

    constexpr split_iterator end() const noexcept
    {
        return split_iterator{};
    }

    constexpr bool operator!=(const split_iterator& other) const noexcept
    {
        return m_not_end != other.m_not_end;
    }

private:
    value_type m_value{};
    delimiter_type m_delimiter{};
    bool m_not_end{};
};

template<typename CharT, typename Traits>
split_iterator<CharT, CharT, Traits> split(std::basic_string_view<CharT, Traits> string, CharT delimiter) noexcept
{
    return split_iterator<CharT, CharT, Traits>{string, delimiter};
}

template<typename CharT, typename Traits>
split_iterator<CharT, std::basic_string_view<CharT, Traits>, Traits> split(std::basic_string_view<CharT, Traits> string, std::basic_string_view<CharT, Traits> delimiter) noexcept
{
    return split_iterator<CharT, std::basic_string_view<CharT, Traits>, Traits>{string, delimiter};
}

}

#endif
