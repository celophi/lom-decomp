#include "common.h"
#include "sdk/libgte.h"

extern SVECTOR D_80139278;
extern SVECTOR D_801398C8;
extern VECTOR D_80182DC0;
extern VECTOR D_80182D48;
extern MATRIX D_8011D0E8;

/** @brief Combine the base and effect transforms and install the resulting matrix. */
void func_8006AEE0(void)
{
    SVECTOR rotation;
    VECTOR translation;

    rotation.vx = D_80139278.vx + D_801398C8.vx;
    rotation.vy = D_80139278.vy + D_801398C8.vy;
    rotation.vz = D_80139278.vz + D_801398C8.vz;
    translation.vx = D_80182DC0.vx + D_80182D48.vx;
    translation.vy = D_80182DC0.vy + D_80182D48.vy;
    translation.vz = D_80182DC0.vz + D_80182D48.vz;
    RotMatrix(&rotation, &D_8011D0E8);
    TransMatrix(&D_8011D0E8, &translation);
    SetRotMatrix(&D_8011D0E8);
    SetTransMatrix(&D_8011D0E8);
}
