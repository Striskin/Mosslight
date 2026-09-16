#include "dialogue.hpp"
namespace moss {
DialogueResult talkTo(const std::string& id,Quest& q,Inventory& inv) {
    if(id=="keeper") {
        if(!q.accepted) {
            q.accepted=true;
            return {{"Welcome home, wayfarer. The Quiet Bell has stopped singing, and the woods have forgotten how to rest.",
                "Bring an Ember Seed from the eastern forest and Moon Dew from Stillwater Cave to the ruined shrine. The old path will wake.",
                "The forest is east of town. Follow its northern path to the shrine. Rest at a lantern to mend your wounds and save your journey."},true};
        }
        if(q.bossDefeated&&!q.rewardClaimed) {
            q.rewardClaimed=true; inv.add(Item::Tonic,3);
            return {{"Listen. Even the leaves are humming again. You gave the guardian back its dream, and the valley its quiet.",
                "Take these three tonics for the roads ahead. This little chapter is complete, but every path is still yours to wander."},true};
        }
        if(q.bossDefeated) return {{"The Quiet Bell rings softly again. Stay a while. The kettle is on, and the valley is yours."},false};
        if(q.shrineLit) return {{"The shrine is awake. Beyond it waits the Hollow Warden. Watch its amber light, then step aside. Let it tire before you strike."},false};
        return {{"The Ember Seed rests in a forest cache. Moon Dew waits in Stillwater Cave, south of the forest. Bring both to the shrine's stone bowl."},false};
    }
    if(id=="weaver") {
        if(inv.get(Item::Coat)) return {{"That mosswoven coat suits you. Eight hearts should make the old roads a little kinder."},false};
        if(inv.take(Item::Fragment,4)) {
            inv.add(Item::Coat,1);
            return {{"Four glow fragments, just enough. Here is your mosswoven coat. It is equipped, and your maximum health is now eight. Rest at the lantern to fill it."},true};
        }
        return {{"I stitch travelling coats. Bring me four glow fragments and I will weave one for you. Woodland creatures shed them when their restless magic settles.",
            "A coat adds two hearts. There is also an old copper blade in the Wandering Hollow, on the forest's northwest path."},false};
    }
    return {{"The woods breathe easier when you take your time. There is no hurry here."},false};
}
std::string objective(const Quest& q,const Inventory& inv) {
    if(q.rewardClaimed) return "The valley is at peace. Keep wandering.";
    if(q.bossDefeated) return "Return to Keeper Aven in the village.";
    if(q.shrineLit) return "Face the Warden beyond the shrine.";
    if(!q.accepted) return "Speak with Keeper Aven by the bell.";
    if(!inv.get(Item::Ember)&&!inv.get(Item::Dew)) return "Find the Ember Seed and Moon Dew.";
    if(!inv.get(Item::Ember)) return "Find the Ember Seed in the forest.";
    if(!inv.get(Item::Dew)) return "Find Moon Dew in Stillwater Cave.";
    return "Bring both relics to the ruined shrine.";
}
}
