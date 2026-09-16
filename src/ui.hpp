#pragma once
#include "game.hpp"
#include "raylib.h"
namespace moss::ui {
inline constexpr Color Ink{20,33,39,255},Paper{239,230,203,255},Muted{151,177,161,255},Gold{232,188,112,255};
void text(const std::string& value,int x,int y,int size=10,Color color=Paper);
void center(const std::string& value,int y,int size=10,Color color=Paper);
void panel(int x,int y,int w,int h);
int wrap(const std::string& value,int x,int y,int width,int size=10,Color color=Paper,int spacing=4);
void heart(int x,int y,bool filled);
void drawHUD(const Game& game);
void drawOverlay(const Game& game);
}
