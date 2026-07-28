#include "dolphin/types.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/k_tan.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/trig_float_helpers.h"
extern float lbl_803E79C0;
extern float lbl_803E79C4;
extern float lbl_803E79C8;
extern float lbl_803E79CC;
extern float lbl_803E79D0;
extern float lbl_803E79D4;
extern float lbl_803E79D8;
extern float lbl_803E79E8;
extern float lbl_803E79EC;
extern float lbl_803E79F0;
extern float lbl_803E79F4;
extern float lbl_803E79F8;
extern float lbl_803E79FC;
extern double lbl_803E79E0;
extern double lbl_803E7A00;
extern float lbl_803E7A08;
extern float lbl_803E7A0C;
extern float lbl_803E7A10;
extern float lbl_803E7A14;
extern float lbl_803E7A18;
extern float lbl_803E7A1C;
extern float lbl_803E7A20;
extern float lbl_803E7A24;
extern float lbl_803E7A28;
extern double lbl_803E7A30;
extern double lbl_803E7A38;
extern double lbl_803E7A40;
extern double lbl_803E7A48;
extern double lbl_803E7A50;
extern double lbl_803E7A58;
extern double lbl_803E7A60;
extern double lbl_803E7A68;
extern double lbl_803E7A70;
extern double lbl_803E7A78;
extern double lbl_803E7A80;
extern double lbl_803E7A88;
extern double lbl_803E7A90;
extern double lbl_803E7A98;
extern double lbl_803E7AA0;
extern double lbl_803E7AA8;
extern double lbl_803E7AB0;


float asinf(float value) {
    float absoluteValue = __fabsf(value);
    float reduced;
    float polynomial;
    float root;

    if (absoluteValue <= lbl_803E79C0) {
        reduced = value * value;
        return value * (lbl_803E79D4 * reduced + lbl_803E79D0);
    }

    reduced = lbl_803E79C0 - lbl_803E79C0 * absoluteValue;
    root = sqrtf_8029312c(reduced);
    polynomial = root * (lbl_803E79D4 * reduced + lbl_803E79D0);
    if (value >= lbl_803E79C4) {
        return lbl_803E79C8 - lbl_803E79CC * polynomial;
    }
    return lbl_803E79CC * polynomial - lbl_803E79C8;
}

float acosf_fast(float value) {
    float absoluteValue = __fabsf(value);
    float reduced;
    float polynomial;
    float root;

    if (absoluteValue <= lbl_803E79C0) {
        reduced = value * value;
        return lbl_803E79C8 - value * (lbl_803E79D4 * reduced + lbl_803E79D0);
    }

    reduced = lbl_803E79C0 - lbl_803E79C0 * absoluteValue;
    root = sqrtf_8029312c(reduced);
    polynomial = root * (lbl_803E79D4 * reduced + lbl_803E79D0);
    if (value >= lbl_803E79C4) {
        return lbl_803E79CC * polynomial;
    }
    return lbl_803E79E8 - lbl_803E79CC * polynomial;
}

float acosf(float value) {
    float absoluteValue = __fabsf(value);
    float reduced;
    float polynomial;
    float root;

    if (absoluteValue <= lbl_803E79C0) {
        reduced = value * value;
        return lbl_803E79C8 - value * (((((lbl_803E79FC * reduced + lbl_803E79F8) * reduced + lbl_803E79F4) * reduced
                                      + lbl_803E79F0) * reduced + lbl_803E79EC) * reduced + lbl_803E79D8);
    }

    reduced = lbl_803E79C0 - lbl_803E79C0 * absoluteValue;
    root = sqrtf_8029312c(reduced);
    polynomial = root
        * (((((lbl_803E79FC * reduced + lbl_803E79F8) * reduced + lbl_803E79F4) * reduced + lbl_803E79F0) * reduced
            + lbl_803E79EC) * reduced + lbl_803E79D8);
    if (value >= lbl_803E79C4) {
        return lbl_803E79CC * polynomial;
    }
    return lbl_803E79E8 - lbl_803E79CC * polynomial;
}

