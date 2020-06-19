#include "translation.hpp"

#include <fstream>
#include <cstring>

#include <nes/hash.hpp>

#include "algorithm.hpp"
#include "engine.hpp"

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

translation_parser::translation_parser(const std::filesystem::path& path)
{
    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.string() + "\"."};

    m_source = std::move(ifs);

    init();
}

translation_parser::translation_parser(const std::string_view& data)
:m_source{memory_stream{data, 0}}
{
    init();
}

translation_parser::translation_parser(std::istream& stream)
:m_source{std::ref(stream)}
{
    init();
}

translation_parser::section_ptr translation_parser::current_section() const noexcept
{
    if(std::empty(m_sections))
    {
        return nullptr;
    }

    return &m_sections[m_current_section];
}

translation_parser::section_ptr translation_parser::next_section()
{
    if(++m_current_section == std::size(m_sections))
    {
        return nullptr;
    }

    return jump_to_section(m_current_section);
}

translation_parser::section_ptr translation_parser::jump_to_section(std::size_t index)
{
    const section_information& output{m_sections[index]};
    seek(output.begin);

    m_current_translation = 0;

    return &output;
}

std::optional<translation_parser::translation> translation_parser::next_translation(translation_parser_load loads)
{
    if(m_current_translation == m_sections[m_current_section].translation_count)
        return std::nullopt;

    ++m_current_translation;

    translation output{};
    output.source_hash = read_uint64();
    output.source_size = read_uint64();
    output.target_size = read_uint64();

    if(static_cast<bool>(loads & translation_parser_load::source_text))
    {
        output.source.resize(output.source_size);
        read(std::data(output.source), std::size(output.source));
    }
    else
    {
        seek(output.source_size, std::ios_base::cur);
    }

    if(static_cast<bool>(loads & translation_parser_load::target_text))
    {
        output.target.resize(output.target_size);
        read(std::data(output.target), std::size(output.target));
    }
    else
    {
        seek(output.target_size, std::ios_base::cur);
    }

    return std::make_optional(std::move(output));
}

void translation_parser::read(void* output, std::size_t size)
{
    if(std::holds_alternative<memory_stream>(m_source))
    {
        auto& view{std::get<memory_stream>(m_source)};

        if(std::size(view.data) < view.position + size)
        {
            throw std::runtime_error{"Bad file content."};
        }

        std::memcpy(output, std::data(view.data) + view.position, size);
        view.position += size;
    }
    else if(std::holds_alternative<std::ifstream>(m_source))
    {
        auto& ifs{std::get<std::ifstream>(m_source)};
        ifs.read(reinterpret_cast<char*>(output), static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(ifs.gcount()) != size)
        {
            throw std::runtime_error{"Bad file content."};
        }
    }
    else if(std::holds_alternative<std::reference_wrapper<std::istream>>(m_source))
    {
        auto& is{std::get<std::reference_wrapper<std::istream>>(m_source).get()};
        is.read(reinterpret_cast<char*>(output), static_cast<std::streamsize>(size));

        if(static_cast<std::size_t>(is.gcount()) != size)
        {
            throw std::runtime_error{"Bad file content."};
        }
    }
}

void translation_parser::seek(std::size_t position, std::ios_base::seekdir dir)
{
    if(std::holds_alternative<memory_stream>(m_source))
    {
        auto& view{std::get<memory_stream>(m_source)};

        if(dir == std::ios_base::beg)
        {
            view.position = position;
        }
        else if(dir == std::ios_base::cur)
        {
            view.position += position;
        }
        else if(dir == std::ios_base::end)
        {
            view.position = std::size(view.data) + position;
        }
    }
    else if(std::holds_alternative<std::ifstream>(m_source))
    {
        auto& ifs{std::get<std::ifstream>(m_source)};
        ifs.seekg(static_cast<std::streamoff>(position), dir);
    }
    else if(std::holds_alternative<std::reference_wrapper<std::istream>>(m_source))
    {
        auto& is{std::get<std::reference_wrapper<std::istream>>(m_source).get()};
        is.seekg(static_cast<std::streamoff>(position), dir);
    }
}

std::uint16_t translation_parser::read_uint16()
{
    std::uint16_t output{};
    read(&output, sizeof(std::uint16_t));

    if constexpr(std::endian::native == std::endian::big)
    {
        output = bswap(output);
    }

    return output;
}

std::uint32_t translation_parser::read_uint32()
{
    std::uint32_t output{};
    read(&output, sizeof(std::uint32_t));

    if constexpr(std::endian::native == std::endian::big)
    {
        output = bswap(output);
    }

    return output;
}

std::uint64_t translation_parser::read_uint64()
{
    std::uint64_t output{};
    read(&output, sizeof(std::uint64_t));

    if constexpr(std::endian::native == std::endian::big)
    {
        output = bswap(output);
    }

    return output;
}

translation_context_t translation_parser::read_context()
{
    translation_context_t output{};
    read(&output, sizeof(translation_context_t));

    return output;
}

