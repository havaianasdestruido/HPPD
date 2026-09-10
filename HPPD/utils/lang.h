#pragma once

#include <Windows.h>
#include <string>

namespace Lang
{
    struct SystemLanguage
    {
        LANGID userDefaultUILanguage;
        LANGID systemDefaultUILanguage;
        LANGID userDefaultLangID;

        std::wstring userDefaultUILanguageName;
        std::wstring systemDefaultUILanguageName;
        std::wstring userDefaultLangIDName;
    };

    // GetUserDefaultUILanguage
    LANGID GetUserUI();

    // GetSystemDefaultUILanguage
    LANGID GetSystemUI();

    // GetUserDefaultLangID
    LANGID GetUserLangID();

    // Converte LANGID para uma string como "pt-BR", "en-US", etc.
    std::wstring LangIDToName(LANGID langID);

    // Obtém todas as informações de idioma de uma vez.
    SystemLanguage GetSystemLanguage();
}