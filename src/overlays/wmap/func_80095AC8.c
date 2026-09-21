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
extern s32 D_801B2BB8;
extern s32 D_801B2BBC;
extern u8 D_80121538[];
extern void func_80096BD0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80095AC8(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1] = 0;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 255;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 2;
    D_80139280[0x6] = 1000;
    D_80139280[0x7] = 200;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 18200;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x7]].field_00 = 0;
        D_80139988[i + 204].field_04 = D_80121538;
    }
    D_801B2BBC = 100;
    D_801B2BB8++;
    func_80096BD0();
}
