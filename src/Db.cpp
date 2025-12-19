#include "resolver/Db.h"
#include <stdexcept>

namespace res {

    const AbilityDef& Db::GetAbility(const std::string& id) const {
        auto it = abilities.find(id);
        if (it == abilities.end()) throw std::runtime_error("Unknown Ability: " + id);
        return it->second;
    }

    const StatusDef& Db::GetStatus(const std::string& id) const {
        auto it = statuses.find(id);
        if (it == statuses.end())
            throw std::runtime_error("Unknown Status: " + id);
        return it->second;
    }

}