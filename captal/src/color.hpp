#ifndef CAPTAL_COLOR_HPP_INCLUDED
#define CAPTAL_COLOR_HPP_INCLUDED

#include "config.hpp"

#include <tephra/image.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace cpt
{

struct color
{
    constexpr color() noexcept = default;

    template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    constexpr color(T r, T g, T b, T a = static_cast<T>(1.0)) noexcept
    :red{static_cast<float>(r)}
    ,green{static_cast<float>(g)}
    ,blue{static_cast<float>(b)}
    ,alpha{static_cast<float>(a)}
    {

    }

    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    constexpr color(T r, T g, T b, T a = static_cast<T>(255)) noexcept
    :color{static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f}
    {

    }

    constexpr color(const glm::vec4& value) noexcept
    :color{value.r, value.g, value.b, value.a}
    {

    }

    constexpr color(const tph::pixel& pixel) noexcept
    :color{pixel.red, pixel.blue, pixel.green, pixel.alpha}
    {

    }

    constexpr color(std::uint32_t rgba_value) noexcept
    :color{static_cast<std::uint8_t>(rgba_value >> 16), static_cast<std::uint8_t>(rgba_value >> 8), static_cast<std::uint8_t>(rgba_value), static_cast<std::uint8_t>(rgba_value >> 24)}
    {

    }

    constexpr explicit operator glm::vec4() const noexcept
    {
        return glm::vec4{glm::vec3{red, green, blue}, alpha};
    }

    constexpr explicit operator tph::pixel() const noexcept
    {
        return tph::pixel{static_cast<std::uint8_t>(red * 255.0f), static_cast<std::uint8_t>(green * 255.0f), static_cast<std::uint8_t>(blue * 255.0f), static_cast<std::uint8_t>(alpha * 255.0f)};
    }

    float red{};
    float green{};
    float blue{};
    float alpha{};
};

namespace colors
{

static constexpr color aliceblue{0.941f, 0.973f, 1.000f};
static constexpr color antiquewhite{0.980f, 0.922f, 0.843f};
static constexpr color aqua{0.000f, 1.000f, 1.000f};
static constexpr color aquamarine{0.498f, 1.000f, 0.831f};
static constexpr color azure{0.941f, 1.000f, 1.000f};
static constexpr color beige{0.961f, 0.961f, 0.863f};
static constexpr color bisque{1.000f, 0.894f, 0.769f};
static constexpr color black{0.000f, 0.000f, 0.000f};
static constexpr color blanchedalmond{1.000f, 0.922f, 0.804f};
static constexpr color blue{0.000f, 0.000f, 1.000f};
static constexpr color blueviolet{0.541f, 0.169f, 0.886f};
static constexpr color brown{0.647f, 0.165f, 0.165f};
static constexpr color burlywood{0.871f, 0.722f, 0.529f};
static constexpr color cadetblue{0.373f, 0.620f, 0.627f};
static constexpr color chartreuse{0.498f, 1.000f, 0.000f};
static constexpr color chocolate{0.824f, 0.412f, 0.118f};
static constexpr color coral{1.000f, 0.498f, 0.314f};
static constexpr color cornflowerblue{0.392f, 0.584f, 0.929f};
static constexpr color cornsilk{1.000f, 0.973f, 0.863f};
static constexpr color crimson{0.863f, 0.078f, 0.235f};
static constexpr color cyan{0.000f, 1.000f, 1.000f};
static constexpr color darkblue{0.000f, 0.000f, 0.545f};
static constexpr color darkcyan{0.000f, 0.545f, 0.545f};
static constexpr color darkgoldenrod{0.722f, 0.525f, 0.043f};
static constexpr color darkgray{0.663f, 0.663f, 0.663f};
static constexpr color darkgrey{0.663f, 0.663f, 0.663f};
static constexpr color darkgreen{0.000f, 0.392f, 0.000f};
static constexpr color darkkhaki{0.741f, 0.718f, 0.420f};
static constexpr color darkmagenta{0.545f, 0.000f, 0.545f};
static constexpr color darkolivegreen{0.333f, 0.420f, 0.184f};
static constexpr color darkorange{1.000f, 0.549f, 0.000f};
static constexpr color darkorchid{0.600f, 0.196f, 0.800f};
static constexpr color darkred{0.545f, 0.000f, 0.000f};
static constexpr color darksalmon{0.914f, 0.588f, 0.478f};
static constexpr color darkseagreen{0.561f, 0.737f, 0.561f};
static constexpr color darkslateblue{0.282f, 0.239f, 0.545f};
static constexpr color darkslategray{0.184f, 0.310f, 0.310f};
static constexpr color darkslategrey{0.184f, 0.310f, 0.310f};
static constexpr color darkturquoise{0.000f, 0.808f, 0.820f};
static constexpr color darkviolet{0.580f, 0.000f, 0.827f};
static constexpr color deeppink{1.000f, 0.078f, 0.576f};
static constexpr color deepskyblue{0.000f, 0.749f, 1.000f};
static constexpr color dimgray{0.412f, 0.412f, 0.412f};
static constexpr color dimgrey{0.412f, 0.412f, 0.412f};
static constexpr color dodgerblue{0.118f, 0.565f, 1.000f};
static constexpr color firebrick{0.698f, 0.133f, 0.133f};
static constexpr color floralwhite{1.000f, 0.980f, 0.941f};
static constexpr color forestgreen{0.133f, 0.545f, 0.133f};
static constexpr color fuchsia{1.000f, 0.000f, 1.000f};
static constexpr color gainsboro{0.863f, 0.863f, 0.863f};
static constexpr color ghostwhite{0.973f, 0.973f, 1.000f};
static constexpr color gold{1.000f, 0.843f, 0.000f};
static constexpr color goldenrod{0.855f, 0.647f, 0.125f};
static constexpr color gray{0.502f, 0.502f, 0.502f};
static constexpr color grey{0.502f, 0.502f, 0.502f};
static constexpr color green{0.000f, 0.502f, 0.000f};
static constexpr color greenyellow{0.678f, 1.000f, 0.184f};
static constexpr color honeydew{0.941f, 1.000f, 0.941f};
static constexpr color hotpink{1.000f, 0.412f, 0.706f};
static constexpr color indianred{0.804f, 0.361f, 0.361f};
static constexpr color indigo{0.294f, 0.000f, 0.510f};
static constexpr color ivory{1.000f, 1.000f, 0.941f};
static constexpr color khaki{0.941f, 0.902f, 0.549f};
static constexpr color lavender{0.902f, 0.902f, 0.980f};
static constexpr color lavenderblush{1.000f, 0.941f, 0.961f};
static constexpr color lawngreen{0.486f, 0.988f, 0.000f};
static constexpr color lemonchiffon{1.000f, 0.980f, 0.804f};
static constexpr color lightblue{0.678f, 0.847f, 0.902f};
static constexpr color lightcoral{0.941f, 0.502f, 0.502f};
static constexpr color lightcyan{0.878f, 1.000f, 1.000f};
static constexpr color lightgoldenrodyellow{0.980f, 0.980f, 0.824f};
static constexpr color lightgray{0.827f, 0.827f, 0.827f};
static constexpr color lightgrey{0.827f, 0.827f, 0.827f};
static constexpr color lightgreen{0.565f, 0.933f, 0.565f};
static constexpr color lightpink{1.000f, 0.714f, 0.757f};
static constexpr color lightsalmon{1.000f, 0.627f, 0.478f};
static constexpr color lightseagreen{0.125f, 0.698f, 0.667f};
static constexpr color lightskyblue{0.529f, 0.808f, 0.980f};
static constexpr color lightslategray{0.467f, 0.533f, 0.600f};
static constexpr color lightslategrey{0.467f, 0.533f, 0.600f};
static constexpr color lightsteelblue{0.690f, 0.769f, 0.871f};
static constexpr color lightyellow{1.000f, 1.000f, 0.878f};
static constexpr color lime{0.000f, 1.000f, 0.000f};
static constexpr color limegreen{0.196f, 0.804f, 0.196f};
static constexpr color linen{0.980f, 0.941f, 0.902f};
static constexpr color magenta{1.000f, 0.000f, 1.000f};
static constexpr color maroon{0.502f, 0.000f, 0.000f};
static constexpr color mediumaquamarine{0.400f, 0.804f, 0.667f};
static constexpr color mediumblue{0.000f, 0.000f, 0.804f};
static constexpr color mediumorchid{0.729f, 0.333f, 0.827f};
static constexpr color mediumpurple{0.576f, 0.439f, 0.859f};
static constexpr color mediumseagreen{0.235f, 0.702f, 0.443f};
static constexpr color mediumslateblue{0.482f, 0.408f, 0.933f};
static constexpr color mediumspringgreen{0.000f, 0.980f, 0.604f};
static constexpr color mediumturquoise{0.282f, 0.820f, 0.800f};
static constexpr color mediumvioletred{0.780f, 0.082f, 0.522f};
static constexpr color midnightblue{0.098f, 0.098f, 0.439f};
static constexpr color mintcream{0.961f, 1.000f, 0.980f};
static constexpr color mistyrose{1.000f, 0.894f, 0.882f};
static constexpr color moccasin{1.000f, 0.894f, 0.710f};
static constexpr color navajowhite{1.000f, 0.871f, 0.678f};
static constexpr color navy{0.000f, 0.000f, 0.502f};
static constexpr color oldlace{0.992f, 0.961f, 0.902f};
static constexpr color olive{0.502f, 0.502f, 0.000f};
static constexpr color olivedrab{0.420f, 0.557f, 0.137f};
static constexpr color orange{1.000f, 0.647f, 0.000f};
static constexpr color orangered{1.000f, 0.271f, 0.000f};
static constexpr color orchid{0.855f, 0.439f, 0.839f};
static constexpr color palegoldenrod{0.933f, 0.910f, 0.667f};
static constexpr color palegreen{0.596f, 0.984f, 0.596f};
static constexpr color paleturquoise{0.686f, 0.933f, 0.933f};
static constexpr color palevioletred{0.859f, 0.439f, 0.576f};
static constexpr color papayawhip{1.000f, 0.937f, 0.835f};
static constexpr color peachpuff{1.000f, 0.855f, 0.725f};
static constexpr color peru{0.804f, 0.522f, 0.247f};
static constexpr color pink{1.000f, 0.753f, 0.796f};
static constexpr color plum{0.867f, 0.627f, 0.867f};
static constexpr color powderblue{0.690f, 0.878f, 0.902f};
static constexpr color purple{0.502f, 0.000f, 0.502f};
static constexpr color rebeccapurple{0.400f, 0.200f, 0.600f};
static constexpr color red{1.000f, 0.000f, 0.000f};
static constexpr color rosybrown{0.737f, 0.561f, 0.561f};
static constexpr color royalblue{0.255f, 0.412f, 0.882f};
static constexpr color saddlebrown{0.545f, 0.271f, 0.075f};
static constexpr color salmon{0.980f, 0.502f, 0.447f};
static constexpr color sandybrown{0.957f, 0.643f, 0.376f};
static constexpr color seagreen{0.180f, 0.545f, 0.341f};
static constexpr color seashell{1.000f, 0.961f, 0.933f};
static constexpr color sienna{0.627f, 0.322f, 0.176f};
static constexpr color silver{0.753f, 0.753f, 0.753f};
static constexpr color skyblue{0.529f, 0.808f, 0.922f};
static constexpr color slateblue{0.416f, 0.353f, 0.804f};
static constexpr color slategray{0.439f, 0.502f, 0.565f};
static constexpr color slategrey{0.439f, 0.502f, 0.565f};
static constexpr color snow{1.000f, 0.980f, 0.980f};
static constexpr color springgreen{0.000f, 1.000f, 0.498f};
static constexpr color steelblue{0.275f, 0.510f, 0.706f};
static constexpr color tan{0.824f, 0.706f, 0.549f};
static constexpr color teal{0.000f, 0.502f, 0.502f};
static constexpr color thistle{0.847f, 0.749f, 0.847f};
static constexpr color tomato{1.000f, 0.388f, 0.278f};
static constexpr color turquoise{0.251f, 0.878f, 0.816f};
static constexpr color violet{0.933f, 0.510f, 0.933f};
static constexpr color wheat{0.961f, 0.871f, 0.702f};
static constexpr color white{1.000f, 1.000f, 1.000f};
static constexpr color whitesmoke{0.961f, 0.961f, 0.961f};
static constexpr color yellow{1.000f, 1.000f, 0.000f};
static constexpr color yellowgreen{0.604f, 0.804f, 0.196f};

}

}

#endif
