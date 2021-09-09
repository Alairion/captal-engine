#ifndef SWELL_APPLICATION_HPP_INCLUDED
#define SWELL_APPLICATION_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <vector>
#include <span>

#include "physical_device.hpp"

namespace swl
{

class host_api
{
public:
    host_api() = default;

    host_api(std::uint32_t id, std::optional<std::uint32_t> default_ouput_device, std::string name) noexcept
    :m_id{id}
    ,m_default_output_device{default_ouput_device}
    ,m_name{std::move(name)}
    {

    }

    ~host_api() = default;
    host_api(const host_api&) = delete;
    host_api& operator=(const host_api&) = delete;
    host_api(host_api&& other) noexcept = default;
    host_api& operator=(host_api&& other) noexcept = default;

    std::uint32_t id() const noexcept
    {
        return m_id;
    }

    std::optional<std::uint32_t> default_output_device() const noexcept
    {
        return m_default_output_device;
    }

    const std::string& name() const noexcept
    {
        return m_name;
    }

private:
    std::uint32_t m_id{};
    std::optional<std::uint32_t> m_default_output_device{};
    std::string m_name{};
};

class SWELL_API application
{
public:
    application();
    ~application();
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept;
    application& operator=(application&& other) noexcept;

    std::span<const host_api> enumerate_host_apis() const noexcept
    {
        return m_host_apis;
    }

    std::span<const physical_device> enumerate_physical_devices() const noexcept
    {
        return m_physical_devices;
    }

    const host_api& default_host_api() const noexcept;
    const physical_device& default_output_device() const noexcept;

private:
    std::vector<host_api> m_host_apis{};
    std::vector<physical_device> m_physical_devices{};
    bool m_free{true};
};

}

#endif
