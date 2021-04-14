#include "inputs.hpp"

#include <SDL.h>

namespace apr
{

scancode to_scancode(application& application [[maybe_unused]], keycode key) noexcept
{
    return static_cast<scancode>(SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key)));
}

keycode to_keycode(application& application [[maybe_unused]], scancode scan) noexcept
{
    return static_cast<keycode>(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scan)));
}

std::string to_string(application& application [[maybe_unused]], keycode key)
{
    return SDL_GetKeyName(static_cast<SDL_Keycode>(key));
}

std::string to_string(application& application [[maybe_unused]], scancode scan)
{
    return SDL_GetScancodeName(static_cast<SDL_Scancode>(scan));
}

void begin_text_input(application& application [[maybe_unused]]) noexcept
{
    SDL_StartTextInput();
}

void end_text_input(application& application [[maybe_unused]]) noexcept
{
    SDL_StopTextInput();
}

}
