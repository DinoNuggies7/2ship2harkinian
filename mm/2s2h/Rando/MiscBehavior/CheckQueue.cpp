#include "MiscBehavior.h"
#include <libultraship/libultraship.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/CustomItem/CustomItem.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/BenGui/Notification.h"

extern "C" {
#include "variables.h"
extern TexturePtr gItemIcons[131];
extern s16 D_801CFF94[250];
}

static bool queued = false;

// This function handles queuing up item gives that the player has been marked as eligible for. If you are looking for
// the behavior of the actual giving itself, the heavy lifting is done by the GameInteractor queue. This function is
// currently called every frame, and loops through the entire list of checks, this works for now but as the check list
// grows we should keep an eye on performance.
//
// Though it seems counter-intuitive, we currently only allow one thing from randommizer to be queued at a time,
// primarily because of the way an item can be converted may change as you queue up multiple items. This is actually
// fine for both the giving/drawing, as we can call ConvertItem() inside the Give/Draw lambda, but the message we
// pass to the queue is static and would need to be updated if we allowed multiple items to be queued at once.
void Rando::MiscBehavior::CheckQueue() {
    if (queued) {
        return;
    }

    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        auto randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];

        if (randoSaveCheck.eligible && !randoSaveCheck.obtained) {
            queued = true;

            GameInteractor::Instance->events.emplace_back(GIEventGiveItem{
                .showGetItemCutscene = !CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0),
                .param = (int16_t)randoCheckId,
                .giveItem =
                    [](Actor* actor, PlayState* play) {
                        auto& randoSaveCheck = RANDO_SAVE_CHECKS[CUSTOM_ITEM_PARAM];
                        RandoItemId randoItemId = Rando::ConvertItem(randoSaveCheck.randoItemId);

                        CustomMessage::Entry entry = {
                            .msg = "You received {{item}}!",
                            .textboxType = 2,
                        };
                        CustomMessage::Replace(&entry.msg, "{{item}}", Rando::StaticData::Items[randoItemId].name);
                        if (Rando::StaticData::Items[randoItemId].getItemId != GI_NONE) {
                            entry.icon = (u8)Rando::StaticData::Items[randoItemId].getItemId;
                        }

                        if (CUSTOM_ITEM_FLAGS & CustomItem::GIVE_ITEM_CUTSCENE) {
                            CustomMessage::SetActiveCustomMessage(entry.msg, entry);
                        } else if (!CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0)) {
                            CustomMessage::StartTextbox(entry.msg + "\x1C\x02\x10", entry);
                        } else {
                            s16 itemId = Rando::StaticData::Items[randoItemId].itemId;
                            if (itemId >= ITEM_RECOVERY_HEART) {
                                itemId = D_801CFF94[Rando::StaticData::Items[randoItemId].getItemId];
                            }

                            Notification::Emit({
                                .itemIcon = itemId < ITEM_RECOVERY_HEART ? (const char*)gItemIcons[itemId] : nullptr,
                                .message = "You found",
                                .suffix = Rando::StaticData::Items[randoItemId].name,
                            });
                        }
                        Rando::GiveItem(randoItemId);
                        randoSaveCheck.obtained = true;
                        queued = false;
                        CUSTOM_ITEM_PARAM = randoItemId;
                    },
                .drawItem =
                    [](Actor* actor, PlayState* play) {
                        RandoItemId randoItemId;

                        // If the item has been given, the CUSTOM_ITEM_PARAM is set to the RI, prior to that it's the RC
                        if (CUSTOM_ITEM_FLAGS & CustomItem::CALLED_ACTION) {
                            randoItemId = (RandoItemId)CUSTOM_ITEM_PARAM;
                        } else {
                            auto& randoSaveCheck = RANDO_SAVE_CHECKS[CUSTOM_ITEM_PARAM];
                            randoItemId = Rando::ConvertItem(randoSaveCheck.randoItemId);
                        }

                        Matrix_Scale(30.0f, 30.0f, 30.0f, MTXMODE_APPLY);
                        Rando::DrawItem(randoItemId);
                    } });
            return;
        }
    }
}

void Rando::MiscBehavior::CheckQueueReset() {
    queued = false;
    GameInteractor::Instance->currentEvent = GIEventNone{};
    GameInteractor::Instance->events.clear();
}
