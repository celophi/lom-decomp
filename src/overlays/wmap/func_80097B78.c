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
extern s32 D_801B2C40;
extern s32 D_801B2C44;
extern void func_800995DC(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80097B78(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = -1;
    D_80139280[0x20] = -4;
    D_80139280[0x21] = 0x20;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 0xC8;
    D_80139280[0x26] = 0x13;
    D_80139280[0x27] = 2;
    D_80139280[0x28] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 200].field_00 = 0;
        D_80139988[i + 204].field_04 = D_8011F538;
    }
    D_801B2C44 = 80;
    D_801B2C40++;
    func_800995DC();
}
