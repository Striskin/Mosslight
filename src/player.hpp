#pragma once
#include "collision.hpp"
#include "inventory.hpp"
namespace moss {
struct Player {
    Vec pos=tileCenter(20,17),facing={0,1},knockback{},dashDirection{};
    int health=6;
    float invulnerable=0,attackTime=0,attackCooldown=0,dashTime=0,dashCooldown=0,walkTime=0;
    float stamina=100,regenDelay=0,guardTime=0,stunned=0,attackDuration=.34f;
    unsigned attackSerial=0;
    bool moving=false,blocking=false,heavyAttack=false,shoot=false,armored=false;
    Weapon attackWeapon=Weapon::Sword;
    void update(const Input& input,const Region& map,float dt,const Inventory& inventory=Inventory{});
    bool hurt(int damage,Vec force);
    bool heal(Inventory& inventory);
    bool spendStamina(float amount);
    bool attackActive() const { return attackTime>attackDuration*.2f&&attackTime<attackDuration*.68f; }
};
}
