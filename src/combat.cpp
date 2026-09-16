#include "combat.hpp"
namespace moss {
void burst(std::vector<Particle>& particles,Vec pos,int color,int count) {
    for(int i=0;i<count;++i) {
        float angle=2*Pi*i/count;
        float speed=18+(i%4)*12.0f;
        particles.push_back({pos,{std::cos(angle)*speed,std::sin(angle)*speed},.38f,.38f,color});
    }
}
CombatResult resolveCombat(Player& p,Inventory& inv,std::vector<Enemy>& enemies,
                           std::vector<Projectile>& shots,const Region& map,std::vector<Particle>& particles) {
    CombatResult result;
    for(auto& e:enemies) {
        if(e.health<=0) continue;
        Vec delta=e.pos-p.pos; float dist=length(delta);
        if(p.attackTime>0&&e.lastHit!=p.attackSerial&&dist<(e.type==2?39:31)&&
           (dist<12||dot(normalized(delta),p.facing)>.1f)&&lineClear(map,p.pos,e.pos)) {
            e.lastHit=p.attackSerial; e.health=std::max(0,e.health-inv.damage()); e.flash=.14f;
            e.knockback=normalized(delta)*(e.type==2?22.0f:140.0f); result.swordHit=true;
            burst(particles,e.pos,0,6);
            if(e.health<=0) {
                ++result.kills; result.bossKilled|=e.type==2; inv.add(Item::Fragment,e.type==2?5:1);
                burst(particles,e.pos,1,16); continue;
            }
        }
        if(dist<(e.type==2?18:12)&&p.hurt(e.type==2&&e.mode==EnemyMode::Dash?2:1,normalized(p.pos-e.pos)*155)) {
            result.playerHit=true; burst(particles,p.pos,2);
        }
    }
    for(auto& s:shots) if(s.life>0&&length(s.pos-p.pos)<s.radius+5) {
        if(p.hurt(s.damage,normalized(s.vel)*95)) { result.playerHit=true; burst(particles,p.pos,2); }
        s.life=0;
    }
    return result;
}
}
