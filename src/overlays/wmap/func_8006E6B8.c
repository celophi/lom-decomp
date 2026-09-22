/* Partial WMAP decompilation: 79.604650% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/rand.h"

/** @brief World-map spark particle: spin angle, radial velocity, and lifetime. */
typedef struct
{
    s16 active;   /* 0x0 */
    s16 angle;    /* 0x2 */
    s32 delta;    /* 0x4 */
    s32 radius;   /* 0x8 */
    u16 timer;    /* 0xC */
    s16 unk0E;    /* 0xE */
    s16 unk10;    /* 0x10 */
    s16 unk12;    /* 0x12 */
} WmapSpark;

/** @brief World-map draw record; helpers use it opaquely, this handler seeds fields. */
typedef struct
{
    u8 pad00[0x2];
    s16 unk02;    /* 0x2 */
    u8 pad04[0x2];
    s8 unk06;     /* 0x6 */
    u8 pad07[0x7];
    s16 unk0E;    /* 0xE */
    s16 unk10;    /* 0x10 */
    u8 pad12[0x10];
    s16 unk22;    /* 0x22 */
    s16 unk24;    /* 0x24 */
    u8 pad26[0x6];
} WmapDraw;

extern WmapSpark D_801AFBD0[];
extern WmapDraw D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern void func_8006CC4C(WmapDraw* obj, u8* a1);
extern void func_80066F9C(WmapDraw* obj, s32 a1, s32 a2, s32 a3, s32 a4);

/**
 * @brief Project and draw active world-map sparks, then respawn empty slots.
 * @note First pass projects each live spark through the GTE and advances its
 *       radius; second pass seeds fresh sparks up to the shared population cap.
 * @note GTE-tagged: best-effort structural match (mirrors func_8007115C); the
 *       gcc280_g0 diff harness cannot assemble the GTE mnemonics for a percent.
 */
void func_8006E6B8(void)
{
    SVECTOR position;
    s32 screen;
    WmapSpark* spark;
    WmapDraw* draw;
    s32 count;
    s32 i;

    count = 0;
    spark = D_801AFBD0;
    draw = D_800D9370;
    for (i = 0; i < 0x1E; i++)
    {
        if (spark->active != 0)
        {
            position.vx = ((spark->radius >> 6) * (ccos(spark->angle) >> 6)) >> 0xC;
            position.vy = ((spark->radius >> 6) * (csin(spark->angle) >> 6)) >> 0xC;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtps();
            spark->radius += spark->delta;
            draw->unk22 = *(u16*)&D_80139980;
            draw->unk24 = *(u16*)&D_80139980;
            gte_stsxy(&screen);
            func_8006CC4C(draw, &D_801399B8[i * 8]);
            func_80066F9C(draw, screen, 9, 0x1F, 0);
            if (--spark->timer == 0)
            {
                spark->active = 0;
            }
            count++;
        }
        spark++;
        draw++;
    }
    spark = D_801AFBD0;
    draw = D_800D9370;
    for (i = 0; i < 0x1E; i++)
    {
        if (spark->active == 0)
        {
            if (D_801B0FD0 < count)
            {
                break;
            }
            count++;
            draw->unk06 = 0xF;
            draw->unk10 = -1;
            draw->unk02 = 0;
            draw->unk0E = 0;
            spark->active = 1;
            spark->angle = rand() >> 3;
            spark->radius = 0;
            spark->delta = rand() / 2 + 0x1000;
            spark->timer = (rand() & 0x3C) + 0x4B;
        }
        spark++;
        draw++;
    }
}
