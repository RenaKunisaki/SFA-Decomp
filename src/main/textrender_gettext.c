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

typedef struct
{
    u32 key;     /* 0x00 */
    u16 u;       /* 0x04 */
    u16 v;       /* 0x06 */
    s8 offsetX;  /* 0x08 */
    s8 advanceX; /* 0x09 */
    s8 offsetY;  /* 0x0a */
    s8 advanceY; /* 0x0b */
    u8 width;    /* 0x0c */
    u8 height;   /* 0x0d */
    u8 lang;     /* 0x0e */
    u8 page;     /* 0x0f */
} TextGlyph;

typedef struct
{
    TextGlyph* glyphs; /* 0x00 */
    u16* entries;      /* 0x04 */
    int glyphCount;    /* 0x08 */
    int entryCount;    /* 0x0c */
    Texture* textures[3]; /* 0x10 */
    int mode;          /* 0x1c */
    f32 timer;         /* 0x20 */
    u8 dirId;          /* 0x24 */
    u8 languageId;     /* 0x25 */
    u8 pad26[2];       /* 0x26 */
} TextFont;


extern TextFont* gameTextFonts;
typedef void (*GameTextDrawFunc)(int x0, int y0, int x1, int y1, f32 u0, f32 v0, f32 u1, f32 v1);
extern GameTextDrawFunc gameTextDrawFunc;

extern u8 gGameTextBase[];
extern u8 sGameTextFallbackBufSlots[];
extern int gGameTextFallbackBuf;
extern u8* gGameTextLastEntry;
extern int gCurTextBuffer;
extern int gGameTextBufferIndex;
extern const f32 gGameTextFadeLimit;
extern void* curGameTextDir;
extern char gGameTextFontData[];
extern char sGameTextBlankFormat[5];

void* gameTextGetPhrase(int textId, int phraseIndex)
{
    char* strings;
    u16* entry;

    strings = gGameTextFontData;
    if (gameTextFonts->mode != 2)
    {
        gGameTextBufferIndex = gGameTextBufferIndex + 1;
        if (gGameTextBufferIndex >= 8)
        {
            gGameTextBufferIndex = 0;
        }
        gGameTextLastEntry = sGameTextFallbackDefs + gGameTextBufferIndex * 0xc;
        gCurTextBuffer = *(int*)*(int**)(gGameTextLastEntry + 8);
        *(u16*)gGameTextLastEntry = 0xffff;
        gGameTextFallbackBuf = (int)(sGameTextFallbackBufSlots + gGameTextBufferIndex * 4);
        switch (gameTextFonts->mode)
        {
        case 0:
            sprintf((char*)gCurTextBuffer, strings + 0xec4);
            break;
        case 1:
            sprintf((char*)gCurTextBuffer, strings + 0xed4);
            break;
        case 3:
            sprintf((char*)gCurTextBuffer, strings + 0xee0);
            break;
        case 4:
            sprintf((char*)gCurTextBuffer, strings + 0xef0);
            break;
        }
        return gGameTextLastEntry;
    }

    entry = gameTextGet(textId);
    if (*entry == 0xffff)
    {
        gGameTextBufferIndex = gGameTextBufferIndex + 1;
        if (gGameTextBufferIndex >= 8)
        {
            gGameTextBufferIndex = 0;
        }
        gGameTextLastEntry = sGameTextFallbackDefs + gGameTextBufferIndex * 0xc;
        gCurTextBuffer = *(int*)*(int**)(gGameTextLastEntry + 8);
        *(u16*)gGameTextLastEntry = 0xffff;
        gGameTextFallbackBuf = (int)(sGameTextFallbackBufSlots + gGameTextBufferIndex * 4);
        sprintf((char*)gCurTextBuffer, strings + 0xefc, textId,
                sMapDirectoryNameTable[(int)curGameTextDir]);
        return gGameTextLastEntry;
    }

    if (phraseIndex >= entry[1])
    {
        gGameTextBufferIndex = gGameTextBufferIndex + 1;
        if (gGameTextBufferIndex >= 8)
        {
            gGameTextBufferIndex = 0;
        }
        gGameTextLastEntry = sGameTextFallbackDefs + gGameTextBufferIndex * 0xc;
        gCurTextBuffer = *(int*)*(int**)(gGameTextLastEntry + 8);
        *(u16*)gGameTextLastEntry = 0xffff;
        gGameTextFallbackBuf = (int)(sGameTextFallbackBufSlots + gGameTextBufferIndex * 4);
        sprintf((char*)gCurTextBuffer, strings + 0xf10, textId, phraseIndex);
        return gGameTextLastEntry;
    }

    return *(void**)(*(int*)((u8*)entry + 8) + phraseIndex * 4);
}

void* gameTextGetStr(int textId)
{
    u8* entry;
    char* strings;
    void* textEntry;

    strings = gGameTextFontData;
    if (gameTextFonts->mode != 2)
    {
        gGameTextBufferIndex = gGameTextBufferIndex + 1;
        if (gGameTextBufferIndex >= 8)
        {
            gGameTextBufferIndex = 0;
        }
        gGameTextLastEntry = sGameTextFallbackDefs + gGameTextBufferIndex * 0xc;
        gCurTextBuffer = *(int*)*(int**)(gGameTextLastEntry + 8);
        *(u16*)gGameTextLastEntry = 0xffff;
        gGameTextFallbackBuf = (int)(sGameTextFallbackBufSlots + gGameTextBufferIndex * 4);
        switch (gameTextFonts->mode)
        {
        case 0:
            sprintf((char*)gCurTextBuffer, strings + 0xec4);
            break;
        case 1:
            sprintf((char*)gCurTextBuffer, strings + 0xed4);
            break;
        case 3:
            sprintf((char*)gCurTextBuffer, strings + 0xee0);
            break;
        case 4:
            sprintf((char*)gCurTextBuffer, strings + 0xef0);
            break;
        }
        return gGameTextLastEntry;
    }
    textEntry = gameTextGet(textId);
    return *(void**)*(u8**)((u8*)textEntry + 8);
}

