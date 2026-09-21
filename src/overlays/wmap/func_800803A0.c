#include "common.h"

extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B27B8;
extern s32 D_801B27BC;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 */
void func_800803A0(void)
{
    s32 pos;

    pos = (D_8011CF4C & 0xFFFF0000) | ((D_8011CF4C - 0xA) & 0xFFFF);
    pos = (pos & 0xFFFF) | (((pos >> 16) - 0xA) << 16);
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, pos, 0x13, 0x1, 0);
    func_8006CC4C(&D_800D9370[0x2C], &D_801399B8[0x8]);
    func_80066F9C(&D_800D9370[0x2C], pos, 0x13, 0x1, 0);
    if (--D_801B27BC == 0)
    {
        D_801B27B8 += 1;
    }
}
