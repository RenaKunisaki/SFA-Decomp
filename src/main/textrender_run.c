#include "ghidra_import.h"
#include "track/intersect_hud_api.h"
#include "track/intersect_render_setup_api.h"
#include "main/hud_visibility_api.h"
#include "main/audio/sfx.h"
#include "main/gametext_api.h"
#define GAMETEXT_COLOR_U8
#include "main/gametext_color_api.h"
#include "main/gameloop_api.h"
#include "main/gametext_charset_api.h"
#include "main/gametext_show_str_api.h"
#include "main/gametext_shared_internal.h"
#include "main/gametext_task_api.h"
#include "main/gx_scissor_api.h"
#include "main/mm.h"
#include "main/texture.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/os/OSFont.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/printf.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/savedata_struct.h"
#include "main/frame_timing.h"
#include "main/fileio.h"
#include "main/textrender_api.h"
#include "main/textrender_internal.h"
#include "main/lightmap_text_color_api.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/dll/dll_0015_save_settings.h"
#include "track/intersect_api.h"
#include "string.h"
#include "main/lightmap.h"

typedef f32 Mtx[3][4];


extern f32 gSubtitleCurTime;

TextFont* gameTextFonts;
int gameTextCharset;
int curLanguage;
int gGameTextLastLanguage;
int curGameTextDir;
int gGameTextLastDir;
int lbl_803DC9D4;
int lbl_803DC9D0;
void* gCurTextBox;
int lbl_803DC9C8;
char* gGameTextCommandStringCursor;
int lbl_803DC9C0;
int gGameTextMeasureOnly;
int gGameTextBoundsMinY;
int gGameTextBoundsMaxY;
int gGameTextBoundsMinX;
int gGameTextBoundsMaxX;
u16 lbl_803DC9AA;
u16 lbl_803DC9A8;
u8 gGameTextColorR;
u8 gGameTextColorG;
u8 gGameTextColorB;
u8 gGameTextColorA;
f32 gGameTextScale;
int gGameTextRevealActive;
int gGameTextDrawnCharIndex;
f32 gGameTextRevealProgress;
u8 gGameTextShadowColorR;
u8 gGameTextShadowColorG;
u8 gGameTextShadowColorB;
int gGameTextShadowOffsetX;
int gGameTextShadowOffsetY;
int gGameTextShadowEnabled;
u8 lbl_803DC980;
int gGameTextBufferIndex;
int gCurTextBuffer;
u8* gGameTextLastEntry;
int gGameTextFallbackBuf;
GameTextDrawFunc gameTextDrawFunc;
u8 gGameTextFontIsSjis;

void gameTextFinalizeLoad(GameTextLoadSlot* loadSlot);

extern u16 gGameTextSjisGlyphTable[];
extern char sGameTextMapPathFormat[];
extern GXColor gGameTextClearColor;
extern int gGameTextFontTexRowPitch;
extern GameTextStateElem gGameTextCharsets[];
SubtitleCmd* subtitleParseControlCmds(char* str, int* count);

void gameTextLoadCancelCallback(s32 result, DVDCommandBlock* block);
void gameTextLoadCompleteCallback(s32 status, DVDFileInfo* fileInfo);

void gameTextLoadDir(int dirId)
{
    GameTextSlot* cmd;
    GXColor color;
    int slotIndex;

    gGameTextColorR = 0xff;
    gGameTextColorG = 0xff;
    gGameTextColorB = 0xff;
    gGameTextColorA = 0xff;

    if (dirId == 3)
    {
        gameTextFonts = (TextFont*)&gGameTextCharsets[GAMETEXT_SLOT_ERROR];
        gameTextCharset = GAMETEXT_SLOT_ERROR;
        color = gGameTextClearColor;
        hudDrawRect(0, 0, 0xa00, 0x780, color);
        gGameTextRevealActive = 0;
        if (gameTextDrawFunc == NULL)
        {
            slotIndex = gGameTextCommandCount;
            gGameTextCommandCount = slotIndex + 1;
            cmd = &gGameTextCommandSlots[slotIndex];
            cmd->opcode = 0xf;
            cmd->arg0 = GAMETEXT_SLOT_ERROR;
        }
    }
    else if (dirId == 0x1c)
    {
        curGameTextDir = dirId;
        gameTextFonts = (TextFont*)&gGameTextCharsets[GAMETEXT_SLOT_HUD];
        gameTextCharset = GAMETEXT_SLOT_HUD;
        if (gameTextDrawFunc == NULL)
        {
            slotIndex = gGameTextCommandCount;
            gGameTextCommandCount = slotIndex + 1;
            cmd = &gGameTextCommandSlots[slotIndex];
            cmd->opcode = 0xf;
            cmd->arg0 = GAMETEXT_SLOT_HUD;
        }
        gameTextLoadForCurMap(GAMETEXT_SLOT_HUD);
    }
    else
    {
        gameTextFonts = (TextFont*)&gGameTextCharsets[GAMETEXT_SLOT_DIALOGUE];
        gameTextCharset = GAMETEXT_SLOT_DIALOGUE;
        if (gameTextDrawFunc == NULL)
        {
            slotIndex = gGameTextCommandCount;
            gGameTextCommandCount = slotIndex + 1;
            cmd = &gGameTextCommandSlots[slotIndex];
            cmd->opcode = 0xf;
            cmd->arg0 = GAMETEXT_SLOT_DIALOGUE;
        }
        curGameTextDir = dirId;
        if ((subtitleIsActive() == 0 || gameTextSaveDir(dirId) == 0) && curGameTextDir != gGameTextLastDir)
        {
            gameTextLoadForCurMap(GAMETEXT_SLOT_DIALOGUE);
        }
    }
}



