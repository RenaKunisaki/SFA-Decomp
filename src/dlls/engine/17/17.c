#include "main/pi_dolphin.h"
#include "main/mapEventTypes.h"
#include "main/dll/dll_0015_curves.h"
#include "main/textrender_api.h"
#include "main/gametext_api.h"

u32 lbl_803DD4AC;
u32 lbl_803DD4A8;
u32 lbl_803DD4A4;
u32 lbl_803DD4A0;

u8 lbl_803A4218[0x10];
extern u8 lbl_803119E0[];

#define TASK_HINT_MAP ((s16*)lbl_803119E0)

void hintTextFn_800ea174(u8* out)
{
    u8* texts = getLastSavedGameTexts();
    s16 i;
    for (i = 0; i < 0xd; i++)
    {
        out[i] = mainGetBit(i + 0xf10);
    }
    out[TASK_HINT_MAP[texts[5]]] = 1;
}

u8 getCurTaskHintTextMap(void)
{
    u8* texts = getLastSavedGameTexts();
    return (u8)(s32)TASK_HINT_MAP[texts[5]];
}

void* saveGameGetCurHint(void)
{
    u8* texts = getLastSavedGameTexts();
    return gameTextGet((s32)texts[5] + 0xf4);
}

int hintTextMapFn_800ea264(void)
{
    int ret = getCurGameText();
    u8* texts = getLastSavedGameTexts();
    gameTextLoadDir(lbl_803A4218[TASK_HINT_MAP[texts[5]]]);
    return ret;
}

u16 getNextTaskHintText(void)
{
    u8* p = getLastSavedGameTexts();
    return p[5];
}

static inline void markTaskBit(u8 id)
{
    int bank;
    u32 mask;
    u32 bits;

    mask = 1 << (id % 32);
    bank = (s16)(((u32)id >> 5) + 0x12f);
    bits = mainGetBit(bank);
    if ((bits & mask) == 0)
    {
        bits |= mask;
        mainSetBits(bank, bits);
    }
}

static inline int setTaskBit(u8 id)
{
    u32 mask;
    int bank;
    u32 bits;

    mask = 1 << (id % 32);
    bank = (s16)(((u32)id >> 5) + 0x12f);
    bits = mainGetBit(bank);
    if ((bits & mask) != 0)
    {
        return 0;
    }
    bits |= mask;
    mainSetBits(bank, bits);
    return 1;
}

void gameBitFn_800ea2e0(u8 id)
{
    u8* texts;
    u8 wasNew;
    u32 i;
    s16 cachedBank;
    u32 cachedBits;
    int dwBank;
    u32 dwMask;
    s16 historyIdx;

    texts = getLastSavedGameTexts();
    cachedBank = -1;

    if (texts[6] == 0)
    {
        for (i = 1; (s16)i < 0xce; i++)
        {
            if ((TASK_HINT_MAP[i] == 0xffff) || (TASK_HINT_MAP[i] == -1))
            {
                markTaskBit((u8)i);
            }
        }
    }

    wasNew = setTaskBit(id);

    if (wasNew)
    {
        if (texts[6] != 5)
        {
            texts[6]++;
        }

        for (historyIdx = 4; historyIdx != 0; historyIdx--)
        {
            texts[historyIdx] = texts[historyIdx - 1];
        }
        texts[0] = id;

        if (texts[5] == id)
        {
            do
            {
                texts[5]++;
                dwBank = (s16)(((u32)texts[5] >> 5) + 0x12f);
                if (dwBank != cachedBank)
                {
                    cachedBank = dwBank;
                    cachedBits = mainGetBit(dwBank);
                }
                dwMask = 1 << (texts[5] % 32);
            } while ((cachedBits & dwMask) != 0);
        }
    }
}

void loadTaskTexts(void)
{
    char** pp;
    int i;
    u8* name;
    int idx;
    u8* dst;
    int n = 0xd;
    dst = &lbl_803A4218[0xd];
    while (dst--, n-- != 0)
    {
        *dst = 0xff;
    }
    i = 0x49;
    pp = &sMapDirectoryNameTable[0x49];
    while (pp--, i-- != 0)
    {
        name = (u8*)*pp;
        if (name[0] == 'T' && name[1] == 'a' && name[2] == 's' && name[3] == 'k' && name[4] == 'T' && name[5] == 'e' &&
            name[6] == 'x' && name[7] == 't' && name[8] == 's')
        {
            idx = (name[9] - '0') * 100 + (name[10] - '0') * 10 + (name[11] - '0');
            if (idx < 0xd)
            {
                lbl_803A4218[idx] = i;
            }
        }
    }
}

