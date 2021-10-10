#ifndef CAPTAL_XML_HPP_INCLUDED
#define CAPTAL_XML_HPP_INCLUDED

#include "config.hpp"

#include <filesystem>
#include <span>
#include <string_view>
#include <optional>
#include <charconv>
#include <cstdlib>
#include <ranges>
#include <concepts>

#include <fast_float/fast_float.h>

namespace cpt
{

enum class xml_node_type : std::uint32_t
{
    unknown = 0,
    element = 1,
    attribute = 2,
    text = 3,
    cdata_section = 4,
    entity_ref = 5,
    entity = 6,
    processing_instruction = 7,
    comment = 8,
    document = 9,
    document_type = 10,
    document_fragment = 11,
    notation =  12,
    html_document = 13,
    document_type_definition = 14,
    element_declaration = 15,
    attribute_declaration = 16,
    entity_declaration = 17,
    namespace_declaration = 18,
};

class xml_attribute;
class xml_attribute_range;
class xml_node;
class xml_node_range;

class CAPTAL_API xml_attribute
{
    friend class xml_attribute_range;

public:
    xml_attribute() = default;
    ~xml_attribute() = default;
    xml_attribute(const xml_attribute&) = default;
    xml_attribute& operator=(const xml_attribute&) = default;
    xml_attribute(xml_attribute&&) = default;
    xml_attribute& operator=(xml_attribute&&) = default;

    std::string_view name() const noexcept;
    std::string_view value() const noexcept;

    template<std::integral T>
    std::optional<T> as(std::uint32_t base = 10) const noexcept
    {
        const auto string{value()};

        T output;
        if(std::from_chars(std::data(string), std::data(string) + std::size(string), output, static_cast<int>(base)).ec == std::errc{})
        {
            return std::make_optional(output);
        }

        return std::nullopt;
    }

    template<std::floating_point T>
    std::optional<T> as(std::chars_format fmt = std::chars_format::general) const noexcept
    {
        const auto string{value()};

        T output;
        if(fast_float::from_chars(std::data(string), std::data(string) + std::size(string), output, fmt).ec == std::errc{})
        {
            return std::make_optional(output);
        }

        return std::nullopt;
    }

private:
    void* m_attribute{};
};

class CAPTAL_API xml_attribute_range
{
    friend class xml_node;

public:
    class CAPTAL_API iterator
    {
        friend class xml_attribute_range;

    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = xml_attribute;
        using pointer           = xml_attribute*;
        using reference         = xml_attribute&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        iterator() = default;
        ~iterator() = default;
        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        iterator& operator++() noexcept;
        iterator& operator--() noexcept;

        iterator operator++(int) noexcept
        {
            iterator output{*this};
            operator++();

            return output;
        }

        iterator operator--(int) noexcept
        {
            iterator output{*this};
            operator--();

            return output;
        }

        reference operator*() const noexcept
        {
            return m_attribute;
        }

        pointer operator->() const noexcept
        {
            return &m_attribute;
        }

        bool operator!=(iterator& other) const noexcept
        {
            return m_attribute.m_attribute != other.m_attribute.m_attribute || m_end != other.m_end;
        }

    private:
        mutable xml_attribute m_attribute{};
        bool m_end{};
    };

public:
    xml_attribute_range() = default;
    ~xml_attribute_range() = default;
    xml_attribute_range(const xml_attribute_range&) = default;
    xml_attribute_range& operator=(const xml_attribute_range&) = default;
    xml_attribute_range(xml_attribute_range&&) = default;
    xml_attribute_range& operator=(xml_attribute_range&&) = default;

    iterator begin() const noexcept
    {
        iterator output{};
        output.m_attribute.m_attribute = m_first;
        output.m_end = !m_first;

        return output;
    }

    iterator end() const noexcept
    {
        iterator output{};
        output.m_attribute.m_attribute = m_last;
        output.m_end = true;

        return output;
    }

private:
    void* m_first{};
    void* m_last{};
};

class CAPTAL_API xml_node
{
    friend class xml_node_range;
    friend class xml_document;

public:
    xml_node() = default;
    ~xml_node() = default;
    xml_node(const xml_node&) = default;
    xml_node& operator=(const xml_node&) = default;
    xml_node(xml_node&&) = default;
    xml_node& operator=(xml_node&&) = default;

    void set_name(std::string_view string);
    void set_content(std::string_view string);

    std::string_view name() const noexcept;
    std::string_view content() const noexcept;
    std::string_view raw_content() const noexcept;
    xml_node_type type() const noexcept;

    xml_attribute_range attributes() const noexcept;
    xml_node_range children() const noexcept;

    xml_node parent() const noexcept;

private:
    void* m_node{};
};

class CAPTAL_API xml_node_range
{
    friend class xml_node;

public:
    class CAPTAL_API iterator
    {
        friend class xml_node_range;

    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = xml_node;
        using pointer           = xml_node*;
        using reference         = xml_node&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        iterator() = default;
        ~iterator() = default;
        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        iterator& operator++() noexcept;
        iterator& operator--() noexcept;

        iterator operator++(int) noexcept
        {
            iterator output{*this};
            operator++();

            return output;
        }

        iterator operator--(int) noexcept
        {
            iterator output{*this};
            operator--();

            return output;
        }

        reference operator*() const noexcept
        {
            return m_node;
        }

        pointer operator->() const noexcept
        {
            return &m_node;
        }

        bool operator!=(iterator& other) const noexcept
        {
            return m_node.m_node != other.m_node.m_node || m_end != other.m_end;
        }

    private:
        mutable xml_node m_node{};
        bool m_end{};
    };

public:
    xml_node_range() = default;
    ~xml_node_range() = default;
    xml_node_range(const xml_node_range&) = default;
    xml_node_range& operator=(const xml_node_range&) = default;
    xml_node_range(xml_node_range&&) = default;
    xml_node_range& operator=(xml_node_range&&) = default;

    iterator begin() const noexcept
    {
        iterator output{};
        output.m_node.m_node = m_first;
        output.m_end = !m_first;

        return output;
    }

    iterator end() const noexcept
    {
        iterator output{};
        output.m_node.m_node = m_last;
        output.m_end = true;

        return output;
    }

private:
    void* m_first{};
    void* m_last{};
};

enum class xml_parse_options : std::uint32_t
{
    none = 0x0000,
    substitute_entities = 0x0002,
    merge_cdata = 0x4000
};

class CAPTAL_API xml_document
{
private:
    struct CAPTAL_API context_deleter
    {
        void operator()(void* ptr) const noexcept;
    };

    struct CAPTAL_API document_deleter
    {
        void operator()(void* ptr) const noexcept;
    };

    using context_handle_type  = std::unique_ptr<void, context_deleter>;
    using document_handle_type = std::unique_ptr<void, document_deleter>;

public:
    xml_document() = default;
    explicit xml_document(const std::filesystem::path& file);
    explicit xml_document(std::span<const std::uint8_t> data);
    explicit xml_document(std::string_view data);
    explicit xml_document(std::istream& stream);
    explicit xml_document(const std::string& root_name, const std::string& root_content);

    ~xml_document() = default;
    xml_document(const xml_document&) = delete;
    xml_document& operator=(const xml_document&) = delete;
    xml_document(xml_document&&) = default;
    xml_document& operator=(xml_document&&) = default;

    xml_node root_node() const noexcept;

private:
    context_handle_type m_context{};
    document_handle_type m_document{};

};

}

#endif
