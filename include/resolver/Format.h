#pragma once
#include <string>
#include <sstream>
#include <iomanip>

namespace res {

    inline std::string FmtFloat(float v, int decimals = 2) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss << std::setprecision(decimals) << v;
        return oss.str();
    }

}