#include "resolver/Resolver.h"
#include <algorithm>
#include <cmath>

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

    static bool AbilityHasTag(const AbilityDef& a, const std::string& tag) {
        return std::find(a.tags.begin(), a.tags.end(), tag) != a.tags.end();
    }

    struct DamageContext {
        const AbilityDef& ability;
        DamageType damageType;
        EntityId casterId;
        EntityId targetId;
    };

    static bool RuleMatches(const ModifierRule &r, const World &world, const Db &db, const DamageContext& ctx) {
        if(r.when.incomingDamageType.has_value() && *r.when.incomingDamageType != ctx.damageType) return false;
        if(r.when.abilityHasTag.has_value() && !AbilityHasTag(ctx.ability, *r.when.abilityHasTag)) return false;
        if(r.when.targetHasStatusTag.has_value()) {
            const auto& target = world.Get(ctx.targetId);
            if(!world.EntityHasAnyStatusWithTag(db, target, *r.when.targetHasStatusTag)) return false;
        }
        return true;
    }

    static float ApplyHookRules(Hook hook, const Entity& owner, const World& world, const Db& db, const DamageContext& ctx, float value, ResolutionTrace& trace) {
        for(const auto& si : owner.statuses) {
            const auto& sdef = db.GetStatus(si.id);

            auto hit = sdef.hooks.find(hook);
            if(hit == sdef.hooks.end()) continue;

            const auto& rules = hit->second;
            for(const auto& r : rules) {
                if(!RuleMatches(r, world, db, ctx)) continue;

                const float before = value;

                //Flat then multiply also scale by stacks
                value += r.modify.addFlat * (float)si.stacks;
                value *= std::pow(r.modify.multiplier, (float)si.stacks);
                trace.Add("Resolving hooks:\nCurrent Status:[" + sdef.id + "] hooks:[" + 
                    std::string(hook == Hook::OnBeforeDealDamage ? "OnBeforeDealDamage" : "OnBeforeTageDamage") +
                "] stacks:[" + std::to_string(si.stacks) +
                "]\nDamage value before:[" + std::to_string(before) + "] after:[" + std::to_string(value) + "]");
                trace.Add("------------------------------------------");
            }
        }
        return value;
    }

    static float EvalAmount(const World &world, const Entity &caster, const ScaledAmount &a)
    {
        const float stat = (float)world.GetStat(caster, a.scalesWith);
        return a.base + stat * a.scale;
    }

    ResolutionTrace Resolver::Resolve(World& world, const ResolveRequest& req) const {
        ResolutionTrace trace;

        const auto& ability = db.GetAbility(req.abilityId);
        auto& caster = world.Get(req.caster);
        trace.Add("------------------------------------------");
        trace.Add("Resolving ability: [" + ability.id + "] caster id: [" + std::to_string(caster.id) + "]");
        trace.Add("------------------------------------------");

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

                        DamageContext dctx { ability, eff.damageType, req.caster, targetId };

                        // Caster hook modifier
                        raw = ApplyHookRules(Hook::OnBeforeDealDamage, caster, world, db, dctx, raw, trace);
                        // Target hook modifier
                        raw = ApplyHookRules(Hook::OnBeforeTakeDamage, target, world, db, dctx, raw, trace);

                        int dmg = (int)raw;
                        if(eff.damageType == DamageType::Physical) {
                            dmg = std::max(0, dmg - target.armor);
                        }

                        int before = target.hp;
                        target.hp -= dmg;
                        trace.Add("Resolving Damage:\nAmount:[" + std::to_string(dmg) + " " + DamageTypeName(eff.damageType) +
                                  "]\nTarget entity:[" + std::to_string(targetId) + "] HP before:[" + std::to_string(before) + 
                                  "] after:[" + std::to_string(target.hp) + "]");
                        trace.Add("------------------------------------------");
                        break;
                    }

                    case AbilityEffectDef::Kind::Heal: {
                        int heal = (int)EvalAmount(world, caster, eff.amount);
                        int before = target.hp;
                        target.hp += heal;

                        trace.Add("Effect: Heal " + std::to_string(heal) + " " + DamageTypeName(eff.damageType) +
                                  " target=" + std::to_string(targetId) + " hp before: " + std::to_string(before) +
                                  " hp now: " + std::to_string(target.hp));
                        trace.Add("------------------------------------------");
                        break;
                    }

                    case AbilityEffectDef::Kind::RemoveStatusByTag: {
                        int removed = world.RemoveStatusesByTag(db, target, eff.tag, eff.maxRemoved);
                        trace.Add("Effect: Remove Status Tag =" + eff.tag + " removed=" + std::to_string(removed) +
                                " target=" + std::to_string(targetId));
                        trace.Add("------------------------------------------");
                        break;
                    }

                    case AbilityEffectDef::Kind::ApplyStatus: {
                        world.AddStatus(db, target, eff.statusId, eff.duration, eff.stacks);
                        trace.Add("Status applied:[" + eff.statusId + "] for:[" + std::to_string(eff.duration) + "] turns");
                        trace.Add("------------------------------------------");
                        break;
                    }
                }
            }
        }
        return trace;
    }

}