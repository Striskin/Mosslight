#pragma once
#include "collision.hpp"
namespace moss {
struct Resident { size_t objectIndex; Vec home,target; float wait=0,walkTime=0; uint32_t random=1; Vec facing{0,1}; bool moving=false; };
std::vector<Resident> populateResidents(Region& map,uint32_t seed);
void updateResidents(Region& map,std::vector<Resident>& residents,Vec player,float dt);
}
