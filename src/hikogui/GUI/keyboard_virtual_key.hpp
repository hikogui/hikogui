// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "keyboard_modifiers.hpp"
#include "../required.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include <unordered_map>

namespace tt::inline v1 {

enum class keyboard_virtual_key : uint8_t {
    Nul,

    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    _0,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Tilde,
    Ampersant,
    Pipe,
    Caret,
    Less,
    Equal,
    Greater,
    OpenParentheses,
    CloseParentheses,
    OpenBracket,
    CloseBracket,
    OpenBrace,
    CloseBrace,
    Period,
    Comma,
    Colon,
    SemiColon,
    Bang,
    Question,
    Space,
    Tab,
    Enter,
    Backtick,
    Quote,
    DoubleQuote,
    At,
    Hash,
    Dollar,
    Underscore,
    Backslash,

    Left,
    Right,
    Up,
    Down,
    PageUp,
    PageDown,
    Home,
    End,
    Backspace,
    Insert,
    Delete,
    Clear,
    Escape,

    PrintScreen,
    PauseBreak,
    Menu,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,

    BrowserBack,
    BrowserForward,
    BrowserRefresh,
    BrowserStop,
    BrowserSearch,
    BrowserFavorites,
    BrowserHome,
    VolumeMute,
    VolumeUp,
    VolumeDown,
    MediaNextTrack,
    MediaPrevTrack,
    MediaStop,
    MediaPlayPause,
};

inline ttlet string_to_keyboard_virtual_key_table = std::unordered_map<std::string, keyboard_virtual_key>{
    {"0", keyboard_virtual_key::_0},
    {"1", keyboard_virtual_key::_1},
    {"2", keyboard_virtual_key::_2},
    {"3", keyboard_virtual_key::_3},
    {"4", keyboard_virtual_key::_4},
    {"5", keyboard_virtual_key::_5},
    {"6", keyboard_virtual_key::_6},
    {"7", keyboard_virtual_key::_7},
    {"8", keyboard_virtual_key::_8},
    {"9", keyboard_virtual_key::_9},
    {"a", keyboard_virtual_key::A},
    {"b", keyboard_virtual_key::B},
    {"c", keyboard_virtual_key::C},
    {"d", keyboard_virtual_key::D},
    {"e", keyboard_virtual_key::E},
    {"f", keyboard_virtual_key::F},
    {"g", keyboard_virtual_key::G},
    {"h", keyboard_virtual_key::H},
    {"i", keyboard_virtual_key::I},
    {"j", keyboard_virtual_key::J},
    {"k", keyboard_virtual_key::K},
    {"l", keyboard_virtual_key::L},
    {"m", keyboard_virtual_key::M},
    {"n", keyboard_virtual_key::N},
    {"o", keyboard_virtual_key::O},
    {"p", keyboard_virtual_key::P},
    {"q", keyboard_virtual_key::Q},
    {"r", keyboard_virtual_key::R},
    {"s", keyboard_virtual_key::S},
    {"t", keyboard_virtual_key::T},
    {"u", keyboard_virtual_key::U},
    {"v", keyboard_virtual_key::V},
    {"w", keyboard_virtual_key::W},
    {"x", keyboard_virtual_key::X},
    {"y", keyboard_virtual_key::Y},
    {"z", keyboard_virtual_key::Z},
    {"plus", keyboard_virtual_key::Plus},
    {"-", keyboard_virtual_key::Minus},
    {"*", keyboard_virtual_key::Star},
    {"/", keyboard_virtual_key::Slash},
    {"%", keyboard_virtual_key::Percent},
    {"~", keyboard_virtual_key::Tilde},
    {"&", keyboard_virtual_key::Ampersant},
    {"|", keyboard_virtual_key::Pipe},
    {"^", keyboard_virtual_key::Caret},
    {"<", keyboard_virtual_key::Less},
    {"=", keyboard_virtual_key::Equal},
    {">", keyboard_virtual_key::Greater},
    {"(", keyboard_virtual_key::OpenParentheses},
    {")", keyboard_virtual_key::CloseParentheses},
    {"[", keyboard_virtual_key::OpenBracket},
    {"]", keyboard_virtual_key::CloseBracket},
    {"{", keyboard_virtual_key::OpenBrace},
    {"}", keyboard_virtual_key::CloseBrace},
    {"backslash", keyboard_virtual_key::Backslash},
    {".", keyboard_virtual_key::Period},
    {",", keyboard_virtual_key::Comma},
    {":", keyboard_virtual_key::Colon},
    {";", keyboard_virtual_key::SemiColon},
    {"!", keyboard_virtual_key::Bang},
    {"?", keyboard_virtual_key::Question},
    {"space", keyboard_virtual_key::Space},
    {"tab", keyboard_virtual_key::Tab},
    {"enter", keyboard_virtual_key::Enter},
    {"`", keyboard_virtual_key::Backtick},
    {"quote", keyboard_virtual_key::Quote},
    {"dquote", keyboard_virtual_key::DoubleQuote},
    {"@", keyboard_virtual_key::At},
    {"#", keyboard_virtual_key::Hash},
    {"$", keyboard_virtual_key::Dollar},
    {"_", keyboard_virtual_key::Underscore},
    {"left", keyboard_virtual_key::Left},
    {"right", keyboard_virtual_key::Right},
    {"up", keyboard_virtual_key::Up},
    {"down", keyboard_virtual_key::Down},
    {"pageup", keyboard_virtual_key::PageUp},
    {"pagedown", keyboard_virtual_key::PageDown},
    {"home", keyboard_virtual_key::Home},
    {"end", keyboard_virtual_key::End},
    {"backspace", keyboard_virtual_key::Backspace},
    {"insert", keyboard_virtual_key::Insert},
    {"delete", keyboard_virtual_key::Delete},
    {"clear", keyboard_virtual_key::Clear},
    {"escape", keyboard_virtual_key::Escape},
    {"print", keyboard_virtual_key::PrintScreen},
    {"pause", keyboard_virtual_key::PauseBreak},
    {"menu", keyboard_virtual_key::Menu},
    {"f1", keyboard_virtual_key::F1},
    {"f2", keyboard_virtual_key::F2},
    {"f3", keyboard_virtual_key::F3},
    {"f4", keyboard_virtual_key::F4},
    {"f5", keyboard_virtual_key::F5},
    {"f6", keyboard_virtual_key::F6},
    {"f7", keyboard_virtual_key::F7},
    {"f8", keyboard_virtual_key::F8},
    {"f9", keyboard_virtual_key::F9},
    {"f10", keyboard_virtual_key::F10},
    {"f11", keyboard_virtual_key::F11},
    {"f12", keyboard_virtual_key::F12},
    {"f13", keyboard_virtual_key::F13},
    {"f14", keyboard_virtual_key::F14},
    {"f15", keyboard_virtual_key::F15},
    {"f16", keyboard_virtual_key::F16},
    {"f17", keyboard_virtual_key::F17},
    {"f18", keyboard_virtual_key::F18},
    {"f19", keyboard_virtual_key::F19},
    {"f20", keyboard_virtual_key::F20},
    {"f21", keyboard_virtual_key::F21},
    {"f22", keyboard_virtual_key::F22},
    {"f23", keyboard_virtual_key::F23},
    {"f24", keyboard_virtual_key::F24},
    {"media-next", keyboard_virtual_key::MediaNextTrack},
    {"media-prev", keyboard_virtual_key::MediaPrevTrack},
    {"media-stop", keyboard_virtual_key::MediaStop},
    {"media-play", keyboard_virtual_key::MediaPlayPause},
    {"browser-back", keyboard_virtual_key::BrowserBack},
    {"browser-forward", keyboard_virtual_key::BrowserForward},
    {"browser-refresh", keyboard_virtual_key::BrowserRefresh},
    {"browser-stop", keyboard_virtual_key::BrowserStop},
    {"browser-search", keyboard_virtual_key::BrowserSearch},
    {"browser-favorites", keyboard_virtual_key::BrowserFavorites},
    {"browser-home", keyboard_virtual_key::BrowserHome},
    {"volume-mute", keyboard_virtual_key::VolumeMute},
    {"volume-up", keyboard_virtual_key::VolumeUp},
    {"volume-down", keyboard_virtual_key::VolumeDown},
};

inline keyboard_virtual_key to_keyboard_virtual_key(std::string_view s)
{
    ttlet lower_s = to_lower(s);
    ttlet i = string_to_keyboard_virtual_key_table.find(lower_s);
    if (i != string_to_keyboard_virtual_key_table.cend()) {
        return i->second;
    }
    throw parse_error(std::format("Could not find virtual key '{}'", s));
}

keyboard_virtual_key to_keyboard_virtual_key(int key_code, bool extended, keyboard_modifiers modifiers);

constexpr char const *to_const_string(keyboard_virtual_key key) noexcept
{
    switch (key) {
    case keyboard_virtual_key::_0: return "0";
    case keyboard_virtual_key::_1: return "1";
    case keyboard_virtual_key::_2: return "2";
    case keyboard_virtual_key::_3: return "3";
    case keyboard_virtual_key::_4: return "4";
    case keyboard_virtual_key::_5: return "5";
    case keyboard_virtual_key::_6: return "6";
    case keyboard_virtual_key::_7: return "7";
    case keyboard_virtual_key::_8: return "8";
    case keyboard_virtual_key::_9: return "9";
    case keyboard_virtual_key::A: return "a";
    case keyboard_virtual_key::B: return "b";
    case keyboard_virtual_key::C: return "c";
    case keyboard_virtual_key::D: return "d";
    case keyboard_virtual_key::E: return "e";
    case keyboard_virtual_key::F: return "f";
    case keyboard_virtual_key::G: return "g";
    case keyboard_virtual_key::H: return "h";
    case keyboard_virtual_key::I: return "i";
    case keyboard_virtual_key::J: return "j";
    case keyboard_virtual_key::K: return "k";
    case keyboard_virtual_key::L: return "l";
    case keyboard_virtual_key::M: return "m";
    case keyboard_virtual_key::N: return "n";
    case keyboard_virtual_key::O: return "o";
    case keyboard_virtual_key::P: return "p";
    case keyboard_virtual_key::Q: return "q";
    case keyboard_virtual_key::R: return "r";
    case keyboard_virtual_key::S: return "s";
    case keyboard_virtual_key::T: return "t";
    case keyboard_virtual_key::U: return "u";
    case keyboard_virtual_key::V: return "v";
    case keyboard_virtual_key::W: return "w";
    case keyboard_virtual_key::X: return "x";
    case keyboard_virtual_key::Y: return "y";
    case keyboard_virtual_key::Z: return "z";
    case keyboard_virtual_key::Plus: return "plus";
    case keyboard_virtual_key::Minus: return "-";
    case keyboard_virtual_key::Star: return "*";
    case keyboard_virtual_key::Slash: return "/";
    case keyboard_virtual_key::Percent: return "%";
    case keyboard_virtual_key::Tilde: return "~";
    case keyboard_virtual_key::Ampersant: return "&";
    case keyboard_virtual_key::Pipe: return "|";
    case keyboard_virtual_key::Caret: return "^";
    case keyboard_virtual_key::Less: return "<";
    case keyboard_virtual_key::Equal: return "=";
    case keyboard_virtual_key::Greater: return ">";
    case keyboard_virtual_key::OpenParentheses: return "(";
    case keyboard_virtual_key::CloseParentheses: return ")";
    case keyboard_virtual_key::OpenBracket: return "[";
    case keyboard_virtual_key::CloseBracket: return "]";
    case keyboard_virtual_key::OpenBrace: return "{";
    case keyboard_virtual_key::CloseBrace: return "}";
    case keyboard_virtual_key::Backslash: return "backslash";
    case keyboard_virtual_key::Period: return ".";
    case keyboard_virtual_key::Comma: return ",";
    case keyboard_virtual_key::Colon: return ":";
    case keyboard_virtual_key::SemiColon: return ";";
    case keyboard_virtual_key::Bang: return "!";
    case keyboard_virtual_key::Question: return "?";
    case keyboard_virtual_key::Space: return "space";
    case keyboard_virtual_key::Tab: return "tab";
    case keyboard_virtual_key::Enter: return "enter";
    case keyboard_virtual_key::Backtick: return "`";
    case keyboard_virtual_key::Quote: return "quote";
    case keyboard_virtual_key::DoubleQuote: return "dquote";
    case keyboard_virtual_key::At: return "@";
    case keyboard_virtual_key::Hash: return "#";
    case keyboard_virtual_key::Dollar: return "$";
    case keyboard_virtual_key::Underscore: return "_";
    case keyboard_virtual_key::Left: return "left";
    case keyboard_virtual_key::Right: return "right";
    case keyboard_virtual_key::Up: return "up";
    case keyboard_virtual_key::Down: return "down";
    case keyboard_virtual_key::PageUp: return "pageup";
    case keyboard_virtual_key::PageDown: return "pagedown";
    case keyboard_virtual_key::Home: return "home";
    case keyboard_virtual_key::End: return "end";
    case keyboard_virtual_key::Backspace: return "backspace";
    case keyboard_virtual_key::Insert: return "insert";
    case keyboard_virtual_key::Delete: return "delete";
    case keyboard_virtual_key::Clear: return "clear";
    case keyboard_virtual_key::Escape: return "escape";
    case keyboard_virtual_key::PrintScreen: return "print";
    case keyboard_virtual_key::PauseBreak: return "pause";
    case keyboard_virtual_key::Menu: return "menu";
    case keyboard_virtual_key::F1: return "f1";
    case keyboard_virtual_key::F2: return "f2";
    case keyboard_virtual_key::F3: return "f3";
    case keyboard_virtual_key::F4: return "f4";
    case keyboard_virtual_key::F5: return "f5";
    case keyboard_virtual_key::F6: return "f6";
    case keyboard_virtual_key::F7: return "f7";
    case keyboard_virtual_key::F8: return "f8";
    case keyboard_virtual_key::F9: return "f9";
    case keyboard_virtual_key::F10: return "f10";
    case keyboard_virtual_key::F11: return "f11";
    case keyboard_virtual_key::F12: return "f12";
    case keyboard_virtual_key::F13: return "f13";
    case keyboard_virtual_key::F14: return "f14";
    case keyboard_virtual_key::F15: return "f15";
    case keyboard_virtual_key::F16: return "f16";
    case keyboard_virtual_key::F17: return "f17";
    case keyboard_virtual_key::F18: return "f18";
    case keyboard_virtual_key::F19: return "f19";
    case keyboard_virtual_key::F20: return "f20";
    case keyboard_virtual_key::F21: return "f21";
    case keyboard_virtual_key::F22: return "f22";
    case keyboard_virtual_key::F23: return "f23";
    case keyboard_virtual_key::F24: return "f24";
    case keyboard_virtual_key::MediaNextTrack: return "media-next";
    case keyboard_virtual_key::MediaPrevTrack: return "media-prev";
    case keyboard_virtual_key::MediaStop: return "media-stop";
    case keyboard_virtual_key::MediaPlayPause: return "media-play";
    case keyboard_virtual_key::BrowserBack: return "browser-back";
    case keyboard_virtual_key::BrowserForward: return "browser-forward";
    case keyboard_virtual_key::BrowserRefresh: return "browser-refresh";
    case keyboard_virtual_key::BrowserStop: return "browser-stop";
    case keyboard_virtual_key::BrowserSearch: return "browser-search";
    case keyboard_virtual_key::BrowserFavorites: return "browser-favourite";
    case keyboard_virtual_key::BrowserHome: return "browser-home";
    case keyboard_virtual_key::VolumeMute: return "volume-mute";
    case keyboard_virtual_key::VolumeUp: return "volume-up";
    case keyboard_virtual_key::VolumeDown: return "volume-down";
    default: tt_no_default();
    }
}

inline std::string to_string(keyboard_virtual_key key) noexcept
{
    return std::string{to_const_string(key)};
}

inline std::ostream &operator<<(std::ostream &lhs, keyboard_virtual_key const &rhs)
{
    return lhs << to_string(rhs);
}

} // namespace tt::inline v1

template<>
struct std::hash<tt::keyboard_virtual_key> {
    [[nodiscard]] std::size_t operator()(tt::keyboard_virtual_key rhs) const noexcept
    {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(rhs));
    }
};

template<typename CharT>
struct std::formatter<tt::keyboard_virtual_key, CharT> : std::formatter<char const *, CharT> {
    auto format(tt::keyboard_virtual_key const &t, auto &fc)
    {
        return std::formatter<char const *, CharT>::format(tt::to_const_string(t), fc);
    }
};
