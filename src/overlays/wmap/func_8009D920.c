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
extern u8 D_8011F538[];
extern s32 D_801B2D00;
extern s32 D_801B2D04;
extern void func_8009F2B0(void);

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009D920(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 4;
    D_80139280[0x6] = 0x320;
    D_80139280[0x7] = 0xB4;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 1;
    D_80139280[0xA] = 0x124F8;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + 180].field_00 = 0;
        D_80139988[i + 180].field_04 = D_8011F538;
    }
    D_801B2D04 = 80;
    D_801B2D00++;
    func_8009F2B0();
}
