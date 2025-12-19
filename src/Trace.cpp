#include "resolver/Trace.h"
#include <sstream>

namespace res {

    std::string ResolutionTrace::ToString() const {
        std::ostringstream oss;
        for(const auto& e : events) oss << e.msg << "\n";
        return oss.str();
    }
}