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

extern FieldSequenceSlot D_80105AE0[];
extern FieldSequenceBank D_800FD818[];
extern FieldSequenceMotion D_800FE3A0[];
extern FieldSequenceResource g_field_resource_entries[];
extern u8 D_8010AED0[];
s32 field_object_has_active_actor_tracks(u8);
void field_stop_actor_animations_for_object(FieldSequenceRecord *, s32);
void func_8006C3FC(FieldSequenceRecord *);
void func_8008A678(s32);
void func_800952DC(FieldSequenceRecord *, s32);
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
    slot_base = D_80105AE0;
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
        active_banks = D_800FD818;
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
    banks = D_800FD818;
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
        final_base = D_80105AE0;
        final_slot = &final_base[record->object_id];
        final_slot->track_flags &= ~0x4000;
    }
}
