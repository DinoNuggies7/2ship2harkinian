#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "spdlog/spdlog.h"

extern "C" {
#include "variables.h"
#include "z64snap.h"
s32 Snap_RecordPictographedActors(PlayState* play);
}

void RegisterBetterPictoMessage() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnOpenText>(
        0xF8, [](u16* textId, bool* loadFromMessageTable) {
            if (!CVarGetInteger("gEnhancements.Equipment.BetterPictoMessage", 0)) {
                return;
            }

            if (!CHECK_QUEST_ITEM(QUEST_PICTOGRAPH)) {
                Snap_RecordPictographedActors(gPlayState);
            }

            std::string target = "";

            std::vector<std::string> actorsInPicture;
            if (Snap_CheckFlag(PICTO_VALID_IN_SWAMP))
                target = "the Swamp";
            if (Snap_CheckFlag(PICTO_VALID_MONKEY))
                target = "a Monkey";
            if (Snap_CheckFlag(PICTO_VALID_BIG_OCTO))
                target = "a Big Octo";
            if (Snap_CheckFlag(PICTO_VALID_LULU_HEAD) && Snap_CheckFlag(PICTO_VALID_LULU_RIGHT_ARM) &&
                Snap_CheckFlag(PICTO_VALID_LULU_LEFT_ARM)) {
                target = "Lulu";
            }
            if (Snap_CheckFlag(PICTO_VALID_SCARECROW))
                target = "a Scarecrow";
            if (Snap_CheckFlag(PICTO_VALID_TINGLE))
                target = "Tingle";
            if (Snap_CheckFlag(PICTO_VALID_PIRATE_GOOD))
                target = "a Pirate";
            if (Snap_CheckFlag(PICTO_VALID_DEKU_KING))
                target = "the Deku King";

            if (target == "") {
                return;
            }

            auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
            entry.autoFormat = false;

            std::string msg = "picture of " + target;
            CustomMessage::Replace(&entry.msg, "picture", msg);

            CustomMessage::LoadCustomMessageIntoFont(entry);
            *loadFromMessageTable = false;
        });
}
