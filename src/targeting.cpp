#include "targeting.hpp"
namespace moss {
namespace {
bool eligible(const Enemy& e,const Region& map,Vec player,float range) {
    return e.spawnId>=0&&e.health>0&&length(e.pos-player)<=range&&lineClear(map,player,e.pos);
}
}
const Enemy* Targeting::enemy(const std::vector<Enemy>& enemies,const Region& map,Vec player) const {
    for(const auto& e:enemies) if(e.spawnId==spawnId&&eligible(e,map,player,156)) return &e;
    return nullptr;
}
void Targeting::update(Input& input,const std::vector<Enemy>& enemies,const Region& map,Vec player) {
    const Enemy* current=enemy(enemies,map,player);
    if(!current) clear();
    // Manual mouse aim deliberately releases focus, including its camera framing.
    if(input.aiming) { clear(); return; }
    if(input.lockTarget&&current) { clear(); return; }
    if(input.lockTarget||input.nextTarget) {
        const Enemy* picked=nullptr;
        if(input.nextTarget&&current) {
            auto start=static_cast<size_t>(current-enemies.data());
            for(size_t step=1;step<=enemies.size();++step) {
                const auto& e=enemies[(start+step)%enemies.size()];
                if(eligible(e,map,player,136)) { picked=&e; break; }
            }
            if(!picked) picked=current;
        } else {
            float nearest=137;
            for(const auto& e:enemies) if(eligible(e,map,player,136)) {
                float distance=length(e.pos-player);
                if(distance<nearest) { nearest=distance; picked=&e; }
            }
        }
        spawnId=picked?picked->spawnId:-1;
    }
    if(const auto* e=enemy(enemies,map,player)) { input.aiming=true; input.aim=e->pos-player; }
}
}
