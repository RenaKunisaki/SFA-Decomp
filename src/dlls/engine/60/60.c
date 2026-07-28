#include "dolphin/os/OSReport.h"
#include "dolphin/TRK_MINNOW_DOLPHIN/MWTrace.h"
#include "main/dll/dll_003C_tumbleweedbush.h"
#include "track/intersect_hud_api.h"
#include "main/gametext_box_api.h"
#include "main/gametext_command_api.h"
#include "main/dll/baddie/dll_003C_TumbleweedBush.h"
#include "main/gametext_show_api.h"
#include "main/gametext_show_str_api.h"
#include "main/hud_visibility_api.h"
#include "main/texture.h"
#include "main/gamebits.h"
#include "main/pad.h"
#include "main/textrender_api.h"
#include "main/frame_timing.h"
#include "main/dll/dll_0035_saveselectscreen.h"
#include "string.h"
#include "main/gametext_color_api.h"
#include "main/vecmath.h"
#include "dolphin/pad.h"

#define LINK_ITEM_SLOTS 25

extern u8 linkTextures[0x30];
s8 gTumbleweedBushInputEnabled[5];
s8 linkSelected;
s8 gTumbleweedBushItemCount;
s8 gTumbleweedBushPulseDir;
s16 linkCount_803dd90e;
s16 linkItemOpacity;
const char* gTumbleweedBushDefaultText;
s16 gTumbleweedBushBaseColorR;
s16 gTumbleweedBushBaseColorG;
s16 gTumbleweedBushBaseColorB;
s16 gTumbleweedBushSelColorR;
s16 gTumbleweedBushSelColorG;
s16 gTumbleweedBushSelColorB;
u8 linkIsRotated;
u8 linkFlag_803dd8f8;
extern char sTumbleweedBushSlotOverflowErr[];
extern char sTumbleweedBushNavLinkRangeErr[];

extern u8 gGameTextFontMetrics[];
#define PAD_ACCEPT_MASK  (PAD_BUTTON_A | PAD_BUTTON_START)


typedef struct LinkMenuItem
{
    u16 textId;
    u16 boxId;
    s16 rightX;
    s16 textLeft;
    s16 slotWidth;
    s16 x;
    s16 y;
    u8 pad0E[2];

    union
    {
        int textureAssetId;
        void* texture;
    };

    u16 width;
    u16 flags;
    u8 pad18[2];
    s8 upLink;
    s8 downLink;
    s8 leftLink;
    s8 rightLink;
    s8 state;
    s8 slots[LINK_ITEM_SLOTS];
    s8 timer;
    u8 pad39[3];
} LinkMenuItem;
STATIC_ASSERT(sizeof(LinkMenuItem) == 0x3C);
STATIC_ASSERT(offsetof(LinkMenuItem, rightX) == 0x04);
STATIC_ASSERT(offsetof(LinkMenuItem, x) == 0x0A);
STATIC_ASSERT(offsetof(LinkMenuItem, y) == 0x0C);
STATIC_ASSERT(offsetof(LinkMenuItem, textureAssetId) == 0x10);
STATIC_ASSERT(offsetof(LinkMenuItem, flags) == 0x16);
STATIC_ASSERT(offsetof(LinkMenuItem, upLink) == 0x1A);
#define LINK_FLAG_DRAW_SLOTS        0x0004

extern LinkMenuItem gTumbleweedBushItems[40];

