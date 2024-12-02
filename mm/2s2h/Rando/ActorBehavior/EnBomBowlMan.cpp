#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"

#include "overlays/actors/ovl_En_Bom_Bowl_Man/z_en_bom_bowl_man.h"
void Player_TalkWithPlayer(PlayState* play, Actor* actor);
}

// TODO: Currently this only accounts for the giving of the notebook in East clock town,
// we need to also handle or just prevent the notebook from being given in north clock town.
void Rando::ActorBehavior::InitEnBomBowlManBehavior() {
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* item = va_arg(args, GetItemId*);
        Actor* actor = va_arg(args, Actor*);
        Player* player = GET_PLAYER(gPlayState);

        if (actor->id != ACTOR_EN_BOM_BOWL_MAN) {
            return;
        }

        EnBomBowlMan* enBomBowlMan = (EnBomBowlMan*)actor;

        if (gPlayState->sceneId == SCENE_BACKTOWN) {
            // Giving bombers notebook after finding the bombers as human is not currently
            // handled, I kept running into issues with hanging dialogue, for now ignore
            return;
        }

        *should = false;
        actor->parent = &player->actor;
        player->talkActor = actor;
        player->talkActorDistance = actor->xzDistToPlayer;
        player->exchangeItemAction = PLAYER_IA_MINUS1;
        // TODO: It seems like when you are at a far enough distance (actor cyl + 40?) this call fails,
        // and the conversation just ends.
        Player_TalkWithPlayer(gPlayState, actor);
        actor->textId = 0x735;
        SET_WEEKEVENTREG(WEEKEVENTREG_84_80);
    });

    // Override the original requirement, which is the absence of the Bombers' Notebook
    COND_VB_SHOULD(VB_BE_ELIGBLE_FOR_BOMBERS_NOTEBOOK, IS_RANDO, {
        *should = CHECK_WEEKEVENTREG(WEEKEVENTREG_73_80) && !RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_BOMBERS_NOTEBOOK].obtained;
    });
};
