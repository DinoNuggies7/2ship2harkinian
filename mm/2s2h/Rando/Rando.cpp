#include "Rando.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <libultraship/libultraship.h>
#include "Rando/ActorBehavior/ActorBehavior.h"
#include "Rando/MiscBehavior/MiscBehavior.h"
#include "Rando/Spoiler/Spoiler.h"
#include "Rando/CheckTracker/CheckTracker.h"

// When a save is loaded, we want to unregister all hooks and re-register them if it's a rando save
void OnSaveLoadHandler(s16 fileNum) {
    Rando::MiscBehavior::OnFileLoad();
    Rando::ActorBehavior::OnFileLoad();
    Rando::CheckTracker::OnFileLoad();
}

// Entry point for the module, run once on game boot
void Rando::Init() {
    Rando::Spoiler::RefreshOptions();
    Rando::MiscBehavior::Init();
    Rando::ActorBehavior::Init();
    Rando::CheckTracker::Init();
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(OnSaveLoadHandler);
}
