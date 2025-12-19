#pragma once
#include "Types.h"
#include "Db.h"
#include "Trace.h"
#include <unordered_map>
#include <unordered_set>

namespace res {

    struct StatusInstance {
        std::string id;
        int stacks = 1;
        int remainingTurns = 1;
    };

    struct Entity {
        EntityId id{};
        int hp = 100;
        int armor = 0;
        int power = 10;

        std::unordered_set<std::string> tags;
        std::vector<StatusInstance> statuses;
    };

    struct World {
        std::unordered_map<EntityId, Entity> entities;

        Entity& Get(EntityId id);

        const Entity& Get(EntityId id) const;

        bool HasTag(const Entity& e, const std::string& tag) const;
        bool HasStatusTag(const Db& db, const StatusInstance& si, const std::string &tag) const;

        int GetStat(const Entity& e, Stat s) const;
        void AddStatus(const Db &db, Entity &target, const std::string &statusId, int duration, int stacks);
        int RemoveStatusesByTag(const Db &db, Entity &target, const std::string &tag, int maxRemoved);
        void TickTurnStart(const Db &db, ResolutionTrace &trace);

        bool EntityHasAnyStatusWithTag(const Db& db, const Entity& e, const std::string& tag) const;
    };

}