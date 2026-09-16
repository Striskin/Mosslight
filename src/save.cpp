#include "save.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
namespace moss {
namespace {
uint64_t checksum(const std::string& bytes) {
    uint64_t value=14695981039346656037ull;
    for(unsigned char c:bytes) { value^=c; value*=1099511628211ull; }
    return value;
}
bool valid(const SaveData& s) {
    auto region=[](RegionId r){return static_cast<int>(r)>=0&&static_cast<int>(r)<6;};
    auto position=[](Vec p){return std::isfinite(p.x)&&std::isfinite(p.y)&&p.x>=0&&p.y>=0&&p.x<=2048&&p.y<=2048;};
    if(!region(s.region)||!region(s.checkpointRegion)||!position(s.position)||!position(s.checkpoint)||
       s.health<1||s.health>s.inventory.maxHealth()||s.openedChests.size()>64||s.seed==0||
       !std::isfinite(s.playSeconds)||s.playSeconds<0||s.playSeconds>1e10) return false;
    if(s.checkpointRegion!=RegionId::Village&&s.checkpointRegion!=RegionId::Shrine) return false;
    for(int n:s.inventory.count) if(n<0||n>99) return false;
    for(Item i:{Item::Sword,Item::Coat,Item::Ember,Item::Dew,Item::Bell}) if(s.inventory.get(i)>1) return false;
    if(s.quest.shrineLit&&(!s.quest.accepted||!s.inventory.get(Item::Ember)||!s.inventory.get(Item::Dew))) return false;
    if(s.quest.bossDefeated&&!s.quest.shrineLit) return false;
    if(s.quest.rewardClaimed&&!s.quest.bossDefeated) return false;
    if(s.region==RegionId::Arena&&!s.quest.shrineLit) return false;
    for(const auto& id:s.openedChests) if(id.empty()||id.size()>48||id.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789")!=std::string::npos) return false;
    return true;
}
}
bool writeSave(const std::filesystem::path& path,const SaveData& s,std::string& error) {
    error.clear();
    if(!valid(s)) { error="The current game state cannot be saved."; return false; }
    try {
        std::ostringstream p; p<<std::setprecision(12);
        p<<static_cast<int>(s.region)<<' '<<s.position.x<<' '<<s.position.y<<' '<<s.health<<'\n';
        p<<static_cast<int>(s.checkpointRegion)<<' '<<s.checkpoint.x<<' '<<s.checkpoint.y<<'\n';
        p<<s.seed<<' '<<s.playSeconds<<'\n';
        for(int n:s.inventory.count) p<<n<<' ';
        p<<'\n';
        p<<s.quest.accepted<<' '<<s.quest.shrineLit<<' '<<s.quest.bossDefeated<<' '<<s.quest.rewardClaimed<<'\n';
        p<<s.openedChests.size()<<'\n'; for(const auto& id:s.openedChests) p<<id<<'\n';
        auto bytes=p.str();
        if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        auto temp=path; temp+=".tmp";
        {
            std::ofstream out(temp,std::ios::binary|std::ios::trunc);
            out<<"MOSSLIGHT_SAVE 1 "<<checksum(bytes)<<'\n'<<bytes; out.flush();
            if(!out) { error="Could not write the save file."; return false; }
        }
        // Only back up a valid previous save. A damaged primary must not replace a good backup.
        SaveData prior; std::string ignored;
        if(readSave(path,prior,ignored)) {
            std::error_code ec; auto backup=path; backup+=".bak";
            std::filesystem::copy_file(path,backup,std::filesystem::copy_options::overwrite_existing,ec);
        }
#ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
            error="Could not replace the save file (Windows error "+std::to_string(GetLastError())+")."; return false;
        }
#else
        std::filesystem::rename(temp,path);
#endif
        return true;
    } catch(const std::exception& e) { error=e.what(); return false; }
}
bool readSave(const std::filesystem::path& path,SaveData& result,std::string& error) {
    error.clear();
    try {
        std::error_code ec; auto size=std::filesystem::file_size(path,ec);
        if(ec) { error="No saved journey was found."; return false; }
        if(size>65536) { error="Save file is too large."; return false; }
        std::ifstream file(path,std::ios::binary); std::string magic; int version; uint64_t hash;
        if(!(file>>magic>>version>>hash)||magic!="MOSSLIGHT_SAVE"||version!=1||file.get()!='\n') { error="Unsupported save format."; return false; }
        std::string bytes((std::istreambuf_iterator<char>(file)),{});
        if(checksum(bytes)!=hash) { error="Save checksum failed. The file may be damaged."; return false; }
        std::istringstream in(bytes); SaveData s; int region,checkpoint;
        if(!(in>>region>>s.position.x>>s.position.y>>s.health>>checkpoint>>s.checkpoint.x>>s.checkpoint.y>>s.seed>>s.playSeconds)) { error="Incomplete save."; return false; }
        s.region=static_cast<RegionId>(region); s.checkpointRegion=static_cast<RegionId>(checkpoint);
        for(int& n:s.inventory.count) if(!(in>>n)) { error="Incomplete inventory."; return false; }
        if(!(in>>s.quest.accepted>>s.quest.shrineLit>>s.quest.bossDefeated>>s.quest.rewardClaimed)) { error="Invalid quest data."; return false; }
        int count;
        if(!(in>>count)||count<0||count>64) { error="Invalid chest data."; return false; }
        for(int i=0;i<count;++i) { std::string id; if(!(in>>id)||!s.openedChests.insert(id).second) { error="Invalid chest identifier."; return false; } }
        in>>std::ws;
        if(!in.eof()||!valid(s)) { error="Save contains invalid values."; return false; }
        result=s; return true;
    } catch(const std::exception& e) { error=e.what(); return false; }
}
std::filesystem::path defaultSavePath(const std::filesystem::path& exe) {
#ifdef _WIN32
    if(const wchar_t* local=_wgetenv(L"LOCALAPPDATA")) return std::filesystem::path(local)/"Mosslight"/"journey.sav";
#else
    if(const char* xdg=std::getenv("XDG_DATA_HOME")) return std::filesystem::path(xdg)/"mosslight"/"journey.sav";
    if(const char* home=std::getenv("HOME")) return std::filesystem::path(home)/".local/share/mosslight/journey.sav";
#endif
    return exe/"saves"/"journey.sav";
}
}