u16 linkGetSelectedItemId(void)
{
    return gTumbleweedBushItems[linkSelected].boxId;
}
void linkInitTextures(LinkMenuItem* item)
{
    int budget;
    int i;

    budget = item->width;
    for (i = 0; i < LINK_ITEM_SLOTS; i++)
    {
        item->slots[i] = -1;
    }
    item->slots[(i = 1) - 1] = 0;
    budget -= linkTextures[6] + linkTextures[14];
    while (budget != 0)
    {
        if (budget >= 80)
        {
            item->slots[i] = randomGetRange(2, 5);
        }
        else if (budget >= 40)
        {
            item->slots[i] = randomGetRange(4, 5);
        }
        else
        {
            item->slots[i] = 5;
        }
        budget -= linkTextures[item->slots[i] * 8 + 6];
        i++;
    }
    item->slots[i++] = 1;
    if (i >= LINK_ITEM_SLOTS)
    {
        OSReport(sTumbleweedBushSlotOverflowErr);
    }
}
void linkDrawFn_801302c0(void)
{
    LinkMenuItem* sel;
    int resetTimer;
    void* iconTex;
    int i;
    int selLeft;
    int selRight;
    int itemLeft;
    int itemRight;
    int iconWidth;

    resetTimer = 4;
    gTumbleweedBushItems[linkSelected].timer = resetTimer;
    sel = &gTumbleweedBushItems[linkSelected];
    if (((sel->flags & LINK_FLAG_DRAW_SLOTS) != 0) && (sel->slots[0] != -1))
    {
        iconTex = *(void**)(linkTextures + sel->slots[0] * 8);
    }
    else
    {
        iconTex = sel->texture;
    }
    if (iconTex != NULL)
    {
        iconWidth = ((Texture*)iconTex)->height;
        selLeft = sel->y;
    }
    else
    {
        if (getCurLanguage() == 4)
        {
            iconWidth = *(u16*)(gGameTextFontMetrics + 0xa) + 2;
        }
        else
        {
            iconWidth = *(u16*)(gGameTextFontMetrics + 0x4a) + 2;
        }
        selLeft = sel->textLeft - 2;
    }
    selRight = selLeft + iconWidth;
    for (i = 0; i < gTumbleweedBushItemCount; i++)
    {
        if (i != linkSelected)
        {
            if (((gTumbleweedBushItems[i].flags & LINK_FLAG_DRAW_SLOTS) != 0) &&
                (gTumbleweedBushItems[i].slots[0] != -1))
            {
                iconTex = *(void**)(linkTextures + gTumbleweedBushItems[i].slots[0] * 8);
            }
            else
            {
                iconTex = gTumbleweedBushItems[i].texture;
            }
            if (iconTex != NULL)
            {
                iconWidth = ((Texture*)iconTex)->height;
                itemLeft = gTumbleweedBushItems[i].y;
            }
            else
            {
                if (getCurLanguage() == 4)
                {
                    iconWidth = *(u16*)(gGameTextFontMetrics + 0xa) + 2;
                }
                else
                {
                    iconWidth = *(u16*)(gGameTextFontMetrics + 0x4a) + 2;
                }
                itemLeft = gTumbleweedBushItems[i].textLeft - 2;
            }
            itemRight = itemLeft + iconWidth;
            if (itemLeft < selRight && itemRight > selLeft)
            {
                gTumbleweedBushItems[i].timer = resetTimer;
            }
        }
    }
}

void titleScreenFn_80130464(u8 v)
{
    linkFlag_803dd8f8 = v;
}
void setLinkNotRotated(void)
{
    linkIsRotated = 0;
}
void setLinkIsRotated(void)
{
    linkIsRotated = 1;
}

