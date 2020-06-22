#include "messagebox.hpp"

#include <stdexcept>
#include <vector>

#include <SDL.h>

#include <captal_foundation/stack_allocator.hpp>

#include "window.hpp"

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
