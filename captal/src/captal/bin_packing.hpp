#ifndef CAPTAL_BIN_PACKING_HPP_INCLUDED
#define CAPTAL_BIN_PACKING_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <array>
#include <optional>

namespace cpt
{

class bin_packer
{
public:
    struct rect
    {
        std::uint32_t x{};
        std::uint32_t y{};
        std::uint32_t width{};
        std::uint32_t height{};
    };

public:
    bin_packer() = default;
    explicit bin_packer(std::uint32_t width, std::uint32_t height);

    bin_packer(const bin_packer&) = delete;
    bin_packer& operator=(const bin_packer&) = delete;
    bin_packer(bin_packer&&) noexcept = default;
    bin_packer& operator=(bin_packer&&) noexcept = default;

    std::optional<rect> append(std::uint32_t image_width, std::uint32_t image_height);
    void resize(std::uint32_t width, std::uint32_t height) noexcept;

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

private:
    struct splits
    {
        std::size_t count{};
        std::array<rect, 2> splits{};
    };

private:
    std::optional<splits> split(std::uint32_t image_width, std::uint32_t image_height, const rect& space) noexcept;

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::vector<rect> m_spaces{};
};

}

#endif
