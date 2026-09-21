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

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B2998;
extern s32 D_801B299C;
extern void func_8008A5D0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80088F18(void)
{
    s32 i;

    D_801B0FD0 = 24;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 1;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 5;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 24].field_04 = D_8011D538;
    }
    D_801B299C = 48;
    D_801B2998++;
    func_8008A5D0();
}
