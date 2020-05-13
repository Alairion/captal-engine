#include "translation.hpp"

#include <fstream>
#include <cstring>

#include <nes/hash.hpp>

#include "algorithm.hpp"

template<typename Kernel>
struct nes::hash<cpt::translation_context_t, Kernel>
{
    using value_type = kernel_hash_value_t<Kernel>;

    constexpr value_type operator()(const cpt::translation_context_t& value) const noexcept(nes::hash_kernels::is_noexcept_v<Kernel>)
    {
        return Kernel{}(std::data(value), std::size(value));
    }
};

namespace cpt
{

template<typename T>
static constexpr std::uint64_t hash_value(const T& value)
{
    return nes::hash<T, nes::hash_kernels::fnv_1a>{}(value)[0];
}

translator::translator(const std::filesystem::path& path, translator_options options)
:m_options{options}
{
    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

    m_source = std::move(ifs);

    init();
}

translator::translator(const std::string_view& data, translator_options options)
:m_options{options}
,m_source{data}
{
    init();
}

translator::translator(std::istream& stream, translator_options options)
:m_options{options}
,m_source{std::ref(stream)}
{
    init();
}

std::string_view translator::translate(const std::string_view& text, const translation_context_t& context, translate_options options) const
{
    if(static_cast<bool>(m_options & translator_options::identity_translator))
    {
        return std::string_view{text};
    }
    else
    {
        const std::uint64_t text_hash{hash_value(text)};
        const std::uint64_t context_hash{hash_value(context)};

        if(const auto section{m_sections.find(context_hash)}; section != std::end(m_sections))
        {
            if(const auto translation{section->second.find(text_hash)}; translation != std::end(section->second))
            {
                return translation->second;
            }
        }

        if(static_cast<bool>(options & translate_options::context_fallback))
        {
            for(const auto& section : m_sections)
            {
                if(const auto translation{section.second.find(text_hash)}; translation != std::end(section.second))
                {
                    return translation->second;
                }
            }
        }

        if(!static_cast<bool>(options & translate_options::input_fallback))
        {
            throw std::runtime_error{"No translation available for \"" + std::string{text} + "\"."};
        }

        return text;
    }
}

bool translator::exists(const translation_context_t& context) const noexcept
{
    const std::uint64_t context_hash{hash_value(context)};

    return m_sections.find(context_hash) != std::end(m_sections);
}

bool translator::exists(const std::string_view& text, const translation_context_t& context) const noexcept
{
    const std::uint64_t text_hash{hash_value(text)};
    const std::uint64_t context_hash{hash_value(context)};

    if(const auto section{m_sections.find(context_hash)}; section != std::end(m_sections))
    {
        return section->second.find(text_hash) != std::end(section->second);
    }

    return false;
}

void translator::read_from_source(char* output, std::uint64_t begin, std::uint64_t size)
{
    if(std::holds_alternative<std::ifstream>(m_source))
    {
        auto& ifs{std::get<std::ifstream>(m_source)};

        ifs.seekg(static_cast<std::streamoff>(begin), std::ios_base::beg);
        ifs.read(output, static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(ifs.gcount()) != size)
            throw std::runtime_error{"Bad file content."};
    }
    else if(std::holds_alternative<std::string_view>(m_source))
    {
        const auto& view{std::get<std::string_view>(m_source)};

        if(std::size(view) < begin + size)
            throw std::runtime_error{"Bad file content."};

        std::memcpy(&m_file_format, std::data(view) + begin, size);
    }
    else if(std::holds_alternative<std::reference_wrapper<std::istream>>(m_source))
    {
        auto& is{std::get<std::reference_wrapper<std::istream>>(m_source).get()};

        is.seekg(static_cast<std::streamoff>(begin), std::ios_base::beg);
        is.read(output, static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(is.gcount()) != size)
            throw std::runtime_error{"Bad file content."};
    }
}

void translator::parse_file_format()
{
    read_from_source(reinterpret_cast<char*>(&m_file_format), file_format_begin, sizeof(file_format));

    if constexpr(endian::native == endian::big)
    {
        m_file_format.version = tph::to_version(bswap(tph::from_version(m_file_format.version)));
    }

    if(m_file_format.magic_word != translation_magic_word)
        throw std::runtime_error{"Bad file format."};

    if(m_file_format.version != tph::version{1, 0, 0})
        throw std::runtime_error{"Bad file version."};
}

void translator::parse_header()
{
    read_from_source(reinterpret_cast<char*>(&m_header), header_begin, sizeof(header));

    if constexpr(endian::native == endian::big)
    {
        m_header.source_language   = bswap(m_header.source_language);
        m_header.source_country    = bswap(m_header.source_country);
        m_header.target_language   = bswap(m_header.target_language);
        m_header.target_country    = bswap(m_header.target_country);
        m_header.translation_count = bswap(m_header.translation_count);
    }
}

void translator::parse_parse_information()
{
    read_from_source(reinterpret_cast<char*>(&m_parse_informations), parse_information_begin, sizeof(parse_information));

    if constexpr(endian::native == endian::big)
    {
        m_parse_informations.section_count = bswap(m_parse_informations.section_count);
    }
}

void translator::parse_section_descriptions()
{
    const std::size_t total_size{sizeof(section_description) * m_parse_informations.section_count};

    std::vector<section_description> sections{};
    sections.resize(m_parse_informations.section_count);

    read_from_source(reinterpret_cast<char*>(std::data(sections)), section_description_begin, total_size);

    if constexpr(endian::native == endian::big)
    {
        for(auto& section : sections)
        {
            section.begin = bswap(section.begin);
            section.translation_count = bswap(section.translation_count);
        }
    }

    parse_sections(sections);
}

void translator::parse_sections(const std::vector<section_description>& sections)
{
    m_sections.reserve(std::size(sections));

    for(const auto& section : sections)
    {
        translation_set_type translations{};
        translations.reserve(section.translation_count);

        std::uint64_t position{section.begin};
        for(std::size_t j{}; j < section.translation_count; ++j)
        {
            translations.emplace(parse_translation(position));
        }

        m_sections.emplace(std::make_pair(hash_value(section.context), std::move(translations)));
    }
}

std::pair<uint64_t, std::string> translator::parse_translation(std::uint64_t& position)
{
    translation_information info{};
    read_from_source(reinterpret_cast<char*>(&info), position, sizeof(translation_information));

    if constexpr(endian::native == endian::big)
    {
        info.source_text_hash = bswap(info.source_text_hash);
        info.source_text_size = bswap(info.source_text_size);
        info.target_text_size = bswap(info.target_text_size);
    }

    position += sizeof(translation_information) + info.source_text_size;
    std::pair<std::uint64_t, std::string> output{info.source_text_hash, parse_target_text(info, position)};
    position += info.target_text_size;

    return output;
}

std::string translator::parse_target_text(const translation_information& info, std::uint64_t position)
{
    std::string output{};
    output.resize(info.target_text_size);

    read_from_source(std::data(output), position, std::size(output));

    return std::string{std::move(output)};
}

void translator::init()
{
    parse_file_format();
    parse_header();
    parse_parse_information();
    parse_section_descriptions();
}

translation_editor::translation_editor(cpt::language source_language, cpt::country source_country, cpt::language target_language, cpt::country target_country)
:m_file_format{translation_magic_word, last_translation_version}
,m_header{source_language, source_country, target_language, target_country}
{

}

translation_editor::translation_editor(const std::filesystem::path& path)
{
    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

    m_source = std::move(ifs);

    init();
}

translation_editor::translation_editor(const std::string_view& data)
:m_source{data}
{
    init();
}

translation_editor::translation_editor(std::istream& stream)
:m_source{std::ref(stream)}
{
    init();
}

bool translation_editor::add(const translation_context_t& context)
{
    return m_sections.emplace(context, translation_set_type{}).second;
}

bool translation_editor::add(std::string source_text, std::string target_text, const translation_context_t& context)
{
    return m_sections[context].emplace(std::move(source_text), std::move(target_text)).second;
}

void translation_editor::add_or_replace(const translation_context_t& context)
{
    const auto it{m_sections.find(context)};
    if(it != std::end(m_sections))
    {
        it->second.clear();
    }
    else
    {
        m_sections.emplace(std::make_pair(context, translation_set_type{}));
    }
}

void translation_editor::add_or_replace(std::string source_text, std::string target_text, const translation_context_t& context)
{
    auto& section{m_sections[context]};

    const auto it{section.find(source_text)};
    if(it != std::end(section))
    {
        it->second = std::move(target_text);
    }
    else
    {
        section.emplace(std::move(source_text), std::move(target_text));
    }
}

bool translation_editor::remove(const translation_context_t& context)
{
    return m_sections.erase(context) > 0;
}

bool translation_editor::remove(const std::string& source_text, const translation_context_t& context)
{
    const auto it{m_sections.find(context)};
    if(it != std::end(m_sections))
    {
        return it->second.erase(source_text) > 0;
    }

    return false;
}

bool translation_editor::exists(const translation_context_t& context) const
{
    return m_sections.find(context) != std::end(m_sections);
}

bool translation_editor::exists(const std::string& source_text, const translation_context_t& context) const
{
    const auto it{m_sections.find(context)};
    if(it != std::end(m_sections))
    {
        return it->second.find(source_text) != std::end(it->second);
    }

    return false;
}

std::string translation_editor::encode() const
{

}

tph::version translation_editor::set_minimum_version(tph::version requested)
{
    for(auto version : translation_versions)
    {
        if(version >= requested)
        {
            m_file_format.version = version;
            return m_file_format.version;
        }
    }

    m_file_format.version = last_translation_version;
    return m_file_format.version;
}

void translation_editor::read_from_source(char* output, std::size_t begin, std::size_t size)
{
    if(std::holds_alternative<std::ifstream>(m_source))
    {
        auto& ifs{std::get<std::ifstream>(m_source)};

        ifs.seekg(static_cast<std::streamoff>(begin), std::ios_base::beg);
        ifs.read(output, static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(ifs.gcount()) != size)
            throw std::runtime_error{"Bad file content."};
    }
    else if(std::holds_alternative<std::string_view>(m_source))
    {
        const auto& view{std::get<std::string_view>(m_source)};

        if(std::size(view) < begin + size)
            throw std::runtime_error{"Bad file content."};

        std::memcpy(&m_file_format, std::data(view) + begin, size);
    }
    else if(std::holds_alternative<std::reference_wrapper<std::istream>>(m_source))
    {
        auto& is{std::get<std::reference_wrapper<std::istream>>(m_source).get()};

        is.seekg(static_cast<std::streamoff>(begin), std::ios_base::beg);
        is.read(output, static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(is.gcount()) != size)
            throw std::runtime_error{"Bad file content."};
    }
}

void translation_editor::parse_file_format()
{
    read_from_source(reinterpret_cast<char*>(&m_file_format), file_format_begin, sizeof(file_format));

    if constexpr(endian::native == endian::big)
    {
        m_file_format.version = tph::to_version(bswap(tph::from_version(m_file_format.version)));
    }

    if(m_file_format.magic_word != translation_magic_word)
        throw std::runtime_error{"Bad file format."};

    if(m_file_format.version != tph::version{1, 0, 0})
        throw std::runtime_error{"Bad file version."};
}

void translation_editor::parse_header()
{
    read_from_source(reinterpret_cast<char*>(&m_header), header_begin, sizeof(header));

    if constexpr(endian::native == endian::big)
    {
        m_header.source_language   = bswap(m_header.source_language);
        m_header.source_country    = bswap(m_header.source_country);
        m_header.target_language   = bswap(m_header.target_language);
        m_header.target_country    = bswap(m_header.target_country);
        m_header.translation_count = bswap(m_header.translation_count);
    }
}

void translation_editor::parse_parse_information()
{
    read_from_source(reinterpret_cast<char*>(&m_parse_informations), parse_information_begin, sizeof(parse_information));

    if constexpr(endian::native == endian::big)
    {
        m_parse_informations.section_count = bswap(m_parse_informations.section_count);
    }
}

void translation_editor::parse_section_descriptions()
{
    const std::size_t total_size{sizeof(section_description) * m_parse_informations.section_count};

    std::vector<section_description> sections{};
    sections.resize(m_parse_informations.section_count);

    read_from_source(reinterpret_cast<char*>(std::data(sections)), section_description_begin, total_size);

    if constexpr(endian::native == endian::big)
    {
        for(auto& section : sections)
        {
            section.begin = bswap(section.begin);
            section.translation_count = bswap(section.translation_count);
        }
    }

    parse_sections(sections);
}

void translation_editor::parse_sections(const std::vector<section_description>& sections)
{
    m_sections.reserve(std::size(sections));

    for(const auto& section : sections)
    {
        translation_set_type translations{};
        translations.reserve(section.translation_count);

        std::uint64_t position{section.begin};
        for(std::size_t j{}; j < section.translation_count; ++j)
        {
            translations.emplace(parse_translation(position));
        }

        m_sections.emplace(std::make_pair(section.context, std::move(translations)));
    }
}

std::pair<std::string, std::string> translation_editor::parse_translation(std::uint64_t& position)
{
    translation_information info{};
    read_from_source(reinterpret_cast<char*>(&info), position, sizeof(translation_information));
    position += sizeof(translation_information);

    if constexpr(endian::native == endian::big)
    {
        info.source_text_hash = bswap(info.source_text_hash);
        info.source_text_size = bswap(info.source_text_size);
        info.target_text_size = bswap(info.target_text_size);
    }

    std::string source{parse_text(info, position)};
    position += info.source_text_size;

    std::string target{parse_text(info, position)};
    position += info.target_text_size;

    return std::make_pair(std::move(source), std::move(target));
}

std::string translation_editor::parse_text(const translation_information& info, std::uint64_t position)
{
    std::string output{};
    output.resize(info.target_text_size);

    read_from_source(std::data(output), position, std::size(output));

    return std::string{std::move(output)};
}

void translation_editor::init()
{
    parse_file_format();
    parse_header();
    parse_parse_information();
    parse_section_descriptions();
}

std::string translation_editor::encode_file_format() const
{
    std::string output{};
    output.resize(sizeof(file_format));

    if constexpr(endian::native == endian::big)
    {
        file_format format{m_file_format};

        format.version = tph::to_version(bswap(tph::from_version(format.version)));

        std::memcpy(std::data(output), &format, std::size(output));
    }
    else
    {
        std::memcpy(std::data(output), &m_file_format, std::size(output));
    }

    return output;
}

std::string translation_editor::encode_header() const
{
    std::string output{};
    output.resize(sizeof(header));

    if constexpr(endian::native == endian::big)
    {
        header header{};

        header.source_language   = bswap(header.source_language);
        header.source_country    = bswap(header.source_country);
        header.target_language   = bswap(header.target_language);
        header.target_country    = bswap(header.target_country);
        header.translation_count = bswap(header.translation_count);

        std::memcpy(std::data(output), &header, std::size(output));
    }
    else
    {
        std::memcpy(std::data(output), &m_header, std::size(output));
    }

    return output;
}

std::string translation_editor::encode_parse_informations() const
{
    parse_information parse_informations{std::size(m_sections)};
    std::vector<section_description> section_descriptions{};
    section_descriptions.reserve(parse_informations.section_count);

    const std::size_t output_position{parse_information_begin};
    std::string tranlations{};

    for(auto&& section : m_sections)
    {

    }
}

std::string translation_editor::encode_translations(const translation_set_type& translations)
{
    std::string output{};

    const auto format_text = [](const std::string& string) -> std::string
    {

    };

    for(auto&& translation : translations)
    {
        translation_information information{};
        information.source_text_hash = hash_value(translation.first);
        information.source_text_size = std::size(translation.first);
        information.target_text_size = std::size(translation.second);
    }

    return output;
}

}
