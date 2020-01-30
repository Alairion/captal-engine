#ifndef TEPHRA_TEST_WINDOW_HPP_INCLUDED
#define TEPHRA_TEST_WINDOW_HPP_INCLUDED

#include <stdexcept>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "src/surface.hpp"
#include "src/hardware.hpp"

namespace utils
{

class window
{
public:
    window()
    {
        SDL_SetMainReady();

        if(SDL_Init(SDL_INIT_VIDEO) != 0)
            throw std::runtime_error{"Can not initialize SDL2: " + std::string{SDL_GetError()}};

        if(m_window = SDL_CreateWindow(u8"Tephra", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE); !m_window)
        {
            SDL_Quit();
            throw std::runtime_error{"Can not create window: " + std::string{SDL_GetError()}};
        }
    }

    ~window()
    {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    tph::surface make_surface(tph::application& application) const
    {
        VkSurfaceKHR surface{};
        if(!SDL_Vulkan_CreateSurface(m_window, tph::underlying_cast<VkInstance>(application), &surface))
            throw std::runtime_error{"Can not create window surface. " + std::string{SDL_GetError()}};

        return tph::surface{tph::vulkan::surface{tph::underlying_cast<VkInstance>(application), surface}};
    }

    bool update()
    {
        SDL_Event event{};
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                return false;
        }

        return true;
    }

    bool wait_restore()
    {
        SDL_Event event{};
        while(event.type != SDL_WINDOWEVENT && event.window.event != SDL_WINDOWEVENT_RESTORED)
        {
            SDL_WaitEvent(&event);

            if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
                return false;
        }

        return true;
    }

    void change_title(const std::string title)
    {
        SDL_SetWindowTitle(m_window, std::data(title));
    }

    std::pair<std::uint32_t, std::uint32_t> size() const
    {
        int width{};
        int height{};
        SDL_GetWindowSize(m_window, &width, &height);

        return std::make_pair(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    }

    operator SDL_Window*() const noexcept
    {
        return m_window;
    }

private:
    SDL_Window* m_window{};
};

}

#endif
