#ifndef APYRE_INPUTS_HPP_INCLUDED
#define APYRE_INPUTS_HPP_INCLUDED

#include "config.hpp"

#include <string>

struct SDL_Cursor;

namespace apr
{

class application;
class window;

enum class scancode : std::uint32_t
{
    a = 4,
    b = 5,
    c = 6,
    d = 7,
    e = 8,
    f = 9,
    g = 10,
    h = 11,
    i = 12,
    j = 13,
    k = 14,
    l = 15,
    m = 16,
    n = 17,
    o = 18,
    p = 19,
    q = 20,
    r = 21,
    s = 22,
    t = 23,
    u = 24,
    v = 25,
    w = 26,
    x = 27,
    y = 28,
    z = 29,

    one = 30,
    two = 31,
    three = 32,
    four = 33,
    five = 34,
    six = 35,
    seven = 36,
    eight = 37,
    nine = 38,
    zero = 39,

    enter = 40,
    escape = 41,
    backspace = 42,
    tab = 43,
    space = 44,

    minus = 45,
    equals = 46,
    leftbracket = 47,
    rightbracket = 48,
    backslash = 49,
    nonushash = 50,
    semicolon = 51,
    apostrophe = 52,
    grave = 53,
    comma = 54,
    period = 55,
    slash = 56,

    capslock = 57,

    f1 = 58,
    f2 = 59,
    f3 = 60,
    f4 = 61,
    f5 = 62,
    f6 = 63,
    f7 = 64,
    f8 = 65,
    f9 = 66,
    f10 = 67,
    f11 = 68,
    f12 = 69,

    printscreen = 70,
    scrolllock = 71,
    pause = 72,
    insert = 73,

    home = 74,
    pageup = 75,
    del = 76,
    end = 77,
    pagedown = 78,
    right = 79,
    left = 80,
    down = 81,
    up = 82,

    numlock = 83,

    keypad_divide = 84,
    keypad_multiply = 85,
    keypad_minus = 86,
    keypad_plus = 87,
    keypad_enter = 88,
    keypad_1 = 89,
    keypad_2 = 90,
    keypad_3 = 91,
    keypad_4 = 92,
    keypad_5 = 93,
    keypad_6 = 94,
    keypad_7 = 95,
    keypad_8 = 96,
    keypad_9 = 97,
    keypad_0 = 98,
    keypad_period = 99,

    left_control = 224,
    left_shift = 225,
    left_alt = 226,
    left_gui = 227,

    right_control = 228,
    right_shift = 229,
    right_alt = 230,
    right_gui = 231,
};

namespace impl
{

inline constexpr std::uint32_t scancode_to_keycode{1 << 30};

}

enum class keycode : std::uint32_t
{
    enter = '\r',
    escape = '\033',
    backspace = '\b',
    tab = '\t',
    space = ' ',
    exclaim = '!',
    double_quote = '"',
    hashtag = '#',
    percent = '%',
    dollar = '$',
    ampersand = '&',
    quote = '\'',
    left_parenthesis = '(',
    right_parenthesis = ')',
    asterisk = '*',
    plus = '+',
    comma = ',',
    minus = '-',
    period = '.',
    slash = '/',
    zero = '0',
    one = '1',
    two = '2',
    three = '3',
    four = '4',
    five = '5',
    six = '6',
    seven = '7',
    eight = '8',
    nine = '9',
    colon = ':',
    semicolon = ';',
    less = '<',
    equals = '=',
    greater = '>',
    question = '?',
    at = '@',

    left_bracket = '[',
    backslash = '\\',
    right_bracket = ']',
    caret = '^',
    underscore = '_',
    backquote = '`',
    a = 'a',
    b = 'b',
    c = 'c',
    d = 'd',
    e = 'e',
    f = 'f',
    g = 'g',
    h = 'h',
    i = 'i',
    j = 'j',
    k = 'k',
    l = 'l',
    m = 'm',
    n = 'n',
    o = 'o',
    p = 'p',
    q = 'q',
    r = 'r',
    s = 's',
    t = 't',
    u = 'u',
    v = 'v',
    w = 'w',
    x = 'x',
    y = 'y',
    z = 'z',

    capslock = 57 | impl::scancode_to_keycode,

    f1 = 58 | impl::scancode_to_keycode,
    f2 = 59 | impl::scancode_to_keycode,
    f3 = 60 | impl::scancode_to_keycode,
    f4 = 61 | impl::scancode_to_keycode,
    f5 = 62 | impl::scancode_to_keycode,
    f6 = 63 | impl::scancode_to_keycode,
    f7 = 64 | impl::scancode_to_keycode,
    f8 = 65 | impl::scancode_to_keycode,
    f9 = 66 | impl::scancode_to_keycode,
    f10 = 67 | impl::scancode_to_keycode,
    f11 = 68 | impl::scancode_to_keycode,
    f12 = 69 | impl::scancode_to_keycode,

    printscreen = 70 | impl::scancode_to_keycode,
    scrolllock = 71 | impl::scancode_to_keycode,
    pause = 72 | impl::scancode_to_keycode,
    insert = 73 | impl::scancode_to_keycode,

