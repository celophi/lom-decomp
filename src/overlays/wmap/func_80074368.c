/* Partial WMAP decompilation: 94.930070% (gcc280_g0). */
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

extern WmapMotion D_801AFBD0[];
extern s32 D_800D922C;
extern s32 D_801B0FD0;
extern s32 rand(void);
extern void func_8006CC4C(void *, void *);
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Project active particles and initialize the first available slot. */
void func_80074368(WmapConfigA *actors, WmapResource *resources, s32 count)
{
    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;

    active_count = 0;
    for (i = 0; i < count; i++)
    {
        motion = &D_801AFBD0[i];
        actor = &actors[i];
        if (motion->state != 0)
        {
            position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->field_0E;
            gte_ldv0(&position);
            gte_rtps();
            motion->field_0E += motion->x;
            motion->z += 1000;
            gte_stsxy(&screen_position);
            func_8006CC4C(actor, &resources[i]);
            func_80066F9C(actor, screen_position, 15, 7, 0);
            motion->scale--;
            if (motion->scale == 0)
            {
                motion->state = 0;
            }
            active_count++;
        }
    }
    D_800D922C = active_count;
    for (i = 0; i < count; i++)
    {
        motion = &D_801AFBD0[i];
        actor = &actors[i];
        if (motion->state == 0)
        {
            if (D_801B0FD0 >= active_count)
            {
                actor->field_06 = 15;
                actor->field_0E = 3;
                actor->field_10 = -1;
                actor->field_22 = 129;
                actor->field_24 = 129;
                actor->field_02 = 0;
                motion->state = 1;
                motion->angle = rand();
                motion->z = 10000;
                motion->x = ((rand() * 5) >> 15) + 1;
                motion->scale = ((rand() * 2) >> 15) + 24;
                motion->field_0E = 0;
            }
            break;
        }
    }
}
