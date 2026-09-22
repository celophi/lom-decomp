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
extern u8 D_80123538[];
extern s32 D_801B2F70;
extern s32 D_801B2F74;
extern void func_800B0DB4(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800AE928(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 8;
    D_80139280[0x3] = 0x40;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 1;
    D_80139280[0x6] = 0x5DC;
    D_80139280[0x7] = 0xAA;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 3;
    D_80139280[0xA] = 0x32C8;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + 170].field_00 = 0;
        D_80139988[i + 170].field_04 = D_80123538;
    }
    D_801B2F74 = 24;
    D_801B2F70++;
    func_800B0DB4();
}
