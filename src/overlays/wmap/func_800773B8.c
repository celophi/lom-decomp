#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 86.000000% (gcc280_g0). */
#include "common.h"

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s16 field_10;
    s16 field_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

/** @brief Packed screen coordinates passed to the sprite renderer. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x, y;
    } point;
} WmapScreenPoint;
extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern s8 D_80051B4C[];
extern u16 D_80182DF4;
extern void func_80066F9C(WmapConfigA *, s32, s32, s32, s32);

/** @brief Advance oscillating actors and draw their sprites.
 * @param start First actor index.
 * @param end Exclusive end index.
 * @param depth Rendering depth.
 */
void func_800773B8(s32 start, s32 end, s32 depth)
{
    s32 i;
    s32 motion_offset;
    s32 x;
    s32 angle;
    WmapMotion *motion;
    WmapConfigA *actor;
    WmapScreenPoint screen;

    motion_offset = start * 20;
    for (i = start; i < end; i++)
    {
        motion = (WmapMotion *)((u8 *)D_801AFBD0 + motion_offset);
        motion_offset += 20;
        actor = &D_800D9268[i];
        motion->field_0E += motion->x;
        if (motion->field_0E >= 3841)
        {
            motion->field_0E = 0;
        }
        motion->field_12 = (motion->field_12 + motion->field_10) & 4095;
        angle = motion->angle;
        x = D_80051B4C[motion->field_12 / 16] * motion->scale;
        screen.point.x = (angle + x / 16) / 16;
        screen.point.y = motion->field_0E / 16;
        actor->field_22 = D_80182DF4;
        actor->field_24 = D_80182DF4;
        func_8006CC4C(actor, &D_80139988[i]);
        func_80066F9C(actor, screen.packed, depth, 4, 0);
    }
}
