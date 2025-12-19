#pragma once
#include "Db.h"
#include <string>

namespace res {

    struct DbLoader {
        static Db LoadFromFiles(const std::string& abilitiesPath,
                                const std::string& statusesPath);
    };
    
}