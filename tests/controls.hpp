#pragma once
void testFocus(const std::filesystem::path& data,const std::filesystem::path& out) {
    Region map=loadRegion(data,RegionId::Shrine); Vec pos=tileCenter(23,26);
    Enemy a,b,dead; a.spawnId=7; a.pos=pos+Vec{40,0}; b.spawnId=12; b.pos=pos+Vec{64,0};
    dead.spawnId=3; dead.pos=pos+Vec{5,0}; dead.health=0;
    std::vector<Enemy> foes{dead,b,a}; Targeting focus;
    Input in; in.lockTarget=true; focus.update(in,foes,map,pos);
    require(focus.enemy(foes,map,pos)&&focus.enemy(foes,map,pos)->spawnId==7&&in.aiming,"Focus chooses nearest living foe and supplies aim");
    foes.erase(foes.begin()); in={}; focus.update(in,foes,map,pos);
    require(focus.enemy(foes,map,pos)->spawnId==7,"Focus survives enemy vector relocation by spawn id");
    in={}; in.nextTarget=true; focus.update(in,foes,map,pos);
    require(focus.enemy(foes,map,pos)->spawnId==12,"Target cycling selects another foe");
    in={}; in.nextTarget=true; focus.update(in,foes,map,pos);
    require(focus.enemy(foes,map,pos)->spawnId==7,"Target cycling wraps predictably");
    in={}; in.lockTarget=true; focus.update(in,foes,map,pos); require(!focus.enemy(foes,map,pos),"F releases active focus");
    in={}; in.nextTarget=true; focus.update(in,foes,map,pos); require(focus.enemy(foes,map,pos),"Cycle can acquire initial focus");
    in={}; in.aiming=true; in.aim={0,-1}; focus.update(in,foes,map,pos);
    require(!focus.enemy(foes,map,pos)&&in.aim.y==-1,"Manual mouse aim overrides and releases focus");
    map.tiles[26][24]='#'; in={}; in.lockTarget=true; focus.update(in,foes,map,pos);
    require(!focus.enemy(foes,map,pos),"Cannot focus through walls");
    map.tiles[26][24]='.'; in={}; in.lockTarget=true; focus.update(in,foes,map,pos);
    map.tiles[26][24]='#'; in={}; focus.update(in,foes,map,pos); map.tiles[26][24]='.';
    require(!focus.enemy(foes,map,pos),"A wall breaks focus without silently reacquiring it");
    foes={a}; in={}; in.lockTarget=true; focus.update(in,foes,map,pos);
    foes[0].pos=pos+Vec{-145,0}; in={}; focus.update(in,foes,map,pos);
    require(focus.enemy(foes,map,pos),"Small leash margin prevents focus flickering near acquisition range");
    foes[0].pos=pos+Vec{-158,0}; in={}; focus.update(in,foes,map,pos);
    require(!focus.enemy(foes,map,pos),"Distant targets release focus");
    foes={a}; in={}; in.lockTarget=true; focus.update(in,foes,map,pos);
    foes[0].health=0; in={}; focus.update(in,foes,map,pos); require(!focus.enemy(foes,map,pos),"Dead targets release focus");
    foes.clear(); in={}; in.nextTarget=true; focus.update(in,foes,map,pos); require(!focus.enemy(foes,map,pos),"Empty encounter cycling is safe");

    Game g(data,out/"focus.sav"); g.newGame(88); g.inventory.add(Item::Sword,1);
    g.enterRegion(RegionId::Bailey,tileCenter(20,18)); in={}; in.lockTarget=true; g.update(in,1.0f/60);
    require(g.lockedEnemy()&&g.lockedEnemy()->spawnId==0,"Gameplay F input acquires a real bailey knight");
    Vec before=g.player.pos; in={}; in.move={0,1}; in.block=true; g.update(in,1.0f/60);
    require(g.player.pos.y>before.y&&g.player.blocking&&g.player.facing.x>.99f,"Locked guard allows strafing without turning away");
    int frames=0;
    for(;frames<600&&g.lockedEnemy()&&g.screen==Screen::Playing;++frames) {
        in={}; Vec delta=g.lockedEnemy()->pos-g.player.pos;
        if(length(delta)>28) in.move=normalized(delta);
        in.heavy=true; g.update(in,1.0f/60);
    }
    require(g.progress().defeated.count(0)&&!g.lockedEnemy()&&g.player.health>0,"Locked heavy attacks defeat actual knight and release on death");
    g.enterRegion(RegionId::Bailey,tileCenter(32,18)); in={}; in.lockTarget=true; g.update(in,1.0f/60);
    require(g.lockedEnemy(),"Can acquire surviving knight"); g.enterRegion(RegionId::Village,tileCenter(20,17));
    require(!g.lockedEnemy(),"Region changes clear transient focus");

    // A stationary unshielded target isolates bow direction from enemy movement.
    g.enterRegion(RegionId::Shrine,pos); a.type=4; a.health=a.maxHealth=4; a.mode=EnemyMode::Recover; a.timer=10;
    g.enemies={a}; g.inventory.add(Item::Bow,1); g.inventory.equip(Weapon::Bow); int arrows=g.inventory.get(Item::Arrow);
    // The preceding duel legitimately leaves a sword cooldown. Let it recover.
    for(int i=0;i<60;++i) g.update({},1.0f/60);
    in={}; in.lockTarget=true; in.attack=true; g.update(in,1.0f/60);
    for(int i=0;i<24;++i) g.update({},1.0f/60);
    require(g.enemies[0].health==2&&g.inventory.get(Item::Arrow)==arrows-1,"Lock-on bow fires in target direction and deals real projectile damage");
    std::cout<<"PASS: focus acquisition/cycle, occlusion/leash/death, strafing guard, locked knight duel and aimed bow hit\n";
}
void testArmorChoice(const std::filesystem::path& data,const std::filesystem::path& out) {
    Inventory inv; require(!inv.wear(Armor::Plate)&&inv.maxHealth()==6,"Cannot wear unowned armor");
    inv.add(Item::KnightArmor,1); inv.add(Item::Fragment,4); Quest quest;
    auto gift=talkTo("weaver",quest,inv);
    require(gift.changed&&inv.get(Item::Coat)&&inv.get(Item::Fragment)==0&&inv.armor==Armor::Plate,"Weaver still offers light armor to a plate owner without replacing chosen plate");
    require(inv.wear(Armor::Coat)&&inv.maxHealth()==8,"Owned coat can replace plate");
    auto map=loadRegion(data,RegionId::Shrine); Player light,heavy; light.pos=heavy.pos=tileCenter(23,26);
    Input move; move.move={1,0}; light.update(move,map,.05f,inv); inv.wear(Armor::Plate); heavy.update(move,map,.05f,inv);
    require(light.pos.x>heavy.pos.x&&!light.armored&&heavy.armored,"Armor selection changes actual movement and protection");
    light.hurt(2,{}); heavy.hurt(2,{}); require(light.health==4&&heavy.health==5,"Only worn plate reduces heavy hits");
    light=Player{}; heavy=Player{}; light.pos=heavy.pos=tileCenter(23,26); Input dodge; dodge.dodge=true;
    inv.wear(Armor::Coat); light.update(dodge,map,1.0f/60,inv); inv.wear(Armor::Plate); heavy.update(dodge,map,1.0f/60,inv);
    require(light.stamina==78&&heavy.stamina==72,"Selected armor changes dodge stamina cost");

    Game g(data,out/"armor-choice.sav"); g.newGame(212); g.inventory=inv; g.player.health=10;
    Input in; in.inventory=true; g.update(in,1.0f/60); g.menuSelection=int(Item::Coat);
    in={}; in.confirm=true; g.update(in,1.0f/60);
    require(g.inventory.armor==Armor::Coat&&g.player.health==8&&!g.player.armored,"Satchel Enter equips lighter armor and clamps hearts");
    g.menuSelection=int(Item::KnightArmor); g.update(in,1.0f/60);
    require(g.inventory.armor==Armor::Plate&&g.player.health==8,"Putting plate back on never restores lost hearts");
    g.update(in,1.0f/60); require(g.inventory.armor==Armor::Travel&&g.player.health==6,"Enter on worn armor returns to travel clothes");
    g.player.attackTime=.2f; g.update(in,1.0f/60); require(g.inventory.armor==Armor::Travel,"Gear changes cannot cancel a committed attack");
    g.player.attackTime=0; g.menuSelection=int(Item::Coat); g.update(in,1.0f/60);
    Game loaded(data,out/"armor-choice.sav"); require(loaded.continueGame(),"Selected armor autosave loads");
    require(loaded.inventory.armor==Armor::Coat&&loaded.inventory.get(Item::KnightArmor)&&loaded.player.health==6,"Save preserves lighter selection despite owning plate");
    loaded.respawn(); require(loaded.inventory.armor==Armor::Coat&&loaded.player.health==8,"Checkpoint respawn respects selected armor capacity");
    std::cout<<"PASS: armor selection, weaver trade, movement/protection/dodge, no healing exploit, autosave and respawn\n";
}
void testGuideAndMigration(const std::filesystem::path& data,const std::filesystem::path& out) {
    Game g(data,out/"guide.sav"); Input help; help.help=true; g.update(help,.05f);
    require(g.screen==Screen::Help&&g.helpReturn==Screen::Title,"Guide opens from title without starting a game");
    Input down; down.down=true; g.update(down,.05f); require(g.helpPage==1,"Guide has working combat page");
    g.update(help,.05f); require(g.screen==Screen::Title,"Guide returns to original title screen");
    g.newGame(212); interactWith(g,"keeper"); auto page=g.dialogue.page; Vec position=g.player.pos;
    g.update(help,.05f); Input move; move.move={1,1}; move.attack=true; g.update(move,.05f);
    require(g.screen==Screen::Help&&length(g.player.pos-position)==0&&g.dialogue.page==page,"Guide freezes simulation and preserves interrupted dialogue");
    g.update(help,.05f); require(g.screen==Screen::Playing&&g.dialogue.active(),"Guide returns to unfinished dialogue");
    g.dialogue={}; Input pause; pause.pause=true; g.update(pause,.05f); g.menuSelection=2;
    Input confirm; confirm.confirm=true; g.update(confirm,.05f); require(g.screen==Screen::Help,"Pause menu opens guide");
    g.update(pause,.05f); require(g.screen==Screen::Pause&&g.menuSelection==2,"Guide back preserves pause selection");

    auto fixture=[&](int version,const std::string& body,const std::filesystem::path& path) {
        uint64_t hash=14695981039346656037ull; for(unsigned char c:body) { hash^=c; hash*=1099511628211ull; }
        std::ofstream file(path,std::ios::binary|std::ios::trunc); file<<"MOSSLIGHT_SAVE "<<version<<' '<<hash<<'\n'<<body;
    };
    std::string prefix="6 376 280 10\n0 328 280\n999 67\n3 4 1 1 0 0 0 1 12 1 1 2\n123 2 37.5";
    std::string suffix="\n0 0 0 0\n1\nbailey_plate\n9\n";
    for(int i=0;i<9;++i) suffix+=(i==1?"1 0\n1\n184 280 7 11 1 4\n":"0\n0\n");
    auto path=out/"legacy-v2.sav"; fixture(2,prefix+suffix,path); SaveData saved; std::string error;
    require(readSave(path,saved,error),"Version-2 save migrates: "+error);
    require(saved.inventory.armor==Armor::Plate&&saved.health==10&&saved.inventory.coins==123&&saved.inventory.weapon==Weapon::Bow&&saved.stamina==37.5f,"Migration retains previous armor, purse, weapon and stamina");
    require(saved.world[1].defeated.count(0)&&saved.world[1].drops.size()==1&&saved.openedChests.count("bailey_plate"),"Migration preserves corpses and opened chests");
    saved.inventory.wear(Armor::Coat); saved.health=8;
    require(writeSave(path,saved,error)&&readSave(path,saved,error)&&saved.inventory.armor==Armor::Coat,"Migrated save persists explicit lighter armor in v3");
    fixture(3,prefix+" 8"+suffix,path); require(!readSave(path,saved,error)&&saved.inventory.armor==Armor::Coat,"Checksummed invalid armor is rejected without changing output");
    saved.inventory.count[int(Item::Coat)]=0; require(!writeSave(path,saved,error),"Cannot save unowned armor selection");
    fixture(1,"0 328 280 8\n0 328 280\n123 12\n3 0 0 1 0 0 0\n0 0 0 0\n0\n",path);
    require(readSave(path,saved,error)&&saved.inventory.armor==Armor::Coat&&saved.health==8,"Original v1 coat saves keep their armor and full health");
    std::cout<<"PASS: title/pause/dialogue guide navigation, version-1/2 armor migration, v3 roundtrip and invalid selection rejection\n";
}
