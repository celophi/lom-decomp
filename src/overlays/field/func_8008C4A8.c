#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x14 - 0x10];
    s32 unk14;
    u8 pad18[0x18D - 0x18];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} FieldSlotState;

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldActorState;

typedef struct
{
    u8 pad0;
    u8 unk1;
    u8 pad2[0x268 - 2];
} FieldActorTypeState;

extern FieldSlotState D_80105AE0[];
extern FieldActorState D_800FDF58[];
extern FieldActorTypeState D_800FD818[];
extern u8 D_800EB068[];

s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);
void func_8008C620(FieldActorState *actor);

/**
 * @brief Advance an actor slot retry counter and restart its associated animation when its retry threshold is reached.
 * @param slot_index Actor slot index to update.
 */
void func_8008C4A8(s32 slot_index)
{
    FieldSlotState *slots;
    FieldSlotState *slot;
    s32 retry_count;
    s32 key;
    FieldSlotState *scan_slot;
    FieldActorState *scan_actor;
    FieldActorState *found_actor;
    s32 i;
    s32 animation_slot;

    slots = D_80105AE0;
    slot = &slots[slot_index];
    if (slot->unkC & 0x8000)
    {
        retry_count = slot->unk18D + 1;
        slot->unk18D = retry_count;
        if ((slot_index < 2) && ((u32)(retry_count & 0xFF) >= (u8)D_800EB068[D_800FD818[slot_index].unk1]))
        {
            key = slot->unk14;
            scan_actor = D_800FDF58;
            scan_slot = slots;
            for (i = 0; i < 0xD; i++, scan_slot++, scan_actor++)
            {
                if (scan_slot->unk14 == key)
                {
                    found_actor = scan_actor;
                    goto scan_done;
                }
            }
            found_actor = (FieldActorState *)-1;
        scan_done:
            if (found_actor != (FieldActorState *)-1)
            {
                animation_slot = func_800839F8(0, 0);
                if ((animation_slot != -1) && (func_80083EEC(found_actor->unk3A, animation_slot, 0x21) != 0))
                {
                    field_start_actor_animation(animation_slot, 0, 0);
                }
            }
            func_8008C620(&D_800FDF58[slot_index]);
        }
    }
}
