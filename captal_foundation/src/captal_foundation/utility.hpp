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
    std::ranges::contiguous_range<T>;
    std::is_standard_layout_v<std::ranges::range_value_t<T>>;
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
