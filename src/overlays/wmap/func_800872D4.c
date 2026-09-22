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
extern u8 D_80121538[];
extern s32 D_801B2940;
extern s32 D_801B2944;
extern void func_800883E0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800872D4(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 1;
    D_80139280[0xD] = 20;
    D_80139280[0xE] = 30;
    D_80139280[0xF] = 4;
    D_80139280[0x10] = 1500;
    D_80139280[0x11] = 70;
    D_80139280[0x12] = 19;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 74].field_04 = D_80121538;
    }
    D_801B2944 = 144;
    D_801B2940++;
    func_800883E0();
}