float atanf_fast(float value) {
    float absoluteValue = __fabsf(value);
    float reciprocal;
    float squared;
    float polynomial;
    float positiveResult;
    float negativeResult;

    if (absoluteValue <= lbl_803E79D8) {
        squared = value * value;
        return value * ((lbl_803E7A18 * squared + lbl_803E7A14) * squared + lbl_803E7A10);
    }

    reciprocal = fastReciprocal(absoluteValue);
    squared = reciprocal * reciprocal;
    polynomial = (lbl_803E7A18 * squared + lbl_803E7A14) * squared + lbl_803E7A10;
    positiveResult = lbl_803E79C8 - reciprocal * polynomial;
    negativeResult = reciprocal * polynomial - lbl_803E79C8;
    if (value >= lbl_803E79C4) {
        return positiveResult;
    }
    return negativeResult;
}

float atanf(float value) {
    float absoluteValue = __fabsf(value);
    double reduced;
    double squared;
    float result;

    if (absoluteValue <= lbl_803E79D8) {
        squared = value * value;
        return (float)(value * (((((((((((((((lbl_803E7AA8 * squared + lbl_803E7AA0) * squared + lbl_803E7A98) * squared
                                       + lbl_803E7A90) * squared + lbl_803E7A88) * squared + lbl_803E7A80) * squared
                                    + lbl_803E7A78) * squared + lbl_803E7A70) * squared + lbl_803E7A68) * squared
                                 + lbl_803E7A60) * squared + lbl_803E7A58) * squared + lbl_803E7A50) * squared
                              + lbl_803E7A48) * squared + lbl_803E7A40) * squared + lbl_803E7A38) * squared
                           + lbl_803E7A30));
    }

    squared = (reduced = lbl_803E7AB0 / absoluteValue) * reduced;
    result = (float)(lbl_803E79E0
                     - reduced * (((((((((((((((lbl_803E7AA8 * squared + lbl_803E7AA0) * squared + lbl_803E7A98) * squared
                                        + lbl_803E7A90) * squared + lbl_803E7A88) * squared + lbl_803E7A80) * squared
                                     + lbl_803E7A78) * squared + lbl_803E7A70) * squared + lbl_803E7A68) * squared
                                  + lbl_803E7A60) * squared + lbl_803E7A58) * squared + lbl_803E7A50) * squared
                               + lbl_803E7A48) * squared + lbl_803E7A40) * squared + lbl_803E7A38) * squared
                            + lbl_803E7A30));
    if (value >= lbl_803E79C4) {
        return result;
    }
    return -result;
}

typedef union FloatWord {
    float value;
    u32 bits;
} FloatWord;

#define ATAN_SIGNS_POS_X_POS_Y 0x00000000
#define ATAN_SIGNS_POS_X_NEG_Y 0x80000000
#define ATAN_SIGNS_NEG_X_POS_Y 0x40000000

static inline u32 float_bits(const float *value) {
    return ((const FloatWord *)value)->bits;
}

float __kernel_cos(float y, float x) {
    float absoluteX = __fabsf(x);
    float absoluteY = __fabsf(y);
    float axisRatio;
    float ratioSquared;
    float firstQuadrantAngle;
    s32 quadrantSigns;

    if (absoluteX > absoluteY) {
        axisRatio = absoluteY / absoluteX;
        ratioSquared = axisRatio * axisRatio;
        firstQuadrantAngle = axisRatio * (lbl_803E7A0C * ratioSquared + lbl_803E7A08);
    } else {
        axisRatio = absoluteX / absoluteY;
        ratioSquared = axisRatio * axisRatio;
        firstQuadrantAngle = lbl_803E79C8 - axisRatio * (lbl_803E7A0C * ratioSquared + lbl_803E7A08);
    }

    quadrantSigns = (float_bits(&y) & 0x80000000) | ((float_bits(&x) & 0x80000000) >> 1);
    switch (quadrantSigns) {
        case ATAN_SIGNS_POS_X_POS_Y:
            return firstQuadrantAngle;
        case ATAN_SIGNS_POS_X_NEG_Y:
            return -firstQuadrantAngle;
        case ATAN_SIGNS_NEG_X_POS_Y:
            return lbl_803E79E8 - firstQuadrantAngle;
        default:
            return firstQuadrantAngle - lbl_803E79E8;
    }
}

