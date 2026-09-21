#include "common.h"

extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 D_8011CF4C;
extern s32 D_801B27C8;
extern s32 D_801B27CC;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 * @note Best match ~88.6% (gcc280_g0); residual is a callee-saved register
 *       allocation tie (pos vs base pointers) shared with func_800803A0.
 */
void func_80080888(void)
{
    s32 pos;

    pos = (D_8011CF4C & 0xFFFF0000) | ((D_8011CF4C + 0x14) & 0xFFFF);
    pos = (pos & 0xFFFF) | (((pos >> 16) - 0xF) << 16);
    func_8006CC4C(D_800D9420, D_801399D8);
    func_80066F9C(D_800D9420, pos, 0x13, 0x1, 0);
    func_8006CC4C(&D_800D9420[0x2C], &D_801399D8[0x8]);
    func_80066F9C(&D_800D9420[0x2C], pos, 0x13, 0x1, 0);
    if (--D_801B27CC == 0)
    {
        D_801B27C8 += 1;
    }
}
