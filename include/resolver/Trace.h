#pragma once
#include <string>
#include <vector>

namespace res {

    struct TraceEvent {
        std::string msg;
    };

    struct ResolutionTrace {
        std::vector<TraceEvent> events;

        void Add(std::string m) { events.push_back({std::move(m)}); }
        std::string ToString() const;
    };
}