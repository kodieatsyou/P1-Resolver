#include "resolver/World.h"
#include "resolver/Trace.h"
#include "resolver/Db.h"
#include <algorithm>

namespace res {

    Entity& World::Get(EntityId id) {return entities.at(id); }
    const Entity& World::Get(EntityId id) const { return entities.at(id); }
    
    bool World::HasTag(const Entity& e, const std::string& tag) const {
        return e.tags.find(tag) != e.tags.end();
    }

    bool World::HasStatusTag(const Db& db, const StatusInstance& si, const std::string& tag) const {
        const auto& def = db.GetStatus(si.id);
        return std::find(def.tags.begin(), def.tags.end(), tag) != def.tags.end();
    }

    int World::GetStat(const Entity& e, Stat s) const {
        switch(s) {
            case Stat::HP: return e.hp;
            case Stat::Armor: return e.armor;
            case Stat::Power: return e.power;
        }
        return 0;
    }

    void World::AddStatus(const Db& db, Entity& target, const std::string& statusId, int duration, int stacks) {
        const auto& def = db.GetStatus(statusId);

        //Check to see if entity already has this status, if so add the stacks and set the duration
        for(auto& si : target.statuses) {
            if(si.id == statusId) {
                //Dont go above max stacks
                si.stacks = std::min(def.maxStacks, si.stacks + stacks);
                si.remainingTurns = std::max(si.remainingTurns, duration);
                return;
            }
        }

        //If not add a new status instance to the targets statuses
        StatusInstance si;
        si.id = statusId;
        si.stacks = stacks;
        si.remainingTurns = duration;
        target.statuses.push_back(std::move(si));
    }

    int World::RemoveStatusesByTag(const Db& db, Entity& target, const std::string& tag, int maxRemoved) {
        int removed = 0;
        auto& v = target.statuses;

        for(int i = (int)v.size() - 1; i >= 0 && removed < maxRemoved; --i) {
            if (HasStatusTag(db, v[i], tag)) {
                v.erase(v.begin() + i);
                ++removed;
            }
        }

        return removed;
    }

    void World::TickTurnStart(const Db& db, ResolutionTrace& trace) {
        for(auto& [id, e] : entities) {
            //Apply damage over time
            for(auto& si: e.statuses) {
                const auto& def = db.GetStatus(si.id);
                if(def.dot.has_value()) {
                    int dmg = def.dot->perStackBase * si.stacks;
                    e.hp -= dmg;
                    trace.Add("Turn Start! \nEntity: [" + std::to_string(id) + "] takes [" + std::to_string(dmg) +
                    " " + (def.dot->damageType == DamageType::Fire ? "Fire" : "Poison") + 
                    "] from " + si.id + " (" + std::to_string(si.stacks) + " stacks).");
                }
            }

            // Decrement status durations
            for (int i = (int)e.statuses.size() - 1; i >= 0; i--) {
                e.statuses[i].remainingTurns -= 1;
                if(e.statuses[i].remainingTurns <= 0) {
                    trace.Add("Turn Start: Entity: " + std::to_string(id) + " status expired: " + e.statuses[i].id);
                    e.statuses.erase(e.statuses.begin() + i);
                }
            }
        }
    }

    bool World::EntityHasAnyStatusWithTag(const Db& db, const Entity& e, const std::string& tag) const {
        for(const auto& si: e.statuses) {
            if(HasStatusTag(db, si, tag)) return true;
        }
        return false;
    }
}