void* gameTextGet(int textId)
{
    u8* gameTextBase;
    char* strings;
    TextFont* fonts;
    u16* entry;
    int count;
    int slotIndex;
    u16* cachedEntry;
    f32 zero;
    f32* cachedAlpha;
    u8* p;

    gameTextBase = gGameTextBase;
    strings = gGameTextFontData;
    fonts = gameTextFonts;

    if (fonts->mode != 2)
    {
        gGameTextBufferIndex++;
        if (gGameTextBufferIndex >= 8)
        {
            gGameTextBufferIndex = 0;
        }
        p = gameTextBase + gGameTextBufferIndex * 0xc;
        gGameTextLastEntry = p + 0x40;
        gCurTextBuffer = *(int*)*(int**)(gGameTextLastEntry + 8);
        *(u16*)gGameTextLastEntry = 0xffff;
        p = gameTextBase + gGameTextBufferIndex * 4;
        gGameTextFallbackBuf = (int)(p + 0x20);

        switch (gameTextFonts->mode)
        {
        case 0:
            sprintf((char*)gCurTextBuffer, strings + 0xec4);
            break;
        case 1:
            sprintf((char*)gCurTextBuffer, strings + 0xed4);
            break;
        case 3:
            sprintf((char*)gCurTextBuffer, strings + 0xee0);
            break;
        case 4:
            sprintf((char*)gCurTextBuffer, strings + 0xef0);
            break;
        }
        return gGameTextLastEntry;
    }

    entry = fonts->entries;
    count = fonts->entryCount;
    while (count != 0)
    {
        if (*entry == textId)
        {
            return entry;
        }
        entry += 6;
        count--;
    }

    slotIndex = 8;
    cachedEntry = (u16*)(gameTextBase + 0xa0);
    while (cachedEntry -= 6, slotIndex-- != 0)
    {
        if (*cachedEntry == textId)
        {
            zero = lbl_803DE704;
            *(f32*)(gameTextBase + slotIndex * 4) = zero;
            cachedAlpha = (f32*)(gameTextBase + 0x20 + slotIndex * 4);
            if (zero < gGameTextFadeLimit)
            {
                f32 av = zero + timeDelta;
                *cachedAlpha = av;
                if (av >= gGameTextFadeLimit)
                {
                    sprintf((char*)*(int*)*(int**)((u8*)cachedEntry + 8), strings + 0xefc, textId,
                            sMapDirectoryNameTable[(int)curGameTextDir]);
                }
            }
            return cachedEntry;
        }
    }

    gGameTextBufferIndex++;
    if (gGameTextBufferIndex >= 8)
    {
        gGameTextBufferIndex = 0;
    }
    p = gameTextBase + gGameTextBufferIndex * 0xc;
    gGameTextLastEntry = p + 0x40;
    gCurTextBuffer = *(int*)*(int**)(gGameTextLastEntry + 8);
    *(u16*)gGameTextLastEntry = 0xffff;
    p = gameTextBase + gGameTextBufferIndex * 4;
    gGameTextFallbackBuf = (int)(p + 0x20);
    sprintf((char*)gCurTextBuffer, sGameTextBlankFormat, textId, sMapDirectoryNameTable[(int)curGameTextDir]);
    *(u16*)gGameTextLastEntry = textId;
    *(f32*)gGameTextFallbackBuf = lbl_803DE704;
    return gGameTextLastEntry;
}

void gameTextResetCursor(int flags)
{
    if (flags & 1)
    {
        lbl_803DC9AA = 0;
        lbl_803DC9A8 = 0;
    }
    if (flags & 2)
    {
        int* p = &gGameTextCommandSlots[gGameTextCommandCount++].opcode;
        *p = 0xb;
    }
}

void gameTextSetCursor(u16 x, u16 y, int flags)
{
    if (flags & 1)
    {
        lbl_803DC9AA = x;
        lbl_803DC9A8 = y;
    }
    if (flags & 2)
    {
        int i = gGameTextCommandCount;
        GameTextSlot* cmd;
        gGameTextCommandCount = i + 1;
        cmd = &gGameTextCommandSlots[i];
        cmd->opcode = 0xa;
        cmd->arg0 = (u16)x;
        cmd->arg1 = (u16)y;
    }
}

void gameTextSetWindowStrPos(int idx, int x, int y)
{
    if (gameTextDrawFunc != NULL)
    {
        gTextBoxes[idx].cursorX = x;
        gTextBoxes[idx].cursorY = y;
    }
    else
    {
        int i = gGameTextCommandCount;
        GameTextSlot* cmd;
        gGameTextCommandCount = i + 1;
        cmd = &gGameTextCommandSlots[i];
        cmd->opcode = 4;
        cmd->arg0 = idx;
        cmd->arg1 = x;
        cmd->arg2 = y;
    }
}

void gameTextSetColor(u8 r, u8 g, u8 b, u8 a)
{
    if (gameTextDrawFunc != NULL)
    {
        gGameTextColorR = r;
        gGameTextColorG = g;
        gGameTextColorB = b;
        gGameTextColorA = a;
    }
    else
    {
        int i = gGameTextCommandCount;
        GameTextSlot* cmd;
        gGameTextCommandCount = i + 1;
        cmd = &gGameTextCommandSlots[i];
        cmd->opcode = 3;
        cmd->arg0 = r;
        cmd->arg1 = g;
        cmd->arg2 = b;
        cmd->arg3 = a;
    }
}