void linkDrawFn_80130484(void)
{
    LinkMenuItem* item;
    void* iconTex;
    int i;
    int minX;
    int maxX;
    int iconWidth;
    int left;
    int right;

    minX = 480;
    maxX = 0;
    i = 0;
    for (; i < gTumbleweedBushItemCount; i++)
    {
        item = &gTumbleweedBushItems[i];
        if (((item->flags & LINK_FLAG_DRAW_SLOTS) != 0) && (item->slots[0] != -1))
        {
            iconTex = *(void**)(linkTextures + item->slots[0] * 8);
        }
        else
        {
            iconTex = item->texture;
        }
        if (iconTex != NULL)
        {
            iconWidth = ((Texture*)iconTex)->height;
            left = item->y;
        }
        else
        {
            if (getCurLanguage() == 4)
            {
                iconWidth = *(u16*)(gGameTextFontMetrics + 0xa) + 2;
            }
            else
            {
                iconWidth = *(u16*)(gGameTextFontMetrics + 0x4a) + 2;
            }
            left = item->textLeft - 2;
        }
        right = left + iconWidth;
        if (left < minX)
        {
            minX = left;
        }
        if (right > maxX)
        {
            maxX = right;
        }
    }
}
void Link_func0F(void)
{
    int i;

    for (i = 0; i < gTumbleweedBushItemCount; i++)
    {
        gTumbleweedBushItems[i].timer = 4;
    }
}
void Link_copy(u8* srcArg)
{
    LinkMenuItem* dst;
    LinkMenuItem* src;
    int i;

    i = 0;
    for (; i < gTumbleweedBushItemCount; i++)
    {
        dst = &gTumbleweedBushItems[i];
        src = &((LinkMenuItem*)srcArg)[i];
        dst->flags = src->flags;
        dst->upLink = src->upLink;
        dst->rightX = src->rightX;
        if (src->textureAssetId != -1)
        {
            if (dst->texture == NULL)
            {
                dst->texture = textureLoadAsset(src->textureAssetId);
            }
        }
        else
        {
            if (dst->texture != NULL)
            {
                textureFree((Texture*)(dst->texture));
            }
            dst->texture = NULL;
        }
    }
}

u8 Link_func0C(void)
{
    return linkCount_803dd90e;
}
void Link_func0B(u8* srcArg)
{
    LinkMenuItem* src;
    int i;

    src = (LinkMenuItem*)srcArg;
    for (i = 0; i < gTumbleweedBushItemCount; i++)
    {
        gTumbleweedBushItems[i].textId = src[i].textId;
        gTumbleweedBushItems[i].boxId = src[i].boxId;
        gTumbleweedBushItems[i].timer = 2;
    }
}
void Link_func0A(int idx, int v)
{
    gTumbleweedBushItems[idx].state = v;
}

s32 Link_func09(int idx)
{
    return gTumbleweedBushItems[idx].state;
}
void Link_setOpacity(u8 v)
{
    linkItemOpacity = v;
}

typedef struct LinkTextureSlot
{
    void* texture;
    s16 assetId;
    u8 width;
    u8 pad7;
} LinkTextureSlot;

#define LINK_FLAG_DRAW_BLACK_SHADOW 0x0100
#define LINK_FLAG_DIM_OPACITY       0x0800
#define LINK_FLAG_FADE_TIMER_ONLY   0x1040
#define LINK_FLAG_HIDDEN            0x4000
#define LINK_FLAG_SELECTED_COLOR    0x0080

void Link_setSelected(int v)
{
    linkSelected = v;
}

#define LINK_FLAG_DISABLE_NAV_TO 0x1000
#define LINK_FLAG_NO_ACCEPT      0x0020
#define LINK_FLAG_INHERIT_X      0x0008
#define LINK_FLAG_NO_SLOTS       0x0010
#define LINK_FLAG_CENTRE         0x0400
#define LINK_IS_NAVIGABLE(index) ((gTumbleweedBushItems[(index)].flags & LINK_FLAG_DISABLE_NAV_TO) == 0)

s32 Link_getSelected(void)
{
    return linkSelected;
}

