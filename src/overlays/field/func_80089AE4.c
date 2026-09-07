#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} StateB80089AE4;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0xA];
    s32 unk1C;
    u8 pad20[8];
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    u8 pad2C[0xE];
    u8 unk3A;
    u8 pad3B[2];
    u8 unk3D;
    u8 pad3E[0x16];
} RecordB80089AE4;

extern StateB80089AE4 D_80105AE0[];
extern RecordB80089AE4 D_800FDF58[];

s32 func_800839F8(s32 arg0, s32 arg1, RecordB80089AE4 *arg2);
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Find a matching field actor slot and update its animation state.
 * @param arg0 Field-state key to locate.
 * @param arg1 Animation argument, or -1 to skip animation dispatch.
 * @return Zero when a matching slot is updated, or -1 when no slot matches.
 */
s32 func_80089AE4(s32 arg0, s32 arg1)
{
    StateB80089AE4 *ra;
    RecordB80089AE4 *rb;
    s32 anim_id;
    RecordB80089AE4 *found;
    s32 i;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecordB80089AE4 *)-1;
check:
    do
    {
        if (found != (RecordB80089AE4 *)-1)
        {
            goto body;
        }
    } while (0);
    return -1;
found_it:
    found = rb;
    goto check;
body:
    if (arg1 != -1)
    {
        anim_id = func_800839F8(found->unk3A, 0, rb);
        if ((anim_id != -1) && (func_80083EEC(found->unk3A, anim_id, arg1) != 0))
        {
            field_start_actor_animation(anim_id, 0, 0);
        }
    }

    found->unk2A = 0xBB;
    found->unk3D = 2;
    found->unk10 = 1;
    found->unk28 = 0;
    found->unk1C = (found->unk1C & ~0x1FF) | 2;
    return 0;
}
