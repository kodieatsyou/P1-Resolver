#include "resolver/Resolver.h"
#include "resolver/DbLoader.h"
#include <cassert>
#include <iostream>

using namespace res;

int main() 
{
    Db db = DbLoader::LoadFromFiles("data/abilities.json", "data/statuses.json");
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