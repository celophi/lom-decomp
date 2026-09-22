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

extern s32 D_801B0FD0;
extern WmapConfig *D_80139280;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_80121538;
extern s32 D_801B294C;
extern s32 D_801B2948;
extern void func_800885D8(void);

/** @brief World-map step: fill spawn descriptor slot 1, clear its slot run, then advance. */
void func_800873C0(void)
{
    s32 i;

    D_801B0FD0 = 0x32;
    D_80139280[1].field_04 = 2;
    D_80139280[1].field_08 = 1;
    D_80139280[1].field_0C = 0x14;
    D_80139280[1].field_10 = 0x1E;
    D_80139280[1].field_14 = 2;
    D_80139280[1].field_18 = 0x5DC;
    D_80139280[1].field_1C = 0x78;
    D_80139280[1].field_20 = 0x13;
    D_80139280[1].field_24 = 2;
    D_80139280[1].field_28 = 0x2710;
    for (i = 0; i < 0x32; i++)
    {
        D_801AFBD0[i + D_80139280[1].field_1C].field_00 = 0;
        D_80139988[i + 0x7C].field_04 = &D_80121538;
    }
    D_801B294C = 0x90;
    D_801B2948 += 1;
    func_800885D8();
}
