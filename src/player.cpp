#include "player.hpp"
namespace moss {
bool Player::spendStamina(float amount) {
    if(stamina<amount) return false;
    stamina-=amount; regenDelay=.45f; return true;
}
void Player::update(const Input& in,const Region& map,float dt,const Inventory& inventory) {
    shoot=false; armored=inventory.armor==Armor::Plate;
    float before=attackTime;
    invulnerable=std::max(0.0f,invulnerable-dt); attackTime=std::max(0.0f,attackTime-dt);
    attackCooldown=std::max(0.0f,attackCooldown-dt); dashCooldown=std::max(0.0f,dashCooldown-dt);
    dashTime=std::max(0.0f,dashTime-dt);
    stunned=std::max(0.0f,stunned-dt); regenDelay=std::max(0.0f,regenDelay-dt);
    if(attackWeapon==Weapon::Bow&&before>attackDuration*.68f&&attackTime<=attackDuration*.68f) shoot=true;
    Vec direction=normalized(in.move); moving=length(direction)>0;
    if(dashTime<=0&&attackTime<=0&&stunned<=0) {
        if(in.aiming&&length(in.aim)>.01f) facing=normalized(in.aim);
        else if(moving&&!in.block) facing=direction;
    }
    bool wasBlocking=blocking;
    blocking=in.block&&inventory.weapon!=Weapon::Bow&&attackTime<=0&&dashTime<=0&&stunned<=0&&stamina>0;
    guardTime=blocking?(wasBlocking?guardTime+dt:0):0;
    if(in.dodge&&dashCooldown<=0&&attackTime<=0&&stunned<=0&&spendStamina(armored?28:22)) {
        dashDirection=moving?direction:facing; dashTime=.18f; dashCooldown=.65f; invulnerable=.24f; blocking=false;
    }
    if((in.attack||in.heavy)&&!blocking&&attackCooldown<=0&&dashTime<=0&&stunned<=0) {
        bool bow=inventory.weapon==Weapon::Bow;
        float cost=bow?12.0f:(in.heavy?28.0f:(inventory.weapon==Weapon::Fists?7.0f:13.0f));
        if((!bow||inventory.get(Item::Arrow)>0)&&spendStamina(cost)) {
            attackWeapon=inventory.weapon; heavyAttack=in.heavy&&!bow;
            attackDuration=bow?.52f:(heavyAttack?.68f:(attackWeapon==Weapon::Fists?.24f:.34f));
            attackTime=attackDuration; attackCooldown=attackDuration+.10f; ++attackSerial;
        }
    }
    bool running=in.sprint&&moving&&!blocking&&attackTime<=0&&stamina>5;
    if(running) { stamina=std::max(0.0f,stamina-9*dt); regenDelay=.25f; }
    if(regenDelay<=0&&dashTime<=0&&attackTime<=0) stamina=std::min(100.0f,stamina+dt*(blocking?9.0f:29.0f));
    float speed=(running?90.0f:68.0f)*(armored?.91f:1.0f)*(blocking?.43f:1.0f)*(attackTime>0?.34f:1.0f);
    if(stunned>0) speed=0;
    Vec velocity=dashTime>0?dashDirection*205.0f:direction*speed;
    moveBody(map,pos,(velocity+knockback)*dt);
    knockback=knockback*std::max(0.0f,1-11*dt);
    if(moving) walkTime+=dt;
}
bool Player::hurt(int damage,Vec force) {
    if(invulnerable>0||health<=0) return false;
    if(armored&&damage>1) --damage;
    health=std::max(0,health-damage); invulnerable=.85f; knockback=force;
    return true;
}
bool Player::heal(Inventory& inv) {
    if(health>=inv.maxHealth()||!inv.take(Item::Tonic)) return false;
    health=std::min(inv.maxHealth(),health+3); return true;
}
}
