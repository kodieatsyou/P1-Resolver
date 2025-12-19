#include "resolver/DbLoader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace res {

    static DamageType ParseDamageTypes(const std::string &s)
    {
        if (s == "Physical")
            return DamageType::Physical;
        if (s == "Fire")
            return DamageType::Fire;
        if (s == "Ice")
            return DamageType::Ice;
        if (s == "Poison")
            return DamageType::Poison;
        throw std::runtime_error("Unknown DamageType: " + s);
    }

    static Stat ParseStats(const std::string &s)
    {
        if (s == "HP")
            return Stat::HP;
        if (s == "Armor")
            return Stat::Armor;
        if (s == "Power")
            return Stat::Power;
        throw std::runtime_error("Unknown Stat: " + s);
    }

    static TargetMode ParseTargetModes(const std::string &s)
    {
        if (s == "Self")
            return TargetMode::Self;
        if (s == "SingleEnemy")
            return TargetMode::SingleEnemy;
        if (s == "SingleAlly")
            return TargetMode::SingleAlly;
        throw std::runtime_error("Unknown TargetMode: " + s);
    }

    static AbilityEffectDef::Kind ParseEffectKinds(const std::string &s)
    {
        if (s == "Damage")
            return AbilityEffectDef::Kind::Damage;
        if (s == "Heal")
            return AbilityEffectDef::Kind::Heal;
        if (s == "ApplyStatus")
            return AbilityEffectDef::Kind::ApplyStatus;
        if (s == "RemoveStatusByTag")
            return AbilityEffectDef::Kind::RemoveStatusByTag;
        throw std::runtime_error("Unknown Effect kind: " + s);
    }

    Db DbLoader::LoadFromFiles(const std::string& abilitiesPath, const std::string& statusesPath) {
        Db db;

        //Load statuses
        {
            std::ifstream f(statusesPath);
            if(!f){
                throw std::runtime_error("Failed to open file: " + statusesPath);
            }
            json root;
            f >> root;
            for(const auto& js: root.at("statuses")) {
                StatusDef s;
                s.id = js.at("id").get<std::string>();
                s.tags = js.value("tags", std::vector<std::string>{});
                s.maxStacks = js.value("maxStacks", 1);

                if(js.contains("statMods")) {
                    for(const auto& jm: js["statMods"]) {
                        s.statMods.push_back({
                            ParseStats(js.at("stat").get<std::string>()),
                            jm.value("add", 0)
                        });
                    }
                }

                if(js.contains("dot")) {
                    DotDef d;
                    d.damageType = ParseDamageTypes(js["dot"].at("damageType").get<std::string>());
                    d.perStackBase = js["dot"].at("perStackBase").get<int>();
                    s.dot = d;
                }

                db.statuses.emplace(s.id, std::move(s));
            }
        }

        //Load abilities
        {
            std::ifstream f(abilitiesPath);
            if(!f) {
                throw std::runtime_error("Failed to open file at: " + abilitiesPath);
            }

            json root;
            f >> root;

            for(const auto& ja: root.at("abilities")) {
                AbilityDef a;
                a.id = ja.at("id").get<std::string>();
                a.tags = ja.value("tags", std::vector<std::string>{});

                a.targeting.mode = ParseTargetModes(ja.at("targeting").at("mode").get<std::string>());

                for(const auto& je: ja.at("effects")) {
                    AbilityEffectDef e;

                    e.kind = ParseEffectKinds(je.at("type").get<std::string>());

                    if(e.kind == AbilityEffectDef::Kind::Damage || e.kind == AbilityEffectDef::Kind::Heal) {
                        e.damageType = ParseDamageTypes(je.value("damageType", "Physical"));
                        const auto& amnt = je.at("amount");
                        e.amount.base = amnt.at("base").get<float>();
                        e.amount.scalesWith = ParseStats(amnt.at("scalesWith").get<std::string>());
                        e.amount.scale = amnt.at("scale").get<float>();
                    }

                    if(e.kind == AbilityEffectDef::Kind::ApplyStatus) {
                        e.statusId = je.at("statusId").get<std::string>();
                        e.duration = je.at("duration").get<int>();
                        e.stacks = je.value("stacks", 1);

                        if(!db.statuses.contains(e.statusId)) {
                            throw std::runtime_error("Ability: " + a.id + " references unknown status: " + e.statusId + "!");
                        }
                    }

                    if(e.kind == AbilityEffectDef::Kind::RemoveStatusByTag) {
                        e.tag = je.at("tag").get<std::string>();
                        e.maxRemoved = je.at("maxRemoved").get<int>();
                    }

                    a.effects.push_back(std::move(e));
                }

                db.abilities.emplace(a.id, std::move(a));
            }
        }

        return db;
    }
}

