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
extern WmapConfigA D_800DA708;
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern WmapResource D_80139D48;
extern s32 D_8011CF74;
extern s32 D_80139284;
extern s32 D_801B2850;
extern s32 D_801B2854;

/** @brief Advance spaced effect actors and copy their trailing samples. */
void func_80081508(void)
{
    s32 next;
    s32 i;
    s32 destination;
    s32 remaining;

    func_8006D014(&D_800DA708, &D_80139D48, 60, 128, 128, 8, 3);
    for (i = 120; i < 180; i += 5)
    {
        if (D_801AFBD0[i].scale != 0)
        {
            D_801AFBD0[i].scale--;
        }
        D_801AFBD0[i].z += 5000;
    }
    if ((D_8011CF74 & 1) == 0)
    {
        for (i = 120; i < 180; i += 5)
        {
            next = i + 1;
            destination = D_80139284 + next;
            D_801AFBD0[destination] = D_801AFBD0[i];
            D_800D9268[destination] = D_800D9268[i];
            D_80139988[destination] = D_80139988[i];
            D_800D9268[destination].field_22 = 0;
        }
        D_80139284 = (D_80139284 + 1) % 4;
    }
    remaining = D_801B2854 - 1;
    D_801B2854 = remaining;
    if (remaining == 0)
    {
        D_801B2850++;
    }
}
