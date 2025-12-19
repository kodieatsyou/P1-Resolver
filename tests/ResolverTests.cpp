#include "resolver/Resolver.h"
#include <cassert>
#include <iostream>

using namespace res;

static Db MakeDb() {

    Db db;

    db.statuses["shielded"] = StatusDef{"shielded", {"Buff"}, 1, {StatModDef{Stat::Armor, 6}}, std::nullopt};
    db.statuses["burning"] = StatusDef{"burning", {"Debuff", "Fire", "DoT"}, 5, {}, DotDef{DamageType::Fire, 4}};
    db.statuses["poisoned"] = StatusDef{"poisoned", {"Debuff", "Poison", "DoT"}, 5, {}, DotDef{DamageType::Poison, 3}};

    auto dmg = [](DamageType dt, float base, Stat s, float scale)
    {
        AbilityEffectDef e;
        e.kind = AbilityEffectDef::Kind::Damage;
        e.damageType = dt;
        e.amount = ScaledAmount{base, s, scale};
        return e;
    };
    auto apply = [](std::string id, int dur, int stacks)
    {
        AbilityEffectDef e;
        e.kind = AbilityEffectDef::Kind::ApplyStatus;
        e.statusId = std::move(id);
        e.duration = dur;
        e.stacks = stacks;
        return e;
    };

    AbilityDef strike;
    strike.id = "strike";
    strike.tags = {"Melee"};
    strike.targeting = {TargetMode::SingleEnemy};
    strike.effects = {dmg(DamageType::Physical, 10, Stat::Power, 0.5f)};
    db.abilities[strike.id] = strike;

    AbilityDef shield;
    shield.id = "shielf";
    shield.tags = {"Buff"};
    shield.targeting = {TargetMode::Self};
    {
        AbilityEffectDef e;
        e.kind = AbilityEffectDef::Kind::ApplyStatus;
        e.statusId = "shielded";
        e.duration = 1;
        e.stacks = 1;
        shield.effects = {e};
    }
    db.abilities[shield.id] = shield;

    AbilityDef firebolt;
    firebolt.id = "firebolt";
    firebolt.tags = {"Spell", "Fire"};
    firebolt.targeting = {TargetMode::SingleEnemy};
    firebolt.effects = {
        dmg(DamageType::Fire, 12, Stat::Power, 0.6f),
        apply("burning", 2, 1)};
    db.abilities[firebolt.id] = firebolt;

    AbilityDef posison;
    posison.id = "posison_dart";
    posison.tags = {"Ranged", "Poison"};
    posison.targeting = {TargetMode::SingleEnemy};
    posison.effects = {
        dmg(DamageType::Physical, 6, Stat::Power, 0.3f),
        apply("poisoned", 3, 1)};
    db.abilities[posison.id] = posison;

    AbilityDef cleanse;
    cleanse.id = "cleanse";
    cleanse.tags = {"Spell", "Heal"};
    cleanse.targeting = {TargetMode::SingleAlly};
    {
        AbilityEffectDef e;
        e.kind = AbilityEffectDef::Kind::RemoveStatusByTag;
        e.tag = "Debuff";
        e.maxRemoved = 99;
        cleanse.effects = {e};
    }
    db.abilities[cleanse.id] = cleanse;

    return db;
}

int main() 
{
    Db db = MakeDb();
    World world;
    world.entities[1] = Entity{1, 100, 0, 10, {"Player"}, {}};
    world.entities[2] = Entity{2, 100, 0, 10, {"Enemy"}, {}};

    Resolver r(db);
    auto t = r.Resolve(world, {"firebolt", 1, {2}});

    // Expected damage = 12 + 10*0.6 = 18
    assert(world.entities[2].hp == 82);
    assert(world.entities[2].statuses.size() == 1);
    assert(world.entities[2].statuses[0].id == "burning");
    std::cout << "All tests passed.\n";
    return 0;
}