void translation_parser::read_header()
{
    read(&m_info.magic_word, sizeof(translation_magic_word_t));

    if(m_info.magic_word != translation_magic_word)
    {
        throw std::runtime_error{"Bad file format."};
    }

    m_info.version.major = read_uint16();
    m_info.version.minor = read_uint16();
    m_info.version.patch = read_uint32();

    if(m_info.version != cpt::version{0, 1, 0}) //Only version as of this file is written
    {
        throw std::runtime_error{"Bad file version."};
    }

    m_header.source_language = static_cast<language>(read_uint32());
    m_header.source_country = static_cast<country>(read_uint32());
    m_header.target_language = static_cast<language>(read_uint32());
    m_header.target_country = static_cast<country>(read_uint32());
    m_header.translation_count = read_uint64();
    m_header.section_count = read_uint64();
}

void translation_parser::read_sections()
{
    m_sections.resize(m_header.section_count);

    for(auto& section : m_sections)
    {
        section.context = read_context();
        section.begin = read_uint64();
        section.translation_count = read_uint64();
    }
}

void translation_parser::init()
{
    read_header();
    read_sections();
}

translator::translator(const std::filesystem::path& path, translator_options options)
:m_options{options}
{
    translation_parser parser{path};
    parse(parser);
}

translator::translator(const std::string_view& data, translator_options options)
:m_options{options}
{
    translation_parser parser{data};
    parse(parser);
}

