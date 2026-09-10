#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <string>
#include <vector>
#include <functional>

// ============================================================
// Reg - thin, friendly Windows Registry wrapper (Unicode).
//
// Every function takes the root hive + the full subkey path,
// so one-liners cover the common cases:
//
//   bool on = Reg::GetDword(HKEY_LOCAL_MACHINE,
//               L"SOFTWARE\\MyApp", L"Enabled", 0) != 0;
//   Reg::SetString(HKEY_CURRENT_USER,
//       L"Software\\MyApp", L"Name", L"aura");
//
// Reads open with KEY_QUERY_VALUE; writes create the key and
// open with KEY_SET_VALUE. Missing keys/values never crash:
// getters fall back to the default, setters return false.
// ============================================================

namespace Reg
{
    // Existence / cleanup
    bool KeyExists(HKEY root, const std::wstring& path);
    bool DeleteValue(HKEY root, const std::wstring& path, const std::wstring& name);
    bool DeleteKey(HKEY root, const std::wstring& path, bool recursive = false);

    // Strings (REG_SZ / REG_EXPAND_SZ; expandable values are
    // expanded via ExpandEnvironmentStringsW automatically).
    std::wstring GetString(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        const std::wstring& def = L""
    );
    bool SetString(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        const std::wstring& value
    );

    // DWORD (REG_DWORD)
    DWORD GetDword(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        DWORD def = 0
    );
    bool SetDword(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        DWORD value
    );

    // QWORD (REG_QWORD): packet counts, file times, etc.
    ULONGLONG GetQword(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        ULONGLONG def = 0
    );
    bool SetQword(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        ULONGLONG value
    );

    // Blobs (REG_BINARY)
    std::vector<BYTE> GetBinary(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name
    );
    bool SetBinary(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        const void* data,
        size_t size
    );

    // ============================================================
    // VBScript-style helpers: full "HIVENAME\key\...\Value" path
    //
    //   Reg::Write(L"HKCU\\Software\\MyApp", L"hello", L"REG_SZ");
    //   Reg::Write(L"HKLM\\Software\\MyApp\\Enabled", L"1", L"REG_DWORD");
    //   std::wstring v = Reg::Read(L"HKCU\\Software\\MyApp");
    //   Reg::Delete(L"HKCU\\Software\\MyApp\\OldValue");
    //
    // Hive prefixes (case-insensitive): HKLM/HKCU/HKCR/HKU/HKCC
    // and the full HKEY_... names. Type strings: REG_SZ,
    // REG_EXPAND_SZ, REG_DWORD, REG_QWORD, REG_BINARY, REG_MULTI_SZ.
    // Numbers are decimal strings; binary is whitespace-separated
    // hex ("AA 0B C1"). A trailing backslash targets the key's
    // default value. Missing values make Read return L"".
    // ============================================================
    bool Write(
        const std::wstring& fullPath,
        const std::wstring& value,
        const std::wstring& type
    );
    std::wstring Read(const std::wstring& fullPath);
    bool Delete(const std::wstring& fullPath);

    // Enumerate a key's value names / subkeys. fn returns false
    // to stop early. Empty result (false) on failure.
    bool EnumValues(
        HKEY root,
        const std::wstring& path,
        const std::function<bool(const std::wstring& name, DWORD type)>& fn
    );
    bool EnumSubkeys(
        HKEY root,
        const std::wstring& path,
        const std::function<bool(const std::wstring& name)>& fn
    );
}