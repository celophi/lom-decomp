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
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern s32 D_8018222C;
extern s32 D_8011CF74;
extern s32 D_80139234;
extern void func_8006CC4C(void *, void *);
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Draw active trail actors and periodically copy four new trail samples. */
void func_800A6800(void)
{
    s32 i;
    s32 destination;
    WmapConfigA *actor;
    WmapMotion *motion;

    if (D_8018222C != 0)
    {
        for (i = 40; i < 104; i++)
        {
            motion = &D_801AFBD0[i];
            if (motion->state != 0)
            {
                actor = &D_800D9268[i];
                func_8006CC4C(actor, &D_80139988[i]);
                func_80066F9C(actor, motion->field_10, 3, motion->scale + 1, 0x400);
            }
        }
        if (D_8011CF74 % 10 == 0)
        {
            if (D_80139234 != -1)
            {
                for (i = 0; i < 4; i++)
                {
                    destination = i + D_80139234 * 4 + 40;
                    D_801AFBD0[destination] = D_801AFBD0[i + 20];
                    D_800D9268[destination] = D_800D9268[i + 20];
                    D_800D9268[destination].field_22 = 0;
                    D_800D9268[destination].field_26 = 2;
                }
                D_80139234 = (D_80139234 + 1) & 15;
            }
        }
    }
}
