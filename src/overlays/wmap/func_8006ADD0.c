#include "common.h"
#include "sdk/libgte.h"

extern SVECTOR D_80139278;
extern SVECTOR D_801398C8;
extern VECTOR D_80182DC0;
extern VECTOR D_80182D48;

/**
 * @brief Combine global and local transforms and install the resulting matrix.
 * @param translation Local translation applied to the base matrix.
 * @param rotation Local rotation composed with the base matrix.
 */
void func_8006ADD0(VECTOR *translation, SVECTOR *rotation)
{
    MATRIX base;
    MATRIX effect;
    SVECTOR combined_rotation;
    VECTOR combined_translation;

    combined_rotation.vx = D_80139278.vx + D_801398C8.vx;
    combined_rotation.vy = D_80139278.vy + D_801398C8.vy;
    combined_rotation.vz = D_80139278.vz + D_801398C8.vz;
    combined_translation.vx = D_80182DC0.vx + D_80182D48.vx;
    combined_translation.vy = D_80182DC0.vy + D_80182D48.vy;
    combined_translation.vz = D_80182DC0.vz + D_80182D48.vz;
    RotMatrix(&combined_rotation, &base);
    TransMatrix(&base, translation);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(rotation, &effect);
    CompMatrix(&base, &effect, &effect);
    TransMatrix(&effect, &combined_translation);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
}
