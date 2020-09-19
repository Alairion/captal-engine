#ifndef SWELL_AUDIO_WORLD_HPP_INCLUDED
#define SWELL_AUDIO_WORLD_HPP_INCLUDED

#include "config.hpp"

#include <vector>

namespace swl
{

class listener
{

};

class audio_world
{
public:


private:
    std::vector<sound> m_sounds{};
    std::vector<listener> m_listeners{};
};

}

#endif
