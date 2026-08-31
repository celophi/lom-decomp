#include "common.h"

typedef struct EntryHeader7 {
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} EntryHeader7;

extern EntryHeader7 D_80140114;
extern s32 D_80160928;
extern s32 D_80160938;
extern s32 D_801609A4;
extern s32 D_801609AC;
extern s32 D_801609BC;
extern u8 D_80164B60[];

/** @see decomp.me (100.00%) */
s32 func_80145B4C(s32 page)
{
    EntryHeader7 buf;

    memcpy(&buf, &D_80140114, 7);
    D_801609AC = 0;
    D_801609BC = 0;
    D_80160938 = 0;
    D_80160928 = 0;
    D_801609A4 = 0;
    ((u8 *)&buf)[2] += page;
    if (func_80016BCC(&buf, D_80164B60 + page * 0x320) != 0)
    {
        func_800B0170(D_80164B60 + page * 0x320 + D_801609A4 * 0x28);
        D_801609A4 += 1;
        return 1;
    }
    return 0;
}
