#include "common.h"

extern u8 D_800D93F4[];
extern u8 D_80182DF0[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;
extern s32 D_801B25C4;
extern s32 D_801B25C0;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_800766A4(void)
{
    u8* obj = D_800D93F4;
    u16 pos = *(u16*)&D_80182DF0[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399D0);
    func_80066F9C(obj, D_8011CF4C, 0x10, 0xB, 0);
    *(s32*)&D_80182DF0[0] += 0x20;
    if (*(s32*)&D_80182DF0[0] >= 0x82)
    {
        *(s32*)&D_80182DF0[0] = 0x81;
    }
    if (--D_801B25C4 == 0)
    {
        D_801B25C0 += 1;
    }
}
