#include "translation.hpp"

#include <fstream>
#include <cstring>

#include <nes/hash.hpp>

#include "encoding.hpp"
#include "algorithm.hpp"

namespace cpt
{

translator::translator(const std::filesystem::path& path, translator_options options [[maybe_unused]])
{
    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

    m_source = std::move(ifs);
}

translator::translator(const std::string_view& data, translator_options options [[maybe_unused]])
{
    m_source = data;
}

translator::translator(std::istream& stream, translator_options options [[maybe_unused]])
{
    m_source = std::ref(stream);
}

void translator::read_version()
{
    if(std::holds_alternative<std::ifstream>(m_source))
    {
        std::ifstream& ifs{std::get<std::ifstream>(m_source)};

        ifs.read(reinterpret_cast<char*>(&m_file_format), sizeof(file_format));
        if(static_cast<std::size_t>(ifs.gcount()) != sizeof(file_format))
            throw std::runtime_error{"Bad file content."};
    }
    else if(std::holds_alternative<std::string_view>(m_source))
    {
        const std::string_view& view{std::get<std::string_view>(m_source)};

        if(std::size(view) < sizeof(file_format))
            throw std::runtime_error{"Bad file content."};

        std::memcpy(&m_file_format, std::data(view), sizeof(file_format));
    }
    else if(std::holds_alternative<std::reference_wrapper<std::istream>>(m_source))
    {
        std::istream& is{std::get<std::reference_wrapper<std::istream>>(m_source).get()};

        is.read(reinterpret_cast<char*>(&m_file_format), sizeof(file_format));
        if(static_cast<std::size_t>(is.gcount()) != sizeof(file_format))
            throw std::runtime_error{"Bad file content."};
    }

    if(m_file_format.magic_word != translation_magic_word)
        throw std::runtime_error{"Bad file format."};

    if constexpr(endian::native == endian::big)
    {
        m_file_format.version = tph::to_version(bswap(tph::from_version(m_file_format.version)));
    }

    if(m_file_format.version != tph::version{1, 0, 0})
        throw std::runtime_error{"Bad file version."};
}

}
