#pragma once
#include <string>

namespace Utils
{
    static std::string GetBaseDir(const std::string& filepath) {
        if (filepath.find_last_of("/\\") != std::string::npos)
            return filepath.substr(0, filepath.find_last_of("/\\"));
        return "";
    }
}
