#ifndef CUTSCENES_H
#define CUTSCENES_H

void RegisterCutscenes();

// MiscInteractions
void RegisterSkipDekuSalesman();
void RegisterSkipFallingMoonsTear();
void RegisterSkipGivingBombersNotebook();
void RegisterSkipGreatFairyCutscene();
void RegisterSkipKafeiReveal();
void RegisterSkipScarecrowDance();
void RegisterSkipTatlInterrupts();
void RegisterSkipMayorsOfficeCutscene();
void RegisterSkipKamaroTeachDance();

// StoryCutscenes
void RegisterSkipClockTowerOpen();
void SkipClockTowerSkullKidEncounter();
void RegisterSkipLearningElegyOfEmptiness();
void RegisterSkipLearningSongOfHealing();
void RegisterSkipLearningSongOfTime();
void RegisterSkipSongOfTimeCutscene();
void RegisterSkipMilkRunCutscenes();
void RegisterSkipBombBagTheftCutscene();
void RegisterSkipRosaSistersDance();
void RegisterSkipLearningSonataOfAwakening();
void RegisterSkipBossCutscenes();
void RegisterSkipGiantsChamber();

void RegisterHideTitleCards();
void RegisterSkipEntranceCutscenes();
void RegisterSkipIntroSequence();
void RegisterSkipToFileSelect();
void HandleGiantsCutsceneSkip();

#endif // CUTSCENES_H
