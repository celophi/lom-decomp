#include "common.h"

extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s16 D_80182D60[];
extern s32 D_80139260;
extern s32 D_801B2D38;
extern s32 D_801B2D3C;

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_800A01F0(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, *(s32*)D_80182D60, 0x8, 0x2, 0);
    D_80182D60[1] = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--D_801B2D3C == 0)
    {
        D_801B2D38 += 1;
    }
}
