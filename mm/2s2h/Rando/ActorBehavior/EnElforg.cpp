#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "functions.h"
#include "overlays/actors/ovl_En_Elforg/z_en_elforg.h"

void EnElforg_SpawnSparkles(EnElforg* thisx, PlayState* play, s32 life);
}

void EnElforg_DrawCustom(Actor* thisx, PlayState* play) {
    EnElforg* enElforg = (EnElforg*)thisx;
    Matrix_Scale(20.0f, 20.0f, 20.0f, MTXMODE_APPLY);
    RandoCheck check = RC_UNKNOWN;

    if (STRAY_FAIRY_TYPE(thisx) == STRAY_FAIRY_TYPE_CLOCK_TOWN) {
        check = RC_CLOCK_TOWN_STRAY_FAIRY;
    } else if (STRAY_FAIRY_TYPE(&enElforg->actor) == STRAY_FAIRY_TYPE_COLLECTIBLE) {
        auto checkData = Rando::StaticData::GetCheckFromFlag(FLAG_CYCL_SCENE_COLLECTIBLE,
                                                             STRAY_FAIRY_GET_FLAG(&enElforg->actor), play->sceneId);
        check = checkData.check;
    } else if (STRAY_FAIRY_TYPE(&enElforg->actor) == STRAY_FAIRY_TYPE_FREE_FLOATING) {
        auto checkData = Rando::StaticData::GetCheckFromFlag(FLAG_CYCL_SCENE_SWITCH,
                                                             STRAY_FAIRY_GET_FLAG(&enElforg->actor), play->sceneId);
        check = checkData.check;
    }

    if (check == RC_UNKNOWN) {
        return;
    }

    auto checkSaveData = gSaveContext.save.shipSaveInfo.rando.checks[check];

    EnElforg_SpawnSparkles(enElforg, play, 16);
    thisx->shape.rot.y = thisx->shape.rot.y + 960;

    Rando::DrawItem(checkSaveData.item);
}

void Rando::ActorBehavior::InitEnElforgBehavior(bool isRando) {
    static uint32_t onActorInitHookId = 0;
    static uint32_t shouldHook1Id = 0;
    static uint32_t shouldHook2Id = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorInit>(onActorInitHookId);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(shouldHook1Id);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(shouldHook2Id);

    onActorInitHookId = 0;
    shouldHook1Id = 0;
    shouldHook2Id = 0;

    if (!isRando) {
        return;
    }

    onActorInitHookId =
        GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(ACTOR_EN_ELFORG, [](Actor* actor) {
            EnElforg* enElforg = (EnElforg*)actor;
            RandoCheck check = RC_UNKNOWN;

            if (STRAY_FAIRY_TYPE(actor) == STRAY_FAIRY_TYPE_CLOCK_TOWN) {
                check = RC_CLOCK_TOWN_STRAY_FAIRY;
            } else if (STRAY_FAIRY_TYPE(&enElforg->actor) == STRAY_FAIRY_TYPE_COLLECTIBLE) {
                auto checkData = Rando::StaticData::GetCheckFromFlag(
                    FLAG_CYCL_SCENE_COLLECTIBLE, STRAY_FAIRY_GET_FLAG(&enElforg->actor), gPlayState->sceneId);
                check = checkData.check;
            } else if (STRAY_FAIRY_TYPE(&enElforg->actor) == STRAY_FAIRY_TYPE_FREE_FLOATING) {
                auto checkData = Rando::StaticData::GetCheckFromFlag(
                    FLAG_CYCL_SCENE_SWITCH, STRAY_FAIRY_GET_FLAG(&enElforg->actor), gPlayState->sceneId);
                check = checkData.check;
            }

            if (check == RC_UNKNOWN) {
                return;
            }

            auto checkSaveData = gSaveContext.save.shipSaveInfo.rando.checks[check];

            // Set up custom draw function
            enElforg->actor.draw = EnElforg_DrawCustom;

            // Override area, as it's used in the sparkle effect
            switch (checkSaveData.item) {
                case RI_WOODFALL_STRAY_FAIRY:
                    enElforg->area = STRAY_FAIRY_AREA_WOODFALL;
                    break;
                default:
                    enElforg->area = STRAY_FAIRY_AREA_CLOCK_TOWN;
                    break;
            }
        });

    shouldHook1Id = REGISTER_VB_SHOULD(GI_VB_GIVE_ITEM_FROM_ELFORG, {
        *should = false;

        if (STRAY_FAIRY_TYPE((Actor*)opt) == STRAY_FAIRY_TYPE_CLOCK_TOWN) {
            Flags_SetRandoInf(RANDO_INF_CLOCK_TOWN_STRAY_FAIRY_COLLECTED);
        }
    });

    shouldHook2Id = REGISTER_VB_SHOULD(GI_VB_KILL_CLOCK_TOWN_STRAY_FAIRY,
                                       { *should = Flags_GetRandoInf(RANDO_INF_CLOCK_TOWN_STRAY_FAIRY_COLLECTED); });
}
