#include "dolphin.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"

extern const float lbl_803E7D40;
extern const float lbl_803E7D44;
extern const float lbl_803E7D48;
extern const float lbl_803E7D4C;
extern const float lbl_803E7D50;
extern const float lbl_803E7D54;
extern const float lbl_803E7D58;
extern const float lbl_803E7D5C;

void mathSinCosf(float x, float* outSin, float* outCos) {
    u16 quadrant;
    float reducedAngle = trigReduceQuadrant(&quadrant, x);
    float reducedSquared = reducedAngle * reducedAngle;
    float sinApprox =
        reducedAngle * (reducedSquared * (lbl_803E7D4C * reducedSquared + lbl_803E7D48) + lbl_803E7D44);
    float cosApprox =
        reducedSquared * (reducedSquared * (lbl_803E7D5C * reducedSquared + lbl_803E7D58) + lbl_803E7D54) +
        lbl_803E7D50;

    switch (quadrant & 6) {
        case 0:
            sinApprox = (x >= lbl_803E7D40) ? sinApprox : -sinApprox;
            *outSin = sinApprox;
            *outCos = cosApprox;
            break;
        case 2:
            cosApprox = (x >= lbl_803E7D40) ? cosApprox : -cosApprox;
            *outSin = cosApprox;
            *outCos = -sinApprox;
            break;
        case 4:
            if (x >= lbl_803E7D40) {
                sinApprox = -sinApprox;
            }
            *outSin = sinApprox;
            *outCos = -cosApprox;
            break;
        default:
            if (x >= lbl_803E7D40) {
                cosApprox = -cosApprox;
            }
            *outSin = cosApprox;
            *outCos = sinApprox;
            break;
    }
}

const float lbl_803E7D40 = 0.0f;
const float lbl_803E7D44 = 0.78539425f;
const float lbl_803E7D48 = -0.08071397f;
const float lbl_803E7D4C = 0.0024270867f;
const float lbl_803E7D50 = 1.0f;
const float lbl_803E7D54 = -0.30842426f;
const float lbl_803E7D58 = 0.015849913f;
const float lbl_803E7D5C = -0.000318879f;