void Link_render(void)
{
    LinkMenuItem* item;
    int i;
    int slotIndex;
    LinkMenuItem* drawItem;
    int textureIndex;
    int opacity;
    int alpha;
    s16 red;
    s16 green;
    s16 blue;
    u16 textId;
    int x;
    int y;
    s8 timer;

    for (i = 0; i < gTumbleweedBushItemCount; i++)
    {
        item = (LinkMenuItem*)&gTumbleweedBushItems[i];
        drawItem = item;

        if ((item->flags & LINK_FLAG_HIDDEN) == 0)
        {
            if ((item->flags & LINK_FLAG_FADE_TIMER_ONLY) != 0)
            {
                timer = (item->timer -= 1);
                if (timer < 0)
                {
                    item->timer = 0;
                }
            }
            else
            {
                if (item->state != -1)
                {
                    drawItem = (LinkMenuItem*)&gTumbleweedBushItems[item->state];
                }

                if ((drawItem->flags & LINK_FLAG_DRAW_SLOTS) != 0)
                {
                    slotIndex = 0;
                    x = drawItem->x;
                    y = drawItem->y;
                    while (drawItem->slots[slotIndex] != -1 && slotIndex < LINK_ITEM_SLOTS)
                    {
                        textureIndex = drawItem->slots[slotIndex];
                        drawTexture(((LinkTextureSlot*)linkTextures)[textureIndex].texture, x, y, 0xff, 0x100);
                        x += ((LinkTextureSlot*)linkTextures)[drawItem->slots[slotIndex]].width;
                        slotIndex++;
                    }
                }

                if ((drawItem->flags & LINK_FLAG_DIM_OPACITY) != 0)
                {
                    opacity = linkItemOpacity * 200 >> 8;
                }
                else
                {
                    opacity = linkItemOpacity;
                }

                MWTRACE(drawItem->boxId);
                if (linkSelected == i)
                {
                    alpha = opacity;
                }
                else
                {
                    alpha = (((int)((u32)opacity >> 31)) + opacity) >> 1;
                }
                *(u8*)((char*)gameTextGetBox(drawItem->boxId) + 0x1e) = alpha;

                if ((drawItem->flags & LINK_FLAG_DRAW_BLACK_SHADOW) != 0)
                {
                    gameTextSetColor(0, 0, 0, (u8)(((linkCount_803dd90e + 1) * linkItemOpacity) >> 8));
                    gameTextFn_80016810(drawItem->textId, 2, 2);
                }

                if ((drawItem->flags & LINK_FLAG_SELECTED_COLOR) != 0)
                {
                    if (linkSelected == i)
                    {
                        red = gTumbleweedBushBaseColorR +
                              ((linkCount_803dd90e * (gTumbleweedBushSelColorR - gTumbleweedBushBaseColorR)) >> 8);
                        green = gTumbleweedBushBaseColorG +
                                ((linkCount_803dd90e * (gTumbleweedBushSelColorG - gTumbleweedBushBaseColorG)) >> 8);
                        blue = gTumbleweedBushBaseColorB +
                               ((linkCount_803dd90e * (gTumbleweedBushSelColorB - gTumbleweedBushBaseColorB)) >> 8);
                        if ((drawItem->flags & LINK_FLAG_DIM_OPACITY) != 0)
                        {
                            alpha = linkItemOpacity * 200 >> 8;
                        }
                        else
                        {
                            alpha = linkItemOpacity * 256 >> 8;
                        }
                        gameTextSetColor((u8)red, (u8)green, (u8)blue, (u8)alpha);
                    }
                    else
                    {
                        gameTextSetColor((u8)gTumbleweedBushBaseColorR, (u8)gTumbleweedBushBaseColorG,
                                         (u8)gTumbleweedBushBaseColorB,
                                         (u8)((((int)((u32)opacity >> 31)) + opacity) >> 1));
                    }
                }
                else
                {
                    gameTextSetColor(0xff, 0xff, 0xff, (u8)opacity);
                }

                textId = drawItem->textId;
                if (textId > 0x14 && textId != 0xffff)
                {
                    gameTextShow(textId);
                }
                else if (textId != 0xffff)
                {
                    gameTextShowStr((char*)saveFileSelect_saveSlots + textId * 0x24, drawItem->boxId, 0, 0);
                }

                if (drawItem->texture != NULL)
                {
                    if ((drawItem->flags & LINK_FLAG_DRAW_SLOTS) != 0)
                    {
                        if ((drawItem->flags & LINK_FLAG_DIM_OPACITY) != 0)
                        {
                            alpha = linkItemOpacity * 200 >> 8;
                        }
                        else
                        {
                            alpha = linkItemOpacity;
                        }
                        drawTexture(drawItem->texture, (f32)(drawItem->x + 11), drawItem->y, alpha & 0xff, 0x100);
                    }
                    else
                    {
                        if ((drawItem->flags & LINK_FLAG_DIM_OPACITY) != 0)
                        {
                            alpha = linkItemOpacity * 200 >> 8;
                        }
                        else
                        {
                            alpha = linkItemOpacity;
                        }
                        drawTexture(drawItem->texture, drawItem->x, drawItem->y, alpha & 0xff, 0x100);
                    }
                }

                timer = (drawItem->timer -= 1);
                if (timer < 0)
                {
                    drawItem->timer = 0;
                }
            }
        }
    }

    MWTRACE(0xff);
}

