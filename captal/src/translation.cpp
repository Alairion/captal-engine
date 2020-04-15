#include "translation.hpp"

#include <fstream>

#include <nes/hash.hpp>

#include "encoding.hpp"

namespace cpt
{

translator::translator(const std::filesystem::path& path, translator_options options)
{
    if(static_cast<bool>(options & translator_options::buffered))
    {
        std::ifstream ifs{path, std::ios_base::binary};
        if(!ifs)
            throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

        m_source = std::string{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};
    }
    else
    {
        std::ifstream ifs{path, std::ios_base::binary};
        if(!ifs)
            throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

        m_source = std::move(ifs);
    }
}

translator::translator(std::string_view data, translator_options options)
{
    if(static_cast<bool>(options & translator_options::buffered))
    {
        m_source = std::string{data};
    }
    else
    {
        m_source = data;
    }
}

translator::translator(std::istream& stream, translator_options options)
{
    if(static_cast<bool>(options & translator_options::buffered))
    {
        m_source = std::string{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    }
    else
    {
        m_source = std::ref(stream);
    }
}

void translator::read_header()
{
    if(std::holds_alternative<std::ifstream>(m_source))
    {
        std::ifstream& ifs{std::get<std::ifstream>(m_source)};

        ifs.read(reinterpret_cast<char*>(&m_header), sizeof(header));
        if(static_cast<std::size_t>(ifs.gcount()) != sizeof(header))
            throw std::runtime_error{"Bad file content."};


    }
    else if(std::holds_alternative<std::string_view>(m_source))
    {

    }
    else if(std::holds_alternative<std::reference_wrapper<std::istream>>(m_source))
    {

    }
    else if(std::holds_alternative<std::string>(m_source))
    {

    }
}

}
