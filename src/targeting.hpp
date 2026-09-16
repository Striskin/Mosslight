#pragma once
#include "enemies.hpp"
namespace moss {
// Runtime-only focus, identified by stable spawn id rather than a vector pointer.
class Targeting {
public:
    const Enemy* enemy(const std::vector<Enemy>& enemies,const Region& map,Vec player) const;
    void update(Input& input,const std::vector<Enemy>& enemies,const Region& map,Vec player);
    void clear() { spawnId=-1; }
private:
    int spawnId=-1;
};
}
