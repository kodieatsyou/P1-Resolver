#pragma once
#include "Types.h"
#include <unordered_map>

namespace res {

    struct Db {
        std::unordered_map<std::string, AbilityDef> abilities;
        std::unordered_map<std::string, StatusDef> statuses;

        const AbilityDef& GetAbility(const std::string& id) const;
        const StatusDef &GetStatus(const std::string &id) const;
    };

}