/* Partial WMAP decompilation: 91.500000% (gcc280_g0). */
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

/** @brief World-map spawn descriptor addressed via D_80139280. */
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
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 field_4C;
    s32 field_50;
} WmapConfig;

extern s32 D_801B0FD0;
extern WmapConfig *D_80139280;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_8011D538;
extern s32 D_801B28E4;
extern s32 D_801B28E0;
extern void func_80086778(void);

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_80085528(void)
{
    s32 i;
    WmapConfig *cfg;

    cfg = D_80139280;
    D_801B0FD0 = 0xA;
    cfg[0].field_2C = 1;
    cfg[0].field_30 = 6;
    cfg[0].field_34 = 0x90;
    cfg[0].field_38 = 2;
    cfg[0].field_3C = 3;
    cfg[0].field_40 = 0x60;
    cfg[0].field_44 = 0x64;
    cfg[0].field_48 = 0xD;
    cfg[0].field_4C = 0;
    cfg[0].field_50 = 0x2EE0;
    for (i = 0; i < 0xA; i++)
    {
        D_801AFBD0[i + cfg[0].field_44].field_00 = 0;
        D_80139988[i + 0x68].field_04 = &D_8011D538;
    }
    D_801B28E4 = 0x29;
    D_801B28E0 += 1;
    func_80086778();
}
