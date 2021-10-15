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

#ifndef APYRE_MESSAGEBOX_HPP_INCLUDED
#define APYRE_MESSAGEBOX_HPP_INCLUDED

#include "config.hpp"

#include <string>
#include <span>

namespace apr
{

class window;

enum class message_box_type : std::uint32_t
{
    error = 0x01,
    warning = 0x02,
    information = 0x04
};

enum class message_box_button_bind : std::uint32_t
{
    no_key = 0,
    return_key = 1,
    escape_key = 2
};

struct message_box_button
{
    std::uint32_t id{};
    std::string text{};
    message_box_button_bind bind{message_box_button_bind::no_key};
};

inline constexpr std::uint32_t no_selection{static_cast<std::uint32_t>(-1)};

APYRE_API std::uint32_t message_box(message_box_type type, const std::string& title, const std::string& message, std::span<const message_box_button> buttons);
APYRE_API std::uint32_t message_box(window& window, message_box_type type, const std::string& title, const std::string& message, std::span<const message_box_button> buttons);
APYRE_API void message_box(message_box_type type, const std::string& title, const std::string& message);
APYRE_API void message_box(window& window, message_box_type type, const std::string& title, const std::string& message);

}

#endif
