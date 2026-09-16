#pragma once
#include "maps.hpp"
namespace moss {
bool blocked(const Region& map,Vec pos,float radius=5);
void moveBody(const Region& map,Vec& pos,Vec delta,float radius=5);
bool lineClear(const Region& map,Vec from,Vec to);
Vec safePosition(const Region& map,Vec desired);
}
