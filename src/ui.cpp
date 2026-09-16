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
    DrawRectangle(0,0,ViewW,47,{16,29,35,245}); DrawRectangle(0,46,ViewW,1,{71,94,87,255});
    for(int i=0;i<g.inventory.maxHealth();++i) heart(12+i*10,7,i<g.player.health);
    DrawRectangle(12,20,98,5,{48,65,63,255}); DrawRectangle(12,20,int(98*g.player.stamina/100),5,g.player.stamina<22?Color{202,133,94,255}:Color{124,179,121,255});
    text("Q "+std::to_string(g.inventory.get(Item::Tonic))+" TONICS",12,33,10,Muted);
    text(std::string(weaponName(g.inventory.weapon))+" / "+std::to_string(g.inventory.weapon==Weapon::Fists?1:(g.inventory.weapon==Weapon::Bow?2:g.inventory.damage())),125,7,10,Paper);
    const char* action=g.inventory.weapon==Weapon::Bow?"J FIRE / RMB AIM":(g.inventory.weapon==Weapon::Fists?"J JAB / L SHOVE":"J CUT / L HEAVY");
    text(g.player.stunned>0?"GUARD BROKEN":(g.player.blocking?"GUARDING":action),125,20,10,g.player.blocking?Gold:Muted);
    text("ARROWS "+std::to_string(g.inventory.get(Item::Arrow)),125,33,10,Muted);
    int w=MeasureText(g.map.name.c_str(),10); text(g.map.name,ViewW-w-13,8,10,Gold);
    text(std::to_string(g.inventory.coins)+" COINS  /  TAB SATCHEL",ViewW-172,20,10,Gold);
    text(g.lockedEnemy()?"F RELEASE  R NEXT  F1 HELP":"F LOCK  R NEXT  F1 HELP",ViewW-172,33,10,g.lockedEnemy()?Gold:Muted);
    DrawRectangle(0,ViewH-20,ViewW,20,{16,29,35,245});
    DrawRectangle(0,ViewH-20,ViewW,1,{71,94,87,255});
    text("*",12,ViewH-13,10,Gold); text(objective(g.quest,g.inventory),25,ViewH-13,10,Paper);
    for(const auto& e:g.enemies) if(e.type==2&&e.health>0) {
        panel(125,51,230,25); center(e.phaseTwo()?"HOLLOW WARDEN / AWAKENED":"HOLLOW WARDEN",55,10,Gold);
        DrawRectangle(137,68,206,3,{61,68,65,255}); DrawRectangle(137,68,206*e.health/e.maxHealth,3,{223,143,83,255});
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
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,185}); panel(24,27,432,218);
        text("ARMS & BELONGINGS",40,38,20,Gold); text(std::to_string(g.inventory.coins)+" COINS",365,43,10,Gold);
        bool plate=g.inventory.armor==Armor::Plate;
        text(std::string(armorName(g.inventory.armor))+" | "+std::to_string(g.inventory.maxHealth())+" HEARTS | DODGE "+(plate?"28":"22")+" | SPEED "+(plate?"91%":"100%"),40,65,10,Paper);
        text("1/2/3 WEAPON  ENTER ARMOR  W/S SCROLL  TAB CLOSE",40,81,10,Muted);
        int selected=std::clamp(g.menuSelection,0,ItemCount-1),start=std::clamp(selected-3,0,ItemCount-6),y=99;
        for(int i=start;i<std::min(ItemCount,start+6);++i) {
            const auto& item=g.items[i]; int n=g.inventory.count[i];
            DrawRectangle(38,y-3,399,17,i==selected?Color{48,67,63,255}:Color{25,41,45,255});
            text(n?std::to_string(n):"-",45,y,10,n?Gold:Muted); text(item.name,69,y,10,n?Paper:Muted);
            bool equipped=(i==int(Item::Bow)&&g.inventory.weapon==Weapon::Bow)||
                (g.inventory.weapon==Weapon::Sword&&((i==int(Item::KnightSword)&&n)||(i==int(Item::Sword)&&n&&!g.inventory.get(Item::KnightSword))))||
                (i==int(Item::KnightArmor)&&g.inventory.armor==Armor::Plate)||(i==int(Item::Coat)&&g.inventory.armor==Armor::Coat);
            if(equipped) text("EQUIPPED",347,y,10,Gold);
            else if(n&&(i==int(Item::Coat)||i==int(Item::KnightArmor))) text("ENTER WEAR",347,y,10,Muted);
            else if(item.sellPrice) text("SELL "+std::to_string(item.sellPrice)+"c",347,y,10,Muted);
            y+=18;
        }
        DrawRectangle(441,96,2,108,{62,79,73,255}); DrawRectangle(441,96+start*9,2,54,Gold);
        std::string description=g.items[selected].description;
        if(selected==int(Item::Coat)||selected==int(Item::KnightArmor)) description+=" Enter toggles wear/remove.";
        wrap(description,40,213,391,10,Muted,3);
    } else if(g.screen==Screen::Shop) {
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,185}); panel(30,39,420,206);
        text(g.merchantName,46,51,10,Gold); text("TRADE & PROVISIONS",46,69,20,Paper);
        text(std::to_string(g.inventory.coins)+" COINS",350,73,10,Gold);
        int y=101;
        for(size_t i=0;i<g.trades.size();++i) {
            const auto& t=g.trades[i]; bool selected=int(i)==g.menuSelection;
            DrawRectangle(44,y-3,390,16,selected?Color{48,67,63,255}:Color{25,41,45,255});
            text(t.selling?"SELL":"BUY",50,y,10,t.selling?Muted:Gold);
            text(std::to_string(t.amount)+" "+g.items[static_cast<int>(t.item)].name,85,y,10,selected?Paper:Muted);
            text(std::to_string(t.price)+"c",334,y,10,Gold); text("x"+std::to_string(g.inventory.get(t.item)),388,y,10,Muted); y+=17;
        }
        wrap(g.shopMessage,46,207,387,10,Gold,2);
        text("W/S SELECT    ENTER TRADE    E / ESC LEAVE",46,232,10,Muted);
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
        DrawLine(284,94,284,170,Muted);
        node(281,89,"Warden",g.map.id==RegionId::Arena);
        node(281,115,"Shrine",g.map.id==RegionId::Shrine);
        node(281,141,"Forest",g.map.id==RegionId::Forest);
        node(281,167,"Stillwater",g.map.id==RegionId::Cave);
        text("Hollow: northwest",278,187,10,g.map.id==RegionId::Hollow?Gold:Muted);
        text("Greywatch: east",278,201,10,g.map.id==RegionId::Bailey?Gold:Muted);
        text("Shops / inn: village",278,215,10,g.map.id==RegionId::Smithy||g.map.id==RegionId::Inn?Gold:Muted);
    } else if(g.screen==Screen::Pause) {
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,170}); panel(130,47,220,174);
        center("A MOMENT OF QUIET",61,20,Gold);
        const char* labels[]={"Continue wandering","Save journey","Controls / Legend","Save and return to title"};
        for(int i=0;i<4;++i) { if(i==g.menuSelection) DrawRectangle(143,92+i*26,194,22,{43,64,62,255}); center(labels[i],98+i*26,10,i==g.menuSelection?Gold:Paper); }
        center("W/S SELECT   ENTER CONFIRM",203,10,Muted);
    } else if(g.screen==Screen::Help) {
        DrawRectangle(0,0,ViewW,ViewH,{7,16,24,220}); panel(12,15,456,239);
        text(g.helpPage==0?"CONTROLS / LEGEND":"COMBAT TACTICS",28,28,20,Gold);
        DrawRectangle(239,60,1,162,{65,87,81,255});
        if(g.helpPage==0) {
            text("COMBAT & MOVEMENT",28,60,10,Gold); text("LIFE ON THE ROAD",254,60,10,Gold);
            const char* left[]={"WASD / ARROWS   Move","SHIFT   Run","J / Z   Attack","L / X   Heavy strike / shove","K / C   Directional guard","SPACE   Dodge","1 / 2 / 3   Fists / sword / bow","F / MIDDLE MOUSE   Lock on/off","R   Next nearby target","RMB   Manual aim, releases lock","Q   Drink a tonic"};
            const char* right[]={"E   Talk / loot / enter / rest","TAB / I   Satchel","W/S   Scroll items / offers","ENTER   Wear / remove armor","M   Quest journal & routes","ENTER   Confirm / buy / sell","ESC   Pause / back","F5   Save journey","F10   Mute sound","F11   Borderless fullscreen","F1 / H   Open this guide"};
            for(int i=0;i<11;++i) { text(left[i],28,80+i*13,10,Paper); text(right[i],254,80+i*13,10,Paper); }
        } else {
            auto tip=[&](int x,int y,const char* label,const char* body) { text(label,x,y,10,Gold); wrap(body,x,y+15,199,10,Paper,2); };
            tip(28,60,"FACE YOUR FOE","F focuses a visible enemy. Strafe with WASD; R cycles foes. Walls or distance break focus.");
            tip(28,117,"BREAK THEIR GUARD","Heavy strikes break shields. Circle behind for quick cuts. Bows consume arrows; shields stop them.");
            tip(28,174,"TIME YOUR DEFENSE","Raise K just before a blow to stagger. Guard only covers your front. Low stamina can break it.");
            tip(254,60,"CHOOSE YOUR ARMOR","Coat: 8 hearts, light movement. Plate: 10 hearts, heavy hit -1, slower steps, costlier dodges.");
            tip(254,117,"LEAVE ROOM TO RECOVER","Attacks commit you until recovery. Ease off to regain stamina. Gear changes never heal you.");
            tip(254,174,"SEARCH, TRADE, REST","E searches remains. Sell shells or fragments. Rest heals and saves; foes return, loose loot fades.");
        }
        center("W/S PAGE "+std::to_string(g.helpPage+1)+"/2    F1 / H / ESC BACK",235,10,Muted);
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
    if(g.toastTime>0&&!g.dialogue.active()&&g.screen!=Screen::Help) {
        int width=std::min(444,MeasureText(g.notification.c_str(),10)+24);
        int lines=(MeasureText(g.notification.c_str(),10)+width-25)/(width-24);
        int height=std::max(27,lines*14+12); int y=g.screen==Screen::Title?ViewH-height-10:ViewH-height-29;
        panel((ViewW-width)/2,y,width,height); wrap(g.notification,(ViewW-width)/2+12,y+9,width-24,10,Paper);
    }
}
}
