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
extern s32 D_801B0FD0;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011D538[];
extern s32 D_801B2DB8;
extern s32 D_801B2DBC;
extern void func_800A2DD0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800A1394(void)
{
    s32 i;

    D_801B0FD0 = 46;
    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xF] = 6;
    D_80139280[0x10] = 400;
    D_80139280[0x11] = 20;
    D_80139280[0x12] = 21;
    D_80139280[0x13] = 0;
    D_80139280[0x14] = 12000;
    for (i = 0; i < 46; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 24].field_04 = D_8011D538;
    }
    D_801B2DBC = 276;
    D_801B2DB8++;
    func_800A2DD0();
}
