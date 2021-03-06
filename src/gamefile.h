#ifndef GAMEFILE_H
#define GAMEFILE_H

#ifdef __cplusplus
extern "C" {
#endif


int GameFile_loadSavedGame(const char *filename);

int GameFile_loadSavedGameFromMissionPack(int missionId);

int GameFile_writeSavedGame(const char *filename);

void GameFile_writeMissionSavedGameIfNeeded();

int GameFile_deleteSavedGame(const char *filename);

int GameFile_loadScenario(const char *filename);


#ifdef __cplusplus
}
#endif

#endif