    home = 74 | impl::scancode_to_keycode,
    pageup = 75 | impl::scancode_to_keycode,
    del = 76 | impl::scancode_to_keycode,
    end = 77 | impl::scancode_to_keycode,
    pagedown = 78 | impl::scancode_to_keycode,
    right = 79 | impl::scancode_to_keycode,
    left = 80 | impl::scancode_to_keycode,
    down = 81 | impl::scancode_to_keycode,
    up = 82 | impl::scancode_to_keycode,

    numlock = 83 | impl::scancode_to_keycode,

    keypad_divide = 84 | impl::scancode_to_keycode,
    keypad_multiply = 85 | impl::scancode_to_keycode,
    keypad_minus = 86 | impl::scancode_to_keycode,
    keypad_plus = 87 | impl::scancode_to_keycode,
    keypad_enter = 88 | impl::scancode_to_keycode,
    keypad_1 = 89 | impl::scancode_to_keycode,
    keypad_2 = 90 | impl::scancode_to_keycode,
    keypad_3 = 91 | impl::scancode_to_keycode,
    keypad_4 = 92 | impl::scancode_to_keycode,
    keypad_5 = 93 | impl::scancode_to_keycode,
    keypad_6 = 94 | impl::scancode_to_keycode,
    keypad_7 = 95 | impl::scancode_to_keycode,
    keypad_8 = 96 | impl::scancode_to_keycode,
    keypad_9 = 97 | impl::scancode_to_keycode,
    keypad_0 = 98 | impl::scancode_to_keycode,
    keypad_period = 99 | impl::scancode_to_keycode,

    left_control = 224 | impl::scancode_to_keycode,
    left_shift = 225 | impl::scancode_to_keycode,
    left_alt = 226 | impl::scancode_to_keycode,
    left_gui = 227 | impl::scancode_to_keycode,

    right_control = 228 | impl::scancode_to_keycode,
    right_shift = 229 | impl::scancode_to_keycode,
    right_alt = 230 | impl::scancode_to_keycode,
    right_gui = 231 | impl::scancode_to_keycode,
};

enum class key_modifier : std::uint32_t
{
    none = 0x0000,
    left_shift = 0x0001,
    right_shift = 0x0002,
    left_control = 0x0040,
    right_control = 0x0080,
    left_alt = 0x0100,
    right_alt = 0x0200,
    left_gui = 0x0400,
    right_gui = 0x0800,
    num = 0x1000,
    caps = 0x2000,
    mode = 0x4000,
};

APYRE_API scancode to_scancode(application& application, keycode key) noexcept;
APYRE_API keycode to_keycode(application& application, scancode scan) noexcept;
APYRE_API std::string to_string(application& application, keycode key);
APYRE_API std::string to_string(application& application, scancode scan);

APYRE_API void begin_text_input(application& application) noexcept;
APYRE_API void end_text_input(application& application) noexcept;
APYRE_API bool is_text_input_active(application& application) noexcept;

APYRE_API std::uint32_t get_keyboard_focus(application& application) noexcept;

enum class mouse_button : std::uint32_t
{
    left = 0x01,
    middle = 0x02,
    right = 0x04,
    side1 = 0x08,
    side2 = 0x10,
};

struct mouse_state
{
    std::int32_t x{};
    std::int32_t y{};
    mouse_button buttons{};
};

APYRE_API mouse_state get_mouse_state(application& application) noexcept;
APYRE_API mouse_state get_global_mouse_state(application& application) noexcept;

APYRE_API void enable_relative_mouse(application& application) noexcept;
APYRE_API void disable_relative_mouse(application& application) noexcept;

APYRE_API void move_mouse(application& application, std::int32_t x, std::int32_t y) noexcept;
APYRE_API void move_mouse(application& application, window& window, std::int32_t x, std::int32_t y) noexcept;
APYRE_API void move_mouse_to(application& application, std::int32_t x, std::int32_t y) noexcept;
APYRE_API void move_mouse_to(application& application, window& window, std::int32_t x, std::int32_t y) noexcept;

APYRE_API std::uint32_t get_mouse_focus(application& application) noexcept;

enum class system_cursor : std::uint32_t
{
    arrow = 0,
    ibeam = 1,
    wait = 2,
    crosshair = 3,
    wait_arrow = 4,
    size_northwest_southeast = 5,
    size_northeast_southwest = 6,
    size_west_east = 7,
    size_north_south = 8,
    size_all = 9,
    no = 10,
    hand = 11,
};

class APYRE_API cursor
{
    friend APYRE_API void activate_cursor(cursor& cursor) noexcept;

public:
    constexpr cursor() = default;
    cursor(application& application, const std::uint8_t* rgba, std::uint32_t width, std::uint32_t height, std::uint32_t hot_x, std::uint32_t hot_y);
    cursor(application& application, system_cursor type);

    ~cursor();
    cursor(const cursor&) = delete;
    cursor& operator=(const cursor&) = delete;
    cursor(cursor&& other) noexcept;
    cursor& operator=(cursor&& other) noexcept;

    void activate() noexcept;

private:
    SDL_Cursor* m_cursor{};
};

APYRE_API void hide_cursor(application& application);
APYRE_API void show_cursor(application& application);
APYRE_API bool is_cursor_visible(application& application) noexcept;

}

template<> struct apr::enable_enum_operations<apr::key_modifier> {static constexpr bool value{true};};
template<> struct apr::enable_enum_operations<apr::mouse_button> {static constexpr bool value{true};};

#endif
