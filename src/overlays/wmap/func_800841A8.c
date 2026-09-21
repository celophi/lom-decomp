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
extern s32 D_801B28A0;
extern s32 D_801B28A4;
extern u8 D_80121538[];
extern void func_80085108(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800841A8(void)
{
    s32 i;

    D_801B0FD0 = 5;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 2;
    D_80139280[0xD] = 32;
    D_80139280[0xE] = 1;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 2000;
    D_80139280[0x11] = 20;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 1000;
    for (i = 0; i < 5; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].field_04 = D_80121538;
    }
    D_801B28A4 = 10;
    D_801B28A0++;
    func_80085108();
}
