#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 89.764046% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief World-map orbiting star: polar position, spin angle, and radius. */
typedef struct
{
    s16 unk00;
    s16 angle;
    u16 delta;
    s16 unk06;
    s32 radius;
    s16 unk0C;
    u16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapStar;

/** @brief World-map draw record; only used opaquely by the primitive helpers. */
typedef struct
{
    u8 pad[0x2C];
} WmapDraw;

extern WmapStar D_801AFBD0[];
extern WmapDraw D_800D9268[];
extern u8 D_80139988[];
extern s32 D_801B2B70;
extern s32 D_801B2B74;

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
void func_80093C48(void)
{
    SVECTOR position;
    s32 screen;
    WmapStar* star;
    WmapDraw* draw;
    s32 i;

    star = &D_801AFBD0[0x96];
    draw = &D_800D9268[0x96];
    for (i = 0x96; i < 0xB4; i++)
    {
        position.vx = ((star->radius >> 6) * (ccos(star->angle) >> 6)) >> 0xC;
        position.vy = ((star->radius >> 6) * (csin(star->angle) >> 6)) >> 0xC;
        position.vz = star->unk0E;
        gte_ldv0(&position);
        gte_rtps();
        func_8006CC4C(draw, &D_80139988[i * 8]);
        gte_stsxy(&screen);
        if (star->angle != 0)
        {
            func_80066F9C(draw, screen, 0xF, 4, 0);
        }
        else
        {
            func_80066F9C(draw, screen, 0xF, 0, 0);
        }
        draw++;
        star->angle = ((u16)star->angle + star->delta) & 0xFFF;
        star++;
    }
    if (--D_801B2B74 == 0)
    {
        D_801B2B70 += 1;
    }
}
