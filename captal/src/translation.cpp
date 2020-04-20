#include "translation.hpp"

#include <fstream>
#include <cstring>

#include <nes/hash.hpp>

#include "encoding.hpp"
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
constexpr std::uint64_t hash_value(const T& value) noexcept
{
    return nes::hash<T, nes::hash_kernels::fnv_1a>{}(value)[0];
}

static constexpr std::uint64_t no_translation_context_hash{hash_value(no_translation_context)};

translator::translator(const std::filesystem::path& path, translator_options options [[maybe_unused]])
{
    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

    m_source = std::move(ifs);

    init();
}

translator::translator(const std::string_view& data, translator_options options [[maybe_unused]])
:m_source{data}
{
    init();
}

translator::translator(std::istream& stream, translator_options options [[maybe_unused]])
:m_source{std::ref(stream)}
{
    init();
}

std::string_view translator::translate(const std::string_view& text, translate_options options) const
{
    const std::uint64_t text_hash{hash_value(text)};

    if(const auto section{m_sections.find(no_translation_context_hash)}; section != std::end(m_sections))
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

std::string_view translator::translate(const std::string_view& text, const translation_context_t& context, translate_options options) const
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

void translator::read_from_source(char* output, std::uint64_t begin, std::uint64_t size, bool stream_jump)
{
    if(std::holds_alternative<std::ifstream>(m_source))
    {
        auto& ifs{std::get<std::ifstream>(m_source)};

        if(stream_jump)
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

        if(stream_jump)
            is.seekg(static_cast<std::streamoff>(begin), std::ios_base::beg);

        is.read(output, static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(is.gcount()) != size)
            throw std::runtime_error{"Bad file content."};
    }
}

void translator::parse_file_format()
{
    read_from_source(reinterpret_cast<char*>(&m_file_format), file_format_begin, sizeof(file_format), false);

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
    read_from_source(reinterpret_cast<char*>(&m_header), header_begin, sizeof(header), false);

    if constexpr(endian::native == endian::big)
    {
        m_header.source_language   = bswap(m_header.source_language);
        m_header.source_country    = bswap(m_header.source_country);
        m_header.source_encoding   = bswap(m_header.source_encoding);
        m_header.target_language   = bswap(m_header.target_language);
        m_header.target_country    = bswap(m_header.target_country);
        m_header.target_encoding   = bswap(m_header.target_encoding);
        m_header.translation_count = bswap(m_header.translation_count);
    }
}

void translator::parse_parse_information()
{
    read_from_source(reinterpret_cast<char*>(&m_parse_informations), parse_information_begin, sizeof(parse_information), false);

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

    read_from_source(reinterpret_cast<char*>(std::data(sections)), section_description_begin, total_size, false);

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
            m_sections.emplace(parse_translation(position));
        }

        m_sections.emplace(std::make_pair(hash_value(section.context), std::move(translations)));
    }
}

std::pair<std::uint64_t, std::string> translator::parse_translation(std::uint64_t& position)
{
    translation_information info{};
    read_from_source(reinterpret_cast<char*>(&info), position, sizeof(translation_information), true);

    if constexpr(endian::native == endian::big)
    {
        info.source_text_hash = bswap(info.source_text_hash);
        info.source_text_size = bswap(info.source_text_size);
        info.destination_text_size = bswap(info.destination_text_size);
    }

    position += sizeof(translation_information) + info.source_text_size;
    std::pair<std::uint64_t, std::string> output{info.source_text_hash, parse_destination_text(info, position)};
    position += info.destination_text_size;

    return output;
}

std::string translator::parse_destination_text(const translation_information& info, std::uint64_t position)
{
    if(m_header.target_encoding == translation_encoding::utf8)
    {
        std::string output{};
        output.resize(info.destination_text_size);

        read_from_source(std::data(output), position, std::size(output), true);

        return output;
    }
    else
    {
        if(m_header.target_encoding == translation_encoding::utf16)
        {
            std::u16string output{};
            output.resize(info.destination_text_size / sizeof(char16_t));

            read_from_source(reinterpret_cast<char*>(std::data(output)), position, info.destination_text_size, true);

            if constexpr(endian::native == endian::big)
            {
                std::transform(std::begin(output), std::end(output), std::begin(output), [](char16_t c)
                {
                    return static_cast<char16_t>(bswap(static_cast<std::uint16_t>(c)));
                });
            }

            return convert<utf16, utf8>(output);
        }
        else if(m_header.target_encoding == translation_encoding::utf32)
        {
            std::u32string output{};
            output.resize(info.destination_text_size / sizeof(char32_t));

            read_from_source(reinterpret_cast<char*>(std::data(output)), position, info.destination_text_size, true);

            if constexpr(endian::native == endian::big)
            {
                std::transform(std::begin(output), std::end(output), std::begin(output), [](char32_t c)
                {
                    return static_cast<char32_t>(bswap(static_cast<std::uint32_t>(c)));
                });
            }

            return convert<utf32, utf8>(output);
        }
    }

    throw std::runtime_error{"Bad file destination encoding."};
}

void translator::init()
{
    parse_file_format();
    parse_header();
    parse_parse_information();
    parse_section_descriptions();
}

}
