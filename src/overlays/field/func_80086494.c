#include "common.h"

/** @brief Color and animation selector bytes in a 0x54-byte object record. */
typedef struct
{
    u8 pad0[0x18];
    u8 unk18, unk19, unk1a;
    u8 pad1b[10];
    u8 unk25;
    u8 pad26[0x14];
    u8 unk3a;
    u8 tail[0x19];
} FieldControlRecord;
/** @brief Current and previous control flags in a 0x23C-byte runtime state. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkc;
    u8 pad10[0x168];
    union
    {
        s32 word;
        struct
        {
            u8 low[2];
            u8 owner;
            u8 high;
        } bytes;
    } status;
    s32 unk17c;
    u8 tail[0xBC];
} FieldControlState;
/** @brief Animation activity and kind in a 0x244-byte animation actor. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x201];
    u16 unk226;
    u8 pad228[0x12];
    u8 unk23a;
    u8 tail[9];
} FieldControlActor;
/** @brief Color bytes in a 0x48-byte visual record. */
typedef struct
{
    u8 pad0[0xE];
    u8 unke, unkf, unk10;
    u8 tail[0x37];
} FieldControlVisual;
void field_start_actor_animation(s32, s32, s32); /* extern */
void func_8006D21C(FieldControlActor *);         /* extern */
s32 func_80083EEC(s32, s32, s32);                /* extern */
extern u32 D_800EB00C[];
extern FieldControlActor D_800FB3C8[];
extern FieldControlRecord D_800FDF58[];
extern FieldControlVisual D_800FE3A0[];
extern FieldControlState D_80105AE0[];

/**
 * @brief Dispatch one changed actor control flag and refresh its displayed colors.
 * @param index Object, runtime-state and animation-actor slot index.
 * @note Action values below 0x100 name animations; larger values are callbacks.
 * @note Separate flag masks and explicit table bases preserve target code generation.
 * @note WIP: approximately 94.16% GCC 2.7.2 CDK, with allocation and scheduling residue.
 */
void func_80086494(s32 index)
{
    s32 flags_before;
    s32 changed_or_current;
    s32 previous_flags;
    s32 record_offset;
    s32 animation_slot;
    u32 changed_flags;
    s32 flags_current;
    s32 old_flags;
    s32 bit_index;
    s32 clear_mask;
    s32 animation_bit;
    s32 highest_bit;
    s32 bit_mask;
    s32 stop_mask;
    u16 animation_kind;
    u32 *action;
    u32 action_value;
    FieldControlActor *actor;
    FieldControlRecord *record;
    FieldControlState *runtime;
    FieldControlRecord *reset_record;
    FieldControlVisual *visual;
    FieldControlRecord *color_record;

    record_offset = index * 0x54;
    record = (FieldControlRecord *)(record_offset + (u8 *)D_800FDF58);
    runtime = &D_80105AE0[index];
    flags_before = runtime->unkc;
    actor = &D_800FB3C8[index];
    if (flags_before & 0x23E4)
    {
        runtime->unkc &= ~0x4000;
        runtime->unkc &= 0xFFFF7FFF;
    }
    flags_current = runtime->unkc;
    previous_flags = runtime->unk17c;
    changed_or_current = (flags_current ^ previous_flags) | flags_current;
    if (changed_or_current != 0)
    {
        if (actor->unk24 == 0)
        {
            bit_mask = 0x8000;
            bit_index = 0xF;
            animation_slot = index + 0x40;
            action = D_800EB00C;
            action += bit_index;
        find_action:
            if ((changed_or_current & bit_mask) && (action_value = *action, (action_value != 0xFF)))
            {
                if (action_value < 0x100U)
                {
                    if (runtime->unkc & bit_mask)
                    {
                        func_80083EEC(index, animation_slot, action_value);
                        field_start_actor_animation(animation_slot, 0, 0);
                        clear_mask = ~bit_mask;
                    }
                    else
                    {
                        goto update_flags;
                    }
                }
                else
                {
                    ((void (*)(FieldControlRecord *, s32))action_value)(
                        (FieldControlRecord *)(record_offset + (u8 *)D_800FDF58), runtime->unkc & bit_mask);
                update_flags:
                    clear_mask = ~bit_mask;
                }
                runtime->unk17c = (s32)((runtime->unk17c & clear_mask) | (runtime->unkc & bit_mask));
            }
            else
            {
                action--;
                bit_index -= 1;
                bit_mask = bit_mask >> 1;
                if (bit_index < 0)
                {
                }
                else
                {
                    goto find_action;
                }
            }
        }
        else
        {
            if (!(flags_current & 0x8000) && (previous_flags & 0x8000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                func_8006D21C(actor);
                stop_mask = 0xFFFF7FFF;
                goto clear_stopped;
            }
            if (!(runtime->unkc & 0x4000) && (runtime->unk17c & 0x4000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                func_8006D21C(actor);
                stop_mask = -0x4001;
            clear_stopped:
                runtime->unk17c = (s32)(runtime->unk17c & stop_mask);
            }
            else
            {
                old_flags = runtime->unk17c;
                changed_flags = (runtime->unkc ^ old_flags) & old_flags;
                if (changed_flags != 0)
                {
                    highest_bit = 0xF;
                find_highest:
                    if ((changed_flags & (1 << highest_bit)) == 0)
                    {
                        highest_bit -= 1;
                        goto find_highest;
                    }
                    animation_kind = actor->unk226;
                    animation_bit = 0x20;
                    switch (animation_kind)
                    {
                    case 5:
                        animation_bit = 0;
                        break;
                    case 6:
                        animation_bit = 1;
                        break;
                    case 7:
                        animation_bit = 2;
                        break;
                    case 8:
                        animation_bit = 3;
                        break;
                    case 11:
                        animation_bit = 4;
                        break;
                    case 10:
                        animation_bit = 5;
                        break;
                    case 9:
                        animation_bit = 6;
                        break;
                    case 12:
                        animation_bit = 7;
                        break;
                    }
                    if (highest_bit == animation_bit)
                    {
                        actor->unk24 = 0U;
                        actor->unk23a = 0;
                        func_8006D21C(actor);
                        if ((runtime->status.word & 1) && (runtime->status.bytes.owner == (record->unk3a + 0x40)))
                        {
                            record->unk25 = 0;
                            runtime->status.word = (s32)(runtime->status.word & ~1);
                        }
                    }
                }
            }
        }
    }
    if (runtime->unkc & 0x10000000)
    {
        reset_record = D_800FDF58;
        reset_record += index;
        reset_record->unk1a = 0x20;
        reset_record->unk19 = 0x20;
        runtime->unkc = (s32)(runtime->unkc & 0xEFFFFFFF);
        return;
    }
    visual = D_800FE3A0;
    visual += index;
    color_record = D_800FDF58;
    color_record += index;
    color_record->unk18 = (u8)visual->unke;
    color_record->unk19 = (u8)visual->unkf;
    color_record->unk1a = (u8)visual->unk10;
}
