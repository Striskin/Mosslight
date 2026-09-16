#pragma once
#include "types.hpp"
#include <filesystem>
namespace moss {
struct ItemInfo { std::string name,description; };
using ItemCatalog=std::array<ItemInfo,ItemCount>;
ItemCatalog loadItems(const std::filesystem::path& path);
struct Inventory {
    std::array<int,ItemCount> count{};
    int get(Item item) const { return count[static_cast<int>(item)]; }
    int add(Item item,int amount);
    bool take(Item item,int amount=1);
    int damage() const { return get(Item::Sword)?2:1; }
    int maxHealth() const { return get(Item::Coat)?8:6; }
};
}
