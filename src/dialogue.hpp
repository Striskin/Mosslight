#pragma once
#include "inventory.hpp"
namespace moss {
struct Dialogue {
    std::string speaker;
    std::vector<std::string> pages;
    size_t page=0;
    bool active() const { return page<pages.size(); }
    void advance() { if(active()) ++page; }
    void open(std::string name,std::vector<std::string> lines) { speaker=std::move(name); pages=std::move(lines); page=0; }
};
struct DialogueResult { std::vector<std::string> lines; bool changed=false; };
DialogueResult talkTo(const std::string& id,Quest& quest,Inventory& inventory);
std::string objective(const Quest& quest,const Inventory& inventory);
}
