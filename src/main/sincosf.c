#include "dolphin.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"


void mathSinCosf(float x, float* outSin, float* outCos) {
    u16 quadrant;
    float reducedAngle = trigReduceQuadrant(&quadrant, x);
    float reducedSquared = reducedAngle * reducedAngle;
    float sinApprox = reducedAngle * (reducedSquared * (0.0024270867f * reducedSquared + -0.08071397f) + 0.78539425f);
    float cosApprox = reducedSquared * (reducedSquared * (-0.000318879f * reducedSquared + 0.015849913f) + -0.30842426f) + 1.0f;

    switch (quadrant & 6) {
        case 0:
            sinApprox = (x >= 0.0f) ? sinApprox : -sinApprox;
            *outSin = sinApprox;
            *outCos = cosApprox;
            break;
        case 2:
            cosApprox = (x >= 0.0f) ? cosApprox : -cosApprox;
            *outSin = cosApprox;
            *outCos = -sinApprox;
            break;
        case 4:
            if (x >= 0.0f) {
                sinApprox = -sinApprox;
            }
            *outSin = sinApprox;
            *outCos = -cosApprox;
            break;
        default:
            if (x >= 0.0f) {
                cosApprox = -cosApprox;
            }
            *outSin = cosApprox;
            *outCos = sinApprox;
            break;
    }
}
