#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include <z64ocarina.h>
}

#define CVAR_NAME "gEnhancements.Songs.EnableSunsSong"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterEnableSunsSong() {
    COND_VB_SHOULD(VB_SONG_AVAILABLE_TO_PLAY, CVAR, {
        uint8_t* songIndex = va_arg(args, uint8_t*);
        // If the currently played song is Sun's Song, set it to be available to be played.
        if (*songIndex == OCARINA_SONG_SUNS) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterEnableSunsSong, { CVAR_NAME });
