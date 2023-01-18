// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file win32_headers.hpp
 *
 * Rules for working with win32 headers.
 *  - Include "win32_headers.hpp" as FIRST file by _impl.cpp files that need it.
 *  - .hpp files are not allowed to include "win32_headers.hpp".
 *  - win32 headers MAY NOT be included in any files.
 *
 * This file includes windows headers in the correct order. Any other order
 * or multiple includes will break the fragile win32 header system.
 */

#pragma once

#ifndef WIN32_NO_STATUS
#error "-DWIN32_NO_STATUS must be defined as a compile option"
#endif

#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <debugapi.h>
#include <shellapi.h>
#include <intrin.h>
#include <winuser.h>

// Window's registry.
#include <winreg.h>
#include <Uxtheme.h>

// Cryptography
#include <bcrypt.h>

// Threading
#include <synchapi.h>

// File IO
#include <ShlObj_core.h>

// Networking
#define IN
#define OUT
#include <WinSock2.h>

// DirectX.
#include <windowsx.h>
#include <ddraw.h>
#include <dwmapi.h>
#include <dxgi.h>

// initguid allows some of the header files to define actual implementations of the GUID.
// However this is incompatible with other headers which causes some values to become undefined.
#include <initguid.h>

// Multimedia and audio.
#include <mmsystem.h>
#include <mmeapi.h>
#include <mmddk.h>
#include <mmreg.h>
#include <winioctl.h>
#include <propsys.h>
#include <ks.h>
#include <ksmedia.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>

// The windows headers create all sort of insane macros.
#undef IN
#undef OUT
#undef small

#include "cast.hpp"
#include "exception.hpp"
#include <string_view>
#include <string>

namespace hi { inline namespace v1 {

/** Convert a UTF-8 std::string to a win32-API compatible std::wstring.
 */
[[nodiscard]] inline std::wstring win32_string_to_wstring(std::string_view s)
{
    auto s_len = narrow_cast<int>(s.size());
    auto r_len = MultiByteToWideChar(CP_UTF8, 0, s.data(), s_len, nullptr, 0);
    if (r_len == 0) {
        throw parse_error("win32_string_to_wstring()");
    }

    auto r = std::wstring(narrow_cast<size_t>(r_len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), s_len, r.data(), r_len);
    return r;
}

/** Convert a win32-API compatible std::wstring to a UTF-8 std::string.
 */
[[nodiscard]] inline std::string win32_wstring_to_string(std::wstring_view s)
{
    auto s_len = narrow_cast<int>(s.size());
    auto r_len = WideCharToMultiByte(CP_UTF8, 0, s.data(), s_len, nullptr, 0, nullptr, nullptr);
    if (r_len == 0) {
        throw parse_error("win32_wstring_to_string()");
    }

    auto r = std::string(narrow_cast<size_t>(r_len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), s_len, r.data(), r_len, nullptr, nullptr);
    return r;
}


}} // namespace hi::v1
