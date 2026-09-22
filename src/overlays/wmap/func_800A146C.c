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
extern u8 D_8011D538[];
extern s32 D_801B2DC0;
extern s32 D_801B2DC4;
extern void func_800A2FC8(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800A146C(void)
{
    s32 i;

    D_80139280[0x15] = 1;
    D_80139280[0x16] = 4;
    D_80139280[0x17] = 0x20;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = 2;
    D_80139280[0x1A] = 0;
    D_80139280[0x1B] = 0x64;
    D_80139280[0x1C] = 0x15;
    D_80139280[0x1D] = 2;
    D_80139280[0x1E] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 100].field_00 = 0;
        D_80139988[i + 104].field_04 = D_8011D538;
    }
    D_801B2DC4 = 80;
    D_801B2DC0++;
    func_800A2FC8();
}