int gameTextGetCharset(void)
{
    return gameTextCharset;
}

void gameTextSetCharset(int charset, int flags)
{
    if (gameTextDrawFunc != NULL || (flags & 1))
    {
        gameTextFonts = (TextFont*)&gGameTextCharsets[charset];
        gameTextCharset = charset;
        if (charset == 2)
        {
            GXColor color = gGameTextClearColor;
            hudDrawRect(0, 0, 0xa00, 0x780, color);
            gGameTextRevealActive = 0;
        }
    }
    if (gameTextDrawFunc == NULL || (flags & 2))
    {
        int i = gGameTextCommandCount;
        GameTextSlot* cmd;
        gGameTextCommandCount = i + 1;
        cmd = &gGameTextCommandSlots[i];
        cmd->opcode = 0xf;
        cmd->arg0 = charset;
    }
}

int gameTextGetState(int i);

int getCurGameText(void)
{
    return curGameTextDir;
}

int getCurLanguage(void)
{
    return curLanguage;
}

f32 gameTextGetTimer(void)
{
    return gameTextFonts->timer;
}


int gameTextGetState(int i)
{
    return gGameTextCharsets[i].state;
}

void gameTextRun(void)
{
    GameTextRuntime* runtime;
    GameTextLoadSlot* loadSlot;
    TextFont* pending;
    int sourceId;
    GameTextSlot* cmd;
    int i;
    GameTextLoadSlot* freeSlot;
    int dirId;
    int languageId;
    GameTextBox* textBox;
    GXColor color;
    double fadeLimit;
    double zero;

    runtime = (GameTextRuntime*)gGameTextBase;
    cmd = runtime->commands;

    loadSlot = runtime->loadSlots;
    i = GAMETEXT_LOAD_SLOT_COUNT - 1;
    do
    {
        if (loadSlot->state == 2)
        {
            gameTextFinalizeLoad(loadSlot);
        }
        loadSlot++;
    } while (i-- != 0);

    sourceId = 0;
    pending = runtime->fonts;
    do
    {
        if (pending->dirId != GAMETEXT_INVALID_DIR)
        {
            loadSlot = runtime->loadSlots;
            dirId = pending->dirId;
            do
            {
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                ++loadSlot;
                if (loadSlot->active == 0)
                {
                    dirId = pending->dirId;
                    break;
                }
                loadSlot = NULL;
            } while (0);
            freeSlot = loadSlot;

            if (freeSlot != NULL)
            {
                languageId = pending->languageId;
                freeSlot->state = 1;
                freeSlot->dirId = (u8)dirId;
                freeSlot->languageId = languageId;
                freeSlot->active = 1;
                freeSlot->sourceId = sourceId;
                sprintf(runtime->path, sGameTextMapPathFormat,
                         sMapDirectoryNameTable[dirId], sLanguageNameTable[languageId].name);
                setFileInfo(&freeSlot->fileInfo);
                freeSlot->loadHandle = loadFileByPathAsync(runtime->path,
                                                           &freeSlot->loadedSize, 1, gameTextLoadCompleteCallback);
                setFileInfo(NULL);
                pending->dirId = GAMETEXT_INVALID_DIR;
                pending->languageId = GAMETEXT_INVALID_LANGUAGE;
            }
        }
        pending++;
        sourceId++;
    } while (sourceId < GAMETEXT_PENDING_SOURCE_COUNT);

    loadSlot = runtime->loadSlots;
    i = GAMETEXT_LOAD_SLOT_COUNT - 1;
    do
    {
        if ((loadSlot->state == 5 || loadSlot->state == 6) && loadSlot->loadHandle != NULL)
        {
            mm_free(loadSlot->loadHandle);
            loadSlot->loadHandle = NULL;
            loadSlot->loadedSize = 0;
            loadSlot->active = 0;
        }
        loadSlot++;
    } while (i-- != 0);

    i = GAMETEXT_LOAD_SLOT_COUNT;
    {
        f32* alpha;
        GameTextFadeEntry* entry;
        f32* timer;
        timer = runtime->fadeTimers + 8;
        alpha = runtime->fadeElapsed + 8;
        entry = runtime->fadeEntries + 8;
        zero = lbl_803DE704;
        fadeLimit = gGameTextFadeLimit;
        while (timer--, alpha--, entry--, i-- != 0)
        {
            if ((double)*timer > zero)
            {
                *alpha += timeDelta;
                if ((double)*alpha > fadeLimit)
                {
                    *timer = zero;
                    *alpha = zero;
                    sprintf(*entry->text, sGameTextBlankFormat);
                }
            }
        }
    }

    if (gameTextFonts->mode == 1)
    {
        gameTextFonts->timer += timeDelta;
    }
    else
    {
        gameTextFonts->timer = lbl_803DE704;
    }

    textBox = gTextBoxes;
    for (i = 148; i != 0; i--)
    {
        textBox->flags &= ~1;
        textBox++;
    }

    gGameTextRevealActive = 0;
    lbl_803DC9AA = 0;
    lbl_803DC9A8 = 0;

    i = gGameTextCommandCount;
    while (i-- != 0)
    {
        switch (cmd->opcode)
        {
        case 3:
        {
            u8 c1, c2, c3;
            c3 = cmd->arg3;
            c2 = cmd->arg2;
            c1 = cmd->arg1;
            gGameTextColorR = cmd->arg0;
            gGameTextColorG = c1;
            gGameTextColorB = c2;
            gGameTextColorA = c3;
            break;
        }
        case 4:
        {
            int t1 = cmd->arg2;
            gTextBoxes[cmd->arg0].cursorX = (s16)cmd->arg1;
            gTextBoxes[cmd->arg0].cursorY = t1;
            break;
        }
        case 1:
            gameTextTickReveal(cmd->arg0, (struct TextDisplayState*)cmd->arg1);
            break;
        case 2:
            gameTextRenderById(cmd->arg0, cmd->arg1, cmd->arg2);
            break;
        case 5:
        {
            int strId = cmd->arg0;
            if (gCurTextBox != NULL)
            {
                gameTextRenderStrs((char*)strId, ((u8*)gCurTextBox - (u8*)gTextBoxes) / 0x20);
            }
            break;
        }
        case 6:
            gameTextRenderStrs((char*)cmd->arg0, cmd->arg1);
            break;
        case 7:
        {
            int t3 = cmd->arg3;
            int t2 = cmd->arg1;
            int t1 = cmd->arg0;
            textBox = &gTextBoxes[t2];
            textBox->cursorX = cmd->arg2;
            textBox->cursorY = t3;
            gameTextRenderStrs((char*)t1, t2);
            break;
        }
        case 8:
            if (cmd->arg0 == 0xff)
            {
                gCurTextBox = NULL;
            }
            else
            {
                gCurTextBox = &gTextBoxes[cmd->arg0];
            }
            break;
        case 9:
            ((void (*)(void))cmd->arg0)();
            break;
        case 10:
        {
            u16 b1 = cmd->arg1;
            lbl_803DC9AA = (u16)cmd->arg0;
            lbl_803DC9A8 = b1;
            break;
        }
        case 11:
            lbl_803DC9AA = 0;
            lbl_803DC9A8 = 0;
            break;
        case 12:
            gGameTextShadowEnabled = cmd->arg0;
            break;
        case 14:
        {
            u8 e1, e2;
            e2 = cmd->arg2;
            e1 = cmd->arg1;
            gGameTextShadowColorR = cmd->arg0;
            gGameTextShadowColorG = e1;
            gGameTextShadowColorB = e2;
            break;
        }
        case 13:
        {
            int sy = cmd->arg1;
            gGameTextShadowOffsetX = cmd->arg0;
            gGameTextShadowOffsetY = sy;
            break;
        }
        case 15:
            gameTextFonts = &runtime->fonts[cmd->arg0];
            gameTextCharset = cmd->arg0;
            if (cmd->arg0 == 2)
            {
                color = gGameTextClearColor;
                hudDrawRect(0, 0, 0xa00, 0x780, color);
                gGameTextRevealActive = 0;
            }
            break;
        }
        cmd++;
    }

    if (gGameTextRevealActive == 0)
    {
        Sfx_StopFromObject(0, SFXTRIG_clock_loop);
    }
    gGameTextCommandCount = 0;
    gGameTextCommandStringCursor = runtime->commandStringBuffer;

    i = 0x94;
    textBox = &gTextBoxes[148];
    while (textBox--, i-- != 0)
    {
        textBox->cursorX = 0;
        textBox->cursorY = 0;
    }
    gCurTextBox = NULL;
}

