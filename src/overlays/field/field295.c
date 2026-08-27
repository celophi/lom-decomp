#include "common.h"

typedef struct StructC63A0
{
    s16 unk0;
    u8 pad2[4];
    s16 unk6;
} StructC63A0;

void func_80087F44(s16 arg0, s32 *arg1);
void func_80087D8C(s16 arg0, s32 arg1, s32 arg2, s32 arg3);
extern StructC63A0 D_80122C0C;

/**
 * @brief Fetch a 3D position for the current object, scale it down and re-emit it.
 *
 * Queries func_80087F44 for the object named by @c D_80122C0C.unk0 into a local
 * vector, divides each component by 256 (rounding toward zero via the +0xFF
 * negative fix-up), subtracts @c D_80122C0C.unk6 from the Y component, and passes
 * the result to func_80087D8C.
 *
 * @note gcc280_g0, 100% match.
 */
void func_800C63A0(void)
{
    s32 vec[3];
    s32 x;
    s32 y;
    s32 z;
    s32 yshift;
    s32 y2;

    func_80087F44(D_80122C0C.unk0, vec);
    x = vec[0];
    if (x < 0)
    {
        x += 0xFF;
    }
    y = vec[1];
    x >>= 8;
    vec[0] = x;
    if (y < 0)
    {
        y += 0xFF;
    }
    z = vec[2];
    yshift = y >> 8;
    vec[1] = yshift;
    if (z < 0)
    {
        z += 0xFF;
    }
    z >>= 8;
    vec[2] = z;
    y2 = yshift - D_80122C0C.unk6;
    vec[1] = y2;
    func_80087D8C(D_80122C0C.unk0, x, y2, z);
}
