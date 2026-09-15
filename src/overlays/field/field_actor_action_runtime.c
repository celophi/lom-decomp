/** @file field_actor_action_runtime.c
 * @brief Validate pending actions, clear completed state, and advance actor sequences.
 */

/* field_actor_pending_actions */
/* func_80093AB8 */
#include "common.h"

/** @brief Actor fields used to track and reset pending action state. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x1C - 8];
    s32 unk1C;
    u8 unk20, unk21;
    u8 pad22[8];
    u16 unk2A;
    u8 pad2C[4];
    u16 unk30;
    u8 pad32[8];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Actor;
/** @brief Action flags and mode bytes within a 0x23C-byte object slot. */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[0x16F - 0x10];
    u8 unk16F;
    u8 pad170[0x18D - 0x170];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} Slot;
/** @brief Party record containing the action-type byte at offset one. */
typedef struct
{
    u8 unk0, unk1;
    u8 pad2[0x268 - 2];
} Party;
extern Slot D_80105AE0[];
extern Party D_800FD818[];
void func_8006C5FC(Actor *);
s32 func_8006C7D8(Actor *);
s32 func_800A29F8(s32, s32, s32);
void func_800A2DD8(s32);
/**
 * @brief Update pending actor actions and clear stale action state.
 * @param input Actor whose object slot and pending-action counter are checked.
 * @return One when the actor enters state 0x95; zero otherwise.
 */
s32 func_80093AB8(Actor *input)
{
    Actor *actor = input;
    s32 selection;
    s32 clear_mask;
    s32 flags;
    u16 retry_count;
    u16 count;
    u8 object_index;
    s32 mode;
    Slot *slot;
    Slot *base;
    Slot *reset_slot;

    if (actor->unk1C & 0x1FF)
    {
        D_80105AE0[actor->unk3A].unkC &= 0xFFFF7FFF;
        actor->unk30 = (u16)(actor->unk30 + 1);
        return 0;
    }
    selection = func_800A29F8(actor->unk3A, ((u8)actor->unk21 >> 7) ^ 1, 1);
    base = D_80105AE0;
    slot = &base[actor->unk3A];
    if (((slot->unk16F == 2) || (actor->unk30 != 0)) && (actor->unk4 == 0))
    {
        if (!(actor->unk1C & 0x1FF))
        {
            flags = slot->unkC;
            if (!(flags & 0x400))
            {
                slot->unkC = (s32)(flags | 0x8000);
            }
        }
        if (selection != 3)
        {
            if (selection < 4)
            {
                if (selection == 2)
                {
                    count = actor->unk30;
                    if (count < 5U)
                    {
                        actor->unk30 = (u16)(count + 1);
                    }
                    if (func_8006C7D8(actor) != 0)
                    {
                        retry_count = actor->unk30;
                        if (retry_count < 5U)
                        {
                            if ((u8)actor->unk3A < 2U)
                            {
                                if (D_800FD818[actor->unk3A].unk1 == 0xA)
                                {
                                    if (retry_count >= 3U)
                                    {
                                        goto cancel_pending;
                                    }
                                    goto check_mode;
                                }
                                goto check_mode;
                            }
                            goto check_mode;
                        }
                    }
                cancel_pending:
                    func_800A2DD8(actor->unk3A);
                    clear_mask = 0xFFFF7FFF;
                    actor->unk30 = 0U;
                    D_80105AE0[actor->unk3A].unk18D = 0;
                    reset_slot = &D_80105AE0[actor->unk3A];
                    goto reset_actor;
                }
                goto clear_pending_counter;
            }
        }
    clear_pending_counter:
        D_80105AE0[actor->unk3A].unk18D = 0;
        actor->unk30 = 0U;
    }
check_mode:
    object_index = actor->unk3A;
    mode = D_80105AE0[object_index].unk16F;
    if (mode == 3)
    {
        if (selection != 4 && selection != 6 && selection != 5 && selection != 7 &&
            selection != 8 && selection != 9 && selection != 10)
        {
            func_800A2DD8(object_index);
            clear_mask = 0xFFFF7FFF;
            D_80105AE0[actor->unk3A].unk18D = 0;
            actor->unk30 = 0;
            reset_slot = &D_80105AE0[actor->unk3A];
        reset_actor:
            flags = reset_slot->unkC;
            flags &= clear_mask;
            reset_slot->unkC = flags;
            func_8006C5FC(actor);
            actor->unk2A = 0x95;
            actor->unk20 = 0x14;
            return 1;
        }
    }
    else
    {
        if ((s32)mode >= 3)
        {
            if ((s32)mode < 11)
            {
                if ((s32)mode >= 8)
                {
                    if (selection != 4 && selection != 6 && selection != 5 && selection != 7)
                    {
                        func_800A2DD8(object_index);

                        D_80105AE0[actor->unk3A].unk18D = 0;
                        actor->unk30 = 0;
                        D_80105AE0[actor->unk3A].unkC &= 0xFFFF7FFF;
                    }
                }
            }
        }
    }
    return 0;
}


