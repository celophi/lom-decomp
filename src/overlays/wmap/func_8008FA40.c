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
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern WmapConfigA D_800D9268[];
extern WmapResource D_80139988[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801B0080[];
extern s32 D_80139234;
extern s32 D_801B2AC0;
extern s32 D_801B2AC4;
extern s32 rand(void);
extern s32 func_8006CC4C(void *, void *);
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Spawn and draw spiraling particles until the effect finishes. */
void func_8008FA40(void)
{
    SVECTOR position;
    s32 screen_position;
    s32 i;
    s32 finished;
    s32 value;
    WmapMotion *motion;
    WmapConfigA *actor;

    if (D_80139234 < 24)
    {
        motion = &D_801B0080[D_80139234];
        motion->state = 1;
        motion->z = 150000;
        motion->angle = rand() & 0xFFF;
        motion->field_0E = 0;
        D_80139234++;
    }
    finished = 1;
    for (i = 60; i < 84; i++)
    {
        motion = &D_801AFBD0[i];
        actor = &D_800D9268[i];
        if (motion->state != 0)
        {
            finished = 0;
            position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->field_0E;
            gte_ldv0(&position);
            gte_rtps();
            func_8006CC4C(actor, &D_80139988[i]);
            value = motion->z - 3500;
            motion->z = value;
            if (value < 5000)
            {
                motion->state = 0;
            }
            motion->angle += 96;
            gte_stsxy(&screen_position);
            func_80066F9C(actor, screen_position, 19, 10, 0);
            if (actor->field_24 < 4)
            {
                motion->state = 0;
            }
        }
    }
    if (finished != 0)
    {
        D_801B2AC4 = 1;
    }
    value = D_801B2AC4 - 1;
    D_801B2AC4 = value;
    if (value == 0)
    {
        D_801B2AC0++;
    }
}
