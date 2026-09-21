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
    u8 pad_00[0x7C];
    s32 field_7C;
    s32 field_80;
    s32 field_84;
    s32 field_88;
    s32 field_8C;
    s32 field_90;
    s32 field_94;
    s32 field_98;
    s32 field_9C;
    s32 field_A0;
} WmapConfig;

extern s32 D_801B0FD0;
extern WmapConfig *D_80139280;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_80123538;
extern s32 D_801B2BD4;
extern s32 D_801B2BD0;
extern void func_800972C4(void);

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance.
 *  @note Best match ~91% (gcc280_g0); residual is a cfg-pointer coloring tie. */
void func_80095D68(void)
{
    s32 i;
    WmapConfig *cfg;

    cfg = D_80139280;
    D_801B0FD0 = 0xC;
    cfg[0].field_7C = 1;
    cfg[0].field_80 = 3;
    cfg[0].field_84 = 0x40;
    cfg[0].field_88 = 0x1E;
    cfg[0].field_8C = -2;
    cfg[0].field_90 = 0x3E8;
    cfg[0].field_94 = 0x14;
    cfg[0].field_98 = 0xF;
    cfg[0].field_9C = 0;
    cfg[0].field_A0 = 0x2710;
    for (i = 0; i < 0xC; i++)
    {
        D_801AFBD0[i + cfg[0].field_94].field_00 = 0;
        D_80139988[i + 0x18].field_04 = &D_80123538;
    }
    D_801B2BD4 = 0x40;
    D_801B2BD0 += 1;
    func_800972C4();
}
