#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x1C - 0xC];
    union
    {
        s32 w;
        struct
        {
            u16 lo;
            u16 hi;
        } h;
    } unk1C;
    u8 pad20[0x25 - 0x20];
    u8 unk25;
    u8 pad26[0x3A - 0x26];
    u8 unk3A;
} Rec80087680;

typedef struct
{
    u8 pad0[0x10];
    s32 flags;
    u8 pad14[0x23C - 0x14];
} State80087680;

Rec80087680 *func_80087C9C(s32 arg0);
void func_8006B4D0(u8 arg0, s32 arg1);

extern State80087680 D_80105AE0[];

/**
 * @brief Update an actor record and propagate its state to the field actor table.
 * @param arg0 Actor record identifier.
 * @param arg1 Value passed to the actor setup helper.
 * @param arg2 Low flag bits stored in the actor state.
 * @param arg3 First fixed-point component.
 * @param arg4 Second fixed-point component.
 * @param arg5 Third fixed-point component.
 */
void func_80087680(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    Rec80087680 *rec;
    s32 flags;

    rec = func_80087C9C(arg0);
    if (rec != (Rec80087680 *)-1)
    {
        flags = rec->unk1C.h.hi & 3;
        func_8006B4D0(rec->unk3A, arg1);
        rec->unk25 = 0;
        rec->unk0 = arg3 << 8;
        rec->unk1C.w = (rec->unk1C.w & 0xFFFCFFFF) | (flags << 0x10);
        rec->unk4 = arg4 << 8;
        rec->unk8 = arg5 << 8;
        D_80105AE0[rec->unk3A].flags = (D_80105AE0[rec->unk3A].flags & ~0xF) | arg2;
        func_8006C3FC(rec);
    }
}
