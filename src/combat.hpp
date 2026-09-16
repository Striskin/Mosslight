#pragma once
#include "enemies.hpp"
namespace moss {
struct CombatResult { int kills=0; bool bossKilled=false,playerHit=false,swordHit=false; };
CombatResult resolveCombat(Player& player,Inventory& inv,std::vector<Enemy>& enemies,
                           std::vector<Projectile>& shots,const Region& map,std::vector<Particle>& particles);
void burst(std::vector<Particle>& particles,Vec pos,int color,int count=10);
}
