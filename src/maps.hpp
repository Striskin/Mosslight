#pragma once
#include "types.hpp"
#include <filesystem>
namespace moss {
enum class ObjectKind { Npc,Chest,Checkpoint,Altar,Sign };
struct Object {
    ObjectKind kind=ObjectKind::Sign;
    std::string id,label;
    Vec pos;
    Item item=Item::Tonic;
    int amount=1;
};
struct Exit { Rect bounds; RegionId target; Vec spawn; bool locked=false; };
struct Spawn { int type; Vec pos; };
struct Region {
    RegionId id=RegionId::Village;
    std::string name,subtitle;
    int width=0,height=0,palette=0;
    std::vector<std::string> tiles;
    std::vector<Object> objects;
    std::vector<Exit> exits;
    std::vector<Spawn> spawns;
    char tile(int x,int y) const;
    bool solid(int x,int y) const;
};
Region loadRegion(const std::filesystem::path& data,RegionId id);
std::vector<std::string> validateRegion(const Region& map);
}