/* func_80093EB4 */
#include "common.h"
typedef struct { u8 pad0[0x1C]; s32 unk1C; u8 pad20[0xA]; s16 unk2A; u8 pad2C[2]; u16 unk2E; u8 pad30[0xA]; u8 unk3A; } FieldRecord;
typedef struct { u8 pad0[0x4A]; s16 unk4A; u8 pad4C[0x128]; s32 unk174; u8 pad178[1]; u8 unk179; u8 pad17A[0xC2]; } FieldState;
typedef struct { u8 pad0[0x228]; u8 unk228; u8 pad229[0x11]; u8 unk23A; u8 pad23B[9]; } ActorSlot;
void func_8008A678(); void func_800952DC(); void func_800A2DD8(s32);
 extern ActorSlot g_field_actor_slots[];

/**
 * @brief Clear field state for a record after validating its linked actor slot.
 * @param arg0 Field record whose state is updated.
 */
void func_80093EB4(FieldRecord *arg0)
{
    ActorSlot *slot;
    FieldState *state;
    FieldState *states = (FieldState *)D_80105AE0;
    u8 index;
    u8 slotIndex;
    s32 gate;

    index = arg0->unk3A;
    state = &states[index];
    slotIndex = state->unk179;
    if (slotIndex == 0xFF)
    {
        if (arg0->unk2E != 0)
        {
            return;
        }
        state->unk174 &= ~0x1800;
        arg0->unk2A = 0;
        arg0->unk1C &= ~0x800;
        states[arg0->unk3A].unk4A = 0;
        if ((u8)arg0->unk3A < 2)
        {
            func_800A2DD8(arg0->unk3A);
        }
    }
    else
    {
        gate = arg0->unk2E;
        slot = &g_field_actor_slots[slotIndex];
        if (gate != 0 || (slot->unk23A != 0 && slot->unk228 == index))
        {
            return;
        }
        gate = index;
        func_8008A678(gate, index);
        arg0->unk2A = 0;
        gate = ~0x1800;
        states[arg0->unk3A].unk174 &= gate;
        if (slot->unk228 == arg0->unk3A)
        {
            func_800952DC(arg0, 1);
        }
        arg0->unk1C &= ~0x800;
        states[arg0->unk3A].unk4A = 0;
        if ((u8)arg0->unk3A < 2)
        {
            func_800A2DD8(arg0->unk3A);
        }
    }
}


/* func_8009403C */
#include "common.h"
#include "vector.h"

/** @brief Accessed fields of an actor sequence record. */
typedef struct
{
    u8 pad00[0x16];
    s16 motion_divisor;
    u8 pad18[4];
    u32 sequence_flags;
    u8 pad20;
    u8 facing_flags;
    u8 pad22[8];
    s16 sequence_state;
    u8 pad2c[2];
    u16 sequence_delay;
    u8 pad30[6];
    s8 motion_remainder;
    u8 pad37[3];
    u8 object_id;
    u8 resource_id;
} FieldSequenceRecord;

/** @brief Sequence state within one 0x23C-byte actor slot. */
typedef struct
{
    u8 pad00[0xC];
    u32 object_flags;
    u8 pad10[0x44 - 0x10];
    s32 script_position;
    u8 pad48[0x171 - 0x48];
    u8 command_delay;
    u8 pad172[2];
    u32 track_flags;
    u8 pad178[3];
    u8 repeat_state;
    u8 pad17c[0x23C - 0x17C];
} FieldSequenceSlot;

/** @brief Script bank selector within a 0x268-byte object record. */
typedef struct
{
    u8 pad00;
    u8 bank;
    u8 pad02[0x268 - 2];
} FieldSequenceBank;

/** @brief Motion multiplier within a 0x48-byte object record. */
typedef struct
{
    u8 pad00[0x2E];
    u8 scale;
    u8 pad2f[0x48 - 0x2F];
} FieldSequenceMotion;

/** @brief Mode byte within one 0x14-byte resource entry. */
typedef struct
{
    u8 pad00[8];
    u8 mode;
    u8 pad09[0x14 - 9];
} FieldSequenceResource;



extern FieldSequenceMotion D_800FE3A0[];
extern FieldSequenceResource g_field_resource_entries[];
extern u8 D_8010AED0[];
s32 field_object_has_active_actor_tracks(u8);
void field_stop_actor_animations_for_object(FieldSequenceRecord *, s32);
void func_8006C3FC(FieldSequenceRecord *);
void func_8008A678(s32);
void func_800952DC();
s32 func_800954F0(FieldSequenceRecord *, s32);
s32 func_80097FA0(FieldSequenceRecord *, Vec3i *, s32);
void func_800A2DD8(s32);

