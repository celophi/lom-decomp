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
extern u8 D_8011F538[];
extern s32 D_801B2D08;
extern s32 D_801B2D0C;
extern void func_8009F4B0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009D9F0(void)
{
    s32 i;

    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xD] = 0x40;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = 5;
    D_80139280[0x10] = 0x258;
    D_80139280[0x11] = 0x1E;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 0x2710;
    for (i = 0; i < 90; i++)
    {
        D_801AFBD0[i + 30].field_00 = 0;
        D_80139988[i + 30].field_04 = D_8011F538;
    }
    D_801B2D0C = 450;
    D_801B2D08++;
    func_8009F4B0();
}
