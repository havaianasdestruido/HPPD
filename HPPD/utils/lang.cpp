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
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH]{};

        if (!LCIDToLocaleName(
    MAKELCID(langID, SORT_DEFAULT),
    localeName))
        {
            return L"unknown";
        }

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
