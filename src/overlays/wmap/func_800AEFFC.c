#include "common.h"

#include "sdk/libgte.h"

extern CVECTOR D_80182D74;
extern CVECTOR D_80182D80;
extern CVECTOR D_80182D8C;
extern CVECTOR D_80182D94;
extern s32 D_801ADAF4;
extern s32 D_801B2EF8;
extern s32 D_801B2EFC;

/** @brief Set two effect colors, clear two colors, and begin a 35-tick sequence step. */
void func_800AEFFC(void)
{
    D_801ADAF4 = 0;
    D_80182D74.r = 0x32;
    D_80182D74.g = 0;
    D_80182D74.b = 0xA0;
    D_80182D80.r = 0x32;
    D_80182D80.g = 0;
    D_80182D80.b = 0xA0;
    D_80182D8C.r = 0;
    D_80182D8C.g = 0;
    D_80182D8C.b = 0;
    D_80182D94.r = 0;
    D_80182D94.g = 0;
    D_80182D94.b = 0;
    D_801B2EFC = 0x23;
    D_801B2EF8 += 1;
}