u32 Link_update(void)
{
    int result;
    LinkMenuItem* item;
    u32 buttons;
    u8 acceptPressed;
    s8 horizontalInput;
    s8 verticalInput;

    item = &gTumbleweedBushItems[linkSelected];
    if (gTumbleweedBushItemCount == 0)
    {
        return -1;
    }

    result = -1;
    if (getHudHiddenFrameCount() != 0)
    {
        return -1;
    }

    padGetAnalogInput(0, &horizontalInput, &verticalInput);
    if (linkIsRotated != 0)
    {
        s8 oldHorizontal = horizontalInput;
        horizontalInput = verticalInput;
        verticalInput = (s8)-oldHorizontal;
    }

    if (verticalInput != 0)
    {
        horizontalInput = 0;
    }

    if (((horizontalInput != 0) || (verticalInput != 0)) && (linkFlag_803dd8f8 != 0))
    {
        if ((verticalInput < 0) && (item->downLink != -1) && LINK_IS_NAVIGABLE(item->downLink))
        {
            padClearAnalogInputY(0);
            linkSelected = item->downLink;
            linkCount_803dd90e = 0xff;
        }
        else if ((verticalInput > 0) && (item->upLink != -1) && LINK_IS_NAVIGABLE(item->upLink))
        {
            padClearAnalogInputY(0);
            linkSelected = item->upLink;
            linkCount_803dd90e = 0xff;
        }

        if (item->state != -1)
        {
            item = &gTumbleweedBushItems[item->state];
            if ((horizontalInput < 0) && (item->leftLink != -1))
            {
                padClearAnalogInputX(0);
                gTumbleweedBushItems[linkSelected].state = item->leftLink;
                linkCount_803dd90e = 0xff;
            }
            else if ((horizontalInput > 0) && (item->rightLink != -1))
            {
                padClearAnalogInputX(0);
                gTumbleweedBushItems[linkSelected].state = item->rightLink;
                linkCount_803dd90e = 0xff;
            }
        }
        else
        {
            if ((horizontalInput < 0) && (item->leftLink != -1) && LINK_IS_NAVIGABLE(item->leftLink))
            {
                padClearAnalogInputX(0);
                linkSelected = item->leftLink;
                linkCount_803dd90e = 0xff;
            }
            else if ((horizontalInput > 0) && (item->rightLink != -1) && LINK_IS_NAVIGABLE(item->rightLink))
            {
                padClearAnalogInputX(0);
                linkSelected = item->rightLink;
                linkCount_803dd90e = 0xff;
            }
        }

        if (linkSelected < 0)
        {
            linkSelected = (s8)(gTumbleweedBushItemCount - 1);
        }
        if (linkSelected >= gTumbleweedBushItemCount)
        {
            linkSelected = 0;
        }
    }

    if (gTumbleweedBushInputEnabled[0] != 0)
    {
        buttons = getButtonsJustPressed(0);
        acceptPressed = 0;
        if ((int)(buttons & PAD_ACCEPT_MASK) != 0)
        {
            acceptPressed = 1;
        }
        if (acceptPressed)
        {
            if (((gTumbleweedBushItems[linkSelected].flags & LINK_FLAG_NO_ACCEPT) == 0) &&
                (mainGetBit(GAMEBIT_MenuRelated044F) == 0))
            {
                buttonDisable(0, PAD_ACCEPT_MASK);
                result = 1;
            }
        }
        else if ((int)(buttons & PAD_BUTTON_B) != 0)
        {
            buttonDisable(0, PAD_BUTTON_B);
            result = 0;
        }
    }

    if (gTumbleweedBushPulseDir != 0)
    {
        linkCount_803dd90e = (s16)(linkCount_803dd90e + framesThisStep * 5);
    }
    else
    {
        linkCount_803dd90e = (s16)(linkCount_803dd90e - framesThisStep * 5);
    }

    if (linkCount_803dd90e > 0xff)
    {
        linkCount_803dd90e = (s16)(0xff - (linkCount_803dd90e - 0xff));
        gTumbleweedBushPulseDir = (s8)(*(s8*)&gTumbleweedBushPulseDir ^ 1);
    }
    else if (linkCount_803dd90e < 0)
    {
        linkCount_803dd90e = (s16)-linkCount_803dd90e;
        gTumbleweedBushPulseDir = (s8)(*(s8*)&gTumbleweedBushPulseDir ^ 1);
    }

    gTumbleweedBushInputEnabled[0] = 1;
    linkDrawFn_801302c0();
    linkDrawFn_80130484();
    return result;
}


