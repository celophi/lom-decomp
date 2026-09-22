#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 99.577540% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

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
} __attribute__((aligned(4))) WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    union { s32 packed; struct { s16 x, y; } point; } screen;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801B0080;
extern WmapResource D_80139988[];
extern DVECTOR D_80182D58;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_801B2988;
extern s32 D_801B298C;

/** @brief Project a spiral emitter and append animated copies along its trail. */
void func_80088B38(void)
{
    SVECTOR position;
    s32 previous;
    s32 next;
    s32 i;
    s32 remaining;
    WmapMotion *motion;
    WmapResource *resource;

    position.vx = ((D_801B0080.z >> 3) * (ccos(D_801B0080.angle) >> 6)) >> 12;
    position.vy = ((D_801B0080.z >> 3) * (csin(D_801B0080.angle) >> 6)) >> 12;
    position.vz = D_801B0080.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    D_801B0080.z += 1500;
    D_801B0080.angle += 192;
    D_8013923C--;
    gte_stsxy(&D_80182D58);
    D_801B0080.screen.point.x = D_80182D58.vx;
    D_801B0080.screen.point.y = D_80182D58.vy;
    if (D_8013923C == 0)
    {
        previous = D_80139234;
        D_80139234 = previous + 1;
        next = previous + 61;
        if (next < 250)
        {
            D_8013923C = 2;
            motion = &D_801B0080 - 60;
            motion[next] = D_801B0080;
            D_80139988[D_80139234 + 60] = D_80139988[60];
            D_800D9268[D_80139234 + 60] = D_800D9268[60];
            D_800D9268[D_80139234 + 60].field_0E = 0;
            D_800D9268[D_80139234 + 60].field_10 = -1;
        }
    }
    for (i = 61; i < D_80139234 + 60; i++)
    {
        resource = &D_80139988[i];
        motion = &D_801AFBD0[i];
        func_8006CC4C(&D_800D9268[i], resource);
        func_80066F9C(&D_800D9268[i], motion->screen.packed, 8, 10, 0);
    }
    remaining = D_801B298C - 1;
    D_801B298C = remaining;
    if (remaining == 0)
    {
        D_801B2988++;
    }
}
