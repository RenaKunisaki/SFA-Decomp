#ifndef MAIN_DLL_DIM_DLL_223_H_
#define MAIN_DLL_DIM_DLL_223_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "main/dll/DIM/DIMbosstonsil.h"

typedef int (*DIMbosstonsilHitReactionCallback)(GameObject* obj, DIMbosstonsilState* state);
typedef int (*DIMbosstonsilUpdateHitReactionCallback)(GameObject* obj, DIMbosstonsilState* state, int unused);

typedef struct DIMbosstonsilStateHandlerTable {
    DIMbosstonsilHitReactionCallback startIdle;
    DIMbosstonsilHitReactionCallback choose;
} DIMbosstonsilStateHandlerTable;

typedef struct DIMbosstonsilSubstateHandlerTable {
    DIMbosstonsilHitReactionCallback enable;
    DIMbosstonsilUpdateHitReactionCallback update;
} DIMbosstonsilSubstateHandlerTable;

int DIMbosstonsil_updateHitReaction(GameObject* obj, DIMbosstonsilState* state, int unused);
int DIMbosstonsil_enableHitReaction(GameObject* obj, DIMbosstonsilState* state);
int DIMbosstonsil_chooseHitReaction(GameObject* obj, DIMbosstonsilState* state);
int DIMbosstonsil_startIdleHitReaction(GameObject* obj, DIMbosstonsilState* state);
void DIMbosstonsil_checkHit(GameObject* obj, DIMbosstonsilState* state);

extern DIMbosstonsilStateHandlerTable gDIMbosstonsilStateHandlers;
extern DIMbosstonsilSubstateHandlerTable gDIMbosstonsilSubstateHandlers;

STATIC_ASSERT(sizeof(DIMbosstonsilStateHandlerTable) == 8);
STATIC_ASSERT(sizeof(DIMbosstonsilSubstateHandlerTable) == 8);

#endif /* MAIN_DLL_DIM_DLL_223_H_ */
