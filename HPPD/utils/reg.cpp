#include "reg.h"

#include <shlwapi.h>

#include <cstdio>
#include <cstdlib>
#include <cwchar>

#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace
{
    bool OpenKey(
        HKEY root,
        const std::wstring& path,
        REGSAM access,
        HKEY& out
    )
    {
        out = nullptr;

        if (path.empty())
            return false;

        return RegOpenKeyExW(
            root,
            path.c_str(),
            0,
            access,
            &out
        ) == ERROR_SUCCESS;
    }

    bool CreateKey(
        HKEY root,
        const std::wstring& path,
        HKEY& out
    )
    {
        out = nullptr;

        return RegCreateKeyExW(
            root,
            path.c_str(),
            0,
            nullptr,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            nullptr,
            &out,
            nullptr
        ) == ERROR_SUCCESS;
    }

    bool QueryRaw(
        HKEY root,
        const std::wstring& path,
        const std::wstring& name,
        REGSAM access,
        DWORD& type,
        std::vector<BYTE>& data
    )
    {
        HKEY key;

        if (!OpenKey(root, path, access, key))
            return false;

        DWORD size = 0;

        LONG r =
            RegQueryValueExW(
                key,
                name.c_str(),
                0,
                nullptr,
                nullptr,
                &size
            );

        if (r == ERROR_SUCCESS && size > 0)
        {
            data.resize(size);

            r =
                RegQueryValueExW(
                    key,
                    name.c_str(),
                    0,
                    &type,
                    data.data(),
                    &size
                );

            if (r != ERROR_SUCCESS)
                data.clear();
        }
        else
        {
            data.clear();
        }

        RegCloseKey(key);

        return !data.empty();
    }

    void TrimTerminators(std::wstring& s)
    {
        while (!s.empty() && s.back() == L'\0')
            s.pop_back();
    }
}

// ============================================================
// Existence / cleanup
// ============================================================

bool Reg::KeyExists(
    HKEY root,
    const std::wstring& path
)
{
    HKEY key;

    if (!OpenKey(root, path, KEY_READ, key))
        return false;

    RegCloseKey(key);

    return true;
}

bool Reg::DeleteValue(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name
)
{
    HKEY key;

    if (!OpenKey(root, path, KEY_SET_VALUE, key))
        return false;

    LONG r = RegDeleteValueW(key, name.c_str());

    RegCloseKey(key);

    return r == ERROR_SUCCESS || r == ERROR_FILE_NOT_FOUND;
}

bool Reg::DeleteKey(
    HKEY root,
    const std::wstring& path,
    bool recursive
)
{
    if (!recursive)
        return RegDeleteKeyW(root, path.c_str()) == ERROR_SUCCESS;

    return SHDeleteKeyW(root, path.c_str()) == ERROR_SUCCESS;
}

// ============================================================
// Strings
// ============================================================

std::wstring Reg::GetString(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    const std::wstring& def
)
{
    DWORD type = 0;
    std::vector<BYTE> data;

    if (!QueryRaw(root, path, name, KEY_QUERY_VALUE, type, data))
        return def;

    if (type != REG_SZ && type != REG_EXPAND_SZ)
        return def;

    std::wstring out(
        reinterpret_cast<const wchar_t*>(data.data()),
        data.size() / sizeof(wchar_t)
    );

    TrimTerminators(out);

    if (type == REG_EXPAND_SZ)
    {
        const DWORD es =
            ExpandEnvironmentStringsW(
                out.c_str(),
                nullptr,
                0
            );

        if (es > 0)
        {
            std::wstring expanded(es, L'\0');

            ExpandEnvironmentStringsW(
                out.c_str(),
                &expanded[0],
                es
            );

            TrimTerminators(expanded);

            return expanded;
        }
    }

    return out;
}

bool Reg::SetString(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    const std::wstring& value
)
{
    HKEY key;

    if (!CreateKey(root, path, key))
        return false;

    const DWORD size =
        static_cast<DWORD>(
            (value.size() + 1) * sizeof(wchar_t)
        );

    LONG r =
        RegSetValueExW(
            key,
            name.c_str(),
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(value.c_str()),
            size
        );

    RegCloseKey(key);

    return r == ERROR_SUCCESS;
}

// ============================================================
// DWORD / QWORD
// ============================================================

