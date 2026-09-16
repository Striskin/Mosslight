#include "audio.hpp"
#include "render.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {
moss::Input readInput() {
    moss::Input in;
    in.move={float(IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT))-float(IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)),
             float(IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN))-float(IsKeyDown(KEY_W)||IsKeyDown(KEY_UP))};
    in.attack=IsKeyDown(KEY_J)||IsKeyDown(KEY_Z); in.dodge=IsKeyPressed(KEY_SPACE);
    in.interact=IsKeyPressed(KEY_E); in.heal=IsKeyPressed(KEY_Q); in.inventory=IsKeyPressed(KEY_TAB)||IsKeyPressed(KEY_I);
    in.journal=IsKeyPressed(KEY_M); in.pause=IsKeyPressed(KEY_ESCAPE); in.confirm=IsKeyPressed(KEY_ENTER);
    in.up=IsKeyPressed(KEY_W)||IsKeyPressed(KEY_UP); in.down=IsKeyPressed(KEY_S)||IsKeyPressed(KEY_DOWN);
    in.save=IsKeyPressed(KEY_F5); in.sprint=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT);
    return in;
}
std::filesystem::path findData(const std::filesystem::path& exe) {
    for(const auto& path:{exe/"data",exe.parent_path()/"data",std::filesystem::current_path()/"data"})
        if(std::filesystem::exists(path/"items.txt")) return path;
    throw std::runtime_error("Game data not found. Keep the data folder beside Mosslight.exe or pass --data <folder>.");
}
}
int main(int argc,char** argv) {
    bool smoke=false,mute=false; int frames=0; std::filesystem::path dataOverride,saveOverride,captures;
    try {
        for(int i=1;i<argc;++i) {
            std::string arg=argv[i];
            if(arg=="--smoke-test") smoke=true;
            else if(arg=="--mute") mute=true;
            else if((arg=="--data"||arg=="--save"||arg=="--captures")&&i+1<argc) {
                std::filesystem::path value=argv[++i]; if(arg=="--data") dataOverride=value; else if(arg=="--save") saveOverride=value; else captures=value;
            } else if(arg=="--help") {
                std::cout<<"Mosslight [--data folder] [--save file] [--mute] [--smoke-test --captures folder]\n"; return 0;
            } else throw std::runtime_error("Unknown or incomplete argument: "+arg);
        }
        auto exe=std::filesystem::absolute(argv[0]).parent_path();
        auto data=dataOverride.empty()?findData(exe):dataOverride;
        if(smoke) {
            if(captures.empty()) captures=exe/"captures";
            std::filesystem::create_directories(captures);
            if(saveOverride.empty()) saveOverride=captures/"smoke.sav";
        }
        auto save=saveOverride.empty()?moss::defaultSavePath(exe):saveOverride;
        moss::Game game(data,save);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_VSYNC_HINT|(smoke?FLAG_WINDOW_HIDDEN:0));
        InitWindow(moss::ViewW*3,moss::ViewH*3,"Mosslight: The Quiet Bell");
        if(!IsWindowReady()) throw std::runtime_error("Could not create a raylib window.");
        SetWindowMinSize(moss::ViewW,moss::ViewH); SetExitKey(KEY_NULL); SetTargetFPS(smoke?120:60);
        {
            moss::Renderer renderer; moss::Audio audio; if(mute) audio.toggleMute();
            while(!WindowShouldClose()&&!game.quitRequested) {
                float dt=smoke?1.0f/60:std::min(GetFrameTime(),.05f);
                moss::Input input=readInput();
                if(!smoke&&!IsWindowFocused()&&game.screen==moss::Screen::Playing&&!game.dialogue.active()) { game.screen=moss::Screen::Pause; game.menuSelection=0; }
                if(IsKeyPressed(KEY_F10)) audio.toggleMute();
                if(IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();
                if(smoke) {
                    input={};
                    if(frames==2) renderer.capture(captures/"01-title.png");
                    if(frames==3) game.newGame(731204);
                    if(frames==30) renderer.capture(captures/"02-village.png");
                    if(frames==31) { game.player.pos=moss::tileCenter(22,15); game.interact(); }
                    if(frames==33) renderer.capture(captures/"03-dialogue.png");
                    if(frames==34) { game.dialogue={}; game.toastTime=0; game.enterRegion(moss::RegionId::Forest,moss::tileCenter(23,17)); }
                    if(frames==36||frames==63||frames==88||frames==113) game.regionBanner=0;
                    if(frames==60) renderer.capture(captures/"04-forest.png");
                    if(frames==61) game.enterRegion(moss::RegionId::Cave,moss::tileCenter(29,24));
                    if(frames==85) renderer.capture(captures/"05-cave.png");
                    if(frames==86) game.enterRegion(moss::RegionId::Hollow,moss::tileCenter(38,25));
                    if(frames==110) renderer.capture(captures/"06-hollow.png");
                    if(frames==111) game.enterRegion(moss::RegionId::Shrine,moss::tileCenter(23,18));
                    if(frames==135) renderer.capture(captures/"07-shrine.png");
                    if(frames==136) {
                        game.quest.accepted=true; game.quest.shrineLit=true; game.inventory.add(moss::Item::Ember,1); game.inventory.add(moss::Item::Dew,1);
                        game.enterRegion(moss::RegionId::Arena,moss::tileCenter(23,21));
                    }
                    if(frames==185) renderer.capture(captures/"08-boss.png");
                    if(frames==186) game.screen=moss::Screen::Inventory;
                    if(frames==188) renderer.capture(captures/"09-inventory.png");
                    if(frames==189) game.screen=moss::Screen::Journal;
                    if(frames==191) renderer.capture(captures/"10-journal.png");
                    if(frames==192) { game.screen=moss::Screen::Death; game.deathTime=1; }
                    if(frames==194) renderer.capture(captures/"11-death.png");
                    if(frames==195) input.confirm=true;
                    if(frames==200) { game.save(false); if(!game.continueGame()) throw std::runtime_error("Smoke save/load failed"); }
                    if(frames==205) { renderer.capture(captures/"12-respawn.png"); break; }
                }
                game.update(input,dt); audio.update(game.cue,dt); renderer.draw(game,dt); ++frames;
            }
            game.close();
        }
        CloseWindow();
        if(smoke) std::cout<<"SMOKE PASS: title, all six regions, dialogue, inventory, journal, death, respawn, save/load; "<<frames<<" frames.\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Mosslight: "<<error.what()<<'\n';
        if(IsWindowReady()) CloseWindow();
        return 1;
    }
}
