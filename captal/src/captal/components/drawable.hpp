#ifndef CAPTAL_DRAWABLE_HPP_INCLUDED
#define CAPTAL_DRAWABLE_HPP_INCLUDED

#include "../config.hpp"

#include <variant>

#include "../renderable.hpp"
#include "../text.hpp"

namespace cpt
{

namespace components
{

template<typename... Types>
class basic_drawable
{
    static_assert(sizeof...(Types) != 0 && (std::is_base_of_v<renderable, Types> && ...), "All Types must derive from cpt::renderable");

public:
    using attachment_type = std::variant<std::monostate, Types...>;

public:
    basic_drawable() = default;

    template<typename T>
    basic_drawable(T&& value) noexcept(std::is_nothrow_constructible_v<attachment_type, decltype(std::forward<T>(value))>)
    :m_attachment{std::forward<T>(value)}
    {

    }

    template<typename T, typename... Args>
    explicit basic_drawable(std::in_place_type_t<T>, Args&&... args) noexcept(std::is_nothrow_constructible_v<attachment_type, std::in_place_type_t<T>, Args...>)
    :m_attachment{std::in_place_type<T>, std::forward<Args>(args)...}
    {

    }

    ~basic_drawable() = default;
    basic_drawable(const basic_drawable&) = delete;
    basic_drawable& operator=(const basic_drawable&) = delete;
    basic_drawable(basic_drawable&&) noexcept = default;
    basic_drawable& operator=(basic_drawable&&) noexcept = default;

    template<typename T>
    T& attach(T&& value) noexcept(std::is_nothrow_constructible_v<attachment_type, decltype(std::forward<T>(value))>)
    {
        return m_attachment. template emplace<std::decay_t<T>>(std::forward<T>(value));
    }

    template<typename T, typename... Args>
    T& attach(Args&&... args) noexcept(std::is_nothrow_constructible_v<attachment_type, Args...>)
    {
        return m_attachment. template emplace<T>(std::forward<Args>(args)...);
    }

    void detach() noexcept
    {
        m_attachment. template emplace<0>();
    }

    attachment_type& attachment() noexcept
    {
        return m_attachment;
    }

    const attachment_type& attachment() const noexcept
    {
        return m_attachment;
    }

    bool has_attachment() const noexcept
    {
        return m_attachment.index() != 0;
    }

    void swap(basic_drawable& other) noexcept
    {
        m_attachment.swap(other.m_attachment);
    }

    explicit operator bool() const noexcept
    {
        return has_attachment();
    }

    attachment_type& operator*() noexcept
    {
        return m_attachment;
    }

    const attachment_type& operator*() const noexcept
    {
        return m_attachment;
    }

    attachment_type* operator->() noexcept
    {
        return &m_attachment;
    }

    const attachment_type* operator->() const noexcept
    {
        return &m_attachment;
    }

    template<typename T>
    T& get()
    {
        return std::get<T>(m_attachment);
    }

    template<typename T>
    const T& get() const
    {
        return std::get<T>(m_attachment);
    }

    cpt::renderable& renderable() noexcept
    {
        assert(has_attachment() && "cpt::basic_drawable::renderable called on empty drawable");

        return std::visit([](auto&& alternative) -> cpt::renderable&
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(alternative)>, std::monostate>)
            {
                std::terminate();
            }
            else
            {
                return static_cast<cpt::renderable&>(alternative);
            }
        }, m_attachment);
    }

    const cpt::renderable& renderable() const noexcept
    {
        assert(has_attachment() && "cpt::basic_drawable::renderable called on empty drawable");

        return std::visit([](auto&& alternative) -> const cpt::renderable&
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(alternative)>, std::monostate>)
            {
                std::terminate();
            }
            else
            {
                return static_cast<const cpt::renderable&>(alternative);
            }
        }, m_attachment);
    }

private:
    attachment_type m_attachment{};
};

using drawable = basic_drawable<sprite, polygon, tilemap, text>;

namespace impl
{

template<typename T>
struct test_drawable : std::false_type{};
template<typename... Types>
struct test_drawable<basic_drawable<Types...>> : std::true_type{};

}

template<typename T>
concept drawable_specialization = impl::test_drawable<T>::value;

}

}

#endif
