#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include "lang.h"

namespace Lang
{
    LANGID GetUserUI()
    {
        return ::GetUserDefaultUILanguage();
    }

    LANGID GetSystemUI()
    {
        return ::GetSystemDefaultUILanguage();
    }

    LANGID GetUserLangID()
    {
        return ::GetUserDefaultLangID();
    }

    std::wstring LangIDToName(LANGID langID)
    {
        // LCIDToLocaleName only exists from Vista and is not in
        // the W7.1A SDK; GetLocaleInfoW + LOCALE_SNAME covers
        // XP SP2 up and compiles everywhere.
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH]{};

        const int n = GetLocaleInfoW(
            MAKELCID(langID, SORT_DEFAULT),
            LOCALE_SNAME,
            localeName,
            LOCALE_NAME_MAX_LENGTH);

        if (n <= 0)
            return L"unknown";

        return localeName;
    }

    SystemLanguage GetSystemLanguage()
    {
        SystemLanguage result{};

        result.userDefaultUILanguage = GetUserUI();
        result.systemDefaultUILanguage = GetSystemUI();
        result.userDefaultLangID = GetUserLangID();

        result.userDefaultUILanguageName =
            LangIDToName(result.userDefaultUILanguage);

        result.systemDefaultUILanguageName =
            LangIDToName(result.systemDefaultUILanguage);

        result.userDefaultLangIDName =
            LangIDToName(result.userDefaultLangID);

        return result;
    }
}

/*
#include <iostream>
#include "lang.h"

int main()
{
    const auto lang = Lang::GetSystemLanguage();

    std::wcout
        << L"User UI: "
        << lang.userDefaultUILanguageName
        << L" (0x"
        << std::hex
        << lang.userDefaultUILanguage
        << L")\n";

    std::wcout
        << L"System UI: "
        << lang.systemDefaultUILanguageName
        << L" (0x"
        << std::hex
        << lang.systemDefaultUILanguage
        << L")\n";

    std::wcout
        << L"User LangID: "
        << lang.userDefaultLangIDName
        << L" (0x"
        << std::hex
        << lang.userDefaultLangID
        << L")\n";
}
*/
