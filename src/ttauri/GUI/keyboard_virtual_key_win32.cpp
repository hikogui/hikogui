// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "keyboard_virtual_key.hpp"
#include "keyboard_modifiers.hpp"
#include <Windows.h>

namespace tt::inline v1 {

keyboard_virtual_key to_keyboard_virtual_key(int key_code, bool extended, keyboard_modifiers modifiers)
{
    switch (key_code) {
    case VK_MENU: return keyboard_virtual_key::Menu;
    case VK_BACK: return keyboard_virtual_key::Backspace;
    case VK_TAB: return keyboard_virtual_key::Tab;
    case VK_CLEAR: return keyboard_virtual_key::Clear;
    case VK_RETURN: return keyboard_virtual_key::Enter;
    case VK_PAUSE: return keyboard_virtual_key::PauseBreak;
    case VK_ESCAPE: return keyboard_virtual_key::Escape;
    case VK_SPACE: return keyboard_virtual_key::Space;
    case VK_PRIOR: return keyboard_virtual_key::PageUp;
    case VK_NEXT: return keyboard_virtual_key::PageDown;
    case VK_END: return keyboard_virtual_key::End;
    case VK_HOME: return keyboard_virtual_key::Home;
    case VK_LEFT: return keyboard_virtual_key::Left;
    case VK_UP: return keyboard_virtual_key::Up;
    case VK_RIGHT: return keyboard_virtual_key::Right;
    case VK_DOWN: return keyboard_virtual_key::Down;
    case VK_PRINT: return keyboard_virtual_key::PrintScreen;
    case VK_SNAPSHOT: return keyboard_virtual_key::PrintScreen;
    case VK_INSERT: return keyboard_virtual_key::Insert;
    case VK_DELETE: return keyboard_virtual_key::Delete;
    case '0': return keyboard_virtual_key::_0;
    case '1': return keyboard_virtual_key::_1;
    case '2': return keyboard_virtual_key::_2;
    case '3': return keyboard_virtual_key::_3;
    case '4': return keyboard_virtual_key::_4;
    case '5': return keyboard_virtual_key::_5;
    case '6': return keyboard_virtual_key::_6;
    case '7': return keyboard_virtual_key::_7;
    case '8': return keyboard_virtual_key::_8;
    case '9': return keyboard_virtual_key::_9;
    case 'A': return keyboard_virtual_key::A;
    case 'B': return keyboard_virtual_key::B;
    case 'C': return keyboard_virtual_key::C;
    case 'D': return keyboard_virtual_key::D;
    case 'E': return keyboard_virtual_key::E;
    case 'F': return keyboard_virtual_key::F;
    case 'G': return keyboard_virtual_key::G;
    case 'H': return keyboard_virtual_key::H;
    case 'I': return keyboard_virtual_key::I;
    case 'J': return keyboard_virtual_key::J;
    case 'K': return keyboard_virtual_key::K;
    case 'L': return keyboard_virtual_key::L;
    case 'M': return keyboard_virtual_key::M;
    case 'N': return keyboard_virtual_key::N;
    case 'O': return keyboard_virtual_key::O;
    case 'P': return keyboard_virtual_key::P;
    case 'Q': return keyboard_virtual_key::Q;
    case 'R': return keyboard_virtual_key::R;
    case 'S': return keyboard_virtual_key::S;
    case 'T': return keyboard_virtual_key::T;
    case 'U': return keyboard_virtual_key::U;
    case 'V': return keyboard_virtual_key::V;
    case 'W': return keyboard_virtual_key::W;
    case 'X': return keyboard_virtual_key::X;
    case 'Y': return keyboard_virtual_key::Y;
    case 'Z': return keyboard_virtual_key::Z;
    case VK_NUMPAD0: return keyboard_virtual_key::_0;
    case VK_NUMPAD1: return keyboard_virtual_key::_1;
    case VK_NUMPAD2: return keyboard_virtual_key::_2;
    case VK_NUMPAD3: return keyboard_virtual_key::_3;
    case VK_NUMPAD4: return keyboard_virtual_key::_4;
    case VK_NUMPAD5: return keyboard_virtual_key::_5;
    case VK_NUMPAD6: return keyboard_virtual_key::_6;
    case VK_NUMPAD7: return keyboard_virtual_key::_7;
    case VK_NUMPAD8: return keyboard_virtual_key::_8;
    case VK_NUMPAD9: return keyboard_virtual_key::_9;
    case VK_MULTIPLY: return keyboard_virtual_key::Star;
    case VK_ADD: return keyboard_virtual_key::Plus;
    case VK_SUBTRACT: return keyboard_virtual_key::Minus;
    case VK_DECIMAL: return keyboard_virtual_key::Period;
    case VK_DIVIDE: return keyboard_virtual_key::Slash;
    case VK_F1: return keyboard_virtual_key::F1;
    case VK_F2: return keyboard_virtual_key::F2;
    case VK_F3: return keyboard_virtual_key::F3;
    case VK_F4: return keyboard_virtual_key::F4;
    case VK_F5: return keyboard_virtual_key::F5;
    case VK_F6: return keyboard_virtual_key::F6;
    case VK_F7: return keyboard_virtual_key::F7;
    case VK_F8: return keyboard_virtual_key::F8;
    case VK_F9: return keyboard_virtual_key::F9;
    case VK_F10: return keyboard_virtual_key::F10;
    case VK_F11: return keyboard_virtual_key::F11;
    case VK_F12: return keyboard_virtual_key::F12;
    case VK_F13: return keyboard_virtual_key::F13;
    case VK_F14: return keyboard_virtual_key::F14;
    case VK_F15: return keyboard_virtual_key::F15;
    case VK_F16: return keyboard_virtual_key::F16;
    case VK_F17: return keyboard_virtual_key::F17;
    case VK_F18: return keyboard_virtual_key::F18;
    case VK_F19: return keyboard_virtual_key::F19;
    case VK_F20: return keyboard_virtual_key::F20;
    case VK_F21: return keyboard_virtual_key::F21;
    case VK_F22: return keyboard_virtual_key::F22;
    case VK_F23: return keyboard_virtual_key::F23;
    case VK_F24: return keyboard_virtual_key::F24;
    case VK_BROWSER_BACK: return keyboard_virtual_key::BrowserBack;
    case VK_BROWSER_FORWARD: return keyboard_virtual_key::BrowserForward;
    case VK_BROWSER_REFRESH: return keyboard_virtual_key::BrowserRefresh;
    case VK_BROWSER_STOP: return keyboard_virtual_key::BrowserStop;
    case VK_BROWSER_SEARCH: return keyboard_virtual_key::BrowserSearch;
    case VK_BROWSER_FAVORITES: return keyboard_virtual_key::BrowserFavorites;
    case VK_BROWSER_HOME: return keyboard_virtual_key::BrowserHome;
    case VK_VOLUME_MUTE: return keyboard_virtual_key::VolumeMute;
    case VK_VOLUME_DOWN: return keyboard_virtual_key::VolumeDown;
    case VK_VOLUME_UP: return keyboard_virtual_key::VolumeUp;
    case VK_MEDIA_NEXT_TRACK: return keyboard_virtual_key::MediaNextTrack;
    case VK_MEDIA_PREV_TRACK: return keyboard_virtual_key::MediaPrevTrack;
    case VK_MEDIA_STOP: return keyboard_virtual_key::MediaStop;
    case VK_MEDIA_PLAY_PAUSE: return keyboard_virtual_key::MediaPlayPause;
    case VK_OEM_1: return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::Colon : keyboard_virtual_key::SemiColon;
    case VK_OEM_PLUS: return keyboard_virtual_key::Plus;
    case VK_OEM_COMMA: return keyboard_virtual_key::Comma;
    case VK_OEM_MINUS: return keyboard_virtual_key::Minus;
    case VK_OEM_PERIOD: return keyboard_virtual_key::Period;
    case VK_OEM_2: return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::Question : keyboard_virtual_key::Slash;
    case VK_OEM_3: return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::Tilde : keyboard_virtual_key::Backtick;
    case VK_OEM_4:
        return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::OpenBrace : keyboard_virtual_key::OpenBracket;
    case VK_OEM_5: return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::Pipe : keyboard_virtual_key::Backslash;
    case VK_OEM_6:
        return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::CloseBrace : keyboard_virtual_key::CloseBracket;
    case VK_OEM_7:
        return any(modifiers & keyboard_modifiers::Shift) ? keyboard_virtual_key::DoubleQuote : keyboard_virtual_key::Quote;
    case VK_PLAY: return keyboard_virtual_key::MediaPlayPause;
    case VK_OEM_CLEAR: return keyboard_virtual_key::Clear;
    default: return keyboard_virtual_key::Nul;
    }
}

} // namespace tt::inline v1
