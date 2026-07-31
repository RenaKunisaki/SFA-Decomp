#include "dolphin/os.h"

int gDefaultHeapInitialized;

const double __constants[3] = {
    0.0,
    4294967296.0,
    2147483648.0,
};

inline static void InitDefaultHeap(void) {
    void* arenaLo;
    void* arenaHi;

    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1);
    OSSetArenaLo(arenaLo);
    arenaLo = (void*)(((u32)arenaLo + 0x1F) & ~0x1F);
    arenaHi = (void*)((u32)arenaHi & ~0x1F);
    OSSetCurrentHeap(OSCreateHeap(arenaLo, arenaHi));
    OSSetArenaLo(arenaHi);
    gDefaultHeapInitialized = 1;
}

void __sys_free(void* p) {
    if (!gDefaultHeapInitialized) {
        InitDefaultHeap();
    }

    OSFreeToHeap(0, p);
}