void Link_free(void)
{
    int i;

    for (i = 0; i < gTumbleweedBushItemCount; i++)
    {
        if (gTumbleweedBushItems[i].texture != NULL)
        {
            textureFree((Texture*)(gTumbleweedBushItems[i].texture));
        }
    }
    gTumbleweedBushItemCount = 0;
}
void Link_setup(LinkMenuItem* items, int count, int selected, const char* defaultMessage, int unused1, int unused2,
                int baseRed, int baseGreen, int baseBlue, int selectedRed, int selectedGreen, int selectedBlue)
{
    int i;
    LinkMenuItem* item;
    const char* defaultText;
    const char* errBase;

    errBase = sTumbleweedBushNavLinkRangeErr;
    defaultText = errBase;
    if (count <= 40)
    {
        gTumbleweedBushItemCount = count;
        linkCount_803dd90e = 0xff;
        linkSelected = selected;
        gTumbleweedBushPulseDir = 0;
        gTumbleweedBushInputEnabled[0] = 0;

        memcpy(gTumbleweedBushItems, items, count * sizeof(LinkMenuItem));

        for (i = 0; i < count; i++)
        {
            item = &gTumbleweedBushItems[i];
            if ((item->upLink < -1) || (item->upLink >= count))
            {
                OSReport(errBase + 0xa4, item->upLink);
            }

            if ((item->downLink < -1) || (item->downLink >= count))
            {
                OSReport(errBase + 0xb8, item->downLink);
            }

            if ((item->leftLink < -1) || (item->leftLink >= count))
            {
                OSReport(errBase + 0xd0, item->leftLink);
            }

            if ((item->rightLink < -1) || (item->rightLink >= count))
            {
                OSReport(errBase + 0xe8, item->rightLink);
            }

            if (items[i].textureAssetId != -1)
            {
                item->texture = textureLoadAsset(items[i].textureAssetId);
            }
            else
            {
                item->texture = NULL;
            }

            if ((item->flags & LINK_FLAG_NO_SLOTS) != 0)
            {
                item->width = 0;
                item->slotWidth = 0;
            }

            if ((item->flags & LINK_FLAG_DRAW_SLOTS) != 0)
            {
                linkInitTextures((LinkMenuItem*)item);
            }

            if ((item->leftLink != -1) && ((item->flags & LINK_FLAG_INHERIT_X) != 0))
            {
                LinkMenuItem* linked = &gTumbleweedBushItems[item->leftLink];
                item->x = linked->x + linked->width;
                item->rightX = linked->rightX + linked->width;
            }

            if ((item->flags & LINK_FLAG_CENTRE) != 0)
            {
                item->x -= item->width >> 1;
                item->rightX = item->x;
            }

            item->timer = 4;
        }

        gTumbleweedBushBaseColorR = baseRed;
        gTumbleweedBushBaseColorG = baseGreen;
        gTumbleweedBushBaseColorB = baseBlue;
        gTumbleweedBushSelColorR = selectedRed;
        gTumbleweedBushSelColorG = selectedGreen;
        gTumbleweedBushSelColorB = selectedBlue;
        if (defaultMessage != NULL)
        {
            defaultText = defaultMessage;
        }
        gTumbleweedBushDefaultText = defaultText;
    }
}

