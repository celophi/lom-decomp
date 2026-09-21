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
extern s32 D_801B2840;
extern s32 D_801B2844;
extern void func_800835DC(void);

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void func_80083548(void)
{
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 0x32; i++)
    {
        D_801AFBD0[i + 0x13].field_00 = 0;
        D_80139988[i + 0x13].field_04 = &D_80125538;
    }
    D_801B2844 = 0x32;
    D_801B2840 += 1;
    func_800835DC();
}
