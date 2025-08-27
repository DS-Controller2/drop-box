#include "Utils.h"
#include <iostream>

namespace Utils {
    void log(const std::string& message) {
        std::cout << "[LOG] " << message << std::endl;
    }
}
