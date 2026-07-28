#include "global.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/mtx.h"
#include "track/intersect.h"
#include "track/intersect_depth_read_api.h"
#include "main/trig.h"
#include "main/vecmath.h"


static const f32 gGxPi = 3.1415927f;

#include "track/intersect_internal.h"

extern int lbl_803DD03C;
extern int lbl_803968C0[];

void mtx44Identity(f32* mat);

/* Queues a GXPeekZ read at (x,y) tagged by an opaque requestKey (callers pass
 * any unique value - object ptrs, loop indices, even a function address) and
 * returns the previously completed result for that key, 0 until ready. */
int depthReadRequestPoll(int x, int y, void* requestKey)
{
    bool ok;
    int i;
    int key = (int)requestKey;
    u16 n;

    ok = false;
    if (x >= 0 && x < 0x280 && y >= 0 && y < 0x1E0)
    {
        ok = true;
    }
    if (ok)
    {
        if (x < 0x10)
            x = 0x10;
        if (y < 6)
            y = 6;
        n = gDepthReadPendingCount;
        if (n < 0x14)
        {
            gDepthReadPendingQueue[n].x = x;
            gDepthReadPendingQueue[n].y = y;
            gDepthReadPendingQueue[n].key = key;
            gDepthReadPendingCount++;
        }
        i = 0;
        n = gDepthReadResultCount;
        for (; (u32)i < n; i++)
        {
            if (key == gDepthReadResults[i].key)
            {
                return gDepthReadResults[i].value;
            }
        }
        return 0;
    }
    return 0;
}

u32 getScreenResolution(void)
{
    u32 v = screenWidth;
    if (v != 0)
    {
        return v | (v << 16);
    }
    return 0x01E00280;
}

void setScreenWidth(u32 width)
{
    screenWidth = width;
}

void clearScreenWidth(void)
{
    screenWidth = 0;
}

void matrixFn_8006ff0c(float* mat, short* out, f32 fov, f32 aspect, f32 near, f32 far, f32 scale)
{
    f32 angle;
    f32 tan;
    int i;

    mtx44Identity((f32*)mat);

    angle = (f32)(s32)(91.022f * fov) * gGxPi / 32768.0f;
    tan = mathCosf(angle) / mathSinf(angle);
    mat[0] = tan / aspect;
    mat[5] = tan;
    mat[10] = -near / (far - near);
    mat[11] = -1.0f;
    mat[14] = -near * far / (far - near);
    mat[15] = 0.0f;

    for (i = 0; i < 16; i++)
    {
        mat[i] *= scale;
    }

    if (out != NULL)
    {
        if ((f32)(near + far) <= 2.0f)
        {
            *(u16*)out = 0xFFFF;
        }
        else
        {
            *(s16*)out = (s16)(131072.0f / (near + far));
            if (*(u16*)out == 0)
            {
                *out = 1;
            }
        }
    }
    gFogNearZ = __fabs(near);
    gFogFarZ = __fabs(far);
    C_MTXPerspective((void*)lbl_803968C0, fov, aspect, gFogNearZ, gFogFarZ);
    lbl_803DD03C = 0;
}

void normalize(f32* x, f32* y, f32* z)
{
    f32 scale;
    f32 len;

    len = sqrtf(*z * *z + (*x * *x + *y * *y));
    scale = 1.0f / len;
    *x = *x * scale;
    *y = *y * scale;
    *z = *z * scale;
}
