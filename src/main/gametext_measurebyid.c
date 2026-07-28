#include "main/gametext_internal.h"
#include "main/gametext_shared_internal.h"

/* In-string formatting control codes (Unicode PUA). */
#define TEXT_CTRL_SCALE 0xf8f4
#define TEXT_CTRL_FONT  0xf8f7

/* Language ids; order fixed by sLanguageNameTable[] below. */
#define LANGUAGE_ENGLISH  0
#define LANGUAGE_FRENCH   1
#define LANGUAGE_GERMAN   2
#define LANGUAGE_ITALIAN  3
#define LANGUAGE_JAPANESE 4
#define LANGUAGE_SPANISH  5

void gameTextMeasureById(int id, int a, int b, int* outMinX, int* outMaxX, int* outMinY, int* outMaxY)
{
    GlyphEntry* e;
    GameTextFont* fonts;
    int count;
    int i;
    int found;

    fonts = gameTextFonts;
    if (fonts->mode != 2)
    {
        found = 0;
    }
    else
    {
        e = fonts->entries;
        count = fonts->count;
        for (i = 0; i != count || (found = 0, 0); i++)
        {
            if (e->id == id)
            {
                found = 1;
                break;
            }
            e++;
        }
    }
    if (!found)
    {
        *outMinX = 0;
        *outMaxX = 0;
        *outMinY = 0;
        *outMaxY = 0;
        return;
    }
    gGameTextMeasureOnly = 1;
    gGameTextBoundsMinX = 0x7FFFFFFF;
    gGameTextBoundsMaxX = 0;
    gGameTextBoundsMinY = 0x7FFFFFFF;
    gGameTextBoundsMaxY = 0;
    gameTextRenderById(id, a, b);
    gGameTextMeasureOnly = 0;
    if (outMinY != NULL)
    {
        *outMinY = gGameTextBoundsMinY >> 2;
    }
    if (outMaxY != NULL)
    {
        *outMaxY = gGameTextBoundsMaxY >> 2;
    }
    if (outMinX != NULL)
    {
        *outMinX = gGameTextBoundsMinX >> 2;
    }
    if (outMaxX != NULL)
    {
        *outMaxX = gGameTextBoundsMaxX >> 2;
    }
}
