#include "render.hpp"
#include <functional>
namespace moss {
namespace {
struct Palette { Color grass,dark,light,path,stone,water; };
constexpr Palette palettes[]={
    {{90,132,100,255},{57,100,78,255},{123,156,103,255},{179,161,119,255},{138,155,143,255},{66,122,135,255}},
    {{58,102,79,255},{37,74,62,255},{91,136,88,255},{153,142,102,255},{106,132,117,255},{47,102,116,255}},
    {{58,70,83,255},{38,45,61,255},{83,92,103,255},{88,94,100,255},{103,111,128,255},{45,87,115,255}},
    {{79,117,99,255},{48,81,74,255},{115,146,106,255},{147,150,123,255},{136,153,140,255},{61,112,130,255}}
};
uint32_t hash(int x,int y) { uint32_t n=static_cast<uint32_t>(x)*374761393u+static_cast<uint32_t>(y)*668265263u; n=(n^(n>>13))*1274126177u; return n^(n>>16); }
Color light(Color c,int amount) { return {static_cast<unsigned char>(std::clamp(int(c.r)+amount,0,255)),static_cast<unsigned char>(std::clamp(int(c.g)+amount,0,255)),static_cast<unsigned char>(std::clamp(int(c.b)+amount,0,255)),c.a}; }
void rect(int x,int y,int w,int h,Color c) { DrawRectangle(x,y,w,h,c); }
void tree(int x,int y,const Palette& p,int variant) {
    rect(x-7,y+3,17,5,{22,49,42,110}); rect(x-2,y-4,5,10,{90,76,59,255}); rect(x,y-3,2,8,{124,99,69,255});
    Color a=light(p.dark,variant*3),b=light(p.grass,variant*3);
    rect(x-10,y-14,21,12,a); rect(x-7,y-21,15,21,a); rect(x-3,y-24,8,5,a);
    rect(x-8,y-15,16,9,b); rect(x-5,y-21,12,13,b); rect(x-2,y-23,6,3,b);
    rect(x-5,y-18,8,3,p.light); rect(x-8,y-10,4,2,p.light); rect(x+5,y-13,4,4,a); rect(x-2,y-6,5,3,a);
}
void person(int x,int y,Vec facing,float walk,Color coat,bool npc=false,bool flash=false) {
    Color outline={29,43,45,255},skin={234,190,144,255},hair={103,67,49,255};
    if(flash) { coat=WHITE; skin=WHITE; }
    int step=walk>0?static_cast<int>(std::sin(walk*13)*2):0;
    rect(x-6,y+2,13,3,{14,33,33,100});
    rect(x-4,y-2+step,3,5,outline); rect(x+2,y-2-step,3,5,outline);
    rect(x-6,y-11,13,10,outline); rect(x-5,y-11,11,9,coat); rect(x-4,y-4,9,2,light(coat,-18));
    rect(x-7,y-10+step,2,5,skin); rect(x+6,y-10-step,2,5,skin);
    rect(x-5,y-20,10,10,outline); rect(x-4,y-19,8,8,skin);
    rect(x-5,y-20,10,4,hair); rect(x-6,y-18,3,4,hair);
    if(facing.y<-.4f) { rect(x-4,y-17,8,5,hair); rect(x-3,y-10,6,6,light(coat,-20)); }
    else { int eye=facing.x>.3f?2:(facing.x<-.3f?-2:0); rect(x-2+eye,y-15,1,2,outline); rect(x+2+eye,y-15,1,2,outline); }
    rect(x-5,y-11,10,2,{235,187,97,255}); rect(x+3,y-9,2,4,{235,187,97,255});
    if(!npc) { rect(x-7,y-6,3,5,{138,88,57,255}); rect(x-7,y-6,3,1,{208,158,94,255}); }
}
void house(int x,int y,int w,int h) {
    const Color edge{60,58,57,255},roof{149,85,70,255},roofLight{180,111,83,255};
    rect(x+3,y+4,w,h,{24,46,40,110}); rect(x+2,y+9,w-4,h-9,edge);
    rect(x+5,y+12,w-10,h-16,{222,194,147,255});
    for(int i=9;i<w-8;i+=20) rect(x+i,y+14,3,h-20,{123,93,68,255});
    rect(x-3,y+6,w+6,h-26,edge); rect(x-1,y+4,w+2,h-26,roof);
    rect(x+3,y,w-6,5,roofLight);
    for(int j=10;j<h-24;j+=6) { rect(x,y+j,w,1,{119,71,62,255}); for(int i=(j%12?4:10);i<w;i+=13) rect(x+i,y+j-4,1,4,roofLight); }
    int door=x+w/2-5;
    rect(door,y+h-19,11,19,edge); rect(door+2,y+h-16,7,15,{120,86,59,255}); rect(door+7,y+h-9,1,2,ui::Gold);
    for(int i:{12,w-21}) { rect(x+i,y+h-22,10,11,edge); rect(x+i+2,y+h-20,6,7,{240,190,106,255}); rect(x+i+5,y+h-20,1,7,{152,107,62,255}); }
    rect(x+w-17,y-6,8,13,{106,113,100,255}); rect(x+w-18,y-7,10,3,{167,160,137,255});
}
void ground(const Region& m,int tx,int ty,float time) {
    const auto& p=palettes[m.palette]; int x=tx*16,y=ty*16; char t=m.tile(tx,ty); uint32_t n=hash(tx,ty);
    Color base=p.grass; if(t=='+'||t=='_') base=p.path; if(t==':'||t=='P') base=p.stone;
    if(t=='#') base=p.dark;
    if(t=='~') base=p.water;
    rect(x,y,16,16,light(base,static_cast<int>(n%3)*2));
    if(t=='~') {
        int phase=static_cast<int>(time*2+n%4)%4;
        rect(x+2+phase,y+4,5,1,light(p.water,18)); rect(x+8-phase,y+11,5,1,light(p.water,12));
        if(m.tile(tx,ty-1)!='~') rect(x,y,16,2,light(p.water,24));
        if(m.tile(tx-1,ty)!='~') rect(x,y,2,16,p.dark);
    } else if(t=='#') {
        rect(x+1,y+1,14,11,light(p.dark,11)); rect(x+2,y+1,12,2,light(p.dark,23)); rect(x+12,y+5,2,6,p.dark);
    } else if(t==':') {
        rect(x,y,16,1,light(p.stone,-22)); rect(x+(ty%2?0:8),y,1,16,light(p.stone,-22));
        if(n%4==0) { rect(x+3,y+9,5,1,p.grass); rect(x+3,y+9,1,4,p.grass); }
    } else if(t=='_') {
        for(int i=1;i<16;i+=4) { rect(x,y+i,16,1,{101,82,63,255}); rect(x+2,y+i+1,1,1,ui::Gold); }
    } else {
        int dx=2+int(n%11),dy=3+int((n>>6)%10);
        rect(x+dx,y+dy,2,1,t=='+'?light(base,-14):p.dark);
        if(t!='+') { rect(x+dx+1,y+dy-2,1,2,p.light); if(n%3==0) rect(x+3,y+12,2,1,p.light); }
        if(t==',') for(int i=0;i<3;++i) {
            int fx=x+3+(i*5+int(n%3))%12,fy=y+4+(i*7)%10;
            rect(fx,fy+1,1,3,p.dark); rect(fx-1,fy,3,2,i%2?Color{220,174,141,255}:Color{230,218,169,255});
        }
        if(t=='*') {
            rect(x+6,y+6,5,8,{82,174,181,255}); rect(x+7,y+3,3,10,{136,215,211,255});
            rect(x+4,y+9,2,5,{74,136,156,255}); rect(x+8,y+5,1,6,{205,240,220,255});
        }
    }
}
void object(const Object& o,const Game& g) {
    int x=static_cast<int>(o.pos.x),y=static_cast<int>(o.pos.y);
    const Color dark{47,53,52,255};
    switch(o.kind) {
    case ObjectKind::Npc:
        person(x,y,normalized(g.player.pos-o.pos),0,o.id=="keeper"?Color{120,140,156,255}:Color{159,102,99,255},true);
        if(o.id=="keeper"&&!g.quest.accepted) { rect(x-1,y-31,3,6,ui::Gold); rect(x-1,y-23,3,2,ui::Gold); }
        break;
    case ObjectKind::Chest: {
        bool open=g.openedChests.count(o.id)>0;
        rect(x-7,y-7,15,11,dark); rect(x-6,y-6,13,9,{145,101,64,255});
        rect(x-6,y-6,13,3,open?dark:Color{183,132,73,255});
        if(open) { rect(x-7,y-12,15,5,{126,89,61,255}); rect(x-6,y-11,13,1,ui::Gold); }
        else { rect(x-7,y-8,15,1,ui::Gold); rect(x-1,y-5,3,4,ui::Gold); }
        rect(x-4,y-7,1,11,ui::Gold); rect(x+4,y-7,1,11,ui::Gold);
        break;
    }
    case ObjectKind::Checkpoint: {
        DrawCircle(x,y-15,18,{232,187,102,17}); DrawCircle(x,y-15,11,{232,187,102,20});
        rect(x-2,y-13,4,17,dark); rect(x-6,y+2,12,3,dark); rect(x-5,y-21,10,12,dark);
        rect(x-3,y-19,6,8,{232,179,87,255}); rect(x-1,y-18,2,6,{255,234,166,255});
        rect(x-6,y-23,12,3,{140,156,123,255}); rect(x-1,y-26,2,3,ui::Gold);
        break;
    }
    case ObjectKind::Altar:
        rect(x-12,y-7,24,13,dark); rect(x-10,y-8,20,11,{154,170,151,255});
        rect(x-8,y-12,16,7,{179,187,157,255}); rect(x-6,y-11,12,4,g.quest.shrineLit?ui::Gold:Color{77,98,90,255});
        rect(x-7,y+3,14,3,{119,143,124,255});
        if(g.quest.shrineLit) { DrawCircle(x,y-15,14,{235,205,128,25}); rect(x-1,y-20,2,6,ui::Gold); }
        break;
    case ObjectKind::Sign:
        rect(x-1,y-6,3,10,{106,80,56,255}); rect(x-8,y-16,17,11,dark);
        rect(x-7,y-15,15,9,{170,136,87,255}); rect(x-5,y-12,9,1,{100,81,62,255}); rect(x-5,y-9,6,1,{100,81,62,255});
        break;
    }
}
void enemy(const Enemy& e) {
    int x=static_cast<int>(e.pos.x),y=static_cast<int>(e.pos.y); const Color outline{35,47,48,255};
    const bool flash=e.flash>0,warn=e.mode==EnemyMode::Windup||e.mode==EnemyMode::Volley;
    rect(x-8,y+2,17,4,{15,28,30,110});
    if(e.type==0) {
        int bob=static_cast<int>(std::sin(e.age*5)*1.5f); y+=bob;
        Color body=flash?WHITE:Color{146,159,89,255};
        rect(x-8,y-10,17,11,outline); rect(x-6,y-14,13,16,outline); rect(x-7,y-10,15,10,body); rect(x-5,y-13,11,13,body);
        rect(x-5,y-16,3,4,{82,109,71,255}); rect(x+3,y-18,3,6,{99,132,77,255}); rect(x+5,y-18,3,2,{134,159,87,255});
        rect(x-4,y-7,2,3,warn?ui::Gold:outline); rect(x+3,y-7,2,3,warn?ui::Gold:outline);
        rect(x-5,y+1,4,3,outline); rect(x+3,y+1,4,3,outline);
    } else if(e.type==1) {
        y-=5+static_cast<int>(std::sin(e.age*5)*2);
        int wing=static_cast<int>(std::sin(e.age*14)*3);
        Color c=flash?WHITE:Color{151,163,189,255};
        rect(x-13,y-10+wing,9,8,c); rect(x+5,y-10-wing,9,8,c);
        rect(x-10,y-4+wing,6,5,{104,122,156,255}); rect(x+5,y-4-wing,6,5,{104,122,156,255});
        rect(x-3,y-12,7,15,outline); rect(x-2,y-11,5,12,warn?ui::Gold:Color{227,180,123,255});
        rect(x-4,y-15,1,4,c); rect(x+4,y-15,1,4,c); rect(x-1,y-8,1,2,outline); rect(x+2,y-8,1,2,outline);
        DrawCircle(x,y-3,7,{236,186,107,25});
    } else {
        Color bark=flash?WHITE:Color{115,129,104,255}; Color glow=e.phaseTwo()?Color{248,156,100,255}:ui::Gold;
        rect(x-16,y-5,10,12,outline); rect(x+7,y-5,10,12,outline);
        rect(x-14,y-4,6,9,bark); rect(x+9,y-4,6,9,bark);
        rect(x-15,y-28,31,26,outline); rect(x-13,y-26,27,24,bark);
        rect(x-19,y-24,7,19,bark); rect(x+13,y-24,7,19,bark);
        rect(x-13,y-32,27,7,{73,103,83,255}); rect(x-8,y-36,17,6,{102,133,85,255});
        rect(x-13,y-44,3,15,bark); rect(x+11,y-44,3,15,bark);
        rect(x-20,y-45,10,3,bark); rect(x+12,y-45,10,3,bark);
        rect(x-21,y-50,3,7,bark); rect(x+20,y-50,3,7,bark);
        rect(x-9,y-22,7,5,outline); rect(x+3,y-22,7,5,outline);
        rect(x-8,y-21,5,3,glow); rect(x+4,y-21,5,3,glow);
        rect(x-3,y-12,7,7,outline); rect(x-1,y-11,3,5,glow);
        rect(x-10,y-10,4,3,{71,92,71,255}); rect(x+7,y-14,4,5,{71,92,71,255});
    }
    if(e.health<e.maxHealth&&e.type!=2) { rect(x-9,y-23,18,3,outline); rect(x-8,y-22,16*e.health/e.maxHealth,1,ui::Gold); }
}
}
Renderer::Renderer() { target=LoadRenderTexture(ViewW,ViewH); SetTextureFilter(target.texture,TEXTURE_FILTER_POINT); }
Renderer::~Renderer() { UnloadRenderTexture(target); }
void Renderer::world(const Game& g,float dt) {
    Vec focus=g.player.pos;
    if(g.map.id==RegionId::Arena) {
        for(const auto& e:g.enemies) if(e.type==2&&e.health>0&&length(e.pos-g.player.pos)<220) {
            focus=g.player.pos*.65f+e.pos*.35f+Vec{0,-24};
            break;
        }
    }
    Vec goal={std::clamp(focus.x,ViewW/2.0f,g.map.width*16-ViewW/2.0f),std::clamp(focus.y,ViewH/2.0f,g.map.height*16-ViewH/2.0f)};
    if(lastRegion!=g.map.id) { camera=goal; lastRegion=g.map.id; }
    camera+=(goal-camera)*std::min(1.0f,dt*9);
    Camera2D cam{}; cam.target={std::floor(camera.x),std::floor(camera.y)}; cam.offset={ViewW/2.0f,ViewH/2.0f}; cam.zoom=1;
    if(g.shake>0) { cam.offset.x+=int(std::sin(g.clock*90)*2); cam.offset.y+=int(std::cos(g.clock*100)); }
    BeginMode2D(cam);
    int left=std::max(0,int(camera.x-ViewW/2)/16-2),right=std::min(g.map.width-1,int(camera.x+ViewW/2)/16+2);
    int top=std::max(0,int(camera.y-ViewH/2)/16-3),bottom=std::min(g.map.height-1,int(camera.y+ViewH/2)/16+3);
    for(int y=top;y<=bottom;++y) for(int x=left;x<=right;++x) ground(g.map,x,y,g.clock);
    for(const auto& ex:g.map.exits) {
        Color color=ex.locked&&!g.quest.shrineLit?Color{224,157,111,255}:Color{185,213,156,255};
        int x=int(ex.bounds.x+ex.bounds.w/2),y=int(ex.bounds.y+ex.bounds.h/2);
        if(ex.bounds.y<32) { for(int i=0;i<3;++i) rect(x-3+i,y+5-i*2,7-i*2,1,color); }
        else if(ex.bounds.y>g.map.height*16-40) { for(int i=0;i<3;++i) rect(x-3+i,y-5+i*2,7-i*2,1,color); }
        else { int d=ex.bounds.x<32?-1:1; for(int i=0;i<3;++i) rect(x+d*i*2,y-3+i,1,7-i*2,color); }
        if(ex.locked&&!g.quest.shrineLit) { rect(int(ex.bounds.x),y,int(ex.bounds.w),3,color); for(int i=0;i<5;++i) rect(int(ex.bounds.x)+i*22,y-14,2,20,color); }
    }
    for(const auto& e:g.enemies) if(e.health>0) {
        bool warn=e.mode==EnemyMode::Windup||e.mode==EnemyMode::Volley;
        if(warn) {
            const Color c={244,179,104,210};
            DrawCircleLines(int(e.pos.x),int(e.pos.y),e.type==2?24.0f:13.0f,c);
            if(e.mode==EnemyMode::Windup&&e.type!=1) {
                Vector2 a{e.pos.x,e.pos.y},b{e.pos.x+e.facing.x*(e.type==2?100:35),e.pos.y+e.facing.y*(e.type==2?100:35)};
                DrawLineEx(a,b,2,{244,179,104,130}); DrawCircleLines(int(b.x),int(b.y),5,c);
            }
            if(e.mode==EnemyMode::Volley) DrawCircleLines(int(e.pos.x),int(e.pos.y),34,c);
        }
    }
    struct DrawEntry { float depth; std::function<void()> draw; };
    std::vector<DrawEntry> entries;
    for(int y=top;y<=bottom;++y) for(int x=left;x<=right;++x) {
        char t=g.map.tile(x,y);
        if(t=='T') entries.push_back({float(y*16+14),[&,x,y]{ tree(x*16+8,y*16+12,palettes[g.map.palette],hash(x,y)%3); }});
        if(t=='P') entries.push_back({float(y*16+15),[x,y]{ int px=x*16,py=y*16; rect(px+3,py-6,11,22,{78,99,91,255}); rect(px+5,py-6,7,21,{149,165,145,255}); rect(px+2,py-8,13,4,{184,188,158,255}); rect(px+1,py+13,15,3,{109,136,117,255}); }});
    }
    // House footprints are sparse; scan their origins even when the origin is just offscreen.
    for(int y=0;y<g.map.height;++y) for(int x=0;x<g.map.width;++x) if(g.map.tile(x,y)=='H'&&g.map.tile(x-1,y)!='H'&&g.map.tile(x,y-1)!='H') {
        int w=0,h=0; while(g.map.tile(x+w,y)=='H') ++w; while(g.map.tile(x,y+h)=='H') ++h;
        entries.push_back({float((y+h)*16),[x,y,w,h]{house(x*16,y*16,w*16,h*16);}});
    }
    for(const auto& o:g.map.objects) entries.push_back({o.pos.y,[&g,&o]{object(o,g);}});
    for(const auto& e:g.enemies) if(e.health>0) entries.push_back({e.pos.y,[&e]{enemy(e);}});
    entries.push_back({g.player.pos.y,[&g]{
        const auto& p=g.player; int x=int(p.pos.x),y=int(p.pos.y);
        person(x,y,p.facing,p.moving?p.walkTime:0,g.inventory.get(Item::Coat)?Color{91,140,103,255}:Color{74,139,146,255},false,p.invulnerable>0&&int(g.clock*18)%2==0);
        if(p.dashTime>0) { rect(x-int(p.dashDirection.x*14)-3,y-int(p.dashDirection.y*14)-8,6,5,{204,219,179,90}); }
        if(p.attackTime>0) {
            float angle=std::atan2(p.facing.y,p.facing.x)*180/Pi;
            DrawRing({p.pos.x,p.pos.y-4},19,23,angle-65,angle+65,12,{252,229,167,180});
            float sweep=(angle-65+(1-p.attackTime/.22f)*130)*Pi/180;
            Vec direction={std::cos(sweep),std::sin(sweep)};
            DrawLineEx({p.pos.x+direction.x*7,p.pos.y-4+direction.y*7},{p.pos.x+direction.x*25,p.pos.y-4+direction.y*25},3,ui::Paper);
            DrawLineEx({p.pos.x+direction.x*5,p.pos.y-4+direction.y*5},{p.pos.x+direction.x*11,p.pos.y-4+direction.y*11},3,ui::Gold);
        }
    }});
    std::stable_sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    for(const auto& entry:entries) entry.draw();
    for(const auto& s:g.projectiles) if(s.life>0) { DrawCircle(int(s.pos.x),int(s.pos.y),6,{233,172,100,40}); rect(int(s.pos.x)-2,int(s.pos.y)-2,5,5,ui::Gold); rect(int(s.pos.x),int(s.pos.y)-1,2,2,ui::Paper); }
    for(const auto& p:g.particles) { Color colors[]={ui::Paper,ui::Gold,{226,126,112,255}}; Color c=colors[p.color%3]; c.a=static_cast<unsigned char>(255*p.life/p.maxLife); rect(int(p.pos.x),int(p.pos.y),2,2,c); }
    EndMode2D();
    // Screen-space motes are subtle, deterministic and cost no textures.
    for(int i=0;i<15;++i) {
        int x=int(std::fmod(i*57+g.clock*(i%3+1),float(ViewW))),y=int(std::fmod(i*31+std::sin(g.clock*.4f+i)*7+ViewH,float(ViewH)));
        rect(x,y,1,1,{238,228,172,static_cast<unsigned char>(35+(i%4)*12)});
    }
    ui::drawHUD(g);
    if(const auto* near=g.nearbyObject();near&&g.screen==Screen::Playing&&!g.dialogue.active()&&g.regionBanner<=1) {
        auto point=GetWorldToScreen2D({near->pos.x,near->pos.y-32},cam);
        std::string label="E ";
        switch(near->kind) {
        case ObjectKind::Npc: label+="Talk"; break;
        case ObjectKind::Chest: label+=g.openedChests.count(near->id)?"Empty":"Open"; break;
        case ObjectKind::Checkpoint: label+="Rest & save"; break;
        case ObjectKind::Altar: label+="Offer relics"; break;
        case ObjectKind::Sign: label+="Read"; break;
        }
        int w=MeasureText(label.c_str(),10)+12; int x=std::clamp(int(point.x)-w/2,5,ViewW-w-5),y=std::clamp(int(point.y),37,221);
        ui::panel(x,y,w,19); ui::text(label,x+6,y+5,10,ui::Gold);
    }
}
void Renderer::title(const Game& g) {
    ClearBackground({28,51,61,255});
    rect(0,65,480,90,{49,79,80,255}); rect(0,100,480,68,{68,100,89,255});
    for(int i=0;i<14;++i) { int h=30+int(hash(i,1)%45); rect(i*39,129-h,40,h,{51,80,74,255}); tree(i*40,143,palettes[1],i%3); }
    rect(0,158,480,112,{72,105,86,255}); rect(266,181,214,89,{65,113,119,255});
    for(int i=0;i<30;++i) { int x=270+int(hash(i,2)%208),y=186+int(hash(i,3)%80); rect(x+int(std::sin(g.clock*.7f+i)*3),y,5+int(hash(i,4)%15),1,{91,137,134,255}); }
    rect(280,151,162,33,{115,140,104,255}); rect(295,139,135,35,{125,148,111,255});
    rect(325,122,80,41,{147,156,124,255}); house(337,75,57,62);
    rect(350,142,15,66,{159,151,108,255}); rect(329,193,55,8,{142,112,78,255});
    for(int i=0;i<7;++i) rect(332+i*8,191,1,14,{88,87,65,255});
    tree(300,147,palettes[0],1); tree(421,155,palettes[0],2); tree(446,179,palettes[1],0);
    person(357,178,{0,1},0,{74,139,146,255});
    DrawCircle(425,38,14,{224,215,165,255}); DrawCircle(431,33,13,{28,51,61,255});
    for(int i=0;i<24;++i) { int x=18+int(hash(i,7)%445),y=12+int(hash(i,8)%70); rect(x,y,1,1,{221,217,166,static_cast<unsigned char>(100+70*std::sin(g.clock+i))}); }
    rect(0,0,269,270,{18,34,39,225}); rect(268,20,1,230,{90,116,99,255});
    ui::text("A WOODLAND TALE",25,29,10,ui::Muted); ui::text("MOSSLIGHT",23,50,40,ui::Paper);
    ui::text("THE QUIET BELL",26,99,20,ui::Gold); rect(26,129,35,2,ui::Gold);
    ui::text("Small wonders. Forgotten roads.",26,140,10,ui::Muted);
    const char* labels[]={"Continue journey","Begin a new journey","Leave the valley"};
    for(int i=0;i<3;++i) {
        if(g.menuSelection==i) { rect(21,163+i*25,227,22,{43,64,60,255}); rect(21,163+i*25,2,22,ui::Gold); }
        ui::text(labels[i],34,169+i*25,10,i==0&&!g.hasSave?Color{94,119,111,255}:(g.menuSelection==i?ui::Gold:ui::Paper));
        if(g.menuSelection==i) ui::text(">",231,169+i*25,10,ui::Gold);
    }
    ui::text("W/S SELECT    ENTER BEGIN",25,248,10,ui::Muted);
    ui::text("C++ / RAYLIB",387,251,10,{179,195,168,255});
}
void Renderer::draw(const Game& g,float dt) {
    BeginTextureMode(target); ClearBackground(ui::Ink);
    if(g.screen==Screen::Title||g.screen==Screen::NewGameConfirm) title(g); else world(g,dt);
    ui::drawOverlay(g); EndTextureMode();
    BeginDrawing(); ClearBackground({9,17,23,255});
    int scale=std::max(1,std::min(GetScreenWidth()/ViewW,GetScreenHeight()/ViewH));
    Rectangle dest{float((GetScreenWidth()-ViewW*scale)/2),float((GetScreenHeight()-ViewH*scale)/2),float(ViewW*scale),float(ViewH*scale)};
    DrawTexturePro(target.texture,{0,0,float(ViewW),-float(ViewH)},dest,{0,0},0,WHITE); EndDrawing();
}
void Renderer::capture(const std::filesystem::path& path) const {
    Image img=LoadImageFromTexture(target.texture); ImageFlipVertical(&img); ExportImage(img,path.string().c_str()); UnloadImage(img);
}
}
