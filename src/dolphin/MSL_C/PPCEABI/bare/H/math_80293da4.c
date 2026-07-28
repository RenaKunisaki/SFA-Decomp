#include "dolphin.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "main/trig.h"

extern float lbl_803E7D74;
extern float lbl_803E7D78;
extern float lbl_803E7D7C;
extern float lbl_803E7D80;
extern float lbl_803E7D84;
extern float lbl_803E7D88;
extern float lbl_803E7D8C;
extern float lbl_803E7D90;
extern float lbl_803E7D94;
extern float lbl_803E7D98;
extern float lbl_803E7D9C;
extern float lbl_803E7DA0;
extern float lbl_803E7DA4;
extern float lbl_803E7DA8;
extern float lbl_803E7DAC;

float mathSinfFast(float angle) {
    u16 quadrant;
    float reducedAngle;
    float reducedSquared;

    reducedAngle = trigReduceQuadrant(&quadrant, angle);
    quadrant += (*(u32*)&angle & 0x80000000) >> 29;
    reducedSquared = reducedAngle * reducedAngle;

    switch (quadrant & 6) {
        case 0:
            return reducedAngle * (-0.07768012f * reducedSquared + 0.7846358f);
        case 2:
            return reducedSquared * (0.015371595f * reducedSquared + -0.30824488f) + 0.99999f;
        case 4:
            return -(reducedAngle * (-0.07768012f * reducedSquared + 0.7846358f));
        default:
            return -(reducedSquared * (0.015371595f * reducedSquared + -0.30824488f) + 0.99999f);
    }
}

float mathSinf(float angle) {
    u16 quadrant;
    float reducedAngle;
    float reducedSquared;

    reducedAngle = trigReduceQuadrant(&quadrant, angle);
    quadrant += (*(u32*)&angle & 0x80000000) >> 29;
    reducedSquared = reducedAngle * reducedAngle;

    switch (quadrant & 6) {
        case 0:
            return reducedAngle * ((lbl_803E7D7C * reducedSquared + lbl_803E7D78) * reducedSquared + lbl_803E7D74);
        case 2:
            return ((lbl_803E7D8C * reducedSquared + lbl_803E7D88) * reducedSquared + lbl_803E7D84) * reducedSquared + lbl_803E7D80;
        case 4:
            return -(reducedAngle * ((lbl_803E7D7C * reducedSquared + lbl_803E7D78) * reducedSquared + lbl_803E7D74));
        default:
            return -(reducedSquared * ((lbl_803E7D8C * reducedSquared + lbl_803E7D88) * reducedSquared + lbl_803E7D84) + lbl_803E7D80);
    }
}

float mathSinfPrecise(float angle) {
    u16 quadrant;
    float reducedAngle;
    float reducedSquared;

    reducedAngle = trigReduceQuadrant(&quadrant, angle);
    quadrant += (*(u32*)&angle & 0x80000000) >> 29;
    reducedSquared = reducedAngle * reducedAngle;

    switch (quadrant & 6) {
        case 0:
            return reducedAngle * (((lbl_803E7D9C * reducedSquared + lbl_803E7D98) * reducedSquared + lbl_803E7D94) * reducedSquared + lbl_803E7D90);
        case 2:
            return (((lbl_803E7DAC * reducedSquared + lbl_803E7DA8) * reducedSquared + lbl_803E7DA4) * reducedSquared + lbl_803E7DA0) * reducedSquared
                   + lbl_803E7D80;
        case 4:
            return -(reducedAngle * (((lbl_803E7D9C * reducedSquared + lbl_803E7D98) * reducedSquared + lbl_803E7D94) * reducedSquared + lbl_803E7D90));
        default:
            return -(reducedSquared * (((lbl_803E7DAC * reducedSquared + lbl_803E7DA8) * reducedSquared + lbl_803E7DA4) * reducedSquared + lbl_803E7DA0)
                     + lbl_803E7D80);
    }
}
