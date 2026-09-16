#include "npcs.hpp"
namespace moss {
namespace {
uint32_t next(uint32_t& state) { state=state*1664525u+1013904223u; return state; }
}
std::vector<Resident> populateResidents(Region& map,uint32_t seed) {
    std::vector<Resident> residents; std::vector<Vec> homes;
    if(map.id==RegionId::Village) homes={tileCenter(25,21),tileCenter(16,18),tileCenter(28,13)};
    if(map.id==RegionId::Inn) homes={tileCenter(19,18),tileCenter(28,19)};
    if(homes.empty()) return residents;
    const char* names[]={"Mira the carter","Orin the pilgrim","Edda the herbalist","Bram the mason","Sella the courier","Alder the scribe"};
    uint32_t random=seed^static_cast<uint32_t>(map.id)*0x9e3779b9u;
    for(size_t i=0;i<homes.size();++i) {
        auto variant=next(random)%6;
        Object o; o.kind=ObjectKind::Npc; o.id="visitor_"+std::to_string(variant); o.label=names[variant]; o.pos=safePosition(map,homes[i]);
        residents.push_back({map.objects.size(),o.pos,o.pos,1+float(next(random)%30)/10,0,next(random),{0,1},false});
        map.objects.push_back(o);
    }
    return residents;
}
void updateResidents(Region& map,std::vector<Resident>& residents,Vec player,float dt) {
    for(auto& r:residents) {
        auto& o=map.objects[r.objectIndex]; r.moving=false;
        if(length(o.pos-player)<35) { r.facing=normalized(player-o.pos); continue; }
        r.wait-=dt;
        if(r.wait<=0&&length(r.target-o.pos)<3) {
            Vec candidate=r.home+Vec{float(int(next(r.random)%7)-3)*16,float(int(next(r.random)%5)-2)*16};
            if(!blocked(map,candidate)&&lineClear(map,o.pos,candidate)) r.target=candidate;
            r.wait=1.5f+float(next(r.random)%20)/10;
        }
        if(r.wait<=0&&length(r.target-o.pos)>3) {
            r.facing=normalized(r.target-o.pos); Vec before=o.pos; moveBody(map,o.pos,r.facing*(16*dt));
            r.moving=length(o.pos-before)>.01f; r.walkTime+=dt;
            if(!r.moving) { r.target=o.pos; r.wait=1; }
        }
    }
}
}
