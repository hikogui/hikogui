// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/KeyboardVirtualKey.hpp"
#include "TTauri/GUI/KeyboardModifiers.hpp"
#include <Windows.h>

namespace TTauri::GUI {

KeyboardVirtualKey to_KeyboardVirtualKey(int key_code, bool extended, KeyboardModifiers modifiers)
{
    switch (key_code) {
    case VK_BACK: return KeyboardVirtualKey::Backspace;
    case VK_TAB: return KeyboardVirtualKey::Tab;
    case VK_CLEAR: return KeyboardVirtualKey::Clear;
    case VK_RETURN: return KeyboardVirtualKey::Enter;
    case VK_PAUSE: return KeyboardVirtualKey::PauseBreak;
    case VK_ESCAPE: return KeyboardVirtualKey::Escape;
    case VK_SPACE: return KeyboardVirtualKey::Space;
    case VK_PRIOR: return KeyboardVirtualKey::PageUp;
    case VK_NEXT: return KeyboardVirtualKey::PageDown;
    case VK_END: return KeyboardVirtualKey::End;
    case VK_HOME: return KeyboardVirtualKey::Home;
    case VK_LEFT: return KeyboardVirtualKey::Left;
    case VK_UP: return KeyboardVirtualKey::Up;
    case VK_RIGHT: return KeyboardVirtualKey::Right;
    case VK_DOWN: return KeyboardVirtualKey::Down;
    case VK_PRINT: return KeyboardVirtualKey::PrintScreen;
    case VK_SNAPSHOT: return KeyboardVirtualKey::PrintScreen;
    case VK_INSERT: return KeyboardVirtualKey::Insert;
    case VK_DELETE: return KeyboardVirtualKey::Delete;
    case '0': return KeyboardVirtualKey::_0;
    case '1': return KeyboardVirtualKey::_1;
    case '2': return KeyboardVirtualKey::_2;
    case '3': return KeyboardVirtualKey::_3;
    case '4': return KeyboardVirtualKey::_4;
    case '5': return KeyboardVirtualKey::_5;
    case '6': return KeyboardVirtualKey::_6;
    case '7': return KeyboardVirtualKey::_7;
    case '8': return KeyboardVirtualKey::_8;
    case '9': return KeyboardVirtualKey::_9;
    case 'A': return KeyboardVirtualKey::A;
    case 'B': return KeyboardVirtualKey::B;
    case 'C': return KeyboardVirtualKey::C;
    case 'D': return KeyboardVirtualKey::D;
    case 'E': return KeyboardVirtualKey::E;
    case 'F': return KeyboardVirtualKey::F;
    case 'G': return KeyboardVirtualKey::G;
    case 'H': return KeyboardVirtualKey::H;
    case 'I': return KeyboardVirtualKey::I;
    case 'J': return KeyboardVirtualKey::J;
    case 'K': return KeyboardVirtualKey::K;
    case 'L': return KeyboardVirtualKey::L;
    case 'M': return KeyboardVirtualKey::M;
    case 'N': return KeyboardVirtualKey::N;
    case 'O': return KeyboardVirtualKey::O;
    case 'P': return KeyboardVirtualKey::P;
    case 'Q': return KeyboardVirtualKey::Q;
    case 'R': return KeyboardVirtualKey::R;
    case 'S': return KeyboardVirtualKey::S;
    case 'T': return KeyboardVirtualKey::T;
    case 'U': return KeyboardVirtualKey::U;
    case 'V': return KeyboardVirtualKey::V;
    case 'W': return KeyboardVirtualKey::W;
    case 'X': return KeyboardVirtualKey::X;
    case 'Y': return KeyboardVirtualKey::Y;
    case 'Z': return KeyboardVirtualKey::Z;
    case VK_NUMPAD0: return KeyboardVirtualKey::_0;
    case VK_NUMPAD1: return KeyboardVirtualKey::_1;
    case VK_NUMPAD2: return KeyboardVirtualKey::_2;
    case VK_NUMPAD3: return KeyboardVirtualKey::_3;
    case VK_NUMPAD4: return KeyboardVirtualKey::_4;
    case VK_NUMPAD5: return KeyboardVirtualKey::_5;
    case VK_NUMPAD6: return KeyboardVirtualKey::_6;
    case VK_NUMPAD7: return KeyboardVirtualKey::_7;
    case VK_NUMPAD8: return KeyboardVirtualKey::_8;
    case VK_NUMPAD9: return KeyboardVirtualKey::_9;
    case VK_MULTIPLY: return KeyboardVirtualKey::Star;
    case VK_ADD: return KeyboardVirtualKey::Plus;
    case VK_SUBTRACT: return KeyboardVirtualKey::Minus;
    case VK_DECIMAL: return KeyboardVirtualKey::Period;
    case VK_DIVIDE: return KeyboardVirtualKey::Slash;
    case VK_F1: return KeyboardVirtualKey::F1;
    case VK_F2: return KeyboardVirtualKey::F2;
    case VK_F3: return KeyboardVirtualKey::F3;
    case VK_F4: return KeyboardVirtualKey::F4;
    case VK_F5: return KeyboardVirtualKey::F5;
    case VK_F6: return KeyboardVirtualKey::F6;
    case VK_F7: return KeyboardVirtualKey::F7;
    case VK_F8: return KeyboardVirtualKey::F8;
    case VK_F9: return KeyboardVirtualKey::F9;
    case VK_F10: return KeyboardVirtualKey::F10;
    case VK_F11: return KeyboardVirtualKey::F11;
    case VK_F12: return KeyboardVirtualKey::F12;
    case VK_F13: return KeyboardVirtualKey::F13;
    case VK_F14: return KeyboardVirtualKey::F14;
    case VK_F15: return KeyboardVirtualKey::F15;
    case VK_F16: return KeyboardVirtualKey::F16;
    case VK_F17: return KeyboardVirtualKey::F17;
    case VK_F18: return KeyboardVirtualKey::F18;
    case VK_F19: return KeyboardVirtualKey::F19;
    case VK_F20: return KeyboardVirtualKey::F20;
    case VK_F21: return KeyboardVirtualKey::F21;
    case VK_F22: return KeyboardVirtualKey::F22;
    case VK_F23: return KeyboardVirtualKey::F23;
    case VK_F24: return KeyboardVirtualKey::F24;
    case VK_BROWSER_BACK: return KeyboardVirtualKey::BrowserBack;
    case VK_BROWSER_FORWARD: return KeyboardVirtualKey::BrowserForward;
    case VK_BROWSER_REFRESH: return KeyboardVirtualKey::BrowserRefresh;
    case VK_BROWSER_STOP: return KeyboardVirtualKey::BrowserStop;
    case VK_BROWSER_SEARCH: return KeyboardVirtualKey::BrowserSearch;
    case VK_BROWSER_FAVORITES: return KeyboardVirtualKey::BrowserFavorites;
    case VK_BROWSER_HOME: return KeyboardVirtualKey::BrowserHome;
    case VK_VOLUME_MUTE: return KeyboardVirtualKey::VolumeMute;
    case VK_VOLUME_DOWN: return KeyboardVirtualKey::VolumeDown;
    case VK_VOLUME_UP: return KeyboardVirtualKey::VolumeUp;
    case VK_MEDIA_NEXT_TRACK: return KeyboardVirtualKey::MediaNextTrack;
    case VK_MEDIA_PREV_TRACK: return KeyboardVirtualKey::MediaPrevTrack;
    case VK_MEDIA_STOP: return KeyboardVirtualKey::MediaStop;
    case VK_MEDIA_PLAY_PAUSE: return KeyboardVirtualKey::MediaPlayPause;
    case VK_OEM_1: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::Colon : KeyboardVirtualKey::SemiColon;
    case VK_OEM_PLUS: return KeyboardVirtualKey::Plus;
    case VK_OEM_COMMA: return KeyboardVirtualKey::Comma;
    case VK_OEM_MINUS: return KeyboardVirtualKey::Minus;
    case VK_OEM_PERIOD: return KeyboardVirtualKey::Period;
    case VK_OEM_2: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::Question : KeyboardVirtualKey::Slash;
    case VK_OEM_3: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::Tilde : KeyboardVirtualKey::Backtick;
    case VK_OEM_4: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::OpenBrace : KeyboardVirtualKey::OpenBracket;
    case VK_OEM_5: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::Pipe : KeyboardVirtualKey::Backslash;
    case VK_OEM_6: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::CloseBrace : KeyboardVirtualKey::CloseBracket;
    case VK_OEM_7: return modifiers >= KeyboardModifiers::Shift ? KeyboardVirtualKey::DoubleQuote : KeyboardVirtualKey::Quote;
    case VK_PLAY: return KeyboardVirtualKey::MediaPlayPause;
    case VK_OEM_CLEAR: return KeyboardVirtualKey::Clear;
    default: return KeyboardVirtualKey::Nul;
    }
}

}
