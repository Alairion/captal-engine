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

#ifndef CAPTAL_FOUNDATION_UTILITY_HPP_INCLUDED
#define CAPTAL_FOUNDATION_UTILITY_HPP_INCLUDED

#include <filesystem>
#include <fstream>
#include <vector>
#include <ranges>
#include <type_traits>
#include <stdexcept>

#include "encoding.hpp"

namespace cpt
{

inline namespace foundation
{

template<typename T>
concept dynamic_contiguous_range = requires(T container, std::size_t count)
{
    requires std::ranges::contiguous_range<T>;
    requires std::is_standard_layout_v<std::ranges::range_value_t<T>>;
    container.resize(count);
};

template<dynamic_contiguous_range Container>
Container read_file(const std::filesystem::path& path)
{
    using value_type = std::ranges::range_value_t<Container>;

    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
    {
        throw std::runtime_error{"Can not open file \"" + convert_to<narrow>(path.u8string()) + "\"."};
    }

    const auto file_size{std::filesystem::file_size(path)};

    Container output{};
    output.resize(file_size / sizeof(value_type));

    const auto stream_size{static_cast<std::streamsize>(file_size)};
    if(ifs.read(reinterpret_cast<char*>(std::data(output)), stream_size).gcount() != stream_size)
    {
        throw std::runtime_error{"Can not read entire file \"" + convert_to<narrow>(path.u8string()) + "\"."};
    }

    return output;
}

}

}

#endif
