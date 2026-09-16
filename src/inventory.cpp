#include "inventory.hpp"
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace moss {
ItemCatalog loadItems(const std::filesystem::path& path) {
    std::ifstream in(path); ItemCatalog out; int id;
    for(int i=0;i<ItemCount;++i) {
        if(!(in>>id>>std::quoted(out[i].name)>>std::quoted(out[i].description))||id!=i)
            throw std::runtime_error("Invalid item catalog: "+path.string());
    }
    return out;
}
int Inventory::add(Item item,int amount) {
    int& n=count[static_cast<int>(item)]; int before=n;
    n=std::clamp(n+amount,0,99); return n-before;
}
bool Inventory::take(Item item,int amount) {
    int& n=count[static_cast<int>(item)]; if(amount<0||n<amount) return false;
    n-=amount; return true;
}
}
