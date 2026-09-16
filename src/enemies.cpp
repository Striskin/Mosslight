#include "enemies.hpp"
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace moss {
EnemyCatalog loadEnemies(const std::filesystem::path& path) {
    std::ifstream in(path); EnemyCatalog defs; int id;
    for(int i=0;i<3;++i) {
        if(!(in>>id>>std::quoted(defs[i].name)>>defs[i].health>>defs[i].speed>>defs[i].notice)||id!=i||defs[i].health<1||defs[i].speed<=0)
            throw std::runtime_error("Invalid enemy catalog");
    }
    return defs;
}
std::vector<Enemy> spawnEnemies(const Region& map,const EnemyCatalog& defs,bool won) {
    std::vector<Enemy> result;
    for(const auto& s:map.spawns) {
        if(s.type==2&&won) continue;
        Enemy e; e.type=s.type; e.health=e.maxHealth=defs[s.type].health; e.pos=e.home=s.pos;
        e.timer=.5f+result.size()*.31f; result.push_back(e);
    }
    return result;
}
void updateEnemies(std::vector<Enemy>& list,std::vector<Projectile>& shots,const Region& map,
                   const Player& p,const EnemyCatalog& defs,float dt) {
    for(auto& e:list) {
        if(e.health<=0) continue;
        e.age+=dt; e.timer-=dt; e.flash=std::max(0.0f,e.flash-dt);
        Vec diff=p.pos-e.pos,dir=normalized(diff); float dist=length(diff);
        bool sees=dist<defs[e.type].notice&&lineClear(map,e.pos,p.pos);
        Vec velocity{};
        if(e.type==0) {
            if(e.mode==EnemyMode::Dash) {
                velocity=e.facing*112;
                if(e.timer<=0) { e.mode=EnemyMode::Recover; e.timer=.75f; }
            } else if(e.mode==EnemyMode::Windup) {
                if(e.timer<=0) { e.mode=EnemyMode::Dash; e.timer=.24f; }
            } else if(e.mode==EnemyMode::Recover&&e.timer>0) {
            } else if(sees) {
                e.facing=dir;
                if(dist<58) { e.mode=EnemyMode::Windup; e.timer=.5f; }
                else { e.mode=EnemyMode::Chase; velocity=dir*defs[0].speed; }
            } else { e.mode=EnemyMode::Idle; if(length(e.home-e.pos)>8) velocity=normalized(e.home-e.pos)*13; }
        } else if(e.type==1) {
            if(sees) {
                e.facing=dir; e.mode=EnemyMode::Chase;
                float toward=dist>95?1.0f:(dist<58?-1.0f:0.0f);
                velocity=dir*(toward*defs[1].speed)+Vec{-dir.y,dir.x}*(std::sin(e.age)*13);
                if(e.timer<.45f) { velocity={}; e.mode=EnemyMode::Windup; }
                if(e.timer<=0) { shots.push_back({e.pos+dir*9,dir*70,3,1,3}); e.timer=2.1f; }
            } else { e.mode=EnemyMode::Idle; if(length(e.home-e.pos)>8) velocity=normalized(e.home-e.pos)*15; }
        } else {
            const bool phase=e.phaseTwo();
            switch(e.mode) {
            case EnemyMode::Idle:
            case EnemyMode::Chase:
                if(sees) { e.mode=EnemyMode::Recover; e.timer=.65f; }
                break;
            case EnemyMode::Recover:
                if(e.timer<=0) {
                    ++e.cycle; e.facing=dir;
                    e.mode=(e.cycle%2==0)?EnemyMode::Volley:EnemyMode::Windup;
                    e.timer=phase?.62f:.9f;
                }
                break;
            case EnemyMode::Windup:
                if(e.timer<=0) { e.mode=EnemyMode::Dash; e.timer=phase?.55f:.48f; }
                break;
            case EnemyMode::Dash:
                velocity=e.facing*(phase?180.0f:140.0f);
                if(e.timer<=0) { e.mode=EnemyMode::Recover; e.timer=phase?.65f:1.1f; }
                break;
            case EnemyMode::Volley:
                if(e.timer<=0) {
                    int n=phase?12:8;
                    float offset=e.cycle*.31f;
                    for(int i=0;i<n;++i) {
                        float angle=2*Pi*i/n+offset; Vec d={std::cos(angle),std::sin(angle)};
                        shots.push_back({e.pos+d*15,d*(phase?84.0f:64.0f),4,1,3});
                    }
                    e.mode=EnemyMode::Recover; e.timer=phase?.85f:1.3f;
                }
                break;
            }
        }
        moveBody(map,e.pos,(velocity+e.knockback)*dt,e.type==2?10.0f:5.0f);
        e.knockback=e.knockback*std::max(0.0f,1-10*dt);
    }
    for(auto& s:shots) {
        s.life-=dt; Vec next=s.pos+s.vel*dt;
        if(!lineClear(map,s.pos,next)) s.life=0;
        s.pos=next;
    }
    shots.erase(std::remove_if(shots.begin(),shots.end(),[](const auto& s){return s.life<=0;}),shots.end());
}
}