DWORD Reg::GetDword(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    DWORD def
)
{
    DWORD type = 0;
    std::vector<BYTE> data;

    if (!QueryRaw(root, path, name, KEY_QUERY_VALUE, type, data))
        return def;

    if (type != REG_DWORD || data.size() < sizeof(DWORD))
        return def;

    const DWORD value =
        *reinterpret_cast<const DWORD*>(data.data());

    return value;
}

bool Reg::SetDword(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    DWORD value
)
{
    HKEY key;

    if (!CreateKey(root, path, key))
        return false;

    LONG r =
        RegSetValueExW(
            key,
            name.c_str(),
            0,
            REG_DWORD,
            reinterpret_cast<const BYTE*>(&value),
            sizeof(value)
        );

    RegCloseKey(key);

    return r == ERROR_SUCCESS;
}

ULONGLONG Reg::GetQword(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    ULONGLONG def
)
{
    DWORD type = 0;
    std::vector<BYTE> data;

    if (!QueryRaw(root, path, name, KEY_QUERY_VALUE, type, data))
        return def;

    if (type != REG_QWORD || data.size() < sizeof(ULONGLONG))
        return def;

    return *reinterpret_cast<const ULONGLONG*>(data.data());
}

bool Reg::SetQword(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    ULONGLONG value
)
{
    HKEY key;

    if (!CreateKey(root, path, key))
        return false;

    LONG r =
        RegSetValueExW(
            key,
            name.c_str(),
            0,
            REG_QWORD,
            reinterpret_cast<const BYTE*>(&value),
            sizeof(value)
        );

    RegCloseKey(key);

    return r == ERROR_SUCCESS;
}

// ============================================================
// Binary blobs
// ============================================================

std::vector<BYTE> Reg::GetBinary(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name
)
{
    DWORD type = 0;
    std::vector<BYTE> data;

    QueryRaw(root, path, name, KEY_QUERY_VALUE, type, data);

    if (type != REG_BINARY)
        data.clear();

    return data;
}

bool Reg::SetBinary(
    HKEY root,
    const std::wstring& path,
    const std::wstring& name,
    const void* data,
    size_t size
)
{
    HKEY key;

    if (!CreateKey(root, path, key))
        return false;

    LONG r =
        RegSetValueExW(
            key,
            name.c_str(),
            0,
            REG_BINARY,
            static_cast<const BYTE*>(data),
            static_cast<DWORD>(size)
        );

    RegCloseKey(key);

    return r == ERROR_SUCCESS;
}

// ============================================================
// Enumeration
// ============================================================

bool Reg::EnumValues(
    HKEY root,
    const std::wstring& path,
    const std::function<bool(const std::wstring& name, DWORD type)>& fn
)
{
    HKEY key;

    if (!OpenKey(root, path, KEY_QUERY_VALUE, key))
        return false;

    bool result = true;

    for (DWORD i = 0; ; ++i)
    {
        wchar_t name[512];
        DWORD nameSize = 512;
        DWORD type = 0;

        LONG r =
            RegEnumValueW(
                key,
                i,
                name,
                &nameSize,
                nullptr,
                &type,
                nullptr,
                nullptr
            );

        if (r == ERROR_NO_MORE_ITEMS)
            break;

        if (r != ERROR_SUCCESS)
        {
            result = false;
            break;
        }

        if (!fn(std::wstring(name, nameSize), type))
            break;
    }

    RegCloseKey(key);

    return result;
}

bool Reg::EnumSubkeys(
    HKEY root,
    const std::wstring& path,
    const std::function<bool(const std::wstring& name)>& fn
)
{
    HKEY key;

    if (!OpenKey(root, path, KEY_ENUMERATE_SUB_KEYS, key))
        return false;

    bool result = true;

    for (DWORD i = 0; ; ++i)
    {
        wchar_t name[512];
        DWORD nameSize = 512;

        LONG r =
            RegEnumKeyExW(
                key,
                i,
                name,
                &nameSize,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            );

        if (r == ERROR_NO_MORE_ITEMS)
            break;

        if (r != ERROR_SUCCESS)
        {
            result = false;
            break;
        }

        if (!fn(std::wstring(name, nameSize)))
            break;
    }

    RegCloseKey(key);

    return result;
}

