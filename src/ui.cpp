#include "ui.hpp"
#include <sstream>
namespace moss::ui {
void text(const std::string& s,int x,int y,int size,Color color) { DrawText(s.c_str(),x,y,size,color); }
void center(const std::string& s,int y,int size,Color color) { text(s,(ViewW-MeasureText(s.c_str(),size))/2,y,size,color); }
void panel(int x,int y,int w,int h) {
    DrawRectangle(x+3,y+3,w,h,{7,17,25,150}); DrawRectangle(x,y,w,h,Ink);
    DrawRectangleLines(x,y,w,h,{78,107,99,255});
    DrawRectangle(x+4,y+4,2,2,Gold); DrawRectangle(x+w-6,y+4,2,2,Gold);
}
int wrap(const std::string& s,int x,int y,int width,int size,Color color,int spacing) {
    std::istringstream stream(s); std::string word,line; const int start=y;
    while(stream>>word) {
        std::string candidate=line.empty()?word:line+" "+word;
        if(!line.empty()&&MeasureText(candidate.c_str(),size)>width) {
            text(line,x,y,size,color); y+=size+spacing; line=word;
        } else line=candidate;
    }
    if(!line.empty()) { text(line,x,y,size,color); y+=size+spacing; }
    return y-start;
}
void heart(int x,int y,bool filled) {
    Color c=filled?Color{224,121,112,255}:Color{62,78,77,255};
    DrawRectangle(x+1,y,2,2,c); DrawRectangle(x+5,y,2,2,c);
    DrawRectangle(x,y+2,8,3,c); DrawRectangle(x+1,y+5,6,1,c);
    DrawRectangle(x+2,y+6,4,1,c); DrawRectangle(x+3,y+7,2,1,c);
    if(filled) DrawPixel(x+1,y+1,{251,185,150,255});
}
void drawHUD(const Game& g) {
    DrawRectangle(0,0,ViewW,33,{16,29,35,245}); DrawRectangle(0,32,ViewW,1,{71,94,87,255});
    for(int i=0;i<g.inventory.maxHealth();++i) heart(12+i*11,9,i<g.player.health);
    text("Q "+std::to_string(g.inventory.get(Item::Tonic))+" TONICS",12,22,10,Muted);
    text("BLADE "+std::to_string(g.inventory.damage()),113,9,10,Paper);
    text(g.player.dashCooldown>0?"DODGE ...":"SPACE DODGE",113,22,10,g.player.dashCooldown>0?Muted:Gold);
    int w=MeasureText(g.map.name.c_str(),10); text(g.map.name,ViewW-w-13,8,10,Gold);
    text("TAB SATCHEL   M JOURNAL   ESC",ViewW-179,22,10,Muted);
    DrawRectangle(0,ViewH-20,ViewW,20,{16,29,35,245});
    DrawRectangle(0,ViewH-20,ViewW,1,{71,94,87,255});
    text("*",12,ViewH-13,10,Gold); text(objective(g.quest,g.inventory),25,ViewH-13,10,Paper);
    for(const auto& e:g.enemies) if(e.type==2&&e.health>0) {
        panel(125,38,230,25); center(e.phaseTwo()?"HOLLOW WARDEN / AWAKENED":"HOLLOW WARDEN",42,10,Gold);
        DrawRectangle(137,55,206,3,{61,68,65,255}); DrawRectangle(137,55,206*e.health/e.maxHealth,3,{223,143,83,255});
    }
    if(g.regionBanner>0&&g.map.id!=RegionId::Arena&&!g.dialogue.active()) {
        float a=std::min(1.0f,g.regionBanner); Color color=Paper; color.a=static_cast<unsigned char>(a*255);
        int w=std::max(MeasureText(g.map.name.c_str(),20),MeasureText(g.map.subtitle.c_str(),10))+24;
        DrawRectangle((ViewW-w)/2,57,w,45,{20,33,39,static_cast<unsigned char>(205*a)});
        center(g.map.name,62,20,color); color=Muted; color.a=static_cast<unsigned char>(a*255);
        center(g.map.subtitle,87,10,color);
    }
}
void drawOverlay(const Game& g) {
    if(g.screen==Screen::Inventory) {
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,165}); panel(38,35,404,203);
        text("THE TRAVELLER'S SATCHEL",54,47,20,Gold); text("TAB / ESC CLOSE     Q DRINK TONIC",54,72,10,Muted);
        int y=94;
        for(int i=0;i<ItemCount;++i) {
            const auto& item=g.items[i]; int n=g.inventory.count[i];
            DrawRectangle(53,y-2,374,17,i%2?Color{27,43,47,255}:Color{23,38,43,255});
            text(n?std::to_string(n):"-",58,y,10,n?Gold:Muted);
            text(item.name,80,y,10,n?Paper:Muted);
            std::string note;
            switch(static_cast<Item>(i)) {
            case Item::Tonic: note="Q: restore 3 hearts"; break;
            case Item::Fragment: note="4: trade for a coat"; break;
            case Item::Sword: note=n?"EQUIPPED / 2 damage":"Wandering Hollow"; break;
            case Item::Coat: note=n?"EQUIPPED / 8 hearts":"Talla, Hearthmere"; break;
            case Item::Ember: note="Forest relic"; break;
            case Item::Dew: note="Stillwater relic"; break;
            case Item::Bell: note="The valley's song"; break;
            default: break;
            }
            text(note,260,y,10,Muted); y+=18;
        }
    } else if(g.screen==Screen::Journal) {
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,165}); panel(24,36,432,200);
        text("A SMALL JOURNEY",40,47,20,Gold); text("M / ESC CLOSE",353,51,10,Muted);
        text("THE QUIET BELL",40,80,10,Gold); wrap(objective(g.quest,g.inventory),40,98,206);
        int y=136;
        const std::array<std::pair<bool,std::string>,4> tasks={{{g.quest.accepted,"Meet Keeper Aven"},{g.inventory.get(Item::Ember)&&g.inventory.get(Item::Dew),"Gather both woodland relics"},{g.quest.shrineLit,"Wake the shrine"},{g.quest.bossDefeated,"Soothe the Hollow Warden"}}};
        for(const auto& task:tasks) { text(task.first?"+":"o",40,y,10,task.first?Gold:Muted); text(task.second,54,y,10,task.first?Paper:Muted); y+=17; }
        DrawRectangle(259,78,1,139,{65,87,81,255});
        auto node=[&](int x,int yy,const char* label,bool active) {
            DrawRectangle(x,yy,7,7,active?Gold:Muted); text(label,x+12,yy-1,10,active?Gold:Paper);
        };
        DrawLine(284,94,284,194,Muted);
        node(281,89,"Warden",g.map.id==RegionId::Arena);
        node(281,120,"Shrine",g.map.id==RegionId::Shrine);
        node(281,152,"Forest",g.map.id==RegionId::Forest);
        node(281,189,"Stillwater",g.map.id==RegionId::Cave);
        text("Hearthmere < west",278,211,10,Muted);
        text("Hollow: northwest",278,172,10,g.map.id==RegionId::Hollow?Gold:Muted);
    } else if(g.screen==Screen::Pause) {
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,170}); panel(130,58,220,148);
        center("A MOMENT OF QUIET",72,20,Gold);
        const char* labels[]={"Continue wandering","Save journey","Save and return to title"};
        for(int i=0;i<3;++i) { if(i==g.menuSelection) DrawRectangle(143,103+i*26,194,22,{43,64,62,255}); center(labels[i],109+i*26,10,i==g.menuSelection?Gold:Paper); }
        center("W/S SELECT   ENTER CONFIRM",219,10,Muted);
    } else if(g.screen==Screen::Death) {
        DrawRectangle(0,0,ViewW,ViewH,{12,24,31,210}); center("THE LANTERN REMEMBERS",91,20,Gold);
        center("Rest a moment. No belongings are lost.",126,10,Paper);
        center("ENTER: return to your last lantern",157,10,Muted);
    } else if(g.screen==Screen::NewGameConfirm) {
        panel(68,77,344,115); center("BEGIN AGAIN?",91,20,Gold);
        center("This replaces your current saved journey.",121,10,Paper);
        center("ENTER: new journey     ESC: keep my save",159,10,Muted);
    }
    if(g.dialogue.active()&&g.screen==Screen::Playing) {
        panel(17,159,446,99); text(g.dialogue.speaker,30,169,10,Gold);
        wrap(g.dialogue.pages[g.dialogue.page],30,187,419,10,Paper,3);
        text("E / ENTER  "+std::to_string(g.dialogue.page+1)+" / "+std::to_string(g.dialogue.pages.size()),349,243,10,Muted);
    }
    if(g.toastTime>0&&!g.dialogue.active()) {
        int width=std::min(444,MeasureText(g.notification.c_str(),10)+24);
        int lines=(MeasureText(g.notification.c_str(),10)+width-25)/(width-24);
        int height=std::max(27,lines*14+12); int y=g.screen==Screen::Title?ViewH-height-10:ViewH-height-29;
        panel((ViewW-width)/2,y,width,height); wrap(g.notification,(ViewW-width)/2+12,y+9,width-24,10,Paper);
    }
}
}
