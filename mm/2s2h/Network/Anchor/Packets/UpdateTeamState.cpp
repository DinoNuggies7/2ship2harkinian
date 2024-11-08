#ifdef ENABLE_NETWORKING

#include "2s2h/Network/Anchor/Anchor.h"
#include "2s2h/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "2s2h/BenPort.h"
#include "2s2h/BenGui/Notification.h"

extern "C" {
#include "variables.h"
extern PlayState* gPlayState;
}

/**
 * UPDATE_TEAM_STATE
 *
 * Pushes the current save state to the server for other teammates to use.
 * 
 * Fires when the server passes on a REQUEST_TEAM_STATE packet, or when this client saves the game
 * 
 * When sending this packet we will assume that the team queue has been emptied for this client, so the queue
 * stored in the server will be cleared.
 * 
 * When receiving this packet, if there is items in the team queue, we will play them back in order.
 */

void Anchor::SendPacket_UpdateTeamState() {
    if (!IsSaveLoaded()) {
        return;
    }

    json payload;
    payload["type"] = UPDATE_TEAM_STATE;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");

    // Assume the team queue has been emptied, so clear it
    payload["queue"] = json::array();

    payload["state"] = gSaveContext;
    // TODO: Manually update scene flags from actorCtx

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_UpdateTeamState(nlohmann::json payload) {
    isHandlingUpdateTeamState = true;
    // This can happen in between file select and the game starting, so we cant use this check, but we need to ensure we
    // be careful to wrap PlayState usage in this check
    // if (!IsSaveLoaded()) {
    //     return;
    // }

    if (payload.contains("state")) {
        SaveContext loadedData = payload["state"].get<SaveContext>();

        // Restore bottle contents (unless it's the Deku Princess)
        for (int i = 0; i < 6; i++) {
            if (gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + i] != ITEM_NONE &&
                gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + i] != ITEM_DEKU_PRINCESS) {
                loadedData.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + i] =
                    gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + i];
            }
        }

        // Restore ammo if it's non-zero, unless it's beans
        for (int i = 0; i < ARRAY_COUNT(gSaveContext.save.saveInfo.inventory.ammo); i++) {
            if (gSaveContext.save.saveInfo.inventory.ammo[i] != 0 && i != SLOT(ITEM_MAGIC_BEANS)) {
                loadedData.save.saveInfo.inventory.ammo[i] = gSaveContext.save.saveInfo.inventory.ammo[i];
            }
        }

        gSaveContext.save.saveInfo.inventory = loadedData.save.saveInfo.inventory;

        Notification::Emit({
            .message = "Save updated from team",
        });
    }

    if (payload.contains("queue")) {
        for (auto& item : payload["queue"]) {
            nlohmann::json itemPayload = nlohmann::json::parse(item.get<std::string>());
            Anchor::Instance->OnIncomingJson(itemPayload);
        }
    }
    isHandlingUpdateTeamState = false;
}

#endif // ENABLE_NETWORKING
