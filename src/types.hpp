#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace moss {
constexpr int Tile = 16, ViewW = 480, ViewH = 270;
constexpr float Pi = 3.14159265359f;
struct Vec {
    float x = 0, y = 0;
    Vec operator+(Vec b) const { return {x+b.x,y+b.y}; }
    Vec operator-(Vec b) const { return {x-b.x,y-b.y}; }
    Vec operator*(float s) const { return {x*s,y*s}; }
    Vec& operator+=(Vec b) { x+=b.x; y+=b.y; return *this; }
};
inline float length(Vec v) { return std::sqrt(v.x*v.x+v.y*v.y); }
inline Vec normalized(Vec v) { float n=length(v); return n>0.001f?v*(1/n):Vec{}; }
inline float dot(Vec a, Vec b) { return a.x*b.x+a.y*b.y; }
inline Vec tileCenter(int x,int y) { return {x*16.0f+8,y*16.0f+8}; }
struct Rect { float x,y,w,h; };
inline bool contains(Rect r,Vec p) { return p.x>=r.x&&p.y>=r.y&&p.x<r.x+r.w&&p.y<r.y+r.h; }
inline bool overlaps(Rect a,Rect b) { return a.x<b.x+b.w&&a.x+a.w>b.x&&a.y<b.y+b.h&&a.y+a.h>b.y; }
enum class RegionId { Village,Forest,Cave,Shrine,Arena,Hollow,Count };
inline const char* regionKey(RegionId r) {
    constexpr const char* names[]={"village","forest","cave","shrine","arena","hollow"};
    return names[static_cast<int>(r)];
}
enum class Item { Tonic,Fragment,Sword,Coat,Ember,Dew,Bell,Count };
constexpr int ItemCount=static_cast<int>(Item::Count);
struct Input {
    Vec move{};
    bool attack=false,dodge=false,interact=false,heal=false,inventory=false,journal=false,pause=false;
    bool confirm=false,up=false,down=false,save=false,sprint=false;
};
struct Quest { bool accepted=false,shrineLit=false,bossDefeated=false,rewardClaimed=false; };
struct Particle { Vec pos,vel; float life=0,maxLife=0; int color=0; };
struct Projectile { Vec pos,vel; float life=0; int damage=1; float radius=3; };
}