// ============================================================
// VBScript-style helpers (full "HIVENAME\key\...\Value" path)
// ============================================================

namespace
{
    const wchar_t* kHives[][2] =
    {
        { L"HKEY_LOCAL_MACHINE", L"HKLM" },
        { L"HKEY_CURRENT_USER",  L"HKCU" },
        { L"HKEY_CLASSES_ROOT",  L"HKCR" },
        { L"HKEY_USERS",         L"HKU"  },
        { L"HKEY_CURRENT_CONFIG", L"HKCC" },
    };

    HKEY HiveRoot(const std::wstring& hive)
    {
        for (size_t i = 0; i < 5; ++i)
        {
            if (_wcsicmp(kHives[i][0], hive.c_str()) == 0 ||
                _wcsicmp(kHives[i][1], hive.c_str()) == 0)
            {
                static const HKEY roots[] =
                {
                    HKEY_LOCAL_MACHINE,
                    HKEY_CURRENT_USER,
                    HKEY_CLASSES_ROOT,
                    HKEY_USERS,
                    HKEY_CURRENT_CONFIG,
                };
                return roots[i];
            }
        }
        return nullptr;
    }

    bool SplitFullPath(
        const std::wstring& full,
        HKEY& root,
        std::wstring& parent,
        std::wstring& name
    )
    {
        root = nullptr;
        parent.clear();
        name.clear();

        const size_t slash = full.find(L'\\');
        if (slash == std::wstring::npos)
            return false;

        root = HiveRoot(full.substr(0, slash));
        if (!root)
            return false;

        std::wstring rest = full.substr(slash + 1);

        // A trailing backslash means the key's default value.
        while (!rest.empty() && rest.back() == L'\\')
            rest.pop_back();

        const size_t last = rest.find_last_of(L'\\');

        if (last == std::wstring::npos)
        {
            parent = rest;
            name.clear();
        }
        else
        {
            parent = rest.substr(0, last);
            name = rest.substr(last + 1);
        }

        return true;
    }

    bool ParseHexBytes(const std::wstring& text, std::vector<BYTE>& bytes)
    {
        bytes.clear();

        size_t i = 0;
        while (i < text.size())
        {
            while (i < text.size() &&
                   (text[i] == L' ' || text[i] == L'\t' ||
                    text[i] == L',' || text[i] == L';'))
                ++i;

            if (i >= text.size())
                break;

            wchar_t* end = nullptr;
            const unsigned long v = wcstoul(text.c_str() + i, &end, 16);

            if (end == text.c_str() + i || v > 0xFF)
                return false;

            bytes.push_back(static_cast<BYTE>(v));

            i = static_cast<size_t>(end - text.c_str());
        }

        return true;
    }
}

bool Reg::Write(
    const std::wstring& fullPath,
    const std::wstring& value,
    const std::wstring& type
)
{
    HKEY root;
    std::wstring parent, name;

    if (!SplitFullPath(fullPath, root, parent, name))
        return false;

    HKEY key;
    if (!CreateKey(root, parent, key))
        return false;

    LONG r = ERROR_SUCCESS;

    if (_wcsicmp(type.c_str(), L"REG_SZ") == 0 ||
        _wcsicmp(type.c_str(), L"REG_EXPAND_SZ") == 0)
    {
        const DWORD ty =
            _wcsicmp(type.c_str(), L"REG_EXPAND_SZ") == 0
                ? REG_EXPAND_SZ
                : REG_SZ;

        r =
            RegSetValueExW(
                key,
                name.c_str(),
                0,
                ty,
                reinterpret_cast<const BYTE*>(value.c_str()),
                static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t))
            );
    }
    else if (_wcsicmp(type.c_str(), L"REG_DWORD") == 0)
    {
        const DWORD v = wcstoul(value.c_str(), nullptr, 10);

        r =
            RegSetValueExW(
                key,
                name.c_str(),
                0,
                REG_DWORD,
                reinterpret_cast<const BYTE*>(&v),
                sizeof(v)
            );
    }
    else if (_wcsicmp(type.c_str(), L"REG_QWORD") == 0)
    {
        const ULONGLONG v = _wcstoui64(value.c_str(), nullptr, 10);

        r =
            RegSetValueExW(
                key,
                name.c_str(),
                0,
                REG_QWORD,
                reinterpret_cast<const BYTE*>(&v),
                sizeof(v)
            );
    }
    else if (_wcsicmp(type.c_str(), L"REG_BINARY") == 0)
    {
        std::vector<BYTE> bytes;

        if (!ParseHexBytes(value, bytes))
        {
            r = ERROR_INVALID_PARAMETER;
        }
        else
        {
            r =
                RegSetValueExW(
                    key,
                    name.c_str(),
                    0,
                    REG_BINARY,
                    bytes.empty() ? nullptr : bytes.data(),
                    static_cast<DWORD>(bytes.size())
                );
        }
    }
    else if (_wcsicmp(type.c_str(), L"REG_MULTI_SZ") == 0)
    {
        // '\n'-separated lines become null-separated strings.
        std::wstring multi;

        size_t start = 0;
        size_t pos = 0;
        do
        {
            pos = value.find(L'\n', start);

            const std::wstring::size_type len =
                (pos == std::wstring::npos)
                    ? std::wstring::npos
                    : (pos - start);

            multi.append(value.substr(start, len));
            multi.push_back(L'\0');

            start = pos + 1;
        } while (pos != std::wstring::npos);

        multi.push_back(L'\0');

        r =
            RegSetValueExW(
                key,
                name.c_str(),
                0,
                REG_MULTI_SZ,
                reinterpret_cast<const BYTE*>(multi.c_str()),
                static_cast<DWORD>(multi.size() * sizeof(wchar_t))
            );
    }
    else
    {
        r = ERROR_INVALID_PARAMETER;
    }

    RegCloseKey(key);

    return r == ERROR_SUCCESS;
}