/**
 * @brief Advance an actor sequence and apply its pending horizontal motion.
 * @param record Actor sequence record to update.
 * @param sequence_index Script row within the object's selected bank.
 * @note Command 0xF1 advances the cursor; 0xEF handles sequence completion.
 */
void func_8009403C(FieldSequenceRecord *record, s32 sequence_index)
{
    Vec3i *scratch = (Vec3i *)0x1F800000;
    FieldSequenceSlot *slot_base;
    FieldSequenceSlot *slot;
    FieldSequenceSlot *timer_slot, *reset_slot, *final_slot;
    FieldSequenceSlot *final_base;
    u8 *active_scripts;
    FieldSequenceBank *active_banks;
    s32 active_address, active_position;
    s32 first_position;
    s32 bank;
    FieldSequenceBank *banks;
    FieldSequenceMotion *motion;
    u8 *scripts;
    s32 object;
    s32 position;
    s32 row_offset;
    s32 row_address;
    s32 combined_address;
    s32 amount;
    u8 delay;
    u8 command;

    if (record->sequence_delay != 0)
    {
        goto apply_motion;
    }
    slot_base = ((FieldSequenceSlot *)D_80105AE0);
    timer_slot = &slot_base[record->object_id];
    delay = timer_slot->command_delay;
    if (delay != 0)
    {
        timer_slot->command_delay = delay - 1;
        if (slot_base[record->object_id].command_delay != 0)
        {
            goto apply_motion;
        }
    }
    if (field_object_has_active_actor_tracks(record->object_id) != 0)
    {
        object = record->object_id;
        active_position = slot_base[object].script_position;
        if (active_position == 1)
        {
            goto apply_motion;
        }
        active_scripts = D_8010AED0;
        do
        {
            active_scripts = D_8010AED0;
        } while (0);
        active_banks = ((FieldSequenceBank *)D_800FD818);
        active_address = (sequence_index << 5) + active_banks[object].bank * 0x300;
        active_address += (s32)active_scripts;
        command = *(u8 *)(active_address + active_position);
        if (command == 0xFF || command == 0xF1)
        {
            goto apply_motion;
        }
    }
    reset_slot = &slot_base[record->object_id];
    if (reset_slot->script_position == 1)
    {
        reset_slot->track_flags &= ~0x1800;
        slot_base[record->object_id].repeat_state = 0;
    }
    scripts = D_8010AED0;
    banks = ((FieldSequenceBank *)D_800FD818);
    object = record->object_id;

    do
    {
        bank = banks[object].bank;
        row_offset = sequence_index << 5;
        row_address = bank << 1;
        row_address += bank;
        row_address <<= 8;
        combined_address = row_offset + row_address;
        row_address = combined_address;
    } while (0);
    slot = &slot_base[object];
    first_position = slot->script_position;
    row_address += (s32)scripts;
    row_address += first_position;
    if (*(u8 *)row_address == 0xF1)
    {
        slot->script_position = first_position + 1;
    }
    object = record->object_id;
    do
    {
        bank = banks[object].bank;
        row_address = bank << 1;
        row_address += bank;
        row_address <<= 8;
        combined_address = row_offset + row_address;
        row_address = combined_address;
    } while (0);
    slot = &slot_base[object];
    position = slot->script_position;
    row_address += (s32)scripts;
    row_address += position;
    if (*(u8 *)row_address == 0xEF)
    {
        if (slot->repeat_state == 0)
        {
            func_8008A678(object);
            field_stop_actor_animations_for_object(record, 1);
            goto stop_sequence;
        }
        slot->script_position = position + 1;
        return;
    }
    if (func_800954F0(record, sequence_index) != 0)
    {
        func_8008A678(record->object_id);
    stop_sequence:
        record->sequence_state = 0;
        func_800952DC(record, 1);
        record->sequence_flags &= ~0x800;
        if (record->object_id < 2U)
        {
            func_800A2DD8(record->object_id);
        }
        slot_base[record->object_id].object_flags &= ~0x4000;
        slot_base[record->object_id].object_flags &= 0xFFFF7FFF;
        return;
    }
    func_8006C3FC(record);
    record->sequence_flags |= 0x800;
apply_motion:
    amount = (s8)record->motion_remainder / (s16)record->motion_divisor;
    motion = &D_800FE3A0[record->object_id];
    record->motion_remainder = (u8)record->motion_remainder - amount;
    if (record->facing_flags & 0x80)
    {
        scratch->x = ((amount << 8) * motion->scale) >> 6;
    }
    else
    {
        scratch->x = (-(amount << 8) * motion->scale) >> 6;
    }
    scratch->z = 0;
    scratch->y = 0;
    func_80097FA0(record, scratch, 1);
    if (g_field_resource_entries[record->resource_id].mode == 0)
    {
        final_base = ((FieldSequenceSlot *)D_80105AE0);
        final_slot = &final_base[record->object_id];
        final_slot->track_flags &= ~0x4000;
    }
}
