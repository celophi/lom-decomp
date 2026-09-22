#include "common.h"

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief World-map spawn descriptor; a 0x50-byte record addressed via D_80139280. */
typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 field_24;
    s32 field_28;
    u8 pad_2C[0x24];
} WmapConfig;

extern s32 D_801B0FD0;
extern WmapConfig *D_80139280;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_8011D538;
extern s32 D_801B28D4;
extern s32 D_801B28D0;
extern void func_800863E4(void);

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_8008543C(void)
{
    s32 i;

    D_801B0FD0 = 0x46;
    D_80139280[0].field_04 = 2;
    D_80139280[0].field_08 = 8;
    D_80139280[0].field_0C = 0x40;
    D_80139280[0].field_10 = 2;
    D_80139280[0].field_14 = 2;
    D_80139280[0].field_18 = 0x190;
    D_80139280[0].field_1C = 0x14;
    D_80139280[0].field_20 = 0xD;
    D_80139280[0].field_24 = 1;
    D_80139280[0].field_28 = 0x3E8;
    for (i = 0; i < 0x46; i++)
    {
        D_801AFBD0[i + D_80139280[0].field_1C].field_00 = 0;
        D_80139988[i + 0x18].field_04 = &D_8011D538;
    }
    D_801B28D4 = 0x8D;
    D_801B28D0 += 1;
    func_800863E4();
}
