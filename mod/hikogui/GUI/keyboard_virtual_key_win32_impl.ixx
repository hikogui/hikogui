// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"



export module hikogui_GUI : keyboard_virtual_key_impl;
import : keyboard_modifiers;
import : keyboard_virtual_key_intf;

export namespace hi::inline v1 {

[[nodiscard]] constexpr keyboard_virtual_key to_keyboard_virtual_key(int key_code, bool extended, keyboard_modifiers modifiers) noexcept
{
    switch (key_code) {
    case VK_MENU:
        return keyboard_virtual_key::menu;
    case VK_BACK:
        return keyboard_virtual_key::backspace;
    case VK_TAB:
        return keyboard_virtual_key::tab;
    case VK_CLEAR:
        return keyboard_virtual_key::clear;
    case VK_RETURN:
        return keyboard_virtual_key::enter;
    case VK_PAUSE:
        return keyboard_virtual_key::pause_break;
    case VK_ESCAPE:
        return keyboard_virtual_key::escape;
    case VK_SPACE:
        return keyboard_virtual_key::space;
    case VK_PRIOR:
        return keyboard_virtual_key::page_up;
    case VK_NEXT:
        return keyboard_virtual_key::page_down;
    case VK_END:
        return keyboard_virtual_key::end;
    case VK_HOME:
        return keyboard_virtual_key::home;
    case VK_LEFT:
        return keyboard_virtual_key::left;
    case VK_UP:
        return keyboard_virtual_key::up;
    case VK_RIGHT:
        return keyboard_virtual_key::right;
    case VK_DOWN:
        return keyboard_virtual_key::down;
    case VK_PRINT:
        return keyboard_virtual_key::print_screen;
    case VK_SNAPSHOT:
        return keyboard_virtual_key::print_screen;
    case VK_INSERT:
        return keyboard_virtual_key::insert;
    case VK_DELETE:
        return keyboard_virtual_key::_delete;
    case '0':
        return keyboard_virtual_key::_0;
    case '1':
        return keyboard_virtual_key::_1;
    case '2':
        return keyboard_virtual_key::_2;
    case '3':
        return keyboard_virtual_key::_3;
    case '4':
        return keyboard_virtual_key::_4;
    case '5':
        return keyboard_virtual_key::_5;
    case '6':
        return keyboard_virtual_key::_6;
    case '7':
        return keyboard_virtual_key::_7;
    case '8':
        return keyboard_virtual_key::_8;
    case '9':
        return keyboard_virtual_key::_9;
    case 'A':
        return keyboard_virtual_key::A;
    case 'B':
        return keyboard_virtual_key::B;
    case 'C':
        return keyboard_virtual_key::C;
    case 'D':
        return keyboard_virtual_key::D;
    case 'E':
        return keyboard_virtual_key::E;
    case 'F':
        return keyboard_virtual_key::F;
    case 'G':
        return keyboard_virtual_key::G;
    case 'H':
        return keyboard_virtual_key::H;
    case 'I':
        return keyboard_virtual_key::I;
    case 'J':
        return keyboard_virtual_key::J;
    case 'K':
        return keyboard_virtual_key::K;
    case 'L':
        return keyboard_virtual_key::L;
    case 'M':
        return keyboard_virtual_key::M;
    case 'N':
        return keyboard_virtual_key::N;
    case 'O':
        return keyboard_virtual_key::O;
    case 'P':
        return keyboard_virtual_key::P;
    case 'Q':
        return keyboard_virtual_key::Q;
    case 'R':
        return keyboard_virtual_key::R;
    case 'S':
        return keyboard_virtual_key::S;
    case 'T':
        return keyboard_virtual_key::T;
    case 'U':
        return keyboard_virtual_key::U;
    case 'V':
        return keyboard_virtual_key::V;
    case 'W':
        return keyboard_virtual_key::W;
    case 'X':
        return keyboard_virtual_key::X;
    case 'Y':
        return keyboard_virtual_key::Y;
    case 'Z':
        return keyboard_virtual_key::Z;
    case VK_NUMPAD0:
        return keyboard_virtual_key::_0;
    case VK_NUMPAD1:
        return keyboard_virtual_key::_1;
    case VK_NUMPAD2:
        return keyboard_virtual_key::_2;
    case VK_NUMPAD3:
        return keyboard_virtual_key::_3;
    case VK_NUMPAD4:
        return keyboard_virtual_key::_4;
    case VK_NUMPAD5:
        return keyboard_virtual_key::_5;
    case VK_NUMPAD6:
        return keyboard_virtual_key::_6;
    case VK_NUMPAD7:
        return keyboard_virtual_key::_7;
    case VK_NUMPAD8:
        return keyboard_virtual_key::_8;
    case VK_NUMPAD9:
        return keyboard_virtual_key::_9;
    case VK_MULTIPLY:
        return keyboard_virtual_key::star;
    case VK_ADD:
        return keyboard_virtual_key::plus;
    case VK_SUBTRACT:
        return keyboard_virtual_key::minus;
    case VK_DECIMAL:
        return keyboard_virtual_key::period;
    case VK_DIVIDE:
        return keyboard_virtual_key::slash;
    case VK_F1:
        return keyboard_virtual_key::F1;
    case VK_F2:
        return keyboard_virtual_key::F2;
    case VK_F3:
        return keyboard_virtual_key::F3;
    case VK_F4:
        return keyboard_virtual_key::F4;
    case VK_F5:
        return keyboard_virtual_key::F5;
    case VK_F6:
        return keyboard_virtual_key::F6;
    case VK_F7:
        return keyboard_virtual_key::F7;
    case VK_F8:
        return keyboard_virtual_key::F8;
    case VK_F9:
        return keyboard_virtual_key::F9;
    case VK_F10:
        return keyboard_virtual_key::F10;
    case VK_F11:
        return keyboard_virtual_key::F11;
    case VK_F12:
        return keyboard_virtual_key::F12;
    case VK_F13:
        return keyboard_virtual_key::F13;
    case VK_F14:
        return keyboard_virtual_key::F14;
    case VK_F15:
        return keyboard_virtual_key::F15;
    case VK_F16:
        return keyboard_virtual_key::F16;
    case VK_F17:
        return keyboard_virtual_key::F17;
    case VK_F18:
        return keyboard_virtual_key::F18;
    case VK_F19:
        return keyboard_virtual_key::F19;
    case VK_F20:
        return keyboard_virtual_key::F20;
    case VK_F21:
        return keyboard_virtual_key::F21;
    case VK_F22:
        return keyboard_virtual_key::F22;
    case VK_F23:
        return keyboard_virtual_key::F23;
    case VK_F24:
        return keyboard_virtual_key::F24;
    case VK_BROWSER_BACK:
        return keyboard_virtual_key::browser_back;
    case VK_BROWSER_FORWARD:
        return keyboard_virtual_key::browser_forward;
    case VK_BROWSER_REFRESH:
        return keyboard_virtual_key::browser_refresh;
    case VK_BROWSER_STOP:
        return keyboard_virtual_key::browser_stop;
    case VK_BROWSER_SEARCH:
        return keyboard_virtual_key::browser_search;
    case VK_BROWSER_FAVORITES:
        return keyboard_virtual_key::browser_favorites;
    case VK_BROWSER_HOME:
        return keyboard_virtual_key::browser_home;
    case VK_VOLUME_MUTE:
        return keyboard_virtual_key::volume_mute;
    case VK_VOLUME_DOWN:
        return keyboard_virtual_key::volume_down;
    case VK_VOLUME_UP:
        return keyboard_virtual_key::volume_up;
    case VK_MEDIA_NEXT_TRACK:
        return keyboard_virtual_key::media_next_track;
    case VK_MEDIA_PREV_TRACK:
        return keyboard_virtual_key::media_prev_track;
    case VK_MEDIA_STOP:
        return keyboard_virtual_key::media_stop;
    case VK_MEDIA_PLAY_PAUSE:
        return keyboard_virtual_key::media_play_pause;
    case VK_OEM_1:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::colon : keyboard_virtual_key::semi_colon;
    case VK_OEM_PLUS:
        return keyboard_virtual_key::plus;
    case VK_OEM_COMMA:
        return keyboard_virtual_key::comma;
    case VK_OEM_MINUS:
        return keyboard_virtual_key::minus;
    case VK_OEM_PERIOD:
        return keyboard_virtual_key::period;
    case VK_OEM_2:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::question : keyboard_virtual_key::slash;
    case VK_OEM_3:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::tilde : keyboard_virtual_key::backtick;
    case VK_OEM_4:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::open_brace :
                                                                keyboard_virtual_key::open_bracket;
    case VK_OEM_5:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::pipe : keyboard_virtual_key::backslash;
    case VK_OEM_6:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::close_brace :
                                                                keyboard_virtual_key::close_bracket;
    case VK_OEM_7:
        return to_bool(modifiers & keyboard_modifiers::shift) ? keyboard_virtual_key::double_quote : keyboard_virtual_key::quote;
    case VK_PLAY:
        return keyboard_virtual_key::media_play_pause;
    case VK_OEM_CLEAR:
        return keyboard_virtual_key::clear;
    default:
        return keyboard_virtual_key::nul;
    }
}

} // namespace hi::inline v1
