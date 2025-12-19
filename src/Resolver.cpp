#include "resolver/Resolver.h"
#include <algorithm>

namespace res {

    static std::string DamageTypeName(DamageType t) {
        switch(t) {
            case DamageType::Physical: return "Physical";
            case DamageType::Fire: return "Fire";
            case DamageType::Ice: return "Ice";
            case DamageType::Poison:return "Poison";
            }
        return "Unknown";
    }

    static float EvalAmount(const World& world, const Entity& caster, const ScaledAmount& a) {
        const float stat = (float)world.GetStat(caster, a.scalesWith);
        return a.base + stat * a.scale;
    }

    ResolutionTrace Resolver::Resolve(World& world, const ResolveRequest& req) const {
        ResolutionTrace trace;

        const auto& ability = db.GetAbility(req.abilityId);
        auto& caster = world.Get(req.caster);
        trace.Add("------------------------------------------");
        trace.Add("Resolving ability: [" + ability.id + "] caster id: [" + std::to_string(caster.id) + "]");


        //If the target mode is self, make sure we are only targeting ourself, otherwise trace an error.
        if(ability.targeting.mode == TargetMode::Self) {
            if(req.targets.size() != 1 || req.targets[0] != req.caster) {
                trace.Add("Error: Self ability requires target to be the caster");
                return trace;
            }
        }

        //If the target mode is a single enemy, make sure we are only targeting one, otherwise trace an error.
        if (ability.targeting.mode == TargetMode::SingleEnemy)
        {
            if (req.targets.size() != 1)
            {
                trace.Add("Error: Single enemy ability can only have one target");
                return trace;
            }
        }

        for(EntityId targetId : req.targets) {
            auto &target = world.Get(targetId);

            for(const auto& eff : ability.effects) {
                switch(eff.kind) {

                    case AbilityEffectDef::Kind::Damage: {
                        float raw = EvalAmount(world, caster, eff.amount);

                        int dmg = (int)raw;
                        if(eff.damageType == DamageType::Physical) {
                            dmg = std::max(0, dmg - target.armor);
                        }

                        int before = target.hp;
                        target.hp -= dmg;

                        trace.Add("Damage: " + std::to_string(dmg) + " " + DamageTypeName(eff.damageType) +
                                  "\nTarget: [" + std::to_string(targetId) + "] HP before: [" + std::to_string(before) + 
                                  "] HP after: [" + std::to_string(target.hp) + "]");
                        break;
                    }

                    case AbilityEffectDef::Kind::Heal: {
                        int heal = (int)EvalAmount(world, caster, eff.amount);
                        int before = target.hp;
                        target.hp += heal;

                        trace.Add("Effect: Heal " + std::to_string(heal) + " " + DamageTypeName(eff.damageType) +
                                  " target=" + std::to_string(targetId) + " hp before: " + std::to_string(before) +
                                  " hp now: " + std::to_string(target.hp));
                        break;
                    }

                    case AbilityEffectDef::Kind::RemoveStatusByTag: {
                        int removed = world.RemoveStatusesByTag(db, target, eff.tag, eff.maxRemoved);
                        trace.Add("Effect: Remove Status Tag =" + eff.tag + " removed=" + std::to_string(removed) +
                                " target=" + std::to_string(targetId));
                        break;
                    }

                    case AbilityEffectDef::Kind::ApplyStatus: {
                        world.AddStatus(db, target, eff.statusId, eff.duration, eff.stacks);
                        trace.Add("Status applied: [" + eff.statusId + "] for: [" + std::to_string(eff.duration) + "] turns");
                        break;
                    }
                }
            }
        }
        return trace;
    }

}