#pragma once
#include "combat.hpp"
#include "dialogue.hpp"
#include "procedural.hpp"
#include "save.hpp"
namespace moss {
enum class Screen { Title,Playing,Pause,Inventory,Journal,Death,NewGameConfirm };
enum class Cue { None,Swing,Hit,Hurt,Loot,Rest,Quest,Victory };
class Game {
public:
    Game(std::filesystem::path data,std::filesystem::path savePath);
    void update(const Input& input,float dt);
    void newGame(uint32_t seed);
    bool continueGame();
    bool save(bool notify=true);
    void enterRegion(RegionId id,Vec spawn);
    void interact();
    void respawn();
    const Object* nearbyObject() const;
    SaveData snapshot() const;
    void toast(std::string text,float seconds=3);
    void close();

    std::filesystem::path dataPath,savePath;
    ItemCatalog items;
    EnemyCatalog enemyDefs;
    Region map;
    Player player;
    Inventory inventory;
    Quest quest;
    RegionId checkpointRegion=RegionId::Village;
    Vec checkpoint=tileCenter(20,17);
    std::set<std::string> openedChests;
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Particle> particles;
    Dialogue dialogue;
    Screen screen=Screen::Title;
    Cue cue=Cue::None;
    int menuSelection=0;
    bool quitRequested=false,hasSave=false,hasSession=false;
    uint32_t seed=1;
    double playSeconds=0;
    float clock=0,regionBanner=0,transitionCooldown=0,toastTime=0,shake=0,deathTime=0;
    std::string notification;
private:
    void apply(const SaveData& data);
};
}
