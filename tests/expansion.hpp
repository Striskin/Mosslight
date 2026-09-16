#pragma once
// Included into the existing standalone test runner; no extra test dependency.
void testTactics(const std::filesystem::path& data) {
    auto map=loadRegion(data,RegionId::Shrine); auto defs=loadEnemies(data/"enemies.txt");
    Player p; p.pos=tileCenter(23,26); p.facing={1,0}; Inventory inv;
    Input attack; attack.attack=true; p.stamina=0; p.update(attack,map,1.0f/60,inv);
    require(p.attackSerial==0,"Exhaustion prevents attacks");
    p.stamina=100; p.update(attack,map,1.0f/60,inv);
    require(p.attackSerial==1&&p.stamina==87&&!p.attackActive(),"Sword has an actual windup and stamina cost");
    for(int i=0;i<10;++i) p.update({},map,1.0f/60,inv);
    require(p.attackActive(),"Sword enters its active window");
    for(int i=0;i<180;++i) p.update({},map,1.0f/60,inv);
    require(p.stamina>99,"Stamina regenerates during recovery");
    inv.weapon=Weapon::Fists; p.update(attack,map,1.0f/60,inv);
    require(p.attackWeapon==Weapon::Fists&&p.attackDuration<.3f,"Fists have their own faster attack");
    std::vector<Projectile> shots; std::vector<Particle> particles;
    Enemy knight; knight.type=3; knight.health=knight.maxHealth=9; knight.pos=p.pos+Vec{25,0}; knight.facing={-1,0}; knight.mode=EnemyMode::Guard;
    p.attackWeapon=Weapon::Sword; p.attackDuration=.34f; p.attackTime=.2f; p.attackSerial=12; p.heavyAttack=false;
    std::vector<Enemy> enemies{knight};
    resolveCombat(p,inv,enemies,shots,map,particles); require(enemies[0].health==9,"Knight shield stops frontal light attacks");
    p.attackSerial=13; p.heavyAttack=true; resolveCombat(p,inv,enemies,shots,map,particles);
    require(enemies[0].health==6&&enemies[0].stagger>0&&enemies[0].mode==EnemyMode::Recover,"Heavy strike breaks knight guard");
    enemies={knight}; enemies[0].facing={1,0}; p.heavyAttack=false; p.attackSerial=14;
    resolveCombat(p,inv,enemies,shots,map,particles); require(enemies[0].health==8,"Flanking bypasses a knight's shield");
    p=Player{}; p.pos=tileCenter(23,26); p.facing={1,0}; p.blocking=true; p.guardTime=.05f; p.invulnerable=0;
    knight.pos=p.pos+Vec{25,0}; knight.facing={-1,0}; knight.mode=EnemyMode::Dash; enemies={knight};
    auto parry=resolveCombat(p,inv,enemies,shots,map,particles);
    require(parry.guarded&&p.health==6&&enemies[0].stagger>1&&p.stamina==92,"Timed directional block staggers knight and saves health");
    p.invulnerable=0; p.guardTime=.5f; p.stamina=10; enemies={knight};
    resolveCombat(p,inv,enemies,shots,map,particles);
    require(p.stunned>0&&p.health==4&&!p.blocking&&p.stamina==0,"Exhausted guard breaks under a heavy blow");
    p=Player{}; p.pos=tileCenter(23,26); p.facing={-1,0}; p.blocking=true; enemies={knight};
    resolveCombat(p,inv,enemies,shots,map,particles); require(p.health==4,"Guard cannot protect the player's back");
    p=Player{}; p.pos=tileCenter(23,26); p.armored=true; p.hurt(2,{}); require(p.health==5,"Plate reduces heavy incoming damage");
    p=Player{}; p.pos=tileCenter(23,26); Enemy beetle; beetle.type=4; beetle.health=4; beetle.pos=p.pos+Vec{20,0}; enemies={beetle};
    shots={Projectile{p.pos+Vec{36,0},{245,0},1,2,2,true,p.pos}};
    resolveCombat(p,inv,enemies,shots,map,particles); require(enemies[0].health==2&&shots[0].life==0,"Swept arrows hit crossed targets and stop after one hit");
    enemies={knight}; enemies[0].mode=EnemyMode::Guard; shots={Projectile{p.pos+Vec{36,0},{245,0},1,2,2,true,p.pos}};
    resolveCombat(p,inv,enemies,shots,map,particles); require(enemies[0].health==9,"Knight shield stops frontal arrows");
    map.tiles[26][24]='#'; shots={Projectile{tileCenter(23,26),{245,0},1,2,2,true,tileCenter(23,26)}};
    updateEnemies(enemies,shots,map,p,defs,.1f); require(shots.empty(),"Stone blocks arrows");
    map=loadRegion(data,RegionId::Shrine); shots.clear(); p.facing={1,0}; p.attackWeapon=Weapon::Fists; p.attackDuration=.24f; p.attackTime=.13f; p.attackSerial=50;
    beetle.pos=p.pos+Vec{24,0}; enemies={beetle};
    resolveCombat(p,inv,enemies,shots,map,particles); require(enemies[0].health==4,"Fists cannot hit at sword range");
    enemies[0].pos=p.pos+Vec{18,0}; resolveCombat(p,inv,enemies,shots,map,particles);
    require(enemies[0].health==3,"Unarmed attacks deal real close-range damage");
    std::cout<<"PASS: stamina, attack windup, fists, heavy guard break, flanking, timed guard, armor and swept arrows\n";
}
void testKnightDuel(const std::filesystem::path& data,const std::filesystem::path& out) {
    Game g(data,out/"knight-duel.sav"); g.newGame(88); g.inventory.add(Item::Sword,1);
    g.enterRegion(RegionId::Bailey,tileCenter(20,18)); int frames=0; bool facedShield=false;
    for(;frames<1800&&!g.progress().defeated.count(0)&&g.screen!=Screen::Death;++frames) {
        auto knight=std::find_if(g.enemies.begin(),g.enemies.end(),[](const Enemy& e){return e.spawnId==0;});
        require(knight!=g.enemies.end(),"First bailey knight exists until defeated");
        facedShield|=knight->mode==EnemyMode::Guard;
        Input in; Vec direction=normalized(knight->pos-g.player.pos); in.aiming=true; in.aim=direction;
        if(length(knight->pos-g.player.pos)>28) in.move=direction;
        in.heavy=true; in.heal=g.player.health<=3;
        g.update(in,1.0f/60);
    }
    require(facedShield&&g.progress().defeated.count(0)&&!g.progress().drops.empty(),"Heavy-attack input wins an actual shield-knight encounter and leaves loot");
    std::cout<<"PASS: normal-input bailey knight duel in "<<frames/60.0f<<" simulated seconds\n";
}
void testEconomy(const std::filesystem::path& data,const std::filesystem::path& out) {
    auto catalog=loadItems(data/"items.txt"); auto shops=loadShops(data/"shops.txt");
    auto offers=shopTrades(shops,catalog,"smith"); require(offers.size()==6,"Smith has real buy and sell offers");
    Inventory inv; std::string message; auto before=inv.count;
    require(!trade(inv,offers[0],message)&&inv.count==before&&inv.coins==0,"Insufficient funds make no partial changes");
    inv.coins=24; require(trade(inv,offers[0],message)&&inv.get(Item::Bow)==1&&inv.coins==6,"Purchase exchanges coins for bow");
    require(!trade(inv,offers[0],message)&&inv.coins==6,"Duplicate equipment purchase cannot spend coins");
    inv.add(Item::Arrow,95); require(!trade(inv,offers[1],message)&&inv.coins==6&&inv.get(Item::Arrow)==95,"Full bundle must fit before charging coins");
    inv.add(Item::Shell,2); Trade sale{Item::Shell,1,6,true};
    require(trade(inv,sale,message)&&inv.get(Item::Shell)==1&&inv.coins==12,"Selling shell grants configured coin reward");
    inv.coins=99999; require(!trade(inv,sale,message)&&inv.get(Item::Shell)==1,"Full purse cannot destroy sold material");
    require(!trade(inv,{Item::Bell,1,100,true},message),"Quest objects cannot be sold");
    require(!trade(inv,{Item::Arrow,-1,-5,false},message),"Malformed trade arguments are rejected");
    Game game(data,out/"expansion-journey.sav"); game.newGame(791);
    interactWith(game,"smith_door"); require(game.map.id==RegionId::Smithy,"Village house door opens actual smith interior");
    interactWith(game,"smith"); require(game.screen==Screen::Shop,"Merchant opens trading screen");
    Input buy; buy.confirm=true; game.update(buy,1.0f/60); require(game.inventory.get(Item::Bow)==1&&game.inventory.coins==6,"Shop UI executes purchase");
    Input leave; leave.pause=true; game.update(leave,1.0f/60);
    game.player.pos=tileCenter(23,32); game.transitionCooldown=0; game.update({},1.0f/60);
    require(game.map.id==RegionId::Village&&!blocked(game.map,game.player.pos),"Shop exit returns to a safe village doorway");
    interactWith(game,"inn_door"); require(game.map.id==RegionId::Inn,"Second house opens an inn");
    interactWith(game,"inn_hearth"); require(game.checkpointRegion==RegionId::Inn,"Inn hearth is a real checkpoint");
    game.enterRegion(RegionId::Forest,tileCenter(8,17)); game.inventory.weapon=Weapon::Bow;
    int arrows=game.inventory.get(Item::Arrow); Input shoot; shoot.attack=true; shoot.aiming=true; shoot.aim={0,-1};
    for(int i=0;i<15;++i) game.update(shoot,1.0f/60);
    require(game.inventory.get(Item::Arrow)==arrows-1,"One arrow consumed once the bow releases");
    require(std::any_of(game.projectiles.begin(),game.projectiles.end(),[](const Projectile& p){return p.friendly;}),"Bow creates a real friendly projectile");
    game.inventory.count[int(Item::Arrow)]=0; game.player.attackCooldown=0; game.player.attackTime=0; unsigned serial=game.player.attackSerial;
    game.update(shoot,1.0f/60); require(game.player.attackSerial==serial,"Empty quiver cannot fire");
    std::cout<<"PASS: atomic trading, coin/stack limits, doors, both interiors, inn checkpoint and actual bow input\n";
}
void testLootPersistence(const std::filesystem::path& data,const std::filesystem::path& out) {
    Game g(data,out/"loot.sav"); g.newGame(57); g.enterRegion(RegionId::Forest,tileCenter(9,17));
    auto victim=std::find_if(g.enemies.begin(),g.enemies.end(),[](const Enemy& e){return e.type==4;});
    require(victim!=g.enemies.end(),"Forest contains lootable beetles"); int spawn=victim->spawnId;
    // Set up a nearly defeated beetle; its final blow and loot use the production path.
    victim->health=1; victim->pos=g.player.pos+Vec{20,0}; victim->mode=EnemyMode::Recover; victim->timer=3;
    g.player.facing={1,0}; g.player.attackTime=.2f; g.player.attackSerial=1;
    int coins=g.inventory.coins; g.update({},1.0f/60);
    require(g.progress().drops.size()==1&&g.inventory.coins==coins&&g.inventory.get(Item::Shell)==0,"Enemy leaves physical loot; no automatic coin or shell award");
    Vec corpse=g.progress().drops[0].pos; require(g.progress().defeated.count(spawn),"Defeated spawn recorded");
    require(g.save(false),"Save pending corpse"); Game reloaded(data,out/"loot.sav"); require(reloaded.continueGame(),"Load pending corpse");
    require(reloaded.progress().drops.size()==1&&std::none_of(reloaded.enemies.begin(),reloaded.enemies.end(),[&](const Enemy& e){return e.spawnId==spawn;}),"Reload preserves corpse without reviving its owner");
    reloaded.player.pos=corpse; reloaded.interact();
    require(reloaded.inventory.coins>coins&&reloaded.inventory.get(Item::Shell)==1&&reloaded.progress().drops.empty(),"E searches corpse and awards coins/material exactly once");
    int collected=reloaded.inventory.coins; reloaded.interact(); require(reloaded.inventory.coins==collected,"Searched corpse cannot duplicate coins");
    reloaded.enterRegion(RegionId::Village,tileCenter(20,17)); reloaded.enterRegion(RegionId::Forest,tileCenter(3,17));
    require(std::none_of(reloaded.enemies.begin(),reloaded.enemies.end(),[&](const Enemy& e){return e.spawnId==spawn;}),"Region travel does not duplicate defeated enemies");
    reloaded.enterRegion(RegionId::Village,tileCenter(20,17)); interactWith(reloaded,"village_lantern");
    reloaded.enterRegion(RegionId::Forest,tileCenter(3,17));
    require(std::any_of(reloaded.enemies.begin(),reloaded.enemies.end(),[&](const Enemy& e){return e.spawnId==spawn;}),"Rest respawns encounters while keeping earned coins");
    require(reloaded.inventory.coins==collected,"Rest preserves purse");
    SaveData s=reloaded.snapshot(); s.inventory.add(Item::KnightArmor,1); s.inventory.add(Item::KnightSword,1); s.inventory.add(Item::Bow,1);
    s.inventory.weapon=Weapon::Bow; s.inventory.coins=123; s.stamina=37.5f; s.region=RegionId::Smithy;
    std::string error; auto file=out/"equipment.sav"; require(writeSave(file,s,error),"Save expanded equipment"); SaveData loaded;
    require(readSave(file,loaded,error)&&loaded.inventory.weapon==Weapon::Bow&&loaded.inventory.coins==123&&loaded.inventory.maxHealth()==10&&loaded.inventory.damage()==3&&loaded.stamina==37.5f,"New items, selected weapon, coins and stamina roundtrip");
    s.inventory.coins=-1; require(!writeSave(file,s,error),"Negative coins rejected"); s.inventory.coins=0;
    s.world[0].drops.push_back({{0,0},-1,Item::Shell,1,4}); require(!writeSave(file,s,error),"Invalid loot rejected");
    std::cout<<"PASS: searchable beetle corpse, persistent ground loot, no duplication, rest reset and expanded save state\n";
}
void testMigrationAndVisitors(const std::filesystem::path& data,const std::filesystem::path& out) {
    // Exact version-1 wire format, including its original seven inventory slots.
    std::string body="0 328 280 6\n0 328 280\n731204 42\n3 4 1 0 1 1 0 \n1 1 0 0\n1\nforest_ember\n";
    uint64_t hash=14695981039346656037ull; for(unsigned char c:body) { hash^=c; hash*=1099511628211ull; }
    auto legacy=out/"legacy-v1.sav";
    { std::ofstream file(legacy,std::ios::binary|std::ios::trunc); file<<"MOSSLIGHT_SAVE 1 "<<hash<<'\n'<<body; }
    SaveData s; std::string error; require(readSave(legacy,s,error),"Original save migrates: "+error);
    require(s.quest.shrineLit&&s.inventory.get(Item::Sword)==1&&s.inventory.get(Item::Fragment)==4&&s.openedChests.count("forest_ember"),"Migration preserves original progress");
    require(s.inventory.coins==24&&s.inventory.get(Item::Arrow)==12,"Migration grants initial expansion purse/quiver");
    s.inventory.coins=1; require(writeSave(legacy,s,error)&&readSave(legacy,s,error)&&s.inventory.coins==1,"Migration gift cannot repeat on current-format loads");
    Region a=loadRegion(data,RegionId::Village),b=a,c=a;
    auto walkers=populateResidents(a,102),same=populateResidents(b,102),other=populateResidents(c,201);
    require(walkers.size()==3&&same.size()==3&&other.size()==3,"Village gains three seeded visitors");
    for(size_t i=0;i<walkers.size();++i) require(a.objects[walkers[i].objectIndex].label==b.objects[same[i].objectIndex].label,"Same seed gives same visitors");
    Vec initial=a.objects[walkers[0].objectIndex].pos; bool moved=false;
    for(int i=0;i<900;++i) {
        updateResidents(a,walkers,{0,0},1.0f/60); moved|=length(a.objects[walkers[0].objectIndex].pos-initial)>2;
        for(const auto& w:walkers) require(!blocked(a,a.objects[w.objectIndex].pos),"Wandering NPCs stay out of walls");
    }
    require(moved,"Ambient visitors actually wander");
    std::cout<<"PASS: version-1 migration, persistent migration gift, seeded visitors and collision-safe wandering\n";
}
