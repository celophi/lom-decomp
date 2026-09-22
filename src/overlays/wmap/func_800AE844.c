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
extern s32 D_801B2F68;
extern s32 D_801B2F6C;
extern u8 D_80123538[];
extern void func_800B0C04(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800AE844(void)
{
    s32 i;

    D_80139280[0x28] = 40;
    D_80139280[0x29] = 200;
    D_80139280[0x2A] = -280;
    D_80139280[0x2B] = 560;
    D_80139280[0x2C] = -280;
    D_80139280[0x2D] = 560;
    D_80139280[0x2E] = 200;
    D_80139280[0x2F] = 80;
    D_80139280[0x30] = 129;
    D_80139280[0x31] = 1;
    D_80139280[0x32] = 2;
    D_80139280[0x33] = 40;
    D_80139280[0x34] = 8;
    D_80139280[0x35] = 2;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 200].field_00 = 0;
        D_80139988[i + 200].field_04 = D_80123538;
    }
    D_801B2F6C = 160;
    D_801B2F68++;
    func_800B0C04();
}