static inline u32 lookupSjisGlyph(int c)
{
    int i = 0xfe;
    u16* p = gGameTextSjisGlyphTable;
    while (i--)
    {
        if (p[0] == c)
        {
            return p[1];
        }
        p++;
    }
    return 0;
}

void gameTextInit(void)
{
    gameTextInitFn_8001c794();
    lbl_803DC980 = 1;
    gameTextLoadDir(0x1c);
}

void gameTextInitRendererState(void)
{
    u8* clearPtr;
    u8* glyphPage;
    u8** glyphPagePtr;
    u8* fontState;
    u8* textWindow;
    u8* gameTextBase;
    int glyphPageCount;
    u8* request;
    u8* p;
    f32 zero;
    int i;
    int j;

    gameTextBase = gGameTextBase;

    i = 0x94;
    p = textWindow = (u8*)gTextBoxes + 0x1280;
    while (p -= 0x20, i-- != 0)
    {
        *(u16*)(p + 8) = *(u16*)(p + 2);
        *(u16*)(p + 0xa) = *(u16*)(p + 6);
    }

    glyphPageCount = GAMETEXT_LOAD_SLOT_COUNT;
    glyphPage = gameTextBase + 0x2c0;
    glyphPagePtr = (u8**)(gameTextBase + 0xc0);
    fontState = gameTextBase + 0xa0;
    while (glyphPage -= 0x40, glyphPagePtr--, fontState -= 0xc, glyphPageCount-- != 0)
    {
        *glyphPagePtr = glyphPage;
        *(u16*)fontState = 0xffff;
        *(u16*)(fontState + 2) = 1;
        fontState[4] = 0xff;
        fontState[5] = 0;
        fontState[6] = 0;
        fontState[7] = 0;
        *(u8***)(fontState + 8) = glyphPagePtr;
    }

    i = 0x94;
    while (textWindow -= 0x20, i-- != 0)
    {
        textWindow[0x1e] = 0xff;
    }

    j = 4;
    request = gameTextBase + GAMETEXT_LOAD_SLOTS_OFFSET;
    zero = lbl_803DE704;
    while (request -= 0x28, j-- != 0)
    {
        *(int*)(request + 8) = 0;
        *(int*)(request + 0xc) = 0;
        *(int*)(request + 0) = 0;
        *(int*)(request + 4) = 0;
        *(int*)(request + 0x1c) = 0;
        *(f32*)(request + 0x20) = zero;
        request[0x24] = 0xff;
        request[0x25] = 6;

        i = 3;
        clearPtr = request + 0xc;
        while (clearPtr -= 4, i-- != 0)
        {
            *(int*)(clearPtr + 0x10) = 0;
        }
    }

    gameTextFonts = (TextFont*)(gameTextBase + GAMETEXT_FONT_SLOT_OFFSET);
    gameTextCharset = 2;
    curLanguage = -1;
    curGameTextDir = -1;
    gCurTextBox = NULL;
    gGameTextLastLanguage = -1;
    gGameTextLastDir = -1;
    gGameTextMeasureOnly = 0;
    gGameTextColorR = 0xff;
    gGameTextColorG = 0xff;
    gGameTextColorB = 0xff;
    gGameTextColorA = 0xff;
    gGameTextCommandCount = 0;
    gGameTextCommandStringCursor = (char*)(gameTextBase + GAMETEXT_COMMAND_STRING_BUFFER_OFFSET);
    gGameTextBufferIndex = 0;
    textWindow = gameTextBase + 0x40;
    gGameTextLastEntry = textWindow;
    gCurTextBuffer = *(int*)*(void**)(textWindow + 8);
    gGameTextShadowColorR = 0;
    gGameTextShadowColorG = 0;
    gGameTextShadowColorB = 0;
    gGameTextShadowOffsetX = 5;
    gGameTextShadowOffsetY = 5;
    gGameTextShadowEnabled = 1;
    lbl_803DC980 = 0;
    gameTextBuildSystemFontAtlas();
    curGameTextDir = 3;
    gGameTextStringStore = (void*)mmCreateMemoryStore(0x800);
}


