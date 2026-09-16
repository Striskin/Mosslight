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
    auto damageEnemy=[&](Enemy& e,int damage,Vec from,bool heavy) {
        bool guarded=e.type==3&&e.mode==EnemyMode::Guard&&e.stagger<=0&&dot(e.facing,normalized(from-e.pos))>.25f;
        result.swordHit=true;
        if(guarded&&!heavy) { burst(particles,e.pos,0,4); e.flash=.05f; return; }
        if(e.stagger>0) ++damage;
        if(heavy&&e.type!=2) { e.stagger=1; e.mode=EnemyMode::Recover; e.timer=1; }
        e.health=std::max(0,e.health-damage); e.flash=.14f;
        e.knockback=normalized(e.pos-from)*(e.type==2?22.0f:(e.type==3?45.0f:140.0f));
        burst(particles,e.pos,0,6);
        if(e.health<=0) {
            ++result.kills; result.bossKilled|=e.type==2;
            result.fallen.push_back({e.spawnId,e.type,e.pos}); burst(particles,e.pos,1,16);
        }
    };
    auto receive=[&](int damage,Vec force,Enemy* attacker) {
        if(p.invulnerable>0||p.health<=0) return;
        if(p.blocking&&dot(p.facing,normalized(force)*-1)>.35f) {
            bool timed=p.guardTime<.16f;
            if(p.spendStamina(timed?8.0f:damage*22.0f)) {
                p.invulnerable=.18f; result.guarded=true; burst(particles,p.pos,1,6);
                if(timed&&attacker) {
                    attacker->stagger=attacker->type==2?.55f:1.25f;
                    attacker->mode=EnemyMode::Recover; attacker->timer=attacker->stagger;
                }
                return;
            }
            p.stamina=0; p.blocking=false; p.stunned=.8f; p.regenDelay=1;
        }
        if(p.hurt(damage,force)) { result.playerHit=true; burst(particles,p.pos,2); }
    };
    for(auto& e:enemies) {
        if(e.health<=0) continue;
        Vec delta=e.pos-p.pos; float dist=length(delta);
        float reach=p.attackWeapon==Weapon::Fists?20.0f:(p.heavyAttack?35.0f:31.0f);
        if(e.type==2) reach+=8;
        if(p.attackWeapon!=Weapon::Bow&&p.attackActive()&&e.lastHit!=p.attackSerial&&dist<reach&&
           (dist<12||dot(normalized(delta),p.facing)>.1f)&&lineClear(map,p.pos,e.pos)) {
            e.lastHit=p.attackSerial;
            damageEnemy(e,(p.attackWeapon==Weapon::Fists?1:inv.damage())+(p.heavyAttack?2:0),p.pos,p.heavyAttack);
            if(e.health<=0) continue;
        }
        if(e.stagger<=0&&lineClear(map,e.pos,p.pos)) {
            if(e.type==3) {
                if(e.mode==EnemyMode::Dash&&dist<33&&dot(e.facing,normalized(p.pos-e.pos))>.15f)
                    receive(2,normalized(p.pos-e.pos)*155,&e);
            } else if(dist<(e.type==2?18:12)) {
                receive((e.type==2||e.type==4)&&e.mode==EnemyMode::Dash?2:1,normalized(p.pos-e.pos)*155,&e);
            }
        }
    }
    for(auto& s:shots) if(s.life>0) {
        if(s.friendly) {
            Enemy* hit=nullptr; float closest=1e9f;
            for(auto& e:enemies) if(e.health>0&&segmentDistance(e.pos,s.previous,s.pos)<(e.type==2?13.0f:8.0f)&&lineClear(map,s.previous,e.pos)) {
                float d=length(e.pos-s.previous); if(d<closest) { closest=d; hit=&e; }
            }
            if(hit) { damageEnemy(*hit,s.damage,s.pos-normalized(s.vel)*30,false); s.life=0; }
        } else if(segmentDistance(p.pos,s.previous,s.pos)<s.radius+5) {
            receive(s.damage,normalized(s.vel)*95,nullptr); s.life=0;
        }
    }
    return result;
}
}
