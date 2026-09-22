#include "wmap_sequence_runtime.h"
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

extern WmapConfigA D_800D95D8[];
extern WmapResource D_80139A28[];
extern WmapMotion D_801AFD60[];
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Project four rotating effect actors and update their draw depths. */
void func_800A66C0(void)
{
    SVECTOR position;
    s32 depth;
    s32 draw_depth;
    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;

    for (i = 0; i < 4; i++)
    {
        motion = &D_801AFD60[i];
        actor = &D_800D95D8[i];
        position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
        position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
        position.vz = motion->field_0E;
        gte_ldv0(&position);
        gte_rtps();
        func_8006CC4C(actor, &D_80139A28[i]);
        gte_stsxy(&motion->field_10);
        gte_stszotz(&depth);
        draw_depth = (7057 - depth) / 4 + 42;
        func_80066F9C(actor, motion->field_10, 3, draw_depth, 0x400);
        motion->scale = draw_depth;
    }
}
