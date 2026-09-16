#include "merchant.hpp"
#include <fstream>
#include <stdexcept>
namespace moss {
ShopCatalog loadShops(const std::filesystem::path& path) {
    std::ifstream in(path); if(!in) throw std::runtime_error("Missing shop catalog");
    ShopCatalog shops; std::string merchant; int item,amount,price;
    while(in>>merchant) {
        if(!(in>>item>>amount>>price)||item<0||item>=ItemCount||amount<1||amount>99||price<1||price>99999||shops.size()>64)
            throw std::runtime_error("Invalid shop offer");
        shops.push_back({merchant,{static_cast<Item>(item),amount,price,false}});
    }
    return shops;
}
std::vector<Trade> shopTrades(const ShopCatalog& shops,const ItemCatalog& items,const std::string& merchant) {
    std::vector<Trade> result;
    for(const auto& entry:shops) if(entry.merchant==merchant) result.push_back(entry.trade);
    for(int i=0;i<ItemCount;++i) if(items[i].sellPrice>0) result.push_back({static_cast<Item>(i),1,items[i].sellPrice,true});
    return result;
}
bool trade(Inventory& inv,const Trade& offer,std::string& message) {
    int index=static_cast<int>(offer.item);
    if(index<0||index>=ItemCount||offer.amount<1||offer.amount>99||offer.price<1||offer.price>99999) {
        message="This offer is invalid."; return false;
    }
    if(offer.selling) {
        if(offer.item!=Item::Fragment&&offer.item!=Item::Shell) { message="That item is not for sale."; return false; }
        if(inv.get(offer.item)<offer.amount) { message="You have none to sell."; return false; }
        if(inv.coins>99999-offer.price) { message="Your coin purse is full."; return false; }
        inv.take(offer.item,offer.amount); inv.addCoins(offer.price); message="Sold. Coins added to your purse.";
    } else {
        if(inv.get(offer.item)>itemLimit(offer.item)-offer.amount) { message=itemLimit(offer.item)==1?"You already own this equipment.":"No room for the whole bundle."; return false; }
        if(inv.coins<offer.price) { message="Not enough coins. Search fallen foes or sell materials."; return false; }
        inv.coins-=offer.price; inv.add(offer.item,offer.amount);
        message=offer.item==Item::Bow?"Bow purchased. Press 3, aim with RMB, fire with J.":
            offer.item==Item::KnightArmor?"Plate equipped. Rest to fill your new hearts.":"Purchased. Packed safely in your satchel.";
    }
    return true;
}
}
