#pragma once
#include <iostream>
#include <string>

namespace Logger {
    inline void Log(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }

    inline void Error(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << std::endl;
    }

    inline void Warning(const std::string& msg) {
        std::cout << "[WARN] " << msg << std::endl;
    }
}
