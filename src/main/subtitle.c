#include "main/gametext_api.h"
#define GAMETEXT_COLOR_U8
#include "main/gametext_color_api.h"
#include "main/gametext_charset_api.h"
#include "main/gametext_show_str_api.h"
#include "main/gametext_shared_internal.h"
#include "main/hud_visibility_api.h"
#include "main/frame_timing.h"
#include "main/mm.h"
#include "main/texture.h"
#include "main/textrender_api.h"
#include "main/textrender_internal.h"

#define SUBTITLE_TIME_NONE 0.0001f

extern f32 gSubtitleCurTime;

SubtitleCmd* subtitleParseControlCmds(char* str, int* count);

void* gSubtitleLineTable[0x100];
char* gSubtitleLineStrs[0x100];
f32 gSubtitleLineTimes[0x100];

void subtitleUpdateAndDraw(int a)
{
    int charset;
    SubtitleCmd* cmds;
    int delay;
    int n;
    int i;
    f32 curTime;

    if (gSubtitleActive == 2)
    {
        if (gGameTextSequenceMode != 0)
        {
            charset = gameTextGetCharset();
            gameTextSetCharset(1, 2);
        }
        if (getHudHiddenFrameCount() == 0)
        {
            gSubtitleElapsedFrames += framesThisStep;
        }
        curTime = gSubtitleElapsedFrames / 60.0f;
        gSubtitleCurTime = curTime;
        i = gSubtitleLineIndex;
        if (i + 1 < gSubtitleLineCount && curTime >= gSubtitleLineTimes[i + 1])
        {
            cmds = subtitleParseControlCmds(gSubtitleLineStrs[i], &n);
            if (cmds != NULL)
            {
                SubtitleCmd* p = &cmds[n];
                while (p--, n-- != 0)
                {
                    if (p->code == TEXT_CTRL_COLOR)
                    {
                        SubtitleCmd* e = &cmds[n];
                        gSubtitleColorR = e->r;
                        gSubtitleColorG = e->g;
                        gSubtitleColorB = e->b;
                        gSubtitleColorA = e->a;
                        break;
                    }
                }
                delay = mmSetFreeDelay(0);
                mm_free(cmds);
                mmSetFreeDelay(delay);
            }
            if (++gSubtitleLineIndex + 1 >= gSubtitleLineCount)
            {
                subtitleStop();
                if (gGameTextSequenceMode != 0)
                {
                    gameTextSetCharset(charset, 2);
                }
                return;
            }
        }
        gameTextSetColor(gSubtitleColorR, gSubtitleColorG, gSubtitleColorB, gSubtitleColorA);
        gameTextShowStr(gSubtitleLineStrs[gSubtitleLineIndex], 10, 0, 0);
        if (gGameTextSequenceMode != 0)
        {
            gameTextSetCharset(charset, 2);
        }
    }
}

void mainLoopDoGameText(void)
{
    if (gGameTextSequenceMode != 0)
    {
        if (gameTextGetState(1) == 2 && gSubtitleActive == 1)
        {
            subtitleBuildLineTable();
        }
    }
    else
    {
        if (gameTextGetState(0) == 2 && gGameTextPendingDir == getCurGameText() && gSubtitleActive == 1)
        {
            subtitleBuildLineTable();
        }
    }
}

void subtitleStop(void)
{
    int none;
    void** slot;
    int i;
    int oldDelay;
    int dir;

    if (gSubtitleActive != 0)
    {
        gSubtitleActive = none = 0;
        i = 0;
        slot = gSubtitleLineTable;
        while (i < gSubtitleBlockCount)
        {
            if (*slot != NULL)
            {
                oldDelay = mmSetFreeDelay(0);
                mm_free(*slot);
                mmSetFreeDelay(oldDelay);
                *slot = (void*)none;
            }
            slot++;
            i++;
        }

        dir = gGameTextSavedDir;
        if (dir != -1)
        {
            gameTextLoadDir(dir);
            gGameTextSavedDir = -1;
        }
    }
}

void subtitleBuildLineTable(void)
{
    int savedCharset;
    SubtitleLineTable* s[1];
    f32 delta;
    f32 curTime;
    SubtitleTextEntry* t;
    u8* win;
    int m;
    int i;
    char* str;
    int k;
    int total;
    int oldDelay;
    char** strLines;
    int found;
    int q;
    int n;
    int count;
    int args[3];
    f32 ftotal;
    void** blk;

    s[0] = (SubtitleLineTable*)gSubtitleLineTable;
    total = 0;
    curTime = 0.0f;
    if (gGameTextSequenceMode != 0)
    {
        savedCharset = gameTextGetCharset();
        gameTextSetCharset(1, 1);
    }
    t = (SubtitleTextEntry*)gameTextGet(gGameTextPendingTextId);
    win = (u8*)gTextBoxes + 0x140;
    gSubtitleLineCount = 0;
    gSubtitleBlockCount = 0;
    for (i = 0; i < SUBTITLE_LINE_COUNT; i++)
    {
        s[0]->times[i] = SUBTITLE_TIME_NONE;
    }
    for (i = 0; i < t->count; i++)
    {
        str = t->strs[i];
        n = GameText_FindControlCodeArgs((u8*)str, TEXT_CTRL_SEQ_TIME, args);
        if (n != 0)
        {
            q = args[2] / 60;
            s[0]->times[gSubtitleLineCount] = (f32)(args[1] + args[0] * 60 + q);
        }
        strLines = gameTextWrapLines(str, (f32)(u32) * (u16*)(win + 2), *(f32*)(win + 0xc), &count, NULL);
        if (strLines != NULL)
        {
            for (k = 0; k < count; k++)
            {
                s[0]->lines[gSubtitleLineCount++] = strLines[k];
            }
            blk = (void**)((u8*)s[0] + gSubtitleBlockCount * 4);
            if (*blk != NULL)
            {
                oldDelay = mmSetFreeDelay(0);
                blk = (void**)((u8*)s[0] + gSubtitleBlockCount * 4);
                mm_free(*blk);
                mmSetFreeDelay(oldDelay);
            }
            blk = (void**)((u8*)s[0] + gSubtitleBlockCount++ * 4);
            *blk = strLines;
        }
    }
    for (k = 0; k < gSubtitleLineCount; k++)
    {
        if (SUBTITLE_TIME_NONE != s[0]->times[k])
        {
            curTime = s[0]->times[k];
            total = GameText_CountPrintableChars((u8*)s[0]->lines[k]);
        }
        else
        {
            found = 0;
            m = k;
            for (i = 0; i < SUBTITLE_LINE_COUNT; i++)
            {
                ftotal = total;
                if (m < 255)
                {
                    if (SUBTITLE_TIME_NONE != s[0]->times[m + 1])
                    {
                        delta = s[0]->times[m + 1] - curTime;
                        found = 1;
                    }
                    n = GameText_CountPrintableChars((u8*)s[0]->lines[m]);
                    s[0]->times[m] = n;
                    total += n;
                    if (found != 0)
                    {
                        for (q = m; q >= k; q--)
                        {
                            s[0]->times[q] = s[0]->times[q + 1] - delta * (s[0]->times[q] / total);
                        }
                        break;
                    }
                    m++;
                }
            }
        }
    }
    gSubtitleLineIndex = 0;
    gSubtitleElapsedFrames = 0;
    gSubtitleActive = 2;
    if (gGameTextSequenceMode != 0)
    {
        gameTextSetCharset(savedCharset, 1);
    }
}
