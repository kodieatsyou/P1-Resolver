#include "resolver/Resolver.h"
#include "resolver/DbLoader.h"
#include <iostream>

int main() {
    using namespace res;

    Db db = DbLoader::LoadFromFiles("data/abilities.json", "data/statuses.json");

    World world;
    world.entities[1] = Entity{1, 100, 2, 12, {"Player"}, {}};
    world.entities[2] = Entity{2, 100, 1, 8, {"Enemy"}, {}};

    Resolver resolver(db);

    {
        auto turn = resolver.Resolve(world, {"firebolt", 1, {2}});
        std::cout << turn.ToString() << "\n";
    }

    {
        ResolutionTrace trace;
        world.TickTurnStart(db, trace);
        std::cout << trace.ToString() << "\n";
    }

    return 0;
}