#include "procedural.hpp"
#include <fstream>
#include <random>
#include <stdexcept>
namespace moss {
Region generateHollow(const std::filesystem::path& data,uint32_t seed) {
    std::ifstream in(data/"rooms.txt"); std::string header; int count=0;
    if(!(in>>header>>count)||header!="MOSSROOMS1"||count<2||count>32) throw std::runtime_error("Invalid room templates");
    std::vector<std::vector<std::string>> templates;
    for(int i=0;i<count;++i) {
        std::vector<std::string> room;
        for(int y=0;y<9;++y) {
            std::string row; in>>row;
            if(row.size()!=11||row.find_first_not_of(".#*E")!=std::string::npos) throw std::runtime_error("Invalid room row");
            room.push_back(row);
        }
        templates.push_back(room);
    }
    std::mt19937 rng(seed);
    // Explicit modulo mapping keeps seeds identical across standard library vendors.
    auto next=[&](uint32_t n) { return rng()%n; };
    Region r; r.id=RegionId::Hollow; r.width=48; r.height=34; r.palette=2;
    r.name="The Wandering Hollow"; r.subtitle="An old path, a different shape";
    r.tiles.assign(r.height,std::string(r.width,'#'));
    std::vector<Vec> centers;
    for(int row=0;row<2;++row) for(int col=0;col<3;++col) {
        int ox=3+col*15,oy=4+row*17;
        const auto& room=templates[next(static_cast<uint32_t>(templates.size()))];
        for(int y=0;y<9;++y) for(int x=0;x<11;++x) {
            char t=room[y][x]; r.tiles[oy+y][ox+x]=t=='E'?'.':t;
            if(t=='E'&&(row!=0||col!=0)) r.spawns.push_back({static_cast<int>(next(2)),tileCenter(ox+x,oy+y)});
        }
        centers.push_back(tileCenter(ox+5,oy+4));
    }
    auto carve=[&](int x,int y) {
        for(int a=-1;a<=1;++a) for(int b=-1;b<=1;++b)
            if(x+a>0&&x+a<r.width-1&&y+b>0&&y+b<r.height-1) r.tiles[y+b][x+a]='.';
    };
    auto connect=[&](Vec a,Vec b) {
        int x=static_cast<int>(a.x)/16,y=static_cast<int>(a.y)/16;
        int tx=static_cast<int>(b.x)/16,ty=static_cast<int>(b.y)/16;
        while(x!=tx) { carve(x,y); x+=x<tx?1:-1; }
        while(y!=ty) { carve(x,y); y+=y<ty?1:-1; }
        carve(x,y);
    };
    // A connected backbone plus a seeded loop. Every room has three-tile corridors.
    connect(centers[0],centers[1]); connect(centers[1],centers[2]);
    connect(centers[3],centers[4]); connect(centers[4],centers[5]);
    int bridge=static_cast<int>(next(3)); connect(centers[bridge],centers[bridge+3]);
    int loop=(bridge+1+static_cast<int>(next(2)))%3; connect(centers[loop],centers[loop+3]);
    connect(tileCenter(1,8),centers[0]);
    for(int y=7;y<=9;++y) r.tiles[y][0]='.';
    r.exits.push_back({{0,7*16.0f,2*16.0f,3*16.0f},RegionId::Forest,tileCenter(4,8),false});
    r.objects.push_back({ObjectKind::Sign,"hollow_note","The path remembers your footsteps. Its shape is saved with your journey.",tileCenter(5,6),Item::Tonic,1});
    r.objects.push_back({ObjectKind::Chest,"hollow_sword","Wayfarer's cache",centers[5],Item::Sword,1});
    const Item loot=next(2)==0?Item::Tonic:Item::Fragment;
    r.objects.push_back({ObjectKind::Chest,"hollow_supply","Lost provisions",centers[3],loot,loot==Item::Tonic?2:3});
    return r;
}
}
