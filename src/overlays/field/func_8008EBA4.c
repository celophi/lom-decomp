#include "common.h"
#include "sdk/libgte.h"

/** @brief Partial field record containing direction and animation state. */
typedef struct FieldMotionRecord
{
    u8 pad00[0x1B];
    u8 direction;
    u8 pad1C[5];
    u8 animation;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 animation_frame;
    u8 pad28[6];
    u16 idle_mode;
    u8 pad30[3];
    u8 movement_flags;
    u8 pad34[5];
    u8 stop_delay;
    u8 pad3A;
    u8 resource_index;
} FieldMotionRecord;

/** @brief Resource metadata entry with animation-layout flags at offset 0x10. */
typedef struct FieldMotionResource
{
    u8 pad00[0x10];
    u32 flags;
} FieldMotionResource;

extern FieldMotionResource g_field_resource_entries[];
extern s32 D_800EB0A4[];
extern s32 D_800EB0C4[];
extern s32 D_800EB0E4[];
extern s32 rand(void);
extern void func_8006C3FC(FieldMotionRecord *record);

/**
 * @brief Select a movement animation from the requested displacement, or settle to idle.
 * @param record Field record whose direction and animation state are updated.
 * @param delta_x Horizontal displacement used to select a direction.
 * @param delta_z Depth displacement used to select a direction.
 * @note Resource flag 1 selects the alternate directional animation tables.
 * @note WIP: 98.165140% gcc272_cdk; two instruction scheduling positions differ.
 */
void func_8008EBA4(FieldMotionRecord *record, s32 delta_x, s32 delta_z)
{
    s32 *vertical_entry;
    s32 *direction_entry;
    s32 sector_index;
    s32 direction_or_animation;
    s32 movement_direction;
    s32 old_animation;
    s32 direction_table_address;
    s32 sector_or_flags;
    s32 direction_offset;
    u8 resource_index;
    u8 current_animation;
    s32 previous_animation;
    s32 low_state;
    s32 idle_state;
    s32 state_mask;
    u8 idle_delay;
    u8 movement_delay;

    resource_index = record->resource_index;
    if (g_field_resource_entries[resource_index].flags & 1)
    {
        if ((delta_x | delta_z) != 0)
        {
            direction_or_animation = ratan2(-delta_z, delta_x);
            direction_or_animation >>= 4;
            direction_or_animation += 0x10;
            if (direction_or_animation < 0)
            {
                direction_or_animation += 0x100;
            }
            if (direction_or_animation >= 0x100)
            {
                direction_or_animation -= 0x100;
            }
            sector_or_flags = direction_or_animation >> 5;
            if ((sector_or_flags == 2) || (sector_or_flags == 6))
            {
                state_mask = ~0x80;
                vertical_entry = &D_800EB0C4[sector_or_flags];
                if ((record->animation & state_mask) != (*vertical_entry & state_mask))
                {
                    record->direction = (s8)(direction_or_animation & 0xE0);
                    direction_or_animation = (u8)*vertical_entry;
                    old_animation = record->animation;
                    record->animation_frame = 0;
                    record->active = 1;
                    direction_or_animation |= old_animation & 0x80;
                    record->animation = direction_or_animation;
                    func_8006C3FC(record);
                }
            }
            else
            {
                sector_index = sector_or_flags;
                current_animation = record->animation;
                if (((current_animation != D_800EB0C4[sector_index]) && (delta_z == 0)) ||
                    ((current_animation != D_800EB0E4[sector_index]) && (delta_z != 0)))
                {
                    record->direction = (s8)(direction_or_animation & 0xE0);
                    if (delta_z != 0)
                    {
                        record->animation = D_800EB0E4[direction_or_animation >> 5];
                    }
                    else
                    {
                        record->animation = D_800EB0C4[direction_or_animation >> 5];
                    }
                    /* Keep the animation reset after the selected direction store. */
                    do
                    {
                        record->animation_frame = 0;
                        record->active = 1;
                        func_8006C3FC(record);
                    } while (0);
                }
            }
            goto keep_moving;
        }
        idle_delay = record->stop_delay;
        if (idle_delay != 0)
        {
            record->stop_delay = (u8)(idle_delay - 1);
        }
        idle_state = record->animation;
        idle_state &= 0x7F;
        if (((idle_state >= 2) && (record->stop_delay == 0)) ||
            ((idle_state < 2) && (record->idle_mode == 0)))
        {
            /* Reuse the no-longer-needed displacement for the idle facing flag. */
            delta_z = record->animation;
            delta_z &= 0x80;
            delta_z += rand() >= 0x6001;
            record->animation = delta_z;
            record->animation_frame = 0;
            record->idle_mode = 1U;
            record->active = 1;
            func_8006C3FC(record);
        }
    }
    else
    {
        if ((delta_x | delta_z) != 0)
        {
            movement_direction = ratan2(-delta_z, delta_x);
            movement_direction >>= 4;
            movement_direction += 0x10;
            if (movement_direction < 0)
            {
                movement_direction += 0x100;
            }
            if (movement_direction >= 0x100)
            {
                movement_direction -= 0x100;
            }
            direction_offset = movement_direction >> 5;
            direction_table_address = (s32)D_800EB0A4;
            direction_offset *= 4;
            direction_entry = (s32 *)(direction_table_address + direction_offset);
            if (record->animation != (*direction_entry + ((record->movement_flags & 1) * 5) + 5))
            {
                record->direction = (s8)(movement_direction & 0xE0);
                direction_or_animation = (u8)*direction_entry;
                sector_or_flags = record->movement_flags;
                record->animation_frame = 0;
                record->active = 1;
                direction_or_animation += (sector_or_flags & 1) * 5;
                direction_or_animation += 5;
                record->animation = (u8)direction_or_animation;
                func_8006C3FC(record);
            }
        keep_moving:
            record->stop_delay = 3U;
            record->active = 1;
            return;
        }
        movement_delay = record->stop_delay;
        if (movement_delay != 0)
        {
            record->stop_delay = (u8)(movement_delay - 1);
        }
        previous_animation = record->animation;
        low_state = previous_animation;
        low_state &= 0x7F;
        if ((low_state >= 5) && (record->stop_delay == 0))
        {
            record->active = 1;
            record->animation_frame = 0;
            record->animation = (u8)((low_state % 5) | (previous_animation & 0x80));
            func_8006C3FC(record);
        }
    }
}