translator::translator(std::istream& stream, translator_options options)
:m_options{options}
{
    translation_parser parser{stream};
    parse(parser);
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

void translator::parse(translation_parser& parser)
{
    m_version = parser.version();
    m_source_language = parser.source_language();
    m_source_country = parser.source_country();
    m_target_language = parser.target_language();
    m_target_country = parser.target_country();
    m_section_count = parser.section_count();
    m_translation_count = parser.translation_count();

    m_sections.reserve(m_section_count);

    for(auto section{parser.current_section()}; section; section = parser.next_section())
    {
        translation_set_type translations{};
        translations.reserve(section->translation_count);

        for(auto translation{parser.next_translation(translation_parser_load::target_text)}; translation; translation = parser.next_translation(translation_parser_load::target_text))
        {
            translations.emplace(std::make_pair(translation->source_hash, std::move(translation->target)));
        }

        m_sections.emplace(std::make_pair(hash_value(section->context), std::move(translations)));
    }
}

static char* write_uint16(char* output, std::uint16_t value)
{
    if constexpr(std::endian::native == std::endian::big)
    {
        value = bswap(value);
    }

    std::memcpy(output, &value, sizeof(std::uint16_t));

    return output + sizeof(std::uint16_t);
}


static char* write_uint32(char* output, std::uint32_t value)
{
    if constexpr(std::endian::native == std::endian::big)
    {
        value = bswap(value);
    }

    std::memcpy(output, &value, sizeof(std::uint32_t));

    return output + sizeof(std::uint32_t);
}

static char* write_uint64(char* output, std::uint64_t value)
{
    if constexpr(std::endian::native == std::endian::big)
    {
        value = bswap(value);
    }

    std::memcpy(output, &value, sizeof(std::uint64_t));

    return output + sizeof(std::uint64_t);
}

static char* write_magic_word(char* output)
{
    std::memcpy(output, std::data(translation_magic_word), std::size(translation_magic_word));

    return output + std::size(translation_magic_word);
}

static char* write_context(char* output, const translation_context_t& context)
{
    std::memcpy(output, std::data(context), std::size(context));

    return output + std::size(context);
}

translation_editor::translation_editor(cpt::language source_language, cpt::country source_country, cpt::language target_language, cpt::country target_country)
:m_version{last_translation_version}
,m_source_language{source_language}
,m_source_country{source_country}
,m_target_language{target_language}
,m_target_country{target_country}
{

}

translation_editor::translation_editor(const std::filesystem::path& path)
{
    translation_parser parser{path};
    parse(parser);
}

translation_editor::translation_editor(const std::string_view& data)
{
    translation_parser parser{data};
    parse(parser);
}

translation_editor::translation_editor(std::istream& stream)
{
    translation_parser parser{stream};
    parse(parser);
}

bool translation_editor::add(const translation_context_t& context)
{
    return m_sections.emplace(context, translation_set_type{}).second;
}

bool translation_editor::add(std::string source_text, std::string target_text, const translation_context_t& context)
{
    return m_sections[context].emplace(std::move(source_text), std::move(target_text)).second;
}

bool translation_editor::replace(const translation_context_t& context)
{
    const auto it{m_sections.find(context)};
    if(it != std::end(m_sections))
    {
        it->second.clear();

        return true;
    }

    return false;
}

bool translation_editor::replace(const std::string& source_text, std::string target_text, const translation_context_t& context)
{
    const auto section{m_sections.find(context)};
    if(section != std::end(m_sections))
    {
        const auto translation{section->second.find(source_text)};

        if(translation != std::end(section->second))
        {
            translation->second = std::move(target_text);

            return true;
        }
    }

    return false;
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
    const std::size_t bound{file_bound()};

    std::string output{};
    output.reserve(bound);

    output += encode_file_information();
    output += encode_header_information();
    output += encode_section_informations(std::size(output), bound - std::size(output));

    return output;
}

cpt::version translation_editor::set_minimum_version(cpt::version requested)
{
    for(auto version : translation_versions)
    {
        if(version >= requested)
        {
            m_version = version;
            return m_version;
        }
    }

    m_version = last_translation_version;
    return m_version;
}

void translation_editor::parse(translation_parser& parser)
{
    m_version = parser.version();
    m_source_language = parser.source_language();
    m_source_country = parser.source_country();
    m_target_language = parser.target_language();
    m_target_country = parser.target_country();

    m_sections.reserve(parser.section_count());

    for(auto section{parser.current_section()}; section; section = parser.next_section())
    {
        translation_set_type translations{};
        translations.reserve(section->translation_count);

        for(auto translation{parser.next_translation()}; translation; translation = parser.next_translation())
        {
            translations.emplace(std::make_pair(std::move(translation->source), std::move(translation->target)));
        }

        m_sections.emplace(std::make_pair(section->context, std::move(translations)));
    }
}

std::size_t translation_editor::file_bound() const
{
    std::size_t output{};

    output += translation_parser::file_information_size;
    output += translation_parser::header_information_size;
    output += translation_parser::section_information_size * section_count();

    for(auto&& [context, translations] : m_sections)
    {
        for(auto&& [source, target] : translations)
        {
            output += sizeof(std::uint64_t) * 3;
            output += std::size(source);
            output += std::size(target);
        }
    }

    return output;
}

std::string translation_editor::encode_file_information() const
{
    std::string output{};
    output.resize(translation_parser::file_information_size);

    char* output_it{std::data(output)};

    output_it = write_magic_word(output_it);
    output_it = write_uint16(output_it, m_version.major);
    output_it = write_uint16(output_it, m_version.minor);
    output_it = write_uint32(output_it, m_version.patch);

    return output;
}

std::string translation_editor::encode_header_information() const
{
    std::string output{};
    output.resize(translation_parser::header_information_size);

    char* output_it{std::data(output)};

    output_it = write_uint32(output_it, static_cast<std::uint32_t>(m_source_language));
    output_it = write_uint32(output_it, static_cast<std::uint32_t>(m_source_country));
    output_it = write_uint32(output_it, static_cast<std::uint32_t>(m_target_language));
    output_it = write_uint32(output_it, static_cast<std::uint32_t>(m_target_country));
    output_it = write_uint64(output_it, section_count());
    output_it = write_uint64(output_it, translation_count());

    return output;
}

std::string translation_editor::encode_section_informations(std::size_t begin, std::size_t bound) const
{
    if(std::empty(m_sections))
    {
        return {};
    }

    const std::size_t sections_total_size{std::size(m_sections) * translation_parser::section_information_size};

    std::string output{};
    output.reserve(bound); //Prevent reallocation
    output.resize(sections_total_size); //Skip sections informations space

    std::size_t current_begin{begin + sections_total_size};
    char* section_info_it{std::data(output)};

    for(auto&& [context, translations] : m_sections)
    {
        section_info_it = write_context(section_info_it, context);
        section_info_it = write_uint64 (section_info_it, current_begin);
        section_info_it = write_uint64 (section_info_it, static_cast<std::uint64_t>(std::size(translations)));

        const std::string translations_data{encode_section(translations)};
        current_begin += std::size(translations_data);
        output += translations_data;
    }

    return output;
}

std::string translation_editor::encode_section(const translation_set_type& translations) const
{
    std::size_t total_size{};
    for(auto&& [source, target] : translations)
    {
        total_size += sizeof(std::uint64_t) * 3;
        total_size += std::size(source);
        total_size += std::size(target);
    }

    std::string output{};
    output.reserve(total_size);

    for(auto&& [source, target] : translations)
    {
        output += encode_translation(source, target);
    }

    return output;
}

std::string translation_editor::encode_translation(const std::string& source, const std::string& target) const
{
    std::string output{};
    output.reserve(sizeof(std::uint64_t) * 3 + std::size(source) + std::size(target));
    output.resize(sizeof(std::uint64_t) * 3);

    char* output_it{std::data(output)};
    output_it = write_uint64(output_it, hash_value(source));
    output_it = write_uint64(output_it, static_cast<std::uint64_t>(std::size(source)));
    output_it = write_uint64(output_it, static_cast<std::uint64_t>(std::size(target)));

    output += source;
    output += target;

    return output;
}

std::string_view translate(const std::string_view& string, const translation_context_t& context, translate_options options)
{
    return engine::cinstance().translator().translate(string, context, options);
}

}
