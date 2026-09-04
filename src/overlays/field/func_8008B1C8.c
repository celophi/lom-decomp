#include "common.h"

typedef struct {
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} ActorSlot;

typedef struct {
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[2];
    s8 unk24;
    u8 pad25[2];
    s8 unk27;
    u8 pad28[6];
    s16 unk2E;
    u8 pad30[0xA];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Entry;

typedef struct {
    u8 pad0[0x174];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} ActorRecord;

extern ActorSlot D_80105AE0[];
extern Entry D_800FDF58[];
void func_8006C3FC(Entry *arg0);

s32 func_8008B1C8(s32 arg0, u8 arg1)
{
    ActorSlot *ra;
    Entry *rb;
    Entry *found;
    s32 i;
    ActorRecord *base;
    ActorRecord *slot;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (Entry *)-1;
check:
    if (found == (Entry *)-1)
    {
        goto fail;
    }
    base = (ActorRecord *)D_80105AE0;
    found->unk21 = arg1;
    found->unk2E = 1;
    found->unk27 = 0;
    found->unk24 = 1;
    slot = &base[found->unk3A];
    slot->unk174 &= ~0x1800;
    func_8006C3FC(found);
    return 0;
found_it:
    found = rb;
    goto check;
fail:
    return -1;
}
