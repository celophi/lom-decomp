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

/** @brief World-map spawn descriptor; a 0x50-byte record addressed via D_80139280. */
typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 field_24;
    s32 field_28;
    u8 pad_2C[0x24];
} WmapConfig;

extern WmapConfig *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern s32 D_801B2D28;
extern s32 D_801B2D2C;
extern void func_8009FC60(void);

/** @brief Configure the effect and reset its forty-eight resource slots. */
void func_8009DE7C(void)
{
    s32 i;

    D_80139280[1].field_04 = 1;
    D_80139280[1].field_08 = 4;
    D_80139280[1].field_0C = 0x20;
    D_80139280[1].field_10 = 0;
    D_80139280[1].field_14 = 4;
    D_80139280[1].field_18 = 1;
    D_80139280[1].field_1C = 0x82;
    D_80139280[1].field_20 = 8;
    D_80139280[1].field_24 = 0;
    D_80139280[1].field_28 = 0x4650;
    for (i = 0; i < 48; i++)
    {
        D_801AFBD0[i + 130].field_00 = 0;
        D_80139988[i + 130].field_04 = D_8011F538;
    }
    D_801B2D2C = 0xC0;
    D_801B2D28++;
    func_8009FC60();
}
