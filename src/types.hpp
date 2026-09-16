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
enum class RegionId { Village,Forest,Cave,Shrine,Arena,Hollow,Smithy,Inn,Bailey,Count };
constexpr int RegionCount=static_cast<int>(RegionId::Count);
inline const char* regionKey(RegionId r) {
    constexpr const char* names[]={"village","forest","cave","shrine","arena","hollow","smithy","inn","bailey"};
    return names[static_cast<int>(r)];
}
// Append identifiers to preserve existing save files and map records.
enum class Item { Tonic,Fragment,Sword,Coat,Ember,Dew,Bell,Bow,Arrow,KnightArmor,KnightSword,Shell,Count };
constexpr int ItemCount=static_cast<int>(Item::Count);
enum class Weapon { Fists,Sword,Bow };
enum class Armor { Travel,Coat,Plate };
inline const char* armorName(Armor a) { return a==Armor::Plate?"KNIGHT PLATE":a==Armor::Coat?"MOSS COAT":"TRAVEL CLOTHES"; }
inline const char* weaponName(Weapon w) { return w==Weapon::Fists?"FISTS":w==Weapon::Bow?"BOW":"SWORD"; }
struct Input {
    Vec move{};
    bool attack=false,dodge=false,interact=false,heal=false,inventory=false,journal=false,pause=false;
    bool confirm=false,up=false,down=false,save=false,sprint=false;
    bool heavy=false,block=false,aiming=false;
    bool lockTarget=false,nextTarget=false,help=false;
    Vec aim{};
    int equip=-1;
};
struct Quest { bool accepted=false,shrineLit=false,bossDefeated=false,rewardClaimed=false; };
struct Particle { Vec pos,vel; float life=0,maxLife=0; int color=0; };
struct Projectile { Vec pos,vel; float life=0; int damage=1; float radius=3; bool friendly=false; Vec previous{}; };
struct LootDrop { Vec pos; int coins=0; Item item=Item::Fragment; int amount=1; int enemyType=0; };
inline float segmentDistance(Vec p,Vec a,Vec b) {
    Vec d=b-a; float squared=dot(d,d);
    float t=squared>.0001f?std::clamp(dot(p-a,d)/squared,0.0f,1.0f):0;
    return length(p-(a+d*t));
}
}
