#include "inventory.hpp"
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace moss {
ItemCatalog loadItems(const std::filesystem::path& path) {
    std::ifstream in(path); ItemCatalog out; int id;
    for(int i=0;i<ItemCount;++i) {
        if(!(in>>id>>std::quoted(out[i].name)>>std::quoted(out[i].description)>>out[i].sellPrice)||id!=i||out[i].sellPrice<0||out[i].sellPrice>999)
            throw std::runtime_error("Invalid item catalog: "+path.string());
    }
    return out;
}
int itemLimit(Item item) {
    return item==Item::Tonic||item==Item::Fragment||item==Item::Arrow||item==Item::Shell?99:1;
}
int Inventory::add(Item item,int amount) {
    int& n=count[static_cast<int>(item)]; int before=n;
    n=static_cast<int>(std::clamp(static_cast<long long>(n)+amount,0LL,static_cast<long long>(itemLimit(item))));
    // First-time armor upgrades equip as before; later manual choices stay selected.
    if(before==0&&n>0) {
        if(item==Item::KnightArmor) armor=Armor::Plate;
        else if(item==Item::Coat&&armor==Armor::Travel) armor=Armor::Coat;
    }
    if(!canWear(armor)) armor=Armor::Travel;
    return n-before;
}
bool Inventory::take(Item item,int amount) {
    int& n=count[static_cast<int>(item)]; if(amount<0||n<amount) return false;
    n-=amount; if(!canWear(armor)) armor=Armor::Travel; return true;
}
int Inventory::addCoins(int amount) {
    int before=coins;
    coins=static_cast<int>(std::clamp(static_cast<long long>(coins)+amount,0LL,99999LL));
    return coins-before;
}
}
