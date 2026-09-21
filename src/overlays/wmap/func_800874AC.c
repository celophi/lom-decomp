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
extern s32 D_801B2950;
extern s32 D_801B2954;
extern void func_800887D0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800874AC(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 1;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 170;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 3;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 174].field_04 = D_80121538;
    }
    D_801B2954 = 144;
    D_801B2950++;
    func_800887D0();
}
