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

#ifndef TEPHRA_APPLICATION_HPP_INCLUDED
#define TEPHRA_APPLICATION_HPP_INCLUDED

#include "config.hpp"

#include <limits>
#include <span>
#include <functional>

#include "vulkan/vulkan.hpp"

#include "hardware.hpp"

namespace tph
{

TEPHRA_API tph::version enumerate_instance_version();

enum class application_layer : std::uint32_t
{
    none = 0x00,
    validation = 0x01
};

enum class application_extension : std::uint32_t
{
    none = 0x00,
    surface = 0x01,
    debug_utils = 0x02
};

class TEPHRA_API application
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    using filter_type = std::function<bool(const physical_device&)>;
    using comparator_type = std::function<bool(const physical_device&, const physical_device&)>;

public:
    constexpr application() = default;
    explicit application(const std::string& name, version app_version, application_layer layers = application_layer::none, application_extension extensions = application_extension::none);
    explicit application(const std::string& name, version app_version, version api_version, application_layer layers = application_layer::none, application_extension extensions  = application_extension::none);

    explicit application(vulkan::instance inst, tph::version api_version, application_layer layers, application_extension extensions);

    ~application() = default;
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept = default;
    application& operator=(application&& other) noexcept = default;

    const physical_device& select_physical_device(const filter_type& required, const comparator_type& comparator = comparator_type{}) const;
    const physical_device& default_physical_device() const;

    template<typename... Surfaces>
    const physical_device& default_physical_device(const Surfaces&... surfs) const
    {
        static_assert((std::is_same_v<tph::surface, std::decay_t<Surfaces>> && ...), "One of the parameters is not a tph::surface.");

        const auto requirements = [&surfs...](const physical_device& phydev) -> bool
        {
            return (... && phydev.support_presentation(surfs));
        };

        return select_physical_device(requirements, default_physical_device_comparator);
    }

    tph::version api_version() const noexcept
    {
        return m_version;
    }

    vulkan::instance_context context() const noexcept
    {
        return m_instance.context();
    }

    const vulkan::functions::instance_level_functions* operator->() const noexcept
    {
        return m_instance.operator->();
    }

    application_layer enabled_layers() const noexcept
    {
        return m_layers;
    }

    application_extension enabled_extensions() const noexcept
    {
        return m_extensions;
    }

    std::span<const physical_device> enumerate_physical_devices() const noexcept
    {
        return m_physical_devices;
    }

private:
    vulkan::instance m_instance{};
    tph::version m_version{};
    application_layer m_layers{};
    application_extension m_extensions{};
    std::vector<physical_device> m_physical_devices{};
};

template<>
inline VkInstance underlying_cast(const application& application) noexcept
{
    return application.m_instance;
}

}

template<> struct tph::enable_enum_operations<tph::application_extension> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::application_layer> {static constexpr bool value{true};};

#endif
