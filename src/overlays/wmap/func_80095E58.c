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
extern s32 D_801B2BD8;
extern s32 D_801B2BDC;
extern u8 D_80123538[];
extern void func_800974BC(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80095E58(void)
{
    s32 i;

    D_801B0FD0 = 20;
    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xF] = 1;
    D_80139280[0x10] = 120;
    D_80139280[0x11] = 100;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 18500;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 104].field_04 = D_80123538;
    }
    D_801B2BDC = 20;
    D_801B2BD8++;
    func_800974BC();
}