void loadGameTextSequence(int sequenceSlotDir, int sequenceId)
{
    GameTextLoadSlot* slot;
    int oldHeap;
    GameTextRuntime* gameTextBase;
    int languageTableOffset;
    u8* languageTable;
    int i;

    gameTextBase = (GameTextRuntime*)gGameTextBase;
    languageTableOffset = curLanguage << 3;
    languageTable = (u8*)sLanguageNameTable;
    oldHeap = testAndSet_onlyUseHeap3(0);
    if (getGameState() != 0 && getGameState() != 1)
    {
        testAndSet_onlyUseHeap3(oldHeap);
        return;
    }

    lbl_803DC9D0 = lbl_803DC9D4;
    if (curLanguage < 0 || curLanguage >= 6)
    {
        testAndSet_onlyUseHeap3(oldHeap);
        return;
    }

    slot = gameTextBase->loadSlots;
    i = GAMETEXT_LOAD_SLOT_COUNT - 1;
    do
    {
        if (slot->sourceId == GAMETEXT_SEQUENCE_SOURCE_ID)
        {
            if (slot->state == 1)
            {
                slot->state = 4;
                DVDCancelAsync(&slot->fileInfo.cb, gameTextLoadCancelCallback);
            }
            if (slot->state == 3 && slot->active != 0)
            {
                mmSetFreeDelay(0);
                mm_free(slot->loadHandle);
                mmSetFreeDelay(2);
                slot->loadHandle = NULL;
                slot->loadedSize = 0;
                slot->active = 0;
            }
        }
        slot++;
    } while (i-- != 0);

    gameTextBase->fonts[GAMETEXT_SLOT_CUTSCENE].mode = 1;
    slot = gameTextBase->loadSlots;
    slot = (slot->active == 0)       ? slot
           : ((++slot)->active == 0) ? slot
           : ((++slot)->active == 0) ? slot
           : ((++slot)->active == 0) ? slot
           : ((++slot)->active == 0) ? slot
           : ((++slot)->active == 0) ? slot
           : ((++slot)->active == 0) ? slot
           : ((++slot)->active == 0) ? slot
                                     : NULL;

    slot->state = 1;
    slot->dirId = sequenceSlotDir;
    slot->languageId = curLanguage;
    slot->active = 1;
    slot->sourceId = GAMETEXT_SEQUENCE_SOURCE_ID;
    sprintf(gameTextBase->path, sGameTextSequencePathFormat, sequenceId,
            ((LanguageName*)(languageTable + languageTableOffset))->name);
    setFileInfo(&slot->fileInfo);
    slot->loadHandle = loadFileByPathAsync(gameTextBase->path,
                                           &slot->loadedSize, 1, gameTextLoadCompleteCallback);
    setFileInfo(NULL);
    testAndSet_onlyUseHeap3(oldHeap);
}

