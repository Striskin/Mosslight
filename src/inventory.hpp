#pragma once
#include "types.hpp"
#include <filesystem>
namespace moss {
struct ItemInfo { std::string name,description; int sellPrice=0; };
using ItemCatalog=std::array<ItemInfo,ItemCount>;
ItemCatalog loadItems(const std::filesystem::path& path);
int itemLimit(Item item);
struct Inventory {
    std::array<int,ItemCount> count{};
    int coins=0;
    Weapon weapon=Weapon::Sword;
    Armor armor=Armor::Travel;
    int get(Item item) const { return count[static_cast<int>(item)]; }
    int add(Item item,int amount);
    bool take(Item item,int amount=1);
    int damage() const { return get(Item::KnightSword)?3:(get(Item::Sword)?2:1); }
    int maxHealth() const { return armor==Armor::Plate?10:(armor==Armor::Coat?8:6); }
    bool canWear(Armor a) const { return a==Armor::Travel||(a==Armor::Coat&&get(Item::Coat)>0)||(a==Armor::Plate&&get(Item::KnightArmor)>0); }
    bool wear(Armor a) { if(!canWear(a)) return false; armor=a; return true; }
    bool canEquip(Weapon w) const { return w==Weapon::Fists||w==Weapon::Sword||(w==Weapon::Bow&&get(Item::Bow)>0); }
    bool equip(Weapon w) { if(!canEquip(w)) return false; weapon=w; return true; }
    int addCoins(int amount);
};
}
