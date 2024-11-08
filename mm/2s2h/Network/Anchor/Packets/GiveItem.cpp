#ifdef ENABLE_NETWORKING

#include "2s2h/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/BenGui/Notification.h"
#include "2s2h/BenPort.h"

extern "C" {
#include "functions.h"
extern PlayState* gPlayState;
}

/**
 * GIVE_ITEM
 */

static bool gettingItem;

void Anchor::SendPacket_GiveItem(u16 modId, s16 getItemId) {
    if (!IsSaveLoaded() || gettingItem) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = GIVE_ITEM;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["addToQueue"] = true;
    payload["modId"] = modId;
    payload["getItemId"] = getItemId;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_GiveItem(nlohmann::json payload) {
    if (!IsSaveLoaded()) {
        return;
    }

    uint32_t clientId = payload["clientId"].get<uint32_t>();
    AnchorClient& client = clients[clientId];

    // GetItemEntry getItemEntry;
    // if (payload["modId"].get<u16>() == MOD_NONE) {
    //     getItemEntry = ItemTableManager::Instance->RetrieveItemEntry(MOD_NONE, payload["getItemId"].get<s16>());
    // } else {
    //     getItemEntry = Rando::StaticData::RetrieveItem(payload["getItemId"].get<RandomizerGet>()).GetGIEntry_Copy();
    // }

    // gettingItem = true;
    // if (getItemEntry.modIndex == MOD_NONE) {
    //     if (getItemEntry.getItemId == GI_SWORD_BGS) {
    //         gSaveContext.bgsFlag = true;
    //     }
    //     Item_Give(gPlayState, getItemEntry.itemId);
    // } else if (getItemEntry.modIndex == MOD_RANDOMIZER) {
    //     if (getItemEntry.getItemId == RG_ICE_TRAP) {
    //         gSaveContext.pendingIceTrapCount++;
    //     } else {
    //         Randomizer_Item_Give(gPlayState, getItemEntry);
    //     }
    // }
    // gettingItem = false;

    // if (getItemEntry.getItemCategory != ITEM_CATEGORY_JUNK) {
    //     if (getItemEntry.modIndex == MOD_NONE) {
    //         Notification::Emit({
    //             .itemIcon = GetTextureForItemId(getItemEntry.itemId),
    //             .prefix = client.name,
    //             .message = "found",
    //             .suffix = SohUtils::GetItemName(getItemEntry.itemId),
    //         });
    //     } else if (getItemEntry.modIndex == MOD_RANDOMIZER) {
    //         Notification::Emit({
    //             .prefix = client.name,
    //             .message = "found",
    //             .suffix = Rando::StaticData::RetrieveItem((RandomizerGet)getItemEntry.getItemId).GetName().english,
    //         });
    //     }
    // }
}

#endif // ENABLE_NETWORKING