void gameTextLoadForCurMap(int sourceId)
{
    u8* dirPtr;
    u8* langPtr;
    int oldHeap;
    int dirId;
    int languageId;
    GameTextLoadSlot* slot;
    GameTextLoadSlot* freeSlot;
    u8* gameTextBase;
    GameTextLoadState* loadState;
    int i;

    gameTextBase = gGameTextBase;
    loadState = (GameTextLoadState*)gameTextBase;
    oldHeap = testAndSet_onlyUseHeap3(0);
    if (getGameState() != 0 && getGameState() != 1)
    {
        testAndSet_onlyUseHeap3(oldHeap);
        return;
    }

    gGameTextLastDir = dirId = curGameTextDir;
    gGameTextLastLanguage = languageId = curLanguage;
    if (dirId < 0 || dirId >= GAMETEXT_MAP_DIR_COUNT || languageId < 0 || languageId >= GAMETEXT_LANGUAGE_COUNT)
    {
        testAndSet_onlyUseHeap3(oldHeap);
        return;
    }

    slot = (GameTextLoadSlot*)(gameTextBase + GAMETEXT_LOAD_SLOTS_OFFSET);
    i = GAMETEXT_LOAD_SLOT_COUNT - 1;
    do
    {
        if (slot->sourceId == sourceId)
        {
            if (slot->state == 1)
            {
                slot->state = 4;
                DVDCancelAsync(&slot->fileInfo.cb, gameTextLoadCancelCallback);
            }
            if (slot->state == 3 && slot->active != 0)
            {
                mmSetFreeDelay(0);
                if (slot->loadHandle != NULL)
                {
                    mm_free(slot->loadHandle);
                }
                mmSetFreeDelay(2);
                slot->loadHandle = NULL;
                slot->loadedSize = 0;
                slot->active = 0;
            }
        }
        slot++;
    } while (i-- != 0);

    loadState->requests[sourceId].state = 1;
    *(dirPtr = &loadState->requests[sourceId].dirId) = (u8)curGameTextDir;
    *(langPtr = &loadState->requests[sourceId].languageId) = curLanguage;

    slot = (GameTextLoadSlot*)(gameTextBase + GAMETEXT_LOAD_SLOTS_OFFSET);
    freeSlot = (slot->active == 0)       ? slot
               : ((++slot)->active == 0) ? slot
               : ((++slot)->active == 0) ? slot
               : ((++slot)->active == 0) ? slot
               : ((++slot)->active == 0) ? slot
               : ((++slot)->active == 0) ? slot
               : ((++slot)->active == 0) ? slot
               : ((++slot)->active == 0) ? slot
                                         : NULL;

    if (freeSlot != NULL)
    {
        int slotDir = *dirPtr;
        int slotLang = *langPtr;
        freeSlot->state = 1;
        freeSlot->dirId = slotDir;
        freeSlot->languageId = slotLang;
        freeSlot->active = 1;
        freeSlot->sourceId = sourceId;
        sprintf((char*)(gameTextBase + GAMETEXT_PATH_BUFFER_OFFSET), sGameTextMapPathFormat,
                sMapDirectoryNameTable[slotDir], sLanguageNameTable[slotLang].name);
        setFileInfo(&freeSlot->fileInfo);
        freeSlot->loadHandle = loadFileByPathAsync((char*)(gameTextBase + GAMETEXT_PATH_BUFFER_OFFSET),
                                                   &freeSlot->loadedSize, 1, gameTextLoadCompleteCallback);
        setFileInfo(NULL);
        *dirPtr = GAMETEXT_INVALID_DIR;
        *langPtr = GAMETEXT_INVALID_LANGUAGE;
    }

    testAndSet_onlyUseHeap3(oldHeap);
}