void Link_release(void)
{
    int i;

    for (i = 0; i < 6; i++)
    {
        textureFree((Texture*)(((LinkTextureSlot*)linkTextures)[i].texture));
    }
    subtitleFreeBoxTextures(3);
}
void Link_initialise(void)
{
    int i;

    for (i = 0; i < 6; i++)
    {
        ((LinkTextureSlot*)linkTextures)[i].texture = textureLoadAsset(((LinkTextureSlot*)linkTextures)[i].assetId);
    }

    padSetStickRepeatDelay(10);
    linkItemOpacity = 0xff;
    subtitleLoadBoxTextures(3);
    linkIsRotated = 0;
    linkFlag_803dd8f8 = 1;
}

LinkMenuItem gTumbleweedBushItems[40];

char sTumbleweedBushNavLinkRangeErr[] = {
    0x00, 0x00, 0x00, 0xF9, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x03, 0x71,
};

u8 linkTextures[0x30] = {
    0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x15, 0x28, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x17, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x19, 0x50, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x18, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1A, 0x14, 0x00,
};

struct LinkObjDescriptor
{
    int unk00[3];
    int unk0C;
    void* fns[16];
};

struct LinkObjDescriptor lbl_8031C1E4 = {
    {0, 0, 0},
    0x000F0000,
    {
        Link_initialise,
        Link_release,
        NULL,
        Link_setup,
        Link_free,
        Link_update,
        Link_render,
        Link_getSelected,
        Link_setSelected,
        Link_func09,
        Link_func0A,
        Link_func0B,
        Link_func0C,
        Link_copy,
        Link_setOpacity,
        Link_func0F,
    },
};

char sTumbleweedBushSlotOverflowErr[] = {
    0x50, 0x49, 0x43, 0x4D, 0x45, 0x4E, 0x55, 0x3A, 0x20, 0x74, 0x65, 0x78, 0x20, 0x6F, 0x76, 0x65, 0x72,
    0x66, 0x6C, 0x6F, 0x77, 0x0A, 0x00, 0x00, 0x55, 0x50, 0x4C, 0x49, 0x4E, 0x4B, 0x20, 0x6F, 0x76, 0x65,
    0x72, 0x66, 0x6C, 0x6F, 0x77, 0x3D, 0x25, 0x64, 0x0A, 0x00, 0x44, 0x4F, 0x57, 0x4E, 0x4C, 0x49, 0x4E,
    0x4B, 0x20, 0x6F, 0x76, 0x65, 0x72, 0x66, 0x6C, 0x6F, 0x77, 0x3D, 0x25, 0x64, 0x0A, 0x00, 0x00, 0x00,
    0x4C, 0x45, 0x46, 0x54, 0x4C, 0x49, 0x4E, 0x4B, 0x20, 0x6F, 0x76, 0x65, 0x72, 0x66, 0x6C, 0x6F, 0x77,
    0x3D, 0x25, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x52, 0x49, 0x47, 0x48, 0x54, 0x4C, 0x49, 0x4E, 0x4B, 0x20,
    0x6F, 0x76, 0x65, 0x72, 0x66, 0x6C, 0x6F, 0x77, 0x3D, 0x25, 0x64, 0x0A, 0x00, 0x00,
};

