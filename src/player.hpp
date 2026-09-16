#pragma once
#include "collision.hpp"
#include "inventory.hpp"
namespace moss {
struct Player {
    Vec pos=tileCenter(20,17),facing={0,1},knockback{},dashDirection{};
    int health=6;
    float invulnerable=0,attackTime=0,attackCooldown=0,dashTime=0,dashCooldown=0,walkTime=0;
    unsigned attackSerial=0;
    bool moving=false;
    void update(const Input& input,const Region& map,float dt);
    bool hurt(int damage,Vec force);
    bool heal(Inventory& inventory);
};
}
