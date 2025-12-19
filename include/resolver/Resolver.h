#pragma once
#include "Db.h"
#include "World.h"
#include "Trace.h"

namespace res {

    struct ResolveRequest {
        std::string abilityId;
        EntityId caster{};
        std::vector<EntityId> targets;
    };

    struct Resolver {
        const Db& db;

        explicit Resolver(const Db& d) : db(d) {}

        ResolutionTrace Resolve(World& world, const ResolveRequest& req) const;   
    };

}