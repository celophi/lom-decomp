#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 93.534090% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"

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
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9370[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_801399B8[];
extern s32 D_801AFBC8;
extern s32 D_801B0FD0;
extern u32 rand(void);
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Spawn radial particles, advance their animation, and count active slots. */
s32 func_8006688C(void)
{
    s32 i;
    s32 actor_offset;
    s32 active;
    s32 remaining;
    s32 random_value;
    union
    {
        struct { s16 x, y; } point;
        s32 packed;
    } screen;
    WmapConfigA *actor;
    WmapConfigA *actor_base;
    WmapMotion *motion;

    active = 0;
    remaining = D_801B0FD0 * 3;
    for (i = 0, actor_offset = 264; i < D_801B0FD0 * 70; i++, actor_offset += 44)
    {
        motion = &D_801AFBD0[i];
        if (motion->state == 0)
        {
            actor = (WmapConfigA *)((u8 *)D_800D9268 + actor_offset);
            if (D_801AFBC8 != 0)
            {
                actor->field_02 = 0;
                actor->field_06 = 15;
                actor->field_0E = ((s32)(rand() * D_801B0FD0) >> 15) + 1;
                actor->field_10 = -1;
                actor->field_22 = 129;
                actor->field_24 = 129;
                motion->state = 1;
                motion->angle = rand() >> 3;
                motion->z = 0;
                motion->x = rand() * D_801B0FD0;
                random_value = rand();
                motion->scale = ((s32)(random_value * (D_801B0FD0 * 20)) >> 15) + 5;
                if (--remaining == 0)
                {
                    break;
                }
            }
        }
    }
    actor_base = D_800D9370 - 6;
    for (i = 0, actor_offset = 264; i < 110; i++, actor_offset += 44)
    {
        motion = &D_801AFBD0[i];
        actor = &D_800D9370[i];
        if (motion->state != 0)
        {
            motion->z += motion->x;
            screen.point.x = ((motion->z * ccos(motion->angle)) >> 24) + 160;
            screen.point.y = ((motion->z * csin(motion->angle)) >> 24) + 120;
            func_8006CC4C(actor, &D_801399B8[i]);
            if (((WmapConfigA *)((u8 *)actor_base + actor_offset))->field_0E == 3)
            {
                func_80066F9C(actor, screen.packed, 6, 4, 0);
            }
            else
            {
                func_80066F9C(actor, screen.packed, 7, 4, 0);
            }
            motion->scale--;
            if (motion->scale == 0)
            {
                motion->state = 0;
            }
            active++;
        }
    }
    return active;
}
