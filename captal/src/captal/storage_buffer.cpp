#include "storage_buffer.hpp"

#include "engine.hpp"

namespace cpt
{

storage_buffer::storage_buffer(std::uint64_t size, tph::buffer_usage usage)
:m_buffer{engine::instance().renderer(), size, usage | tph::buffer_usage::storage | tph::buffer_usage::device_only}
{

}

#ifdef CAPTAL_DEBUG
void storage_buffer::set_name(std::string_view name)
{
    tph::set_object_name(engine::instance().renderer(), m_buffer, std::string{name});
}
#endif

}