void screens_run(int unused)
{
    if (lbl_803DD4A0 != 0)
    {
        mm_free((void*)lbl_803DD4A0);
        lbl_803DD4A0 = 0;
        lbl_803DD4A4 = 0;
        lbl_803DD4AC = (u32)-1;
    }
}

void screens_remove(void)
{
    if (lbl_803DD4A0 != 0)
    {
        mm_free((void*)lbl_803DD4A0);
        lbl_803DD4A0 = 0;
        lbl_803DD4AC = (u32)-1;
        lbl_803DD4A4 = 0;
        lbl_803DD4A8 = 0;
    }
}

void screens_show(int id)
{
    int* asset = NULL;
    int* p;
    int count;
    int offset, size;
    if ((int)lbl_803DD4AC != id)
    {
        loadAssetFileById((void**)&asset, MLDF_FILEID_SCREENS_TAB);
        count = 0;
        while (asset[count] != -1)
        {
            count++;
        }
        if (id < 0 || id >= count - 1)
            id = 0;
        offset = asset[id];
        size = asset[id + 1] - offset;
        if (size != (int)lbl_803DD4A4)
        {
            if (lbl_803DD4A0 != 0)
                mm_free((void*)lbl_803DD4A0);
            lbl_803DD4A0 = (u32)mmAlloc(size, 2, 0);
        }
        lbl_803DD4A4 = size;
        getTabEntry((void*)lbl_803DD4A0, MLDF_FILEID_SCREENS_BIN, offset, size);
        mm_free(asset);
        lbl_803DD4AC = id;
    }
    lbl_803DD4A8 = 1;
}

void screens_release(void)
{
}

void screens_initialise(void)
{
    lbl_803DD4AC = (u32)-1;
    lbl_803DD4A0 = 0;
    lbl_803DD4A4 = 0;
    lbl_803DD4A8 = 0;
}

u8 lbl_803119E0[512] = {
    255, 255, 0,   0,   0,   0,   0,   0,   0,   0,   255, 255, 255, 255, 0,   0,   0,   0,   0,   6,   0,   6,   0,
    6,   0,   6,   0,   6,   0,   6,   255, 255, 255, 255, 0,   2,   0,   2,   0,   5,   0,   5,   0,   5,   0,   5,
    0,   6,   255, 255, 255, 255, 0,   6,   0,   6,   0,   6,   0,   6,   0,   6,   0,   6,   255, 255, 0,   5,   255,
    255, 0,   5,   0,   6,   0,   7,   255, 255, 0,   7,   0,   7,   0,   7,   0,   7,   0,   7,   0,   7,   0,   7,
    0,   7,   0,   7,   0,   7,   255, 255, 255, 255, 255, 255, 255, 255, 0,   7,   0,   6,   0,   9,   0,   9,   0,
    10,  0,   10,  0,   10,  0,   10,  0,   10,  255, 255, 0,   9,   0,   9,   0,   9,   0,   9,   0,   9,   0,   9,
    0,   6,   255, 255, 0,   0,   0,   0,   0,   12,  255, 255, 0,   12,  255, 255, 255, 255, 0,   12,  0,   6,   0,
    11,  255, 255, 0,   11,  0,   11,  0,   11,  0,   11,  0,   11,  0,   11,  255, 255, 255, 255, 255, 255, 255, 255,
    0,   11,  255, 255, 0,   12,  0,   8,   0,   8,   0,   8,   0,   8,   255, 255, 255, 255, 255, 255, 0,   6,   0,
    4,   0,   4,   0,   4,   255, 255, 0,   4,   255, 255, 255, 255, 255, 255, 0,   4,   255, 255, 0,   0,   0,   6,
    0,   6,   0,   3,   255, 255, 0,   3,   0,   3,   0,   3,   0,   3,   255, 255, 255, 255, 255, 255, 0,   3,   0,
    10,  0,   10,  0,   10,  0,   10,  255, 255, 0,   6,   255, 255, 0,   6,   0,   5,   0,   5,   0,   5,   255, 255,
    0,   0,   255, 255, 0,   6,   0,   1,   255, 255, 255, 255, 0,   1,   0,   1,   0,   1,   0,   1,   255, 255, 0,
    1,   0,   1,   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0,   1,   0,   12,  0,   8,   255, 255,
    0,   8,   255, 255, 255, 255, 0,   6,   0,   6,   0,   3,   0,   3,   255, 255, 0,   3,   255, 255, 255, 255, 255,
    255, 0,   3,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 0,   0,   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 0,   0};

void* lbl_80311BE0[10] = {(void*)0x00000000,  (void*)0x00000000, (void*)0x00000000, (void*)0x00050000,
                          screens_initialise, screens_release,   (void*)0x00000000, screens_show,
                          screens_remove,     screens_run};
