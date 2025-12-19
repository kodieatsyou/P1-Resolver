#include "resolver/Resolver.h"
#include "resolver/DbLoader.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace res;


static void AssertGolden(const std::string& name, const std::string& got, const std::string& expected) {
    if(got != expected) {
        std::cerr << "MISMATCH: " << name << "\n";
        std::cerr << "\n------- EXPECTED-------\n" << expected;
        std::cerr << "\n------- GOT -------\n" << got;
        assert(false);
    }
}

int main() 
{
    Db db = DbLoader::LoadFromFiles("data/abilities.json", "data/statuses.json");
    Resolver resolver(db);

    // Case: firebolt into burning target should include modifier and higher damage
    {
        World w;
        w.entities[1] = Entity{1, 100, 0, 10, {"Player"}, {}};
        w.entities[2] = Entity{2, 100, 0, 10, {"Enemy"}, {}};

        w.AddStatus(db, w.entities[2], "burning", 2, 1);

        auto trace = resolver.Resolve(w, {"firebolt", 1, {2}});
        const std::string got = trace.ToString();

        const std::string expected =
            "------------------------------------------\n"
            "Resolving ability: [firebolt] caster id: [1]\n"
            "------------------------------------------\n"
            "Resolving hooks:\n"
            "Current Status:[burning] hooks:[OnBeforeTakeDamage] stacks:[1]\n"
            "Damage value before:[18.00] after:[21.60]\n"
            "------------------------------------------\n"
            "Resolving Damage:\n"
            "Amount:[21 Fire]\n"
            "Target entity:[2] HP before:[100] after:[79]\n"
            "------------------------------------------\n"
            "Status applied:[burning] for:[2] turns\n"
            "------------------------------------------\n";
        AssertGolden("firebolt_vs_burning", got, expected);
    }

    // Case: strike into shielded target should include modifier and reduced damage
    {
        World w;
        w.entities[1] = Entity{1, 100, 0, 10, {"Player"}, {}};
        w.entities[2] = Entity{2, 100, 0, 10, {"Enemy"}, {}};

        w.AddStatus(db, w.entities[2], "shielded", 2, 1);

        auto trace = resolver.Resolve(w, {"strike", 1, {2}});
        const std::string got = trace.ToString();

        const std::string expected =
            "------------------------------------------\n"
            "Resolving ability: [strike] caster id: [1]\n"
            "------------------------------------------\n"
            "Resolving hooks:\n"
            "Current Status:[shielded] hooks:[OnBeforeTakeDamage] stacks:[1]\n"
            "Damage value before:[15.00] after:[12.00]\n"
            "------------------------------------------\n"
            "Resolving Damage:\n"
            "Amount:[12 Physical]\n"
            "Target entity:[2] HP before:[100] after:[88]\n"
            "------------------------------------------\n";

        AssertGolden("strike_vs_shielded", got, expected);
    }

    std::cout << "All tests passed.\n";
    return 0;
}