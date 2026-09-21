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
extern s32 D_801B2AC8;
extern s32 D_801B2ACC;
extern u8 D_80121538[];
extern void func_800915C8(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8008FC40(void)
{
    s32 i;

    for (i = 100; i < 150; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].field_04 = D_80121538;
    }
    D_801B2ACC = 200;
    D_80139280[0x1E] = 1;
    D_80139280[0x1F] = 2;
    D_80139280[0x20] = -1000;
    D_80139280[0x21] = 6000;
    D_80139280[0x22] = 5000;
    D_80139280[0x23] = 5;
    D_80139280[0x24] = -100;
    D_80139280[0x25] = 0;
    D_80139280[0x26] = -300;
    D_80139280[0x27] = 8;
    D_80139280[0x28] = 19;
    D_80139280[0x29] = 3;
    D_801B2AC8++;
    func_800915C8();
}
