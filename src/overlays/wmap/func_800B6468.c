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
extern u8 D_8011F538[];
extern s32 D_801B3080;
extern s32 D_801B3084;
extern void func_800B88B4(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800B6468(void)
{
    s32 i;

    D_801B0FD0 = 32;
    D_80139280[0x15] = -3;
    D_80139280[0x16] = 8;
    D_80139280[0x17] = 20;
    D_80139280[0x18] = 30;
    D_80139280[0x19] = 1;
    D_80139280[0x1A] = 1500;
    D_80139280[0x1B] = 60;
    D_80139280[0x1C] = 15;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 10000;
    for (i = 0; i < 32; i++)
    {
        D_801AFBD0[i + D_80139280[0x1B]].field_00 = 0;
        D_80139988[i + 64].field_04 = D_8011F538;
    }
    D_801B3084 = 32;
    D_801B3080++;
    func_800B88B4();
}
