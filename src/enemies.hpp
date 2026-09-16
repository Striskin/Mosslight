#pragma once
#include "player.hpp"
namespace moss {
struct EnemyDef { std::string name; int health=3; float speed=25,notice=130; };
constexpr int EnemyTypeCount=5;
using EnemyCatalog=std::array<EnemyDef,EnemyTypeCount>;
EnemyCatalog loadEnemies(const std::filesystem::path& path);
enum class EnemyMode { Idle,Chase,Windup,Dash,Recover,Volley,Guard };
struct Enemy {
    int type=0,health=3,maxHealth=3;
    Vec pos{},home{},facing={0,1},knockback{};
    EnemyMode mode=EnemyMode::Idle;
    float timer=0,age=0,flash=0;
    int cycle=0;
    int spawnId=-1;
    float stagger=0;
    unsigned lastHit=0;
    bool phaseTwo() const { return type==2&&health*2<=maxHealth; }
};
std::vector<Enemy> spawnEnemies(const Region& map,const EnemyCatalog& defs,bool bossDefeated);
void updateEnemies(std::vector<Enemy>& enemies,std::vector<Projectile>& shots,const Region& map,
                   const Player& player,const EnemyCatalog& defs,float dt);
}
