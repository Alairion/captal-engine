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

#include "messagebox.hpp"

#include <stdexcept>
#include <vector>

#include <captal_foundation/stack_allocator.hpp>

#include "window.hpp"

#include <SDL.h>

namespace apr
{

std::uint32_t message_box(message_box_type type, const std::string& title, const std::string& message, std::span<const message_box_button> buttons)
{
    stack_memory_pool<256> pool{};

    auto native_buttons{cpt::make_stack_vector<SDL_MessageBoxButtonData>(pool)};
    native_buttons.reserve(std::size(buttons));
    for(auto&& button : buttons)
    {
        SDL_MessageBoxButtonData& native_button{native_buttons.emplace_back()};
        native_button.flags = static_cast<SDL_MessageBoxButtonFlags>(button.bind);
        native_button.buttonid = static_cast<int>(button.id);
        native_button.text = std::data(button.text);
    }

    SDL_MessageBoxData data{};
    data.flags = static_cast<SDL_MessageBoxFlags>(type);
    data.title = std::data(title);
    data.message = std::data(message);
    data.numbuttons = static_cast<int>(std::size(native_buttons));
    data.buttons = std::data(native_buttons);

    int id{};
    if(SDL_ShowMessageBox(&data, &id))
        throw std::runtime_error{"Can not show message box. " + std::string{SDL_GetError()}};

    return static_cast<std::uint32_t>(id);
}

std::uint32_t message_box(window& window, message_box_type type, const std::string& title, const std::string& message, std::span<const message_box_button> buttons)
{
    stack_memory_pool<256> pool{};

    auto native_buttons{cpt::make_stack_vector<SDL_MessageBoxButtonData>(pool)};
    native_buttons.reserve(std::size(buttons));
    for(auto&& button : buttons)
    {
        SDL_MessageBoxButtonData& native_button{native_buttons.emplace_back()};
        native_button.flags = static_cast<SDL_MessageBoxButtonFlags>(button.bind);
        native_button.buttonid = static_cast<int>(button.id);
        native_button.text = reinterpret_cast<const char*>(std::data(button.text));
    }

    SDL_MessageBoxData data{};
    data.flags = static_cast<SDL_MessageBoxFlags>(type);
    data.title = std::data(title);
    data.message = std::data(message);
    data.numbuttons = static_cast<int>(std::size(native_buttons));
    data.buttons = std::data(native_buttons);
    data.window = SDL_GetWindowFromID(window.id());

    int id{};
    if(SDL_ShowMessageBox(&data, &id))
        throw std::runtime_error{"Can not show message box. " + std::string{SDL_GetError()}};

    return static_cast<std::uint32_t>(id);
}

void message_box(message_box_type type, const std::string& title, const std::string& message)
{
    if(SDL_ShowSimpleMessageBox(static_cast<SDL_MessageBoxFlags>(type), std::data(title), std::data(message), nullptr))
        throw std::runtime_error{"Can not show message box. " + std::string{SDL_GetError()}};
}

void message_box(window& window, message_box_type type, const std::string& title, const std::string& message)
{
    if(SDL_ShowSimpleMessageBox(static_cast<SDL_MessageBoxFlags>(type), std::data(title), std::data(message), SDL_GetWindowFromID(window.id())))
        throw std::runtime_error{"Can not show message box. " + std::string{SDL_GetError()}};
}

}
