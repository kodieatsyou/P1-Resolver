#include "resolver/Resolver.h"
#include "resolver/DbLoader.h"
#include <iostream>

int main() {
    using namespace res;

    Db db = DbLoader::LoadFromFiles("data/abilities.json", "data/statuses.json");
    std::cout << "burning hooks count: " << db.statuses.at("burning").hooks.size() << "\n";

    Resolver resolver(db);

    World w;
    w.entities[1] = Entity{1, 100, 0, 10, {"Player"}, {}};
    w.entities[2] = Entity{2, 100, 0, 10, {"Enemy"}, {}};
    w.AddStatus(db, w.entities[2], "burning", 2, 1);

    auto trace = resolver.Resolve(w, {"firebolt", 1, {2}});
    std::cout << trace.ToString() << "\n";

    w.AddStatus(db, w.entities[2], "shielded", 2, 1);
    trace = resolver.Resolve(w, {"strike", 1, {2}});
    std::cout << trace.ToString() << "\n";

    return 0;
}