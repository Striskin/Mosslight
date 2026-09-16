#pragma once
#include "player.hpp"
#include <set>
namespace moss {
struct RegionProgress { std::set<int> defeated; std::vector<LootDrop> drops; };
using WorldProgress=std::array<RegionProgress,RegionCount>;
struct SaveData {
    RegionId region=RegionId::Village,checkpointRegion=RegionId::Village;
    Vec position=tileCenter(20,17),checkpoint=tileCenter(20,17);
    int health=6;
    Inventory inventory;
    Quest quest;
    std::set<std::string> openedChests;
    uint32_t seed=1;
    double playSeconds=0;
    float stamina=100;
    WorldProgress world;
};
bool writeSave(const std::filesystem::path& path,const SaveData& data,std::string& error);
bool readSave(const std::filesystem::path& path,SaveData& data,std::string& error);
std::filesystem::path defaultSavePath(const std::filesystem::path& executableDirectory);
}
