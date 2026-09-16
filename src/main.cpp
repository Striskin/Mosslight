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
    in.heavy=IsKeyDown(KEY_L)||IsKeyDown(KEY_X); in.block=IsKeyDown(KEY_K)||IsKeyDown(KEY_C);
    in.lockTarget=IsKeyPressed(KEY_F)||IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
    in.nextTarget=IsKeyPressed(KEY_R); in.help=IsKeyPressed(KEY_F1)||IsKeyPressed(KEY_H);
    if(IsKeyPressed(KEY_ONE)) in.equip=0;
    if(IsKeyPressed(KEY_TWO)) in.equip=1;
    if(IsKeyPressed(KEY_THREE)) in.equip=2;
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
                input.aiming=IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
                if(input.aiming) input.aim=renderer.screenToWorld(GetMousePosition())-game.player.pos;
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
                    if(frames==205) { renderer.capture(captures/"12-respawn.png"); game.enterRegion(moss::RegionId::Smithy,moss::tileCenter(23,17)); game.toastTime=0; game.regionBanner=0; }
                    if(frames==225) renderer.capture(captures/"13-smithy.png");
                    if(frames==226) { game.player.pos=moss::tileCenter(23,16); game.interact(); }
                    if(frames==228) renderer.capture(captures/"14-merchant.png");
                    if(frames==229) input.confirm=true;
                    if(frames==230) {
                        if(!game.inventory.get(moss::Item::Bow)) throw std::runtime_error("Smoke merchant purchase failed");
                        input.pause=true;
                    }
                    if(frames==231) { game.enterRegion(moss::RegionId::Inn,moss::tileCenter(23,15)); game.regionBanner=0; }
                    if(frames==250) renderer.capture(captures/"15-inn.png");
                    if(frames==251) {
                        game.inventory.add(moss::Item::KnightArmor,1); game.inventory.add(moss::Item::KnightSword,1); game.inventory.equip(moss::Weapon::Bow); game.player.health=10;
                        game.enterRegion(moss::RegionId::Bailey,moss::tileCenter(21,21)); game.regionBanner=0;
                    }
                    if(frames>=252&&frames<=267) { input.aiming=true; input.aim={0,-1}; input.attack=true; }
                    if(frames==266) renderer.capture(captures/"16-knight-bow.png");
                    if(frames==278) { game.inventory.equip(moss::Weapon::Sword); game.player.attackTime=0; input.block=true; }
                    if(frames==279) { renderer.capture(captures/"17-knight-guard.png"); game.screen=moss::Screen::Inventory; game.menuSelection=9; }
                    if(frames==281) renderer.capture(captures/"18-equipment.png");
                    if(frames==282) {
                        game.screen=moss::Screen::Playing; game.enterRegion(moss::RegionId::Forest,moss::tileCenter(11,17)); game.regionBanner=0;
                        game.progress().drops.push_back({game.player.pos+ moss::Vec{16,0},7,moss::Item::Shell,1,4});
                    }
                    if(frames==284) renderer.capture(captures/"19-loot.png");
                    if(frames==285) {
                        if(!game.save(false)||!game.continueGame()) throw std::runtime_error("Expansion save/load failed");
                        game.enterRegion(moss::RegionId::Bailey,moss::tileCenter(20,20)); game.regionBanner=0; game.toastTime=0;
                    }
                    if(frames==286) input.lockTarget=true;
                    if(frames==287&&!game.lockedEnemy()) throw std::runtime_error("Smoke lock-on failed");
                    if(frames>=287&&frames<=299) { input.move={1,0}; input.block=true; }
                    if(frames==300) renderer.capture(captures/"20-target-focus.png");
                    if(frames==301) input.help=true;
                    if(frames==303) renderer.capture(captures/"21-controls-guide.png");
                    if(frames==304) input.down=true;
                    if(frames==306) renderer.capture(captures/"22-combat-guide.png");
                    if(frames==307) input.pause=true;
                    if(frames==308) input.inventory=true;
                    if(frames==310) { game.inventory.add(moss::Item::Coat,1); game.menuSelection=int(moss::Item::Coat); }
                    if(frames==311) input.confirm=true;
                    if(frames==312) {
                        if(game.inventory.armor!=moss::Armor::Coat||game.player.health>8) throw std::runtime_error("Smoke armor selection failed");
                        game.toastTime=0;
                    }
                    if(frames==314) renderer.capture(captures/"23-light-loadout.png");
                    if(frames==315) input.pause=true;
                    if(frames==320) renderer.capture(captures/"24-coat-in-world.png");
                    if(frames==321) input.inventory=true;
                    if(frames==322) { game.menuSelection=int(moss::Item::KnightArmor); input.confirm=true; }
                    if(frames==324) {
                        if(!game.save(false)||!game.continueGame()||game.inventory.armor!=moss::Armor::Plate||game.player.health>8||game.lockedEnemy())
                            throw std::runtime_error("Tactics save/load failed");
                    }
                    if(frames==325) input.pause=true;
                    if(frames==326||frames==327) input.down=true;
                    if(frames==328) { renderer.capture(captures/"25-pause-legend.png"); input.confirm=true; }
                    if(frames==330) {
                        if(game.screen!=moss::Screen::Help||game.helpReturn!=moss::Screen::Pause||game.helpPage!=0)
                            throw std::runtime_error("Pause controls legend did not open");
                        renderer.capture(captures/"26-pause-controls.png");
                    }
                    if(frames==331) input.down=true;
                    if(frames==333) renderer.capture(captures/"27-pause-tactics.png");
                    if(frames==334) input.pause=true;
                    if(frames==335) {
                        if(game.screen!=moss::Screen::Pause||game.menuSelection!=2)
                            throw std::runtime_error("Controls legend did not return to pause");
                        input.pause=true;
                    }
                    if(frames==336) {
                        if(game.screen!=moss::Screen::Playing) throw std::runtime_error("Pause did not resume after reading controls");
                        break;
                    }
                }
                game.update(input,dt); audio.update(game.cue,dt); renderer.draw(game,dt); ++frames;
            }
            game.close();
        }
        CloseWindow();
        if(smoke) std::cout<<"SMOKE PASS: nine regions, shops, weapons/loot, target focus, controls/combat guide, armor selection, respawn and save/load; "<<frames<<" frames.\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Mosslight: "<<error.what()<<'\n';
        if(IsWindowReady()) CloseWindow();
        return 1;
    }
}
