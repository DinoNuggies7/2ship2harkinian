#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME "gModes.HyperEnemies"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterHyperEnemies() {
    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->category != ACTORCAT_ENEMY) {
            return;
        }

        if (Player_InBlockingCsMode(gPlayState, GET_PLAYER(gPlayState))) {
            return;
        }

        if (actor->update != NULL) {
            // Everywhere we need to disable the collision checks already checks if frameAdvance is enabled, so we abuse
            // that temporarily. This doesn't have any _known_ side effects :)
            bool prevFrameAdv = gPlayState->frameAdvCtx.enabled;
            gPlayState->frameAdvCtx.enabled = true;
            actor->update(actor, gPlayState);
            gPlayState->frameAdvCtx.enabled = prevFrameAdv;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterHyperEnemies, { CVAR_NAME });
