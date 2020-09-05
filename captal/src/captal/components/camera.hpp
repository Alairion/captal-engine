#ifndef CAPTAL_COMPONENTS_CAMERA_HPP_INCLUDED
#define CAPTAL_COMPONENTS_CAMERA_HPP_INCLUDED

#include "../config.hpp"

#include "../view.hpp"

#include "attachment.hpp"

namespace cpt::components
{

class camera : public impl::basic_attachement<view>
{

};

}

#endif
