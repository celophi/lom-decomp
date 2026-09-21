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
extern s32 D_801B2C98;
extern s32 D_801B2C9C;
extern u8 D_80121538[];
extern void func_8009C880(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009B4D4(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 4;
    D_80139280[0x21] = 32;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 140;
    D_80139280[0x26] = 8;
    D_80139280[0x27] = 3;
    D_80139280[0x28] = 8000;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 140].field_00 = 0;
        D_80139988[i + 144].field_04 = D_80121538;
    }
    D_801B2C9C = 80;
    D_801B2C98++;
    func_8009C880();
}
