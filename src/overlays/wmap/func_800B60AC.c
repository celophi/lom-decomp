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
extern s32 D_801B3058;
extern s32 D_801B305C;
extern void func_800B7F0C(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800B60AC(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = -8;
    D_80139280[0x20] = 20;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 1;
    D_80139280[0x24] = 3900;
    D_80139280[0x25] = 60;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 2;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 64].field_04 = D_8011F538;
    }
    D_801B305C = 40;
    D_801B3058++;
    func_800B7F0C();
}
