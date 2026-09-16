#pragma once
#include "inventory.hpp"
namespace moss {
struct Trade { Item item; int amount=1,price=1; bool selling=false; };
struct ShopOffer { std::string merchant; Trade trade; };
using ShopCatalog=std::vector<ShopOffer>;
ShopCatalog loadShops(const std::filesystem::path& path);
std::vector<Trade> shopTrades(const ShopCatalog& shops,const ItemCatalog& items,const std::string& merchant);
bool trade(Inventory& inventory,const Trade& offer,std::string& message);
}
