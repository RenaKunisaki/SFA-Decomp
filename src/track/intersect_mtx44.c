#include "global.h"

extern f32 lbl_803DEE98;
extern f32 lbl_803DEE9C;

/* 4x4 identity fill. */
void mtx44Identity(f32* mat)
{
    int i = 0, j;
    f32 zero, one;
    one = lbl_803DEE98;
    zero = lbl_803DEE9C;
    for (; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (i == j)
                mat[j] = one;
            else
                mat[j] = zero;
        }
        mat += 4;
    }
}
