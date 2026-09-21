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
extern s32 D_801B2A90;
extern s32 D_801B2A94;
extern void func_80090B84(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8008F218(void)
{
    s32 i;

    D_801B0FD0 = 10;
    D_80139280[0xB] = 1;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 144;
    D_80139280[0xE] = 1;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 6;
    D_80139280[0x11] = 40;
    D_80139280[0x12] = 29;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 12000;
    for (i = 0; i < 10; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 44].field_04 = D_8011D538;
    }
    D_801B2A94 = 40;
    D_801B2A90++;
    func_80090B84();
}
