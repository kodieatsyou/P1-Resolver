#include "resolver/Resolver.h"
#include "resolver/DbLoader.h"
#include <cassert>
#include <iostream>

using namespace res;

static Db MakeDb()
{
    Db db;

    // Status: burning increases fire damage taken by 20%
    {
        StatusDef s;
        s.id = "burning";
        s.tags = {"Debuff", "Fire", "DoT"};
        s.maxStacks = 5;
        s.dot = DotDef{DamageType::Fire, 4};

        ModifierRule r;
        r.when.incomingDamageType = DamageType::Fire;
        r.modify.multiplier = 1.2f;
        s.hooks[Hook::OnBeforeTakeDamage].push_back(r);

        db.statuses[s.id] = s;
    }

    // Status: shielded reduces physical damage taken by 20%
    {
        StatusDef s;
        s.id = "shielded";
        s.tags = {"Buff"};
        s.maxStacks = 1;
        s.statMods = {StatModDef{Stat::Armor, 0}};

        ModifierRule r;
        r.when.incomingDamageType = DamageType::Physical;
        r.modify.multiplier = 0.8f;
        s.hooks[Hook::OnBeforeTakeDamage].push_back(r);

        db.statuses[s.id] = s;
    }

    // Ability: firebolt (base 12 + Power*0.6), Fire
    {
        AbilityDef a;
        a.id = "firebolt";
        a.tags = {"Spell", "Fire"};
        a.targeting = {TargetMode::SingleEnemy};

        AbilityEffectDef e;
        e.kind = AbilityEffectDef::Kind::Damage;
        e.damageType = DamageType::Fire;
        e.amount = ScaledAmount{12.f, Stat::Power, 0.6f};

        a.effects = {e};
        db.abilities[a.id] = a;
    }

    // Ability: strike (base 10 + Power*0.5), Physical
    {
        AbilityDef a;
        a.id = "strike";
        a.tags = {"Melee"};
        a.targeting = {TargetMode::SingleEnemy};

        AbilityEffectDef e;
        e.kind = AbilityEffectDef::Kind::Damage;
        e.damageType = DamageType::Physical;
        e.amount = ScaledAmount{10.f, Stat::Power, 0.5f};

        a.effects = {e};
        db.abilities[a.id] = a;
    }

    return db;
}

int main() 
{
    Db db = MakeDb();
    Resolver r(db);

    // --- Test 1: burning increases fire damage taken ---
    {
        World w;
        w.entities[1] = Entity{1, 100, 0, 10, {"Player"}, {}};
        w.entities[2] = Entity{2, 100, 0, 10, {"Enemy"}, {}};
        w.AddStatus(db, w.entities[2], "burning", 2, 1);

        auto trace = r.Resolve(w, {"firebolt", 1, {2}});

        // raw = 12 + 10*0.6 = 18
        // burning: *1.2 => 21.6 -> int 21
        assert(w.entities[2].hp == 79);

        // Ensure trace contains a modifier line
        bool found = false;
        for (auto &ev : trace.events)
        {
            if (ev.msg.find("Resolving hooks:\nCurrent Status:[burning]") != std::string::npos)
            {
                found = true;
                break;
            }
        }
        assert(found);
    }

    // --- Test 2: shield reduces physical damage taken ---
    {
        World w;
        w.entities[1] = Entity{1, 100, 0, 10, {"Player"}, {}};
        w.entities[2] = Entity{2, 100, 0, 10, {"Enemy"}, {}};
        w.AddStatus(db, w.entities[2], "shielded", 2, 1);

        auto trace = r.Resolve(w, {"strike", 1, {2}});

        // raw = 10 + 10*0.5 = 15
        // guarded: *0.8 => 12 -> int 12
        assert(w.entities[2].hp == 88);

        // Ensure trace contains a modifier line
        bool found = false;
        for (auto &ev : trace.events)
        {
            if (ev.msg.find("Resolving hooks:\nCurrent Status:[shielded]") != std::string::npos)
            {
                found = true;
                break;
            }
        }
        assert(found);
    }

    std::cout << "All tests passed.\n";
    return 0;
}