float atan2f(float y, float x) {
    float absoluteX = __fabsf(x);
    float absoluteY = __fabsf(y);
    float axisRatio;
    float ratioSquared;
    float firstQuadrantAngle;
    int quadrantSigns;

    if (absoluteX > absoluteY) {
        axisRatio = absoluteY / absoluteX;
        ratioSquared = axisRatio * axisRatio;
        firstQuadrantAngle = axisRatio * (((lbl_803E7A28 * ratioSquared + lbl_803E7A24) * ratioSquared + lbl_803E7A20) * ratioSquared + lbl_803E7A1C);
    } else {
        axisRatio = absoluteX / absoluteY;
        ratioSquared = axisRatio * axisRatio;
        firstQuadrantAngle = lbl_803E79C8 - axisRatio * (((lbl_803E7A28 * ratioSquared + lbl_803E7A24) * ratioSquared + lbl_803E7A20) * ratioSquared + lbl_803E7A1C);
    }

    quadrantSigns = (float_bits(&y) & 0x80000000) | ((float_bits(&x) & 0x80000000) >> 1);
    switch (quadrantSigns) {
        case ATAN_SIGNS_POS_X_POS_Y:
            return firstQuadrantAngle;
        case ATAN_SIGNS_POS_X_NEG_Y:
            return -firstQuadrantAngle;
        case ATAN_SIGNS_NEG_X_POS_Y:
            return lbl_803E79E8 - firstQuadrantAngle;
        default:
            return firstQuadrantAngle - lbl_803E79E8;
    }
}

float atan2fHighPrecision(float y, float x) {
    float absoluteX = __fabsf(x);
    float absoluteY = __fabsf(y);
    double axisRatio;
    double ratioSquared;
    double firstQuadrantAngle;
    int quadrantSigns;

    if (absoluteX >= absoluteY) {
        axisRatio = absoluteY / absoluteX;
        ratioSquared = axisRatio * axisRatio;
        firstQuadrantAngle = axisRatio * (((((((((((((((lbl_803E7AA8 * ratioSquared + lbl_803E7AA0) * ratioSquared + lbl_803E7A98) * ratioSquared + lbl_803E7A90) * ratioSquared
                       + lbl_803E7A88) * ratioSquared + lbl_803E7A80) * ratioSquared + lbl_803E7A78) * ratioSquared + lbl_803E7A70) * ratioSquared
                    + lbl_803E7A68) * ratioSquared + lbl_803E7A60) * ratioSquared + lbl_803E7A58) * ratioSquared + lbl_803E7A50) * ratioSquared
                 + lbl_803E7A48) * ratioSquared + lbl_803E7A40) * ratioSquared + lbl_803E7A38) * ratioSquared + lbl_803E7A30);
    } else {
        axisRatio = absoluteX / absoluteY;
        ratioSquared = axisRatio * axisRatio;
        firstQuadrantAngle = lbl_803E79E0 - axisRatio * (((((((((((((((lbl_803E7AA8 * ratioSquared + lbl_803E7AA0) * ratioSquared + lbl_803E7A98) * ratioSquared + lbl_803E7A90) * ratioSquared
                       + lbl_803E7A88) * ratioSquared + lbl_803E7A80) * ratioSquared + lbl_803E7A78) * ratioSquared + lbl_803E7A70) * ratioSquared
                    + lbl_803E7A68) * ratioSquared + lbl_803E7A60) * ratioSquared + lbl_803E7A58) * ratioSquared + lbl_803E7A50) * ratioSquared
                 + lbl_803E7A48) * ratioSquared + lbl_803E7A40) * ratioSquared + lbl_803E7A38) * ratioSquared + lbl_803E7A30);
    }

    quadrantSigns = (float_bits(&y) & 0x80000000) | ((float_bits(&x) & 0x80000000) >> 1);
    switch (quadrantSigns) {
        case ATAN_SIGNS_POS_X_POS_Y:
            return (float)firstQuadrantAngle;
        case ATAN_SIGNS_POS_X_NEG_Y:
            return (float)-firstQuadrantAngle;
        case ATAN_SIGNS_NEG_X_POS_Y:
            return (float)(lbl_803E7A00 - firstQuadrantAngle);
        default:
            return (float)(firstQuadrantAngle - lbl_803E7A00);
    }
}
