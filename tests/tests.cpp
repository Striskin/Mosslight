#include "game.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>

using namespace moss;
namespace {
int checks=0;
void require(bool condition,const std::string& message) {
    ++checks; if(!condition) throw std::runtime_error(message);
}
bool near(float a,float b,float tolerance=.01f) { return std::abs(a-b)<tolerance; }
std::vector<bool> reachable(const Region& m,Vec start) {
    std::vector<bool> visited(m.width*m.height,false); std::queue<std::pair<int,int>> todo;
    int sx=int(start.x)/16,sy=int(start.y)/16; todo.push({sx,sy}); visited[sy*m.width+sx]=true;
    const int dx[]={1,-1,0,0},dy[]={0,0,1,-1};
    while(!todo.empty()) {
        auto [x,y]=todo.front(); todo.pop();
        for(int i=0;i<4;++i) {
            int nx=x+dx[i],ny=y+dy[i];
            if(nx<0||ny<0||nx>=m.width||ny>=m.height||m.solid(nx,ny)||visited[ny*m.width+nx]) continue;
            visited[ny*m.width+nx]=true; todo.push({nx,ny});
        }
    }
    return visited;
}
void checkConnectivity(const Region& m) {
    require(validateRegion(m).empty(),m.name+" has invalid objects");
    Vec start=m.exits.front().bounds.x<32?tileCenter(3,int(m.exits.front().bounds.y)/16+1):
        safePosition(m,{m.exits.front().bounds.x+16,m.exits.front().bounds.y+16});
    auto seen=reachable(m,start);
    for(const auto& o:m.objects) require(seen[int(o.pos.y)/16*m.width+int(o.pos.x)/16],m.name+" unreachable object "+o.id);
    for(const auto& e:m.exits) {
        bool found=false;
        for(int y=int(e.bounds.y)/16;y<(e.bounds.y+e.bounds.h)/16;++y)
            for(int x=int(e.bounds.x)/16;x<(e.bounds.x+e.bounds.w)/16;++x) found=found||seen[y*m.width+x];
        require(found,m.name+" unreachable exit");
    }
    for(const auto& s:m.spawns) require(seen[int(s.pos.y)/16*m.width+int(s.pos.x)/16],m.name+" unreachable enemy");
}
void interactWith(Game& g,const std::string& id) {
    g.dialogue={}; g.screen=Screen::Playing;
    for(const auto& o:g.map.objects) if(o.id==id) {
        g.player.pos=o.pos+Vec{0,14}; g.interact(); return;
    }
    throw std::runtime_error("Missing interaction: "+id);
}
void testMaps(const std::filesystem::path& data) {
    for(int i=0;i<RegionCount;++i) {
        if(i==static_cast<int>(RegionId::Hollow)) continue;
        Region m=loadRegion(data,static_cast<RegionId>(i)); checkConnectivity(m);
        for(const auto& e:m.exits) {
            Region target=e.target==RegionId::Hollow?generateHollow(data,11):loadRegion(data,e.target);
            require(!blocked(target,e.spawn),m.name+" exit has blocked destination");
            for(const auto& back:target.exits) require(!contains(back.bounds,e.spawn),m.name+" arrival immediately re-enters an exit");
        }
        for(const auto& o:m.objects) if(o.kind==ObjectKind::Door) {
            auto target=loadRegion(data,o.target); require(!blocked(target,o.arrival),"Door destination must be walkable");
            for(const auto& ex:target.exits) require(!contains(ex.bounds,o.arrival),"Door must not immediately eject the player");
        }
    }
    auto first=generateHollow(data,42),same=generateHollow(data,42),other=generateHollow(data,43);
    require(first.tiles==same.tiles,"Seed must reproduce cave tiles");
    require(first.spawns.size()==same.spawns.size(),"Seed must reproduce cave encounters");
    require(first.objects[2].item==same.objects[2].item,"Seed must reproduce cave loot");
    require(first.tiles!=other.tiles,"Different seeds should vary caves");
    for(uint32_t seed=1;seed<=256;++seed) checkConnectivity(generateHollow(data,seed));
    std::cout<<"PASS: handcrafted map routes, destination spawns, 256 seeded cave layouts\n";
}
void testMovement(const std::filesystem::path& data) {
    Region m=loadRegion(data,RegionId::Shrine); Player a,b;
    a.pos=b.pos=tileCenter(23,26); Input straight,diagonal; straight.move={1,0}; diagonal.move={1,1};
    Vec origin=a.pos;
    for(int i=0;i<30;++i) { a.update(straight,m,1.0f/60); b.update(diagonal,m,1.0f/60); }
    require(near(length(a.pos-origin),length(b.pos-origin),.03f),"Diagonal movement must be normalized");
    require(b.facing.x>0&&b.facing.y>0,"Eight-direction facing");
    Region wall=m; wall.tiles[26][24]='#'; Vec p=tileCenter(23,26);
    moveBody(wall,p,{90,0}); require(p.x<24*16-4,"Fast movement must not tunnel through walls");
    p=tileCenter(23,26); moveBody(wall,p,{20,20}); require(p.y>tileCenter(23,26).y,"Collision should slide along walls");
    Player dodger; dodger.pos=tileCenter(23,26); Input dodge; dodge.move={1,0}; dodge.dodge=true;
    dodger.update(dodge,m,1.0f/60); require(dodger.dashTime>0&&dodger.invulnerable>0,"Dodge grants invulnerability");
    require(!dodger.hurt(1,{100,0}),"Damage ignored during dodge");
    std::cout<<"PASS: movement normalization, facing, swept collision, wall slide, dodge\n";
}
void testCombat(const std::filesystem::path& data) {
    auto m=loadRegion(data,RegionId::Shrine); auto defs=loadEnemies(data/"enemies.txt");
    Player p; p.pos=tileCenter(23,26); p.facing={1,0}; p.attackTime=.2f; p.attackSerial=1;
    Inventory inv; std::vector<Projectile> shots; std::vector<Particle> particles;
    Enemy e; e.pos=p.pos+Vec{23,0}; std::vector<Enemy> enemies{e};
    resolveCombat(p,inv,enemies,shots,m,particles); require(enemies[0].health==2,"Sword damages in facing arc");
    resolveCombat(p,inv,enemies,shots,m,particles); require(enemies[0].health==2,"One hit per enemy per sword swing");
    p.attackSerial=2; p.facing={-1,0}; resolveCombat(p,inv,enemies,shots,m,particles); require(enemies[0].health==2,"Sword must not hit behind player");
    p.facing={1,0}; inv.add(Item::Sword,1); auto result=resolveCombat(p,inv,enemies,shots,m,particles);
    require(enemies[0].health==0&&result.kills==1&&result.fallen.size()==1&&inv.get(Item::Fragment)==0,"Equipment damages enemies; loot requires searching remains");
    p.invulnerable=0; require(p.hurt(1,{40,0}),"First contact hurts"); require(!p.hurt(1,{40,0}),"Invulnerability prevents repeated damage");
    require(p.health==5&&p.knockback.x==40,"Damage and knockback are applied");
    inv.add(Item::Tonic,1); require(p.heal(inv)&&p.health==6&&inv.get(Item::Tonic)==0,"Tonic heals and is consumed");
    inv.add(Item::Tonic,1); require(!p.heal(inv)&&inv.get(Item::Tonic)==1,"Full health does not waste tonic");
    enemies={e}; enemies[0].lastHit=0; p.attackSerial=3; p.attackTime=.2f; m.tiles[26][24]='#';
    resolveCombat(p,inv,enemies,shots,m,particles); require(enemies[0].health==3,"Sword cannot hit through walls");
    m=loadRegion(data,RegionId::Shrine); p.pos=tileCenter(23,18);
    Enemy bramble; bramble.type=0; bramble.pos=bramble.home=p.pos+Vec{40,0};
    Enemy moth; moth.type=1; moth.pos=moth.home=p.pos+Vec{0,80}; moth.timer=.1f;
    enemies={bramble,moth}; shots.clear(); bool sawLunge=false,sawShot=false;
    for(int i=0;i<90;++i) { updateEnemies(enemies,shots,m,p,defs,1.0f/60); sawLunge|=enemies[0].mode==EnemyMode::Dash; sawShot|=!shots.empty(); }
    require(sawLunge&&sawShot,"Both distinct enemy attacks execute");
    Enemy boss; boss.type=2; boss.health=15; boss.maxHealth=32; boss.pos=boss.home=tileCenter(23,13);
    enemies={boss}; shots.clear(); bool charge=false,volley=false; size_t maxShots=0;
    for(int i=0;i<700;++i) { updateEnemies(enemies,shots,m,p,defs,1.0f/60); charge|=enemies[0].mode==EnemyMode::Dash; volley|=enemies[0].mode==EnemyMode::Volley; maxShots=std::max(maxShots,shots.size()); }
    require(enemies[0].phaseTwo()&&charge&&volley&&maxShots>=12,"Boss phase two and both attacks execute");
    std::cout<<"PASS: sword arc and hit deduplication, equipment, loot, health, wall occlusion, both enemies and boss phases\n";
}
void testSaves(const std::filesystem::path& data,const std::filesystem::path& out) {
    auto file=out/"roundtrip.sav"; SaveData s; s.inventory.add(Item::Tonic,5); s.seed=91827;
    s.position={323.25f,289.5f}; s.openedChests={"village_supplies"}; s.quest.accepted=true; s.playSeconds=102.125;
    std::string error; require(writeSave(file,s,error),"Write save: "+error); SaveData loaded;
    require(readSave(file,loaded,error),"Read save: "+error);
    require(near(loaded.position.x,s.position.x)&&near(loaded.position.y,s.position.y)&&loaded.health==s.health,"Position and health roundtrip");
    require(loaded.inventory.count==s.inventory.count&&loaded.openedChests==s.openedChests&&loaded.seed==s.seed&&loaded.quest.accepted&&loaded.playSeconds==s.playSeconds,"Progress and seed roundtrip");
    s.health=4; require(writeSave(file,s,error),"Replace save");
    SaveData previous; require(readSave(file.string()+".bak",previous,error)&&previous.health==6,"Previous valid save retained as backup");
    { std::ofstream corrupt(file,std::ios::binary|std::ios::app); corrupt<<"damage"; }
    loaded.health=3; require(!readSave(file,loaded,error)&&loaded.health==3,"Corrupt save rejected without mutating output");
    Game recover(data,file); require(recover.continueGame()&&recover.player.health==6,"Game recovers previous backup");
    require(recover.save(false),"Saving after recovery");
    require(readSave(file.string()+".bak",previous,error)&&previous.health==6,"Corrupt primary cannot overwrite good backup");
    s.health=-1; require(!writeSave(file,s,error),"Negative health rejected"); s.health=6;
    s.inventory.count[0]=100; require(!writeSave(file,s,error),"Oversized inventory rejected"); s.inventory.count[0]=1;
    s.position.x=std::numeric_limits<float>::quiet_NaN(); require(!writeSave(file,s,error),"Nonfinite position rejected"); s.position.x=323;
    s.quest.bossDefeated=true; require(!writeSave(file,s,error),"Inconsistent quest rejected");
    auto invalidTarget=out/"directory-instead-of-save"; std::filesystem::create_directories(invalidTarget);
    Game cannotWrite(data,invalidTarget); cannotWrite.newGame(5);
    require(cannotWrite.notification.find("Save failed:")==0,"New-game greeting must not hide a save failure");
    cannotWrite.respawn();
    require(cannotWrite.notification.find("Save failed:")==0,"Respawn greeting must not hide a save failure");
    std::cout<<"PASS: exact save roundtrip, atomic replacement, corruption rejection, backup recovery and validation\n";
}
void testQuest(const std::filesystem::path& data,const std::filesystem::path& out) {
    Game g(data,out/"quest.sav"); require(g.screen==Screen::Title,"Starts at title"); g.newGame(444);
    interactWith(g,"village_supplies"); require(g.inventory.get(Item::Tonic)==5,"Welcome chest loot");
    interactWith(g,"village_supplies"); require(g.inventory.get(Item::Tonic)==5,"Opened chest cannot duplicate loot");
    interactWith(g,"keeper"); require(g.quest.accepted&&g.dialogue.active(),"Keeper starts quest with dialogue");
    Vec pos=g.player.pos; int hp=g.player.health; Input moving; moving.move={1,0}; g.update(moving,.05f);
    require(near(length(g.player.pos-pos),0)&&g.player.health==hp,"Dialogue pauses simulation");
    g.dialogue={}; g.enterRegion(RegionId::Shrine,tileCenter(23,4));
    g.player.pos=tileCenter(23,1); g.transitionCooldown=0; g.update({},1.0f/60);
    require(g.map.id==RegionId::Shrine,"Boss gate is locked before relics");
    interactWith(g,"quiet_bowl"); require(!g.quest.shrineLit,"Altar cannot activate without relics");
    g.enterRegion(RegionId::Forest,tileCenter(23,17)); interactWith(g,"forest_ember");
    g.enterRegion(RegionId::Cave,tileCenter(23,4)); interactWith(g,"cave_dew"); interactWith(g,"cave_fragments");
    require(g.inventory.get(Item::Ember)==1&&g.inventory.get(Item::Dew)==1,"Both relic caches work");
    g.enterRegion(RegionId::Hollow,tileCenter(3,8)); interactWith(g,"hollow_sword");
    require(g.inventory.damage()==2,"Optional cave grants real equipment upgrade");
    g.enterRegion(RegionId::Shrine,tileCenter(23,27)); interactWith(g,"quiet_bowl");
    require(g.quest.shrineLit,"Two relics unlock shrine");
    g.dialogue={}; interactWith(g,"shrine_lantern");
    require(g.checkpointRegion==RegionId::Shrine&&g.player.health==g.inventory.maxHealth(),"Lantern sets checkpoint and heals");
    g.player.pos=tileCenter(23,1); g.transitionCooldown=0; g.update({},1.0f/60);
    require(g.map.id==RegionId::Arena,"Unlocked gate transitions to boss arena");
    g.player.health=1; g.player.invulnerable=0; g.projectiles.push_back({g.player.pos,{},1,1,3}); g.update({},1.0f/60);
    require(g.screen==Screen::Death,"Lethal damage enters death screen");
    g.update({},.05f); g.deathTime=1; Input confirm; confirm.confirm=true; g.update(confirm,1.0f/60);
    require(g.map.id==RegionId::Shrine&&g.screen==Screen::Playing&&g.player.health==6,"Death respawns at actual checkpoint");
    require(g.quest.shrineLit&&g.inventory.get(Item::Ember)&&g.openedChests.count("cave_dew"),"Death preserves persistent progress");
    require(g.save(false),"Quest save"); Game resumed(data,out/"quest.sav"); require(resumed.continueGame(),"Quest load");
    require(resumed.quest.shrineLit&&resumed.inventory.damage()==2&&resumed.openedChests==g.openedChests,"Quest and gear survive reload");
    std::cout<<"PASS: title/new game, NPC, chests, pause, full relic quest, gate, checkpoint, death/respawn and continuation\n";
}
void testBossFight(const std::filesystem::path& data,const std::filesystem::path& out) {
    Game g(data,out/"boss.sav"); g.newGame(9);
    // A prepared player's loadout. The fight itself uses only normal simulation input.
    g.quest.accepted=true; g.quest.shrineLit=true; g.inventory.add(Item::Ember,1); g.inventory.add(Item::Dew,1);
    g.inventory.add(Item::Sword,1); g.inventory.add(Item::Coat,1); g.player.health=8; g.inventory.add(Item::Tonic,2);
    g.enterRegion(RegionId::Arena,tileCenter(23,26)); bool phaseTwo=false; int frames=0;
    for(;frames<60*90&&!g.quest.bossDefeated&&g.screen!=Screen::Death;++frames) {
        const auto& boss=g.enemies.front(); phaseTwo|=boss.phaseTwo();
        Vec toward=normalized(boss.pos-g.player.pos); float dist=length(boss.pos-g.player.pos);
        Input in; in.attack=true; in.heal=g.player.health<=4;
        if(dist>17) in.move=toward;
        if(boss.mode==EnemyMode::Windup&&boss.timer<.15f) { in.attack=false; in.move={-boss.facing.y,boss.facing.x}; in.dodge=true; }
        if(boss.mode==EnemyMode::Volley&&boss.timer<.1f) { in.attack=false; in.move=toward*-1; in.dodge=true; }
        g.update(in,1.0f/60);
    }
    require(g.quest.bossDefeated&&phaseTwo,"Prepared player bot must complete both boss phases using combat input");
    require(g.inventory.get(Item::Bell)==1&&g.projectiles.empty(),"Boss victory grants Bellheart and clears hazards");
    g.dialogue={}; g.enterRegion(RegionId::Arena,tileCenter(23,26)); require(g.enemies.empty(),"Defeated boss remains defeated");
    g.enterRegion(RegionId::Village,tileCenter(20,17)); interactWith(g,"keeper");
    require(g.quest.rewardClaimed,"Return to keeper completes ending"); int tonics=g.inventory.get(Item::Tonic);
    interactWith(g,"keeper"); require(g.inventory.get(Item::Tonic)==tonics,"Ending reward is granted only once");
    std::cout<<"PASS: boss defeated in "<<frames/60.0f<<" simulated seconds, persistent victory and ending\n";
}
}
#include "expansion.hpp"
#include "controls.hpp"
int main(int argc,char** argv) {
    try {
        if(argc!=3) throw std::runtime_error("Usage: MosslightTests <data-folder> <output-folder>");
        std::filesystem::path data=argv[1],out=argv[2]; std::filesystem::create_directories(out);
        testMaps(data); testMovement(data); testCombat(data); testSaves(data,out); testQuest(data,out); testBossFight(data,out);
        testTactics(data); testEconomy(data,out); testLootPersistence(data,out); testMigrationAndVisitors(data,out);
        testKnightDuel(data,out);
        testFocus(data,out); testArmorChoice(data,out); testGuideAndMigration(data,out);
        std::cout<<"ALL PASS: "<<checks<<" checks\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n'; return 1; }
}
