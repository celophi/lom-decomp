#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} SlotA;

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x19];
} EntryB;

extern SlotA D_80105AE0[];
extern EntryB D_800FDF58[];

s32 func_800839F8(s32 arg0, s32 arg1, EntryB *arg2);
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);

s32 func_8008B42C(s32 arg0, s32 arg1)
{
    SlotA *ra;
    EntryB *rb;
    EntryB *found;
    s32 i;
    s32 anim;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (EntryB *) -1;
check:
    if (found != (EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(found->unk3A, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}
