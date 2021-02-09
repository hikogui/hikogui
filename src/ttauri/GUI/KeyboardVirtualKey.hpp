// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "KeyboardModifiers.hpp"
#include "../required.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include <unordered_map>

namespace tt {

enum class KeyboardVirtualKey : uint8_t {
    Nul,

    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9,

    Plus, Minus, Star, Slash, Percent,
    Tilde, Ampersant, Pipe, Caret,
    Less, Equal, Greater,
    OpenParentheses, CloseParentheses, OpenBracket, CloseBracket, OpenBrace, CloseBrace,
    Period, Comma, Colon, SemiColon, Bang, Question,
    Space, Tab, Enter,
    Backtick, Quote, DoubleQuote,
    At, Hash, Dollar, Underscore, Backslash,

    Left, Right, Up, Down, PageUp, PageDown, Home, End,
    Backspace, Insert, Delete, Clear, Escape,

    PrintScreen, PauseBreak,

    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,

    BrowserBack, BrowserForward, BrowserRefresh, BrowserStop,
    BrowserSearch, BrowserFavorites, BrowserHome,
    VolumeMute, VolumeUp, VolumeDown,
    MediaNextTrack, MediaPrevTrack, MediaStop, MediaPlayPause,
};

inline ttlet string_to_KeyboardVirtualKey_table = std::unordered_map<std::string,KeyboardVirtualKey>{
    {"0", KeyboardVirtualKey::_0}, 
    {"1", KeyboardVirtualKey::_1}, 
    {"2", KeyboardVirtualKey::_2}, 
    {"3", KeyboardVirtualKey::_3}, 
    {"4", KeyboardVirtualKey::_4}, 
    {"5", KeyboardVirtualKey::_5}, 
    {"6", KeyboardVirtualKey::_6}, 
    {"7", KeyboardVirtualKey::_7}, 
    {"8", KeyboardVirtualKey::_8}, 
    {"9", KeyboardVirtualKey::_9}, 
    {"a", KeyboardVirtualKey::A}, 
    {"b", KeyboardVirtualKey::B}, 
    {"c", KeyboardVirtualKey::C}, 
    {"d", KeyboardVirtualKey::D}, 
    {"e", KeyboardVirtualKey::E}, 
    {"f", KeyboardVirtualKey::F}, 
    {"g", KeyboardVirtualKey::G}, 
    {"h", KeyboardVirtualKey::H}, 
    {"i", KeyboardVirtualKey::I}, 
    {"j", KeyboardVirtualKey::J}, 
    {"k", KeyboardVirtualKey::K}, 
    {"l", KeyboardVirtualKey::L}, 
    {"m", KeyboardVirtualKey::M}, 
    {"n", KeyboardVirtualKey::N}, 
    {"o", KeyboardVirtualKey::O}, 
    {"p", KeyboardVirtualKey::P}, 
    {"q", KeyboardVirtualKey::Q}, 
    {"r", KeyboardVirtualKey::R}, 
    {"s", KeyboardVirtualKey::S}, 
    {"t", KeyboardVirtualKey::T}, 
    {"u", KeyboardVirtualKey::U}, 
    {"v", KeyboardVirtualKey::V}, 
    {"w", KeyboardVirtualKey::W}, 
    {"x", KeyboardVirtualKey::X}, 
    {"y", KeyboardVirtualKey::Y}, 
    {"z", KeyboardVirtualKey::Z}, 
    {"plus", KeyboardVirtualKey::Plus}, 
    {"-", KeyboardVirtualKey::Minus}, 
    {"*", KeyboardVirtualKey::Star}, 
    {"/", KeyboardVirtualKey::Slash}, 
    {"%", KeyboardVirtualKey::Percent}, 
    {"~", KeyboardVirtualKey::Tilde}, 
    {"&", KeyboardVirtualKey::Ampersant}, 
    {"|", KeyboardVirtualKey::Pipe}, 
    {"^", KeyboardVirtualKey::Caret}, 
    {"<", KeyboardVirtualKey::Less}, 
    {"=", KeyboardVirtualKey::Equal}, 
    {">", KeyboardVirtualKey::Greater}, 
    {"(", KeyboardVirtualKey::OpenParentheses}, 
    {")", KeyboardVirtualKey::CloseParentheses}, 
    {"[", KeyboardVirtualKey::OpenBracket}, 
    {"]", KeyboardVirtualKey::CloseBracket}, 
    {"{", KeyboardVirtualKey::OpenBrace}, 
    {"}", KeyboardVirtualKey::CloseBrace}, 
    {"backslash", KeyboardVirtualKey::Backslash},
    {".", KeyboardVirtualKey::Period},
    {",", KeyboardVirtualKey::Comma},
    {":", KeyboardVirtualKey::Colon},
    {";", KeyboardVirtualKey::SemiColon},
    {"!", KeyboardVirtualKey::Bang},
    {"?", KeyboardVirtualKey::Question},
    {"space", KeyboardVirtualKey::Space},
    {"tab", KeyboardVirtualKey::Tab},
    {"enter", KeyboardVirtualKey::Enter},
    {"`", KeyboardVirtualKey::Backtick},
    {"quote", KeyboardVirtualKey::Quote},
    {"dquote", KeyboardVirtualKey::DoubleQuote},
    {"@", KeyboardVirtualKey::At},
    {"#", KeyboardVirtualKey::Hash},
    {"$", KeyboardVirtualKey::Dollar},
    {"_", KeyboardVirtualKey::Underscore},
    {"left", KeyboardVirtualKey::Left},
    {"right", KeyboardVirtualKey::Right},
    {"up", KeyboardVirtualKey::Up},
    {"down", KeyboardVirtualKey::Down},
    {"pageup", KeyboardVirtualKey::PageUp},
    {"pagedown", KeyboardVirtualKey::PageDown},
    {"home", KeyboardVirtualKey::Home},
    {"end", KeyboardVirtualKey::End},
    {"backspace", KeyboardVirtualKey::Backspace},
    {"insert", KeyboardVirtualKey::Insert},
    {"delete", KeyboardVirtualKey::Delete},
    {"clear", KeyboardVirtualKey::Clear},
    {"escape", KeyboardVirtualKey::Escape},
    {"print", KeyboardVirtualKey::PrintScreen},
    {"pause", KeyboardVirtualKey::PauseBreak},
    {"f1", KeyboardVirtualKey::F1},
    {"f2", KeyboardVirtualKey::F2},
    {"f3", KeyboardVirtualKey::F3},
    {"f4", KeyboardVirtualKey::F4},
    {"f5", KeyboardVirtualKey::F5},
    {"f6", KeyboardVirtualKey::F6},
    {"f7", KeyboardVirtualKey::F7},
    {"f8", KeyboardVirtualKey::F8},
    {"f9", KeyboardVirtualKey::F9},
    {"f10", KeyboardVirtualKey::F10},
    {"f11", KeyboardVirtualKey::F11},
    {"f12", KeyboardVirtualKey::F12},
    {"f13", KeyboardVirtualKey::F13},
    {"f14", KeyboardVirtualKey::F14},
    {"f15", KeyboardVirtualKey::F15},
    {"f16", KeyboardVirtualKey::F16},
    {"f17", KeyboardVirtualKey::F17},
    {"f18", KeyboardVirtualKey::F18},
    {"f19", KeyboardVirtualKey::F19},
    {"f20", KeyboardVirtualKey::F20},
    {"f21", KeyboardVirtualKey::F21},
    {"f22", KeyboardVirtualKey::F22},
    {"f23", KeyboardVirtualKey::F23},
    {"f24", KeyboardVirtualKey::F24},
    {"media-next", KeyboardVirtualKey::MediaNextTrack},
    {"media-prev", KeyboardVirtualKey::MediaPrevTrack},
    {"media-stop", KeyboardVirtualKey::MediaStop},
    {"media-play", KeyboardVirtualKey::MediaPlayPause},
    {"browser-back", KeyboardVirtualKey::BrowserBack},
    {"browser-forward", KeyboardVirtualKey::BrowserForward},
    {"browser-refresh", KeyboardVirtualKey::BrowserRefresh},
    {"browser-stop", KeyboardVirtualKey::BrowserStop},
    {"browser-search", KeyboardVirtualKey::BrowserSearch},
    {"browser-favorites", KeyboardVirtualKey::BrowserFavorites},
    {"browser-home", KeyboardVirtualKey::BrowserHome},
    {"volume-mute", KeyboardVirtualKey::VolumeMute},
    {"volume-up", KeyboardVirtualKey::VolumeUp},
    {"volume-down", KeyboardVirtualKey::VolumeDown},
};

inline KeyboardVirtualKey to_KeyboardVirtualKey(std::string_view s)
{
    ttlet lower_s = to_lower(s);
    ttlet i = string_to_KeyboardVirtualKey_table.find(lower_s);
    if (i != string_to_KeyboardVirtualKey_table.cend()) {
        return i->second;
    }
    throw parse_error("Could not find virtual key '{}'", s);
}

KeyboardVirtualKey to_KeyboardVirtualKey(int key_code, bool extended, KeyboardModifiers modifiers);


constexpr char const *to_const_string(KeyboardVirtualKey key) noexcept
{
    switch (key) {
    case KeyboardVirtualKey::_0: return "0"; 
    case KeyboardVirtualKey::_1: return "1"; 
    case KeyboardVirtualKey::_2: return "2";  
    case KeyboardVirtualKey::_3: return "3"; 
    case KeyboardVirtualKey::_4: return "4"; 
    case KeyboardVirtualKey::_5: return "5"; 
    case KeyboardVirtualKey::_6: return "6"; 
    case KeyboardVirtualKey::_7: return "7"; 
    case KeyboardVirtualKey::_8: return "8"; 
    case KeyboardVirtualKey::_9: return "9"; 
    case KeyboardVirtualKey::A: return "a"; 
    case KeyboardVirtualKey::B: return "b"; 
    case KeyboardVirtualKey::C: return "c"; 
    case KeyboardVirtualKey::D: return "d"; 
    case KeyboardVirtualKey::E: return "e"; 
    case KeyboardVirtualKey::F: return "f"; 
    case KeyboardVirtualKey::G: return "g"; 
    case KeyboardVirtualKey::H: return "h"; 
    case KeyboardVirtualKey::I: return "i"; 
    case KeyboardVirtualKey::J: return "j"; 
    case KeyboardVirtualKey::K: return "k"; 
    case KeyboardVirtualKey::L: return "l"; 
    case KeyboardVirtualKey::M: return "m"; 
    case KeyboardVirtualKey::N: return "n"; 
    case KeyboardVirtualKey::O: return "o"; 
    case KeyboardVirtualKey::P: return "p"; 
    case KeyboardVirtualKey::Q: return "q"; 
    case KeyboardVirtualKey::R: return "r"; 
    case KeyboardVirtualKey::S: return "s"; 
    case KeyboardVirtualKey::T: return "t"; 
    case KeyboardVirtualKey::U: return "u"; 
    case KeyboardVirtualKey::V: return "v"; 
    case KeyboardVirtualKey::W: return "w"; 
    case KeyboardVirtualKey::X: return "x"; 
    case KeyboardVirtualKey::Y: return "y"; 
    case KeyboardVirtualKey::Z: return "z"; 
    case KeyboardVirtualKey::Plus: return "plus";
    case KeyboardVirtualKey::Minus: return "-";
    case KeyboardVirtualKey::Star: return "*";
    case KeyboardVirtualKey::Slash: return "/";
    case KeyboardVirtualKey::Percent: return "%";
    case KeyboardVirtualKey::Tilde: return "~";
    case KeyboardVirtualKey::Ampersant: return "&";
    case KeyboardVirtualKey::Pipe: return "|"; 
    case KeyboardVirtualKey::Caret: return "^"; 
    case KeyboardVirtualKey::Less: return "<";
    case KeyboardVirtualKey::Equal: return "=";
    case KeyboardVirtualKey::Greater: return ">";
    case KeyboardVirtualKey::OpenParentheses: return "(";
    case KeyboardVirtualKey::CloseParentheses: return ")";
    case KeyboardVirtualKey::OpenBracket: return "[";
    case KeyboardVirtualKey::CloseBracket: return "]";
    case KeyboardVirtualKey::OpenBrace: return "{";
    case KeyboardVirtualKey::CloseBrace: return "}";
    case KeyboardVirtualKey::Backslash: return "backslash";
    case KeyboardVirtualKey::Period: return ".";
    case KeyboardVirtualKey::Comma: return ",";
    case KeyboardVirtualKey::Colon: return ":";
    case KeyboardVirtualKey::SemiColon: return ";";
    case KeyboardVirtualKey::Bang: return "!";
    case KeyboardVirtualKey::Question: return "?";
    case KeyboardVirtualKey::Space: return "space";
    case KeyboardVirtualKey::Tab: return "tab";
    case KeyboardVirtualKey::Enter: return "enter";
    case KeyboardVirtualKey::Backtick: return "`";
    case KeyboardVirtualKey::Quote: return "quote";
    case KeyboardVirtualKey::DoubleQuote: return "dquote";
    case KeyboardVirtualKey::At: return "@";
    case KeyboardVirtualKey::Hash: return "#";
    case KeyboardVirtualKey::Dollar: return "$";
    case KeyboardVirtualKey::Underscore: return "_";
    case KeyboardVirtualKey::Left: return "left";
    case KeyboardVirtualKey::Right: return "right";
    case KeyboardVirtualKey::Up: return "up";
    case KeyboardVirtualKey::Down: return "down";
    case KeyboardVirtualKey::PageUp: return "pageup";
    case KeyboardVirtualKey::PageDown: return "pagedown";
    case KeyboardVirtualKey::Home: return "home";
    case KeyboardVirtualKey::End: return "end";
    case KeyboardVirtualKey::Backspace: return "backspace";
    case KeyboardVirtualKey::Insert: return "insert";
    case KeyboardVirtualKey::Delete: return "delete";
    case KeyboardVirtualKey::Clear: return "clear";
    case KeyboardVirtualKey::Escape: return "escape";
    case KeyboardVirtualKey::PrintScreen: return "print";
    case KeyboardVirtualKey::PauseBreak: return "pause";
    case KeyboardVirtualKey::F1: return "f1";
    case KeyboardVirtualKey::F2: return "f2";
    case KeyboardVirtualKey::F3: return "f3";
    case KeyboardVirtualKey::F4: return "f4";
    case KeyboardVirtualKey::F5: return "f5";
    case KeyboardVirtualKey::F6: return "f6";
    case KeyboardVirtualKey::F7: return "f7";
    case KeyboardVirtualKey::F8: return "f8";
    case KeyboardVirtualKey::F9: return "f9";
    case KeyboardVirtualKey::F10: return "f10";
    case KeyboardVirtualKey::F11: return "f11";
    case KeyboardVirtualKey::F12: return "f12";
    case KeyboardVirtualKey::F13: return "f13";
    case KeyboardVirtualKey::F14: return "f14";
    case KeyboardVirtualKey::F15: return "f15";
    case KeyboardVirtualKey::F16: return "f16";
    case KeyboardVirtualKey::F17: return "f17";
    case KeyboardVirtualKey::F18: return "f18";
    case KeyboardVirtualKey::F19: return "f19";
    case KeyboardVirtualKey::F20: return "f20";
    case KeyboardVirtualKey::F21: return "f21";
    case KeyboardVirtualKey::F22: return "f22";
    case KeyboardVirtualKey::F23: return "f23";
    case KeyboardVirtualKey::F24: return "f24";
    case KeyboardVirtualKey::MediaNextTrack: return "media-next";
    case KeyboardVirtualKey::MediaPrevTrack: return "media-prev";
    case KeyboardVirtualKey::MediaStop: return "media-stop";
    case KeyboardVirtualKey::MediaPlayPause: return "media-play";
    case KeyboardVirtualKey::BrowserBack: return "browser-back";
    case KeyboardVirtualKey::BrowserForward: return "browser-forward";
    case KeyboardVirtualKey::BrowserRefresh: return "browser-refresh";
    case KeyboardVirtualKey::BrowserStop: return "browser-stop";
    case KeyboardVirtualKey::BrowserSearch: return "browser-search";
    case KeyboardVirtualKey::BrowserFavorites: return "browser-favourite";
    case KeyboardVirtualKey::BrowserHome: return "browser-home";
    case KeyboardVirtualKey::VolumeMute: return "volume-mute";
    case KeyboardVirtualKey::VolumeUp: return "volume-up";
    case KeyboardVirtualKey::VolumeDown: return "volume-down";
    default: tt_no_default();
    }
}

inline std::string to_string(KeyboardVirtualKey key) noexcept
{
    return std::string{to_const_string(key)};
}

inline std::ostream &operator<<(std::ostream &lhs, KeyboardVirtualKey const &rhs) {
    return lhs << to_string(rhs);
}

}

namespace std {

template<>
struct hash<tt::KeyboardVirtualKey> {
    [[nodiscard]] size_t operator() (tt::KeyboardVirtualKey rhs) const noexcept {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(rhs));
    }
};

}
