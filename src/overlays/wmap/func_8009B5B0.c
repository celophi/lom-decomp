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
extern u8 D_80121538[];
extern s32 D_801B2CA8;
extern s32 D_801B2CAC;
extern void func_8009CCF4(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009B5B0(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 4;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 2;
    D_80139280[0x6] = 0;
    D_80139280[0x7] = 0x14;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 0x2EE0;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].field_04 = D_80121538;
    }
    D_801B2CAC = 80;
    D_801B2CA8++;
    func_8009CCF4();
}
