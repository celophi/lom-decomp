#include "common.h"

typedef struct EntryHeader7 {
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} EntryHeader7;

extern EntryHeader7 D_80140114;
extern s32 D_80164AE0;
extern s32 D_80164AEC;
extern s32 D_80164B78;
extern s32 D_80164B7C;
extern s32 D_80164B88;
extern u8 D_80165018[];

/** @see decomp.me (100.00%) */
s32 func_80145CBC(s32 page)
{
    EntryHeader7 buf;

    memcpy(&buf, &D_80140114, 7);
    D_80164B7C = 0;
    D_80164B88 = 0;
    D_80164AEC = 0;
    D_80164AE0 = 0;
    D_80164B78 = 0;
    ((u8 *)&buf)[2] += page;
    if (func_80016BCC(&buf, D_80165018 + page * 0x320) != 0)
    {
        func_800B0170(D_80165018 + page * 0x320 + D_80164B78 * 0x28);
        D_80164B78 += 1;
        return 1;
    }
    return 0;
}
