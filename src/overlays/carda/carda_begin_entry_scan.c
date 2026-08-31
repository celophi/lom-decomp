#include "common.h"

typedef struct EntryHeader7 {
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} EntryHeader7;

extern EntryHeader7 D_80140244;
extern s32 D_80165FFC;
extern s32 D_80165F38;
extern s32 D_80166104;
extern s32 D_80165FF4;
extern s32 D_80165FEC;
extern s32 D_80166078;
extern u8 D_80166440[];

s32 func_8014991C(s32 page)
{
    EntryHeader7 buf;
    s32 i;

    memcpy(&buf, &D_80140244, 7);
    D_80165FFC = 0;
    D_80165F38 = 0;
    D_80166104 = 0;
    D_80165FF4 = 0;
    ((u8 *)&buf)[2] += page;
    func_8001729C(page);
    D_80165FEC = 0;
    i = 0;
    do
    {
        if (func_80016BCC(&buf, D_80166440 + page * 0x320) != 0)
        {
            func_800B0170(D_80166440 + page * 0x320 + D_80165FEC * 0x28);
            D_80165FEC += 1;
            return 1;
        }
        i++;
    } while (i < 20);
    if (D_80166078 == 1 || D_80166078 == 3)
        return 0;
    return 1;
}
