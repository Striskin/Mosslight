#pragma once
#include "game.hpp"
#include "raylib.h"
namespace moss {
class Audio {
public:
    Audio();
    ~Audio();
    void update(Cue cue,float dt);
    void toggleMute();
private:
    std::array<Sound,9> sounds{};
    bool ready=false,muted=false;
    float ambience=1;
};
}
