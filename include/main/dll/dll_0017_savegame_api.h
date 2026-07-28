#ifndef MAIN_DLL_DLL_0017_SAVEGAME_API_H_
#define MAIN_DLL_DLL_0017_SAVEGAME_API_H_

#include "main/dll/savedata_struct.h"

#define SAVEGAME_OBJECT_POSITION_COUNT  0x3f
#define SAVEGAME_OBJECT_POSITION_OFFSET 0x168

typedef struct SaveGameObjectPosition
{
    u32 objectId;
    f32 x;
    f32 y;
    f32 z;
} SaveGameObjectPosition;

extern u8 gSaveGameData[];
/* SaveData describes the settings prefix of this persisted byte buffer. */
extern u8 saveData[SAVE_DATA_SIZE];

void mapClearBit(int idx, int bit);
void* getHighScoreEntry(u8 fileIdx, u8 rank);
int saveGame_restoreObjectPosToRomList(void* object);

#endif /* MAIN_DLL_DLL_0017_SAVEGAME_API_H_ */