void gameTextBuildSystemFontAtlas(void)
{
    int wbytes;
    u8* base30;
    TextFont* charset;
    u8* buf;
    int sizeA;
    int y;
    int sizeB;
    int x;
    u8* bufA;
    OSFontHeader* bufB;
    int savedHeap;
    int count;
    TextGlyph* glyph;
    u8* fontData;
    u8 s[3];
    s32 width;

    fontData = (u8*)gGameTextFontData;
    base30 = gGameTextFontMetrics;
    charset = (TextFont*)&gGameTextCharsets[GAMETEXT_SLOT_ERROR];
    savedHeap = testAndSet_onlyUseHeap3(0);
    buf = mmAlloc(0x120, 0x1a, 0);
    switch (OSGetFontEncode())
    {
    case 0:
        sizeA = 0x3000;
        sizeB = 0x10120;
        curLanguage = 0;
        gGameTextFontIsSjis = 0;
        break;
    case 1:
        sizeA = 0x4d000;
        sizeB = 0x90ee4;
        curLanguage = 4;
        gGameTextFontIsSjis = 1;
        break;
    }
    bufA = mmAlloc(sizeA, 0x1a, 0);
    bufB = mmAlloc(sizeB, 0x1a, 0);
    OSLoadFont(bufB, bufA);
    if (charset->glyphCount == 0)
    {
        if (gGameTextFontIsSjis)
        {
            charset->glyphs = (TextGlyph*)fontData;
            charset->glyphCount = 0x55;
            charset->entries = (u16*)(fontData + 0x8ec);
            charset->entryCount = 7;
        }
        else
        {
            charset->glyphs = (TextGlyph*)(fontData + 0x940);
            charset->glyphCount = 0x2b;
            charset->entries = (u16*)(fontData + 0xe24);
            charset->entryCount = 7;
        }
    }
    charset->textures[0] = (Texture*)textureAlloc(0x200, 0x60, 0, 0, 0, 0, 0, 1, 1);
    *(u16*)(base30 + 0x60) = charset->glyphCount;
    *(u8*)(base30 + 0x64) = 0x30;
    *(u8*)(base30 + 0x65) = 0x20;
    *(u16*)(base30 + 0x68) = 0;
    *(u16*)(base30 + 0x6a) = 0x18;
    count = charset->glyphCount;
    glyph = charset->glyphs;
    x = 0;
    y = 0;
    while (count--)
    {
        if (gGameTextFontIsSjis)
        {
            int c;
            u32 val;
            int hi;
            u8 lo;
            c = glyph->key;
            val = lookupSjisGlyph(c);
            hi = (val >> 8) & 0xff;
            lo = val;
            if (hi == 0)
            {
                s[0] = lo;
                s[1] = 0;
            }
            else
            {
                s[0] = hi;
                s[1] = lo;
                s[2] = 0;
            }
        }
        else
        {
            s[0] = glyph->key;
            s[1] = 0;
        }
        OSGetFontWidth((const char*)s, &width);
        if (width > *(u16*)(base30 + 0x68))
        {
            *(u16*)(base30 + 0x68) = width;
        }
        wbytes = width >> 3;
        if ((width & 7) != 0)
        {
            wbytes++;
        }
        {
            int j;
            u32* q = (u32*)buf;
            j = 0x48;
            while (j--)
            {
                *q++ = 0;
            }
        }
        OSGetFontTexel((const char*)s, buf, 0, 6, &width);
        if (x + 0x18 > 0x200)
        {
            x = 0;
            y += 0x18;
        }
        glyph->u = x;
        glyph->v = y;
        glyph->offsetX = 0;
        glyph->advanceX = 0;
        glyph->offsetY = 0;
        glyph->advanceY = 0;
        glyph->width = width;
        glyph->height = 0x18;
        glyph->lang = 6;
        glyph->page = 0;
        {
            int ty;
            int tyEnd;
            int tx;
            int txEnd;
            int row;
            u8* dst;
            u32* src;
            int j2;

            src = (u32*)buf;
            tx = glyph->u >> 3;
            ty = glyph->v >> 3;
            row = ty;
            txEnd = tx + 3;
            tyEnd = ty + 3;
            for (; row < tyEnd; row++)
            {
                for (j2 = tx; j2 < txEnd; j2++)
                {
                    int k;
                    dst = (u8*)charset->textures[0] + (j2 << 5);
                    dst += row * gGameTextFontTexRowPitch;
                    for (k = 0; k < 8; k++)
                    {
                        *(u32*)(dst + 0x60 + k * 4) = *src++;
                    }
                }
            }
        }
        x += wbytes << 3;
        glyph++;
    }
    DCFlushRange((u8*)charset->textures[0] + 0x60, 0x20000);
    mm_free(bufA);
    mm_free(bufB);
    mm_free(buf);
    testAndSet_onlyUseHeap3(savedHeap);
    charset->mode = 2;
}