std::wstring Reg::Read(const std::wstring& fullPath)
{
    HKEY root;
    std::wstring parent, name;

    if (!SplitFullPath(fullPath, root, parent, name))
        return L"";

    DWORD type = 0;
    std::vector<BYTE> data;

    if (!QueryRaw(root, parent, name, KEY_QUERY_VALUE, type, data))
        return L"";

    switch (type)
    {
    case REG_SZ:
    case REG_EXPAND_SZ:
        return GetString(root, parent, name);

    case REG_DWORD:
    {
        if (data.size() < sizeof(DWORD))
            return L"";

        wchar_t buf[16];
        swprintf(
            buf,
            16,
            L"%lu",
            static_cast<unsigned long>(
                *reinterpret_cast<const DWORD*>(data.data())
            )
        );
        return buf;
    }

    case REG_QWORD:
    {
        if (data.size() < sizeof(ULONGLONG))
            return L"";

        wchar_t buf[24];
        swprintf(
            buf,
            24,
            L"%I64u",
            *reinterpret_cast<const ULONGLONG*>(data.data())
        );
        return buf;
    }

    case REG_BINARY:
    {
        wchar_t buf[4];
        std::wstring hex;

        for (size_t i = 0; i < data.size(); ++i)
        {
            swprintf(buf, 4, L"%02X ", data[i]);
            hex.append(buf);
        }

        return hex;
    }

    case REG_MULTI_SZ:
    {
        const wchar_t* p =
            reinterpret_cast<const wchar_t*>(data.data());
        const size_t n = data.size() / sizeof(wchar_t);
        size_t i = 0;
        std::wstring out;

        while (i < n)
        {
            size_t len = 0;
            while (i + len < n && p[i + len] != L'\0')
                ++len;

            if (!out.empty())
                out.append(L"\r\n");

            out.append(p + i, len);

            i += len;
            while (i < n && p[i] == L'\0')
                ++i;
        }

        return out;
    }

    default:
        return L"";
    }
}

bool Reg::Delete(const std::wstring& fullPath)
{
    HKEY root;
    std::wstring parent, name;

    if (!SplitFullPath(fullPath, root, parent, name))
        return false;

    // Empty value name => delete the (leaf) key, like VBS.
    if (name.empty())
        return RegDeleteKeyW(root, parent.c_str()) == ERROR_SUCCESS;

    DWORD type = 0;
    std::vector<BYTE> data;

    if (QueryRaw(root, parent, name, KEY_QUERY_VALUE, type, data))
        return DeleteValue(root, parent, name);

    // No such value: last segment is a leaf key.
    return RegDeleteKeyW(root, fullPath.c_str()) == ERROR_SUCCESS;
}