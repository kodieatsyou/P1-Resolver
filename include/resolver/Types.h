#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace res
{

    using EntityId = uint32_t;

    enum class DamageType
    {
        Physical,
        Fire,
        Ice,
        Poison
    };

    enum class TargetMode
    {
        Self,
        SingleEnemy,
        SingleAlly
    };

    enum class Stat
    {
        HP,
        Armor,
        Power
    };

    struct ScaledAmount
    {
        float base = 0.f;
        Stat scalesWith = Stat::Power;
        float scale = 0.f;
    };

    struct AbilityTargeting
    {
        TargetMode mode = TargetMode::SingleEnemy;
    };

    struct AbilityEffectDef
    {
        enum class Kind
        {
            Damage,
            Heal,
            ApplyStatus,
            RemoveStatusByTag
        } kind;

        DamageType damageType = DamageType::Physical;
        ScaledAmount amount{};

        std::string statusId;
        int duration = 0;
        int stacks = 0;

        std::string tag;
        int maxRemoved = 0;
    };

    struct AbilityDef
    {
        std::string id;
        std::vector<std::string> tags;
        AbilityTargeting targeting{};
        std::vector<AbilityEffectDef> effects;
    };

    struct StatModDef
    {
        Stat stat = Stat::Armor;
        int add = 0;
    };

    struct DotDef
    {
        DamageType damageType = DamageType::Fire;
        int perStackBase = 0;
    };

    struct StatusDef
    {
        std::string id;
        std::vector<std::string> tags;
        int maxStacks = 1;
        std::vector<StatModDef> statMods;
        std::optional<DotDef> dot;
    };

}