/* Install a completed language/charset load, upload its textures, and compact
   the relocatable text tables. */
void gameTextFinalizeLoad(GameTextLoadSlot* loadSlot)
{
    int** textureSlot;
    u16* p;
    u32 bpp;
    int ofs;
    int* table;
    u32 w;
    u32 h;
    int i;
    u8* txt;
    int* texHdr;
    u8* hdr;
    u16* texStart;
    int* data;
    u16 kind;
    u8* entries;
    int numStrings;
    int* strs;
    int n;
    u32 size;
    u16* newBuf;
    u16* old;
    int delta;
    int* strs2;
    GameTextCharset* cs;

    DCStoreRange(loadSlot->loadHandle, loadSlot->loadedSize);
    if (loadSlot->sourceId == 1)
    {
        cs = (GameTextCharset*)&gGameTextCharsets[1];
    }
    else if (loadSlot->sourceId == 3)
    {
        cs = (GameTextCharset*)&gGameTextCharsets[3];
    }
    else
    {
        cs = (GameTextCharset*)&gGameTextCharsets[0];
        curGameTextDir = loadSlot->dirId;
        curLanguage = loadSlot->languageId;
    }
    data = loadSlot->loadHandle;
    cs->headerCount = data[0];
    if (cs->headerCount == 0)
    {
        cs->status = 3;
        loadSlot->state = 6;
        return;
    }
    cs->strings = (u8*)(data + 1);
    hdr = (u8*)data + cs->headerCount * 16;
    cs->count = *(u16*)(hdr + 4);
    ofs = *(u16*)(hdr + 6);
    entries = hdr + 8;
    cs->entries = entries;
    table = (int*)(entries + cs->count * 12);
    numStrings = table[0];
    strs = table + 1;
    for (i = 0; i < cs->count; i++)
    {
        *(int**)(cs->entries + i * 12 + 8) = strs + *(int*)(cs->entries + i * 12 + 8);
    }
    txt = (u8*)(table + numStrings) + 4;
    {
        int j;
        for (j = 0; j < numStrings; j++)
        {
            strs[j] = strs[j] + (int)txt;
        }
    }
    texHdr = (int*)(txt + ofs);
    p = (u16*)((u8*)texHdr + texHdr[0]);
    p += 2;
    texStart = p;
    textureSlot = (int**)cs;
    while (1)
    {
        kind = p[0];
        bpp = p[1];
        w = p[2];
        h = p[3];
        p += 4;
        if (w == 0 && h == 0)
        {
            break;
        }
        switch (kind)
        {
        case 1:
            kind = 5;
            break;
        case 2:
            kind = 0;
            break;
        }
        if (textureSlot[4] != NULL)
        {
            mmSetFreeDelay(0);
            mm_free(textureSlot[4]);
            mmSetFreeDelay(2);
        }
        textureSlot[4] = (int*)textureAlloc(w, h, kind, 0, 0, 0, 0, 1, 1);
        if (textureSlot[4] != NULL)
        {
            if (bpp == 4)
            {
                u8* src8 = (u8*)p;
                u8* dst8 = (u8*)textureSlot[4] + 0x60;
                n = (int)(w * h) >> 1;
                while (n--)
                {
                    *dst8++ = *src8++;
                }
                DCFlushRange((u8*)textureSlot[4] + 0x60, ((Texture*)textureSlot[4])->dataSize);
            }
            else
            {
                u16* src16 = p;
                u16* dst16 = (u16*)((u8*)textureSlot[4] + 0x60);
                n = w * h;
                while (n--)
                {
                    *dst16++ = *src16++;
                }
                DCFlushRange((u8*)textureSlot[4] + 0x60, ((Texture*)textureSlot[4])->dataSize);
            }
        }
        {
            u32 area = w * h;
            p += (int)(area * bpp) >> 4;
        }
        textureSlot = textureSlot + 1;
    }
    size = (u32)((u8*)texStart - (u8*)loadSlot->loadHandle);
    newBuf = mmAlloc(size, 0x1a, 0);
    n = size >> 1;
    {
        u16* d = newBuf;
        u16* s;
        old = loadSlot->loadHandle;
        s = old;
        delta = (int)newBuf - (int)old;
        while (n--)
        {
            *d++ = *s++;
        }
    }
    cs->strings = cs->strings + delta;
    cs->entries = cs->entries + delta;
    for (i = 0; i < cs->count; i++)
    {
        int ev = *(int*)(cs->entries + i * 12 + 8);
        *(int*)(cs->entries + i * 12 + 8) = ev + delta;
    }
    strs2 = (int*)((u8*)strs + delta);
    for (i = 0; i < numStrings; i++)
    {
        strs2[i] += delta;
    }
    mmSetFreeDelay(0);
    mm_free(loadSlot->loadHandle);
    loadSlot->loadHandle = NULL;
    mmSetFreeDelay(2);
    loadSlot->loadHandle = newBuf;
    cs->status = 2;
    loadSlot->state = 3;
}



void gameTextLoadCancelCallback(s32 result, DVDCommandBlock* block)
{
    int i;
    GameTextLoadSlot* slot = curGameTexts;
    (void)result;
    for (i = 8; i != 0; i--)
    {
        if (block == &slot->fileInfo.cb)
        {
            slot->state = 5;
            return;
        }
        slot++;
    }
}

void gameTextLoadCompleteCallback(s32 status, DVDFileInfo* fileInfo)
{
    int i;
    GameTextLoadSlot* slot = curGameTexts;
    if (status != -1 && status != -3)
    {
        for (i = 8; i != 0; i--)
        {
            if (fileInfo == &slot->fileInfo)
            {
                slot->state = 2;
                return;
            }
            slot++;
        }
    }
    else
    {
        slot = curGameTexts;
        for (i = 8; i != 0; i--)
        {
            if (fileInfo == &slot->fileInfo)
            {
                slot->state = 5;
                return;
            }
            slot++;
        }
    }
}

void gameTextSetDrawFunc(void* fn)
{
    gameTextDrawFunc = fn;
}

int gameTextSaveDir(int x)
{
    if (gGameTextSequenceMode == 0)
    {
        gGameTextSavedDir = x;
        return 1;
    }
    return 0;
}
