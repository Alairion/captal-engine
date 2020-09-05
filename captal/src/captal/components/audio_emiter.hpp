#ifndef CAPTAL_COMPONENTS_AUDIO_EMITER_HPP_INCLUDED
#define CAPTAL_COMPONENTS_AUDIO_EMITER_HPP_INCLUDED

#include "../config.hpp"

#include "../sound.hpp"

#include "attachment.hpp"

namespace cpt::components
{

class audio_emiter : public impl::basic_attachement<sound>
{

};

}

#endif
