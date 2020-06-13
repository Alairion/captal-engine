#ifndef CAPTAL_COLOR_HPP_INCLUDED
#define CAPTAL_COLOR_HPP_INCLUDED

#include "config.hpp"

#include <cmath>

#include <tephra/image.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace cpt
{

struct color
{
    constexpr color() noexcept = default;

    template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    constexpr color(T r, T g, T b, T a = static_cast<T>(1)) noexcept
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

inline color hsv_to_rgb(float hue, float saturation, float value, float alpha = 1.0f) noexcept
{
    assert((0.0f <= hue && hue <= 360.0f) && "HSV value out of range.");
    assert((0.0f <= saturation && saturation <= 1.0f) && "HSV value out of range.");
    assert((0.0f <= value && value <= 1.0f) && "HSV value out of range.");

    const float chroma{value * saturation};
    const float prime{hue / 60.0f};
    const float discriminant{chroma * (1.0f - std::fabs(std::fmod(prime, 2.0f) - 1.0f))};
    const float remainder{value - chroma};

    const auto convert = [prime, chroma, discriminant, remainder, alpha]
    {
        switch (static_cast<std::int32_t>(prime))
        {
            case 0:  return color{chroma + remainder, discriminant + remainder, remainder, alpha};
            case 1:  return color{discriminant + remainder, chroma + remainder, remainder, alpha};
            case 2:  return color{remainder, chroma + remainder, discriminant + remainder, alpha};
            case 3:  return color{remainder, discriminant + remainder, chroma + remainder, alpha};
            case 4:  return color{discriminant + remainder, remainder, chroma + remainder, alpha};
            case 5:  return color{chroma + remainder, remainder, discriminant + remainder, alpha};
            default: return color{0.0f, 0.0f, 0.0f, alpha};
        }

    };

    return convert();
}

constexpr color gradient(const color& first, const color& second, float advance) noexcept
{
    assert((0.0f <= advance && advance <= 1.0f) && "advance must be in the range [0; 1].");

    const auto compute = [advance](float first, float second) -> float
    {
        if(first < second)
            return first + (second - first) * advance;

        return first - (first - second) * advance;
    };

    color output{};
    output.red = compute(first.red, second.red);
    output.green = compute(first.green, second.green);
    output.blue = compute(first.blue, second.blue);
    output.alpha = compute(first.alpha, second.alpha);

    return output;
}

namespace colors
{

inline constexpr color aliceblue{0.941f, 0.973f, 1.000f};
inline constexpr color antiquewhite{0.980f, 0.922f, 0.843f};
inline constexpr color aqua{0.000f, 1.000f, 1.000f};
inline constexpr color aquamarine{0.498f, 1.000f, 0.831f};
inline constexpr color azure{0.941f, 1.000f, 1.000f};
inline constexpr color beige{0.961f, 0.961f, 0.863f};
inline constexpr color bisque{1.000f, 0.894f, 0.769f};
inline constexpr color black{0.000f, 0.000f, 0.000f};
inline constexpr color blanchedalmond{1.000f, 0.922f, 0.804f};
inline constexpr color blue{0.000f, 0.000f, 1.000f};
inline constexpr color blueviolet{0.541f, 0.169f, 0.886f};
inline constexpr color brown{0.647f, 0.165f, 0.165f};
inline constexpr color burlywood{0.871f, 0.722f, 0.529f};
inline constexpr color cadetblue{0.373f, 0.620f, 0.627f};
inline constexpr color chartreuse{0.498f, 1.000f, 0.000f};
inline constexpr color chocolate{0.824f, 0.412f, 0.118f};
inline constexpr color coral{1.000f, 0.498f, 0.314f};
inline constexpr color cornflowerblue{0.392f, 0.584f, 0.929f};
inline constexpr color cornsilk{1.000f, 0.973f, 0.863f};
inline constexpr color crimson{0.863f, 0.078f, 0.235f};
inline constexpr color cyan{0.000f, 1.000f, 1.000f};
inline constexpr color darkblue{0.000f, 0.000f, 0.545f};
inline constexpr color darkcyan{0.000f, 0.545f, 0.545f};
inline constexpr color darkgoldenrod{0.722f, 0.525f, 0.043f};
inline constexpr color darkgray{0.663f, 0.663f, 0.663f};
inline constexpr color darkgrey{0.663f, 0.663f, 0.663f};
inline constexpr color darkgreen{0.000f, 0.392f, 0.000f};
inline constexpr color darkkhaki{0.741f, 0.718f, 0.420f};
inline constexpr color darkmagenta{0.545f, 0.000f, 0.545f};
inline constexpr color darkolivegreen{0.333f, 0.420f, 0.184f};
inline constexpr color darkorange{1.000f, 0.549f, 0.000f};
inline constexpr color darkorchid{0.600f, 0.196f, 0.800f};
inline constexpr color darkred{0.545f, 0.000f, 0.000f};
inline constexpr color darksalmon{0.914f, 0.588f, 0.478f};
inline constexpr color darkseagreen{0.561f, 0.737f, 0.561f};
inline constexpr color darkslateblue{0.282f, 0.239f, 0.545f};
inline constexpr color darkslategray{0.184f, 0.310f, 0.310f};
inline constexpr color darkslategrey{0.184f, 0.310f, 0.310f};
inline constexpr color darkturquoise{0.000f, 0.808f, 0.820f};
inline constexpr color darkviolet{0.580f, 0.000f, 0.827f};
inline constexpr color deeppink{1.000f, 0.078f, 0.576f};
inline constexpr color deepskyblue{0.000f, 0.749f, 1.000f};
inline constexpr color dimgray{0.412f, 0.412f, 0.412f};
inline constexpr color dimgrey{0.412f, 0.412f, 0.412f};
inline constexpr color dodgerblue{0.118f, 0.565f, 1.000f};
inline constexpr color firebrick{0.698f, 0.133f, 0.133f};
inline constexpr color floralwhite{1.000f, 0.980f, 0.941f};
inline constexpr color forestgreen{0.133f, 0.545f, 0.133f};
inline constexpr color fuchsia{1.000f, 0.000f, 1.000f};
inline constexpr color gainsboro{0.863f, 0.863f, 0.863f};
inline constexpr color ghostwhite{0.973f, 0.973f, 1.000f};
inline constexpr color gold{1.000f, 0.843f, 0.000f};
inline constexpr color goldenrod{0.855f, 0.647f, 0.125f};
inline constexpr color gray{0.502f, 0.502f, 0.502f};
inline constexpr color grey{0.502f, 0.502f, 0.502f};
inline constexpr color green{0.000f, 0.502f, 0.000f};
inline constexpr color greenyellow{0.678f, 1.000f, 0.184f};
inline constexpr color honeydew{0.941f, 1.000f, 0.941f};
inline constexpr color hotpink{1.000f, 0.412f, 0.706f};
inline constexpr color indianred{0.804f, 0.361f, 0.361f};
inline constexpr color indigo{0.294f, 0.000f, 0.510f};
inline constexpr color ivory{1.000f, 1.000f, 0.941f};
inline constexpr color khaki{0.941f, 0.902f, 0.549f};
inline constexpr color lavender{0.902f, 0.902f, 0.980f};
inline constexpr color lavenderblush{1.000f, 0.941f, 0.961f};
inline constexpr color lawngreen{0.486f, 0.988f, 0.000f};
inline constexpr color lemonchiffon{1.000f, 0.980f, 0.804f};
inline constexpr color lightblue{0.678f, 0.847f, 0.902f};
inline constexpr color lightcoral{0.941f, 0.502f, 0.502f};
inline constexpr color lightcyan{0.878f, 1.000f, 1.000f};
inline constexpr color lightgoldenrodyellow{0.980f, 0.980f, 0.824f};
inline constexpr color lightgray{0.827f, 0.827f, 0.827f};
inline constexpr color lightgrey{0.827f, 0.827f, 0.827f};
inline constexpr color lightgreen{0.565f, 0.933f, 0.565f};
inline constexpr color lightpink{1.000f, 0.714f, 0.757f};
inline constexpr color lightsalmon{1.000f, 0.627f, 0.478f};
inline constexpr color lightseagreen{0.125f, 0.698f, 0.667f};
inline constexpr color lightskyblue{0.529f, 0.808f, 0.980f};
inline constexpr color lightslategray{0.467f, 0.533f, 0.600f};
inline constexpr color lightslategrey{0.467f, 0.533f, 0.600f};
inline constexpr color lightsteelblue{0.690f, 0.769f, 0.871f};
inline constexpr color lightyellow{1.000f, 1.000f, 0.878f};
inline constexpr color lime{0.000f, 1.000f, 0.000f};
inline constexpr color limegreen{0.196f, 0.804f, 0.196f};
inline constexpr color linen{0.980f, 0.941f, 0.902f};
inline constexpr color magenta{1.000f, 0.000f, 1.000f};
inline constexpr color maroon{0.502f, 0.000f, 0.000f};
inline constexpr color mediumaquamarine{0.400f, 0.804f, 0.667f};
inline constexpr color mediumblue{0.000f, 0.000f, 0.804f};
inline constexpr color mediumorchid{0.729f, 0.333f, 0.827f};
inline constexpr color mediumpurple{0.576f, 0.439f, 0.859f};
inline constexpr color mediumseagreen{0.235f, 0.702f, 0.443f};
inline constexpr color mediumslateblue{0.482f, 0.408f, 0.933f};
inline constexpr color mediumspringgreen{0.000f, 0.980f, 0.604f};
inline constexpr color mediumturquoise{0.282f, 0.820f, 0.800f};
inline constexpr color mediumvioletred{0.780f, 0.082f, 0.522f};
inline constexpr color midnightblue{0.098f, 0.098f, 0.439f};
inline constexpr color mintcream{0.961f, 1.000f, 0.980f};
inline constexpr color mistyrose{1.000f, 0.894f, 0.882f};
inline constexpr color moccasin{1.000f, 0.894f, 0.710f};
inline constexpr color navajowhite{1.000f, 0.871f, 0.678f};
inline constexpr color navy{0.000f, 0.000f, 0.502f};
inline constexpr color oldlace{0.992f, 0.961f, 0.902f};
inline constexpr color olive{0.502f, 0.502f, 0.000f};
inline constexpr color olivedrab{0.420f, 0.557f, 0.137f};
inline constexpr color orange{1.000f, 0.647f, 0.000f};
inline constexpr color orangered{1.000f, 0.271f, 0.000f};
inline constexpr color orchid{0.855f, 0.439f, 0.839f};
inline constexpr color palegoldenrod{0.933f, 0.910f, 0.667f};
inline constexpr color palegreen{0.596f, 0.984f, 0.596f};
inline constexpr color paleturquoise{0.686f, 0.933f, 0.933f};
inline constexpr color palevioletred{0.859f, 0.439f, 0.576f};
inline constexpr color papayawhip{1.000f, 0.937f, 0.835f};
inline constexpr color peachpuff{1.000f, 0.855f, 0.725f};
inline constexpr color peru{0.804f, 0.522f, 0.247f};
inline constexpr color pink{1.000f, 0.753f, 0.796f};
inline constexpr color plum{0.867f, 0.627f, 0.867f};
inline constexpr color powderblue{0.690f, 0.878f, 0.902f};
inline constexpr color purple{0.502f, 0.000f, 0.502f};
inline constexpr color rebeccapurple{0.400f, 0.200f, 0.600f};
inline constexpr color red{1.000f, 0.000f, 0.000f};
inline constexpr color rosybrown{0.737f, 0.561f, 0.561f};
inline constexpr color royalblue{0.255f, 0.412f, 0.882f};
inline constexpr color saddlebrown{0.545f, 0.271f, 0.075f};
inline constexpr color salmon{0.980f, 0.502f, 0.447f};
inline constexpr color sandybrown{0.957f, 0.643f, 0.376f};
inline constexpr color seagreen{0.180f, 0.545f, 0.341f};
inline constexpr color seashell{1.000f, 0.961f, 0.933f};
inline constexpr color sienna{0.627f, 0.322f, 0.176f};
inline constexpr color silver{0.753f, 0.753f, 0.753f};
inline constexpr color skyblue{0.529f, 0.808f, 0.922f};
inline constexpr color slateblue{0.416f, 0.353f, 0.804f};
inline constexpr color slategray{0.439f, 0.502f, 0.565f};
inline constexpr color slategrey{0.439f, 0.502f, 0.565f};
inline constexpr color snow{1.000f, 0.980f, 0.980f};
inline constexpr color springgreen{0.000f, 1.000f, 0.498f};
inline constexpr color steelblue{0.275f, 0.510f, 0.706f};
inline constexpr color tan{0.824f, 0.706f, 0.549f};
inline constexpr color teal{0.000f, 0.502f, 0.502f};
inline constexpr color thistle{0.847f, 0.749f, 0.847f};
inline constexpr color tomato{1.000f, 0.388f, 0.278f};
inline constexpr color transparent{0.0f, 0.0f, 0.0f, 0.0f};
inline constexpr color turquoise{0.251f, 0.878f, 0.816f};
inline constexpr color violet{0.933f, 0.510f, 0.933f};
inline constexpr color wheat{0.961f, 0.871f, 0.702f};
inline constexpr color white{1.000f, 1.000f, 1.000f};
inline constexpr color whitesmoke{0.961f, 0.961f, 0.961f};
inline constexpr color yellow{1.000f, 1.000f, 0.000f};
inline constexpr color yellowgreen{0.604f, 0.804f, 0.196f};

}

}

#endif
