#include "common.h"

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

extern s32 D_801B0FD0;
extern s32 D_80139980;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_80125538;
extern s32 D_801B2848;
extern s32 D_801B284C;
extern void func_800837EC(void);

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void func_80083758(void)
{
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 0x8; i++)
    {
        D_801AFBD0[i + 0x54].field_00 = 0;
        D_80139988[i + 0x54].field_04 = &D_80125538;
    }
    D_801B284C = 2;
    D_801B2848 += 1;
    func_800837EC();
}
