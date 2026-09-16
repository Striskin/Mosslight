#include "maps.hpp"
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace moss {
char Region::tile(int x,int y) const { return x<0||y<0||x>=width||y>=height?'#':tiles[y][x]; }
bool Region::solid(int x,int y) const { char t=tile(x,y); return t=='#'||t=='T'||t=='~'||t=='H'||t=='P'||t=='b'||t=='='||t=='f'||t=='s'; }
Region loadRegion(const std::filesystem::path& data,RegionId id) {
    const auto path=data/"maps"/(std::string(regionKey(id))+".map");
    std::ifstream in(path); Region r; r.id=id; std::string tag;
    if(!(in>>tag>>r.width>>r.height>>r.palette>>std::quoted(r.name)>>std::quoted(r.subtitle))||
       tag!="MOSSMAP1"||r.width<30||r.width>128||r.height<17||r.height>128||r.palette<0||r.palette>4)
        throw std::runtime_error("Invalid map header: "+path.string());
    for(int y=0;y<r.height;++y) {
        std::string row; in>>row;
        if(static_cast<int>(row.size())!=r.width||row.find_first_not_of(".#T~HP,+_:*b=fsrB")!=std::string::npos)
            throw std::runtime_error("Invalid map row: "+path.string());
        r.tiles.push_back(row);
    }
    while(in>>tag) {
        if(tag=="exit") {
            int x,y,w,h,target,sx,sy,lock;
            if(!(in>>x>>y>>w>>h>>target>>sx>>sy>>lock)||target<0||target>=RegionCount||w<1||h<1||x<0||y<0||x+w>r.width||y+h>r.height||sx<0||sy<0||sx>127||sy>127)
                throw std::runtime_error("Invalid exit: "+path.string());
            r.exits.push_back({{x*16.0f,y*16.0f,w*16.0f,h*16.0f},static_cast<RegionId>(target),tileCenter(sx,sy),lock!=0});
        } else if(tag=="enemy") {
            int type,x,y;
            if(!(in>>type>>x>>y)||type<0||type>=5||r.spawns.size()>=64) throw std::runtime_error("Invalid enemy spawn");
            r.spawns.push_back({type,tileCenter(x,y)});
        } else if(tag=="object") {
            Object o; int kind,x,y,item;
            if(!(in>>kind>>o.id>>x>>y>>item>>o.amount>>std::quoted(o.label))||kind<0||kind>6||kind==5||item<0||item>=ItemCount||o.amount<1||o.amount>99)
                throw std::runtime_error("Invalid map object: "+path.string());
            o.kind=static_cast<ObjectKind>(kind); o.pos=tileCenter(x,y); o.item=static_cast<Item>(item);
            r.objects.push_back(o);
        } else if(tag=="door") {
            Object o; int x,y,target,sx,sy; o.kind=ObjectKind::Door;
            if(!(in>>o.id>>x>>y>>target>>sx>>sy>>std::quoted(o.label))||target<0||target>=RegionCount||sx<0||sy<0||sx>127||sy>127)
                throw std::runtime_error("Invalid door: "+path.string());
            o.pos=tileCenter(x,y); o.target=static_cast<RegionId>(target); o.arrival=tileCenter(sx,sy);
            r.objects.push_back(o);
        } else throw std::runtime_error("Unknown map record: "+tag);
    }
    auto errors=validateRegion(r);
    if(!errors.empty()) throw std::runtime_error(r.name+": "+errors.front());
    return r;
}
std::vector<std::string> validateRegion(const Region& r) {
    std::vector<std::string> errors;
    for(const auto& s:r.spawns) if(r.solid(static_cast<int>(s.pos.x)/16,static_cast<int>(s.pos.y)/16)) errors.push_back("Blocked enemy spawn");
    for(const auto& o:r.objects) if(r.solid(static_cast<int>(o.pos.x)/16,static_cast<int>(o.pos.y)/16)) errors.push_back("Blocked object: "+o.id);
    for(const auto& e:r.exits) {
        bool floor=false;
        for(int y=static_cast<int>(e.bounds.y)/16;y<(e.bounds.y+e.bounds.h)/16;++y)
            for(int x=static_cast<int>(e.bounds.x)/16;x<(e.bounds.x+e.bounds.w)/16;++x) floor|=!r.solid(x,y);
        if(!floor) errors.push_back("Blocked exit");
    }
    return errors;
}
}
