//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#ifndef TEPHRA_CONFIG_HPP_INCLUDED
#define TEPHRA_CONFIG_HPP_INCLUDED

#include <captal_foundation/base.hpp>
#include <captal_foundation/version.hpp>
#include <captal_foundation/enum_operations.hpp>
#include <captal_foundation/optional_ref.hpp>

#ifdef _WIN32
    #if defined(TEPHRA_SHARED_BUILD)
        #define TEPHRA_API __declspec(dllexport)
    #elif !defined(TEPHRA_STATIC_BUILD)
        #define TEPHRA_API __declspec(dllimport)
    #else
        #define TEPHRA_API
    #endif
#else
    #define TEPHRA_API
#endif

#include <variant>

namespace tph
{

using namespace cpt::foundation;

template<typename VulkanObject, typename... Args>
VulkanObject underlying_cast(const Args&...) noexcept
{
    //Declare a compatible type:
    //    template<typename VulkanObject, typename... Args>
    //    friend VulkanObject underlying_cast(const Args&...) noexcept;

    static_assert(!std::is_same<VulkanObject, VulkanObject>::value,
        "tph::underlying_cast called with incompatible arguments. (A common mistake is calling a valid underlying_cast, but without including the header where it is declared.)");
}

using bool32_t = std::uint32_t;

}

#endif
