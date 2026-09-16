#include "player.hpp"
namespace moss {
void Player::update(const Input& in,const Region& map,float dt) {
    invulnerable=std::max(0.0f,invulnerable-dt); attackTime=std::max(0.0f,attackTime-dt);
    attackCooldown=std::max(0.0f,attackCooldown-dt); dashCooldown=std::max(0.0f,dashCooldown-dt);
    dashTime=std::max(0.0f,dashTime-dt);
    Vec direction=normalized(in.move); moving=length(direction)>0;
    if(moving&&dashTime<=0&&attackTime<=0) facing=direction;
    if(in.dodge&&dashCooldown<=0&&attackTime<=0) {
        dashDirection=moving?direction:facing; dashTime=.18f; dashCooldown=.7f; invulnerable=.24f;
    }
    if(in.attack&&attackCooldown<=0&&dashTime<=0) { attackTime=.22f; attackCooldown=.36f; ++attackSerial; }
    float speed=(in.sprint?90.0f:68.0f)*(attackTime>0?.48f:1.0f);
    Vec velocity=dashTime>0?dashDirection*205.0f:direction*speed;
    moveBody(map,pos,(velocity+knockback)*dt);
    knockback=knockback*std::max(0.0f,1-11*dt);
    if(moving) walkTime+=dt;
}
bool Player::hurt(int damage,Vec force) {
    if(invulnerable>0||health<=0) return false;
    health=std::max(0,health-damage); invulnerable=.85f; knockback=force;
    return true;
}
bool Player::heal(Inventory& inv) {
    if(health>=inv.maxHealth()||!inv.take(Item::Tonic)) return false;
    health=std::min(inv.maxHealth(),health+3); return true;
}
}
