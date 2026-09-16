#include "game.hpp"
#include <chrono>
namespace moss {
Game::Game(std::filesystem::path data,std::filesystem::path path):dataPath(std::move(data)),savePath(std::move(path)) {
    items=loadItems(dataPath/"items.txt"); enemyDefs=loadEnemies(dataPath/"enemies.txt");
    map=loadRegion(dataPath,RegionId::Village);
    std::error_code ec; hasSave=std::filesystem::exists(savePath,ec)||std::filesystem::exists(savePath.string()+".bak",ec);
    menuSelection=hasSave?0:1;
}
void Game::toast(std::string text,float seconds) { notification=std::move(text); toastTime=seconds; }
void Game::newGame(uint32_t newSeed) {
    player=Player{}; inventory=Inventory{}; inventory.add(Item::Tonic,3); quest=Quest{};
    openedChests.clear(); checkpointRegion=RegionId::Village; checkpoint=tileCenter(20,17);
    playSeconds=0; seed=newSeed?newSeed:1; dialogue={}; hasSession=true; screen=Screen::Playing;
    enterRegion(RegionId::Village,checkpoint);
    if(save(false)) toast("A small journey begins. Find Keeper Aven.",4);
}
SaveData Game::snapshot() const {
    SaveData s; s.region=map.id; s.position=player.pos; s.health=player.health;
    s.checkpointRegion=checkpointRegion; s.checkpoint=checkpoint; s.inventory=inventory;
    s.quest=quest; s.openedChests=openedChests; s.seed=seed; s.playSeconds=playSeconds; return s;
}
void Game::apply(const SaveData& s) {
    // Load data before replacing live state, so a missing map does not destroy a session.
    Region next=s.region==RegionId::Hollow?generateHollow(dataPath,s.seed):loadRegion(dataPath,s.region);
    inventory=s.inventory; quest=s.quest; openedChests=s.openedChests; seed=s.seed; playSeconds=s.playSeconds;
    checkpointRegion=s.checkpointRegion; checkpoint=s.checkpoint;
    player=Player{}; player.health=s.health; player.pos=safePosition(next,s.position); player.invulnerable=1;
    map=std::move(next); enemies=spawnEnemies(map,enemyDefs,quest.bossDefeated); projectiles.clear(); particles.clear();
    screen=Screen::Playing; dialogue={}; hasSession=true; regionBanner=3; transitionCooldown=.7f;
}
bool Game::continueGame() {
    SaveData s; std::string error;
    if(!readSave(savePath,s,error)) {
        auto backup=savePath; backup+=".bak"; std::string backupError;
        if(!readSave(backup,s,backupError)) { toast(error,6); return false; }
        apply(s); toast("Recovered the previous journey from backup.",5); return true;
    }
    apply(s); toast("Welcome back, wayfarer."); return true;
}
bool Game::save(bool notify) {
    if(!hasSession||player.health<=0) return false;
    std::string error;
    if(!writeSave(savePath,snapshot(),error)) { toast("Save failed: "+error,7); return false; }
    hasSave=true; if(notify) toast("Journey saved."); return true;
}
void Game::enterRegion(RegionId id,Vec spawn) {
    Region next=id==RegionId::Hollow?generateHollow(dataPath,seed):loadRegion(dataPath,id);
    map=std::move(next); player.pos=safePosition(map,spawn); player.knockback={}; player.dashTime=0;
    player.attackTime=0; player.invulnerable=1;
    enemies=spawnEnemies(map,enemyDefs,quest.bossDefeated); projectiles.clear(); particles.clear();
    regionBanner=3.2f; transitionCooldown=.7f;
}
const Object* Game::nearbyObject() const {
    const Object* nearest=nullptr; float distance=27;
    for(const auto& o:map.objects) {
        float d=length(o.pos-player.pos);
        if(d<distance&&lineClear(map,player.pos,o.pos)) { nearest=&o; distance=d; }
    }
    return nearest;
}
void Game::interact() {
    const Object* found=nearbyObject(); if(!found) return;
    const Object o=*found;
    switch(o.kind) {
    case ObjectKind::Npc: {
        auto result=talkTo(o.id,quest,inventory); dialogue.open(o.label,std::move(result.lines));
        if(result.changed) { cue=Cue::Quest; save(false); }
        break;
    }
    case ObjectKind::Chest:
        if(openedChests.count(o.id)) { toast("This cache is empty. Its gifts are yours."); break; }
        if(inventory.get(o.item)+o.amount>99) { toast("Your satchel is full of that item."); break; }
        inventory.add(o.item,o.amount); openedChests.insert(o.id); cue=Cue::Loot; burst(particles,o.pos,1,18);
        toast("Found "+std::to_string(o.amount)+" x "+items[static_cast<int>(o.item)].name+"!",4);
        if(o.item==Item::Sword) dialogue.open("A blade for the road",{"An old copper blade, still keen. You equip it. Your sword now deals two damage per strike."});
        save(false); break;
    case ObjectKind::Checkpoint:
        checkpointRegion=map.id; checkpoint=player.pos; player.health=inventory.maxHealth(); player.invulnerable=1;
        projectiles.clear(); cue=Cue::Rest; burst(particles,o.pos,1,18);
        if(save(false)) toast("Rested. Hearts restored, checkpoint set, journey saved.",4);
        break;
    case ObjectKind::Altar:
        if(quest.shrineLit) { dialogue.open("The Quiet Bell",{"The bowl glows with dew and ember. The Warden's garden lies north. The valley is waiting."}); break; }
        if(!quest.accepted) { dialogue.open("An unfamiliar inscription",{"Two small lights, one sleeping bell. Keeper Aven in the village may know what these words mean."}); break; }
        if(!inventory.get(Item::Ember)||!inventory.get(Item::Dew)) {
            dialogue.open("The empty bowl",{"The inscription asks for an Ember Seed from the forest and Moon Dew from Stillwater Cave. Something is still missing."}); break;
        }
        quest.shrineLit=true; cue=Cue::Quest; burst(particles,o.pos,1,28);
        dialogue.open("A soft note",{"Ember warms the dew. A clear note drifts through the stone, and the northern seal opens.",
            "Rest at the shrine lantern before you enter the Warden's garden. Watch for amber warnings, dodge with SPACE, then strike while it rests."});
        save(false); break;
    case ObjectKind::Sign: dialogue.open("A weathered note",{o.label}); break;
    }
}
void Game::respawn() {
    player=Player{}; player.health=inventory.maxHealth(); screen=Screen::Playing; dialogue={};
    enterRegion(checkpointRegion,checkpoint);
    if(save(false)) toast("The lantern calls you home. Your belongings are safe.",4);
}
void Game::close() {
    if(hasSession) {
        if(player.health<=0) respawn();
        save(false);
    }
}
void Game::update(const Input& in,float dt) {
    dt=std::clamp(dt,0.0f,.05f); clock+=dt; cue=Cue::None;
    toastTime=std::max(0.0f,toastTime-dt); shake=std::max(0.0f,shake-dt);
    if(screen==Screen::Title) {
        if(in.up) menuSelection=(menuSelection+2)%3;
        if(in.down) menuSelection=(menuSelection+1)%3;
        if(in.confirm) {
            if(menuSelection==0) { if(hasSave) continueGame(); else toast("Begin a new journey first."); }
            if(menuSelection==1) {
                if(hasSave) screen=Screen::NewGameConfirm;
                else newGame(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
            }
            if(menuSelection==2) quitRequested=true;
        }
        return;
    }
    if(screen==Screen::NewGameConfirm) {
        if(in.pause) screen=Screen::Title;
        if(in.confirm) newGame(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
        return;
    }
    if(screen==Screen::Death) { deathTime+=dt; if(deathTime>.6f&&(in.confirm||in.interact)) respawn(); return; }
    if(screen==Screen::Pause) {
        if(in.pause) { screen=Screen::Playing; return; }
        if(in.up) menuSelection=(menuSelection+2)%3;
        if(in.down) menuSelection=(menuSelection+1)%3;
        if(in.confirm) {
            if(menuSelection==0) screen=Screen::Playing;
            if(menuSelection==1) save();
            if(menuSelection==2&&save(false)) { screen=Screen::Title; hasSession=false; menuSelection=0; }
        }
        return;
    }
    if(screen==Screen::Inventory||screen==Screen::Journal) {
        if(in.pause||in.inventory||in.journal) screen=Screen::Playing;
        if(in.heal&&player.heal(inventory)) { cue=Cue::Rest; save(false); }
        return;
    }
    if(dialogue.active()) { if(in.interact||in.confirm||in.pause) dialogue.advance(); return; }
    if(in.pause) { screen=Screen::Pause; menuSelection=0; return; }
    if(in.inventory) { screen=Screen::Inventory; return; }
    if(in.journal) { screen=Screen::Journal; return; }
    playSeconds+=dt; regionBanner=std::max(0.0f,regionBanner-dt); transitionCooldown=std::max(0.0f,transitionCooldown-dt);
    if(in.save) save();
    if(in.interact) { interact(); if(dialogue.active()) return; }
    if(in.heal) {
        if(player.heal(inventory)) { cue=Cue::Rest; burst(particles,player.pos,1); }
        else toast(player.health>=inventory.maxHealth()?"Your hearts are already full.":"No tonics left. Rest at a lantern.");
    }
    unsigned serial=player.attackSerial; player.update(in,map,dt);
    if(player.attackSerial!=serial) cue=Cue::Swing;
    updateEnemies(enemies,projectiles,map,player,enemyDefs,dt);
    auto result=resolveCombat(player,inventory,enemies,projectiles,map,particles);
    if(result.swordHit) { cue=Cue::Hit; shake=.08f; }
    if(result.playerHit) { cue=Cue::Hurt; shake=.18f; }
    if(result.bossKilled) {
        quest.bossDefeated=true; inventory.add(Item::Bell,1); projectiles.clear(); cue=Cue::Victory;
        player.health=std::max(player.health,1);
        dialogue.open("The valley exhales",{"The Warden lowers its antlers. Beneath the roots, a small silver bell begins to sing.",
            "You found the Bellheart. The restless magic has settled. Return to Keeper Aven in Hearthmere when you are ready."});
        save(false);
    }
    for(auto& p:particles) { p.life-=dt; p.pos+=p.vel*dt; p.vel=p.vel*std::max(0.0f,1-3*dt); }
    particles.erase(std::remove_if(particles.begin(),particles.end(),[](const auto& p){return p.life<=0;}),particles.end());
    if(player.health<=0) { screen=Screen::Death; deathTime=0; return; }
    if(transitionCooldown<=0) for(const auto& exit:map.exits) if(contains(exit.bounds,player.pos)) {
        if(exit.locked&&!quest.shrineLit) {
            toast("The northern seal sleeps. Offer both relics at the shrine.");
            transitionCooldown=1.5f; break;
        }
        const auto target=exit.target; const auto spawn=exit.spawn;
        enterRegion(target,spawn); save(false); break;
    }
}
}
