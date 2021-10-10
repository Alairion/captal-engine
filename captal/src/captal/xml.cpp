#include "xml.hpp"

#include <iostream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlerror.h>

#include <captal_foundation/utility.hpp>

namespace cpt
{

static const xmlChar* to_xml_char(const char* ptr)
{
    return reinterpret_cast<const xmlChar*>(ptr);
}

static const char* from_xml_char(const xmlChar* ptr)
{
    return reinterpret_cast<const char*>(ptr);
}

static std::string_view ltrim_spaces(std::string_view string) noexcept
{
    const auto begin{std::find_if(std::begin(string), std::end(string), [](char c)
    {
        return !std::isspace(c);
    })};

    return std::string_view{begin, std::end(string)};
}

static std::string_view rtrim_spaces(std::string_view string) noexcept
{
    const auto end{std::find_if(std::rbegin(string), std::rend(string), [](char c)
    {
        return !std::isspace(c);
    })};

    return std::string_view{std::begin(string), end.base()};
}

static std::string_view trim_spaces(std::string_view string) noexcept
{
    return ltrim_spaces(rtrim_spaces(string));
}

std::string_view xml_attribute::name() const noexcept
{
    return std::string_view{from_xml_char(static_cast<xmlAttr*>(m_attribute)->name)};
}

std::string_view xml_attribute::value() const noexcept
{
    return std::string_view{from_xml_char(static_cast<xmlAttr*>(m_attribute)->children->content)};
}

xml_attribute_range::iterator& xml_attribute_range::iterator::operator++() noexcept
{
    if(auto next{static_cast<xmlAttr*>(m_attribute.m_attribute)->next}; next)
    {
        m_attribute.m_attribute = next;
    }
    else
    {
        m_end = true;
    }

    return *this;
}

xml_attribute_range::iterator& xml_attribute_range::iterator::operator--() noexcept
{
    if(m_end)
    {
        m_end = false;
    }
    else
    {
        m_attribute.m_attribute = static_cast<xmlAttr*>(m_attribute.m_attribute)->prev;
    }

    return *this;
}

xml_node_range::iterator& xml_node_range::iterator::operator++() noexcept
{
    if(auto next{static_cast<xmlNode*>(m_node.m_node)->next}; next)
    {
        m_node.m_node = next;
    }
    else
    {
        m_end = true;
    }

    return *this;
}

xml_node_range::iterator& xml_node_range::iterator::operator--() noexcept
{
    if(m_end)
    {
        m_end = false;
    }
    else
    {
        m_node.m_node = static_cast<xmlNode*>(m_node.m_node)->prev;
    }

    return *this;
}

std::string_view xml_node::name() const noexcept
{
    return std::string_view{from_xml_char(static_cast<xmlNode*>(m_node)->name)};
}

std::string_view xml_node::content() const noexcept
{
    return trim_spaces(std::string_view{from_xml_char(static_cast<xmlNode*>(m_node)->content)});
}

std::string_view xml_node::raw_content() const noexcept
{
    return std::string_view{from_xml_char(static_cast<xmlNode*>(m_node)->content)};
}

xml_node_type xml_node::type() const noexcept
{
    return static_cast<xml_node_type>(static_cast<xmlNode*>(m_node)->type);
}

xml_attribute_range xml_node::attributes() const noexcept
{
    xmlAttr* first{static_cast<xmlNode*>(m_node)->properties};
    xmlAttr* last {first};

    if(first) //Find last attr
    {
        while(last->next)
        {
            last = last->next;
        }
    }

    xml_attribute_range output{};
    output.m_first = first;
    output.m_last  = last;

    return output;
}

xml_node_range xml_node::children() const noexcept
{
    xml_node_range output{};
    output.m_first = static_cast<xmlNode*>(m_node)->children;
    output.m_last  = static_cast<xmlNode*>(m_node)->last;

    return output;
}

void xml_document::context_deleter::operator()(void* context) const noexcept
{
    xmlFreeParserCtxt(static_cast<xmlParserCtxt*>(context));
}

void xml_document::document_deleter::operator()(void* document) const noexcept
{
    xmlFreeDoc(static_cast<xmlDoc*>(document));
}

xml_document::xml_document(const std::string& root_name, const std::string& root_content)
{
    m_context = context_handle_type{xmlNewParserCtxt()};
    if(auto error{xmlCtxtGetLastError(m_context.get())}; error)
        throw std::runtime_error{"Can not create xml context \"" + std::string{error->message} + "\""};

    m_document = document_handle_type{xmlNewDoc(to_xml_char("1.0"))};

    xmlDoc*  document{static_cast<xmlDoc*>(m_document.get())};
    xmlNode* root    {xmlNewDocNode(document, nullptr, to_xml_char(std::data(root_name)), to_xml_char(std::data(root_content)))};
    xmlDocSetRootElement(document, root);
}

xml_document::xml_document(const std::filesystem::path& file)
:xml_document{read_file<std::vector<std::uint8_t>>(file)}
{

}

xml_document::xml_document(std::span<const std::uint8_t> data)
:xml_document{std::string_view{reinterpret_cast<const char*>(std::data(data)), std::size(data)}}
{

}

xml_document::xml_document(std::string_view data)
{
    m_context = context_handle_type{xmlNewParserCtxt()};
    if(auto error{xmlCtxtGetLastError(m_context.get())}; error)
        throw std::runtime_error{"Can not create xml context \"" + std::string{error->message} + "\""};

    m_document = document_handle_type{xmlCtxtReadMemory(static_cast<xmlParserCtxt*>(m_context.get()), std::data(data), static_cast<int>(std::size(data)), "", nullptr, XML_PARSE_NOBLANKS | XML_PARSE_NOCDATA)};
    if(auto error{xmlCtxtGetLastError(m_context.get())}; error)
        throw std::runtime_error{"Can not create xml context \"" + std::string{error->message} + "\""};
}

xml_node xml_document::root_node() const noexcept
{
    xml_node output{};
    output.m_node = xmlDocGetRootElement(static_cast<xmlDoc*>(m_document.get()));

    return output;
}

}
