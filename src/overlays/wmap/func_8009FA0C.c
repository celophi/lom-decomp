#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s16 D_80182D64[];
extern s32 D_8013924C;
extern s32 D_801B2D20;
extern s32 D_801B2D24;

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position downward and advance when the frame counter expires.
 */
void func_8009FA0C(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, *(s32*)D_80182D64, 0x17, 0x2, 0);
    D_80182D64[1] = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--D_801B2D24 == 0)
    {
        D_801B2D20 += 1;
    }
}
