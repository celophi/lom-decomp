#include "common.h"

extern s32 akao_set_song_params(s32, s32, s32, s32);
extern void func_800B2844(s32, void *, s32);

#define FIELD_SLOT_COUNT 5
#define FIELD_SLOT_SIZE 0x60
#define FIELD_SLOT_TABLE_OFFSET 0x2EF4
#define FIELD_SLOT_VALUE_OFFSET 0x2F50
#define FIELD_RANDOM_VALUE_MASK 0xFF00FF00
#define FIELD_FIXED_VALUE_MASK 0x00FF00FF

extern u8 *D_80122B74;
extern u16 g_scene_mode;

extern u8 *func_800C1E40(s32 arg0);
extern s32 *func_800C1EC8(s32 *src, s32 *dest, s32 n);
extern s32 rand(void);

/**
 * @brief Copy an indexed resource record into the first free field slot.
 * @param arg0 Index of the source record in resource 7.
 * @return Allocated slot index, FIELD_SLOT_COUNT if all slots are occupied, or 0xFF if resource 7 is unavailable.
 */
s32 func_800C2264(s32 arg0)
{
    u8 *resource;
    u8 *source_record;
    s32 slot_index;
    s32 slot_offset;
    s32 copy_offset;
    s32 retry_value;
    s32 value;
    s32 random_high;
    s32 scan_index;
    s32 mode;
    u8 *slot;

    resource = func_800C1E40(7);
    if (resource == NULL)
    {
        mode = 0x8001;
        akao_set_song_params(mode, 0x78, arg0, g_scene_mode);
        return 0xFF;
    }

    mode = 1;
    source_record = resource + ((((arg0 << mode) + arg0) << 5) + 4);
    func_800B2844(mode, source_record, 0x15);

    slot_index = 0;
    do
    {
        copy_offset = slot_index * FIELD_SLOT_SIZE + FIELD_SLOT_TABLE_OFFSET;
        slot_offset = slot_index * FIELD_SLOT_SIZE;
        if (*(D_80122B74 + slot_offset + FIELD_SLOT_TABLE_OFFSET) == 0)
        {
            func_800C1EC8((s32 *)source_record, (s32 *)(D_80122B74 + copy_offset), FIELD_SLOT_SIZE);
            retry_value = -1;
            do
            {
                random_high = rand();
                value = (((random_high << 0x10) + rand()) & FIELD_RANDOM_VALUE_MASK) |
                        ((*(u16 *)(D_80122B74 + 0xD4) + (*(u16 *)(D_80122B74 + 0xD6) << 0x10)) & FIELD_FIXED_VALUE_MASK);
                if (value != 0)
                {
                    retry_value = 0;
                }
                scan_index = 0;
                do
                {
                    slot = D_80122B74 + scan_index * FIELD_SLOT_SIZE;
                    if (slot[FIELD_SLOT_TABLE_OFFSET] != 0 && *(s32 *)(slot + FIELD_SLOT_VALUE_OFFSET) == value)
                    {
                        retry_value = -1;
                    }
                    scan_index++;
                } while (scan_index < FIELD_SLOT_COUNT);
            } while (retry_value != 0);
            *(s32 *)(D_80122B74 + slot_offset + FIELD_SLOT_VALUE_OFFSET) = value;
            return slot_index;
        }
        slot_index++;
    } while (slot_index < FIELD_SLOT_COUNT);
    return slot_index;
}


extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;
extern s32 D_801227F0;

s32 func_800C23F4(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < 5)
        {
            u8 **buffer = &D_80122B74;
            s32 offset = index * 0x60 + 0x2EF4;
            func_800B2844(0, *buffer + offset, 0x15);
            index = g_gosub_result_values[0];
            if (index != *(s32 *)(*buffer + 0x2EF0))
            {
                {
                    u8 *entry = *buffer;
                    entry += index * 0x60;
                    entry[0x2EF4] = 0;
                }
                return g_gosub_result_values[0];
            }
            return 5;
        }
        akao_set_song_params(0x8001, 0x6E, index, 0);
    }
    return 0xFF;
}


extern u8 *D_80122B74;

/**
 * @brief Process the indexed field audio entry and report its state.
 * @param index Field audio entry index.
 * @return Entry state code, or the audio command result for indices outside the table.
 */
s32 func_800C24BC(s32 index)
{
    s32 offset;
    s32 value;
    u8 *base;

    if (index >= 5)
    {
        return akao_set_song_params(0x8001, 0x76, index, 0);
    }

    base = D_80122B74;
    offset = index * 0x60;
    if ((base + offset)[0x2EF4] != 0)
    {
        u8 *entry;

        func_800B2844(0, D_80122B74 + (offset + 0x2EF4), 0x15);
        entry = D_80122B74 + offset;
        value = *(s32 *)(entry + 0x2F38);
        if (value < 0)
        {
            if (*(u16 *)(entry + 0x2F36) != 0)
            {
                return 0;
            }
            *(s32 *)(entry + 0x2F38) = value & 0x7FFFFFFF;
            return 1;
        }
        if (((u32) value >> 30) & 1)
        {
            return 2;
        }
        return 3;
    }
    return 0xFF;
}


extern u8 *D_80122B74;

/**
 * @brief Re-arm the pending-input record for a field object, or notify audio.
 *
 * For an in-range @p arg0 (< 5), resolves the object's 0x60-stride record at
 * @c D_80122B74 + 0x2EF4 and hands it to func_800B2844 and field_run_name_entry (using
 * the record's @c 0x2F09 count byte). Out-of-range indices notify the audio
 * driver instead.
 *
 * @param arg0 Field-object index; >= 5 takes the audio-notification path.
 * @see decomp.me (100%) TODO
 */
void func_800C25A0(s32 arg0)
{
    s32 temp_s1;
    s32 off;
    u8 *addr;

    if (arg0 >= 5)
    {
        akao_set_song_params(0x8001, 0x77, arg0, 0);
        return;
    }
    /* Force the D_80122B74 base high-half to materialize before the index. */
    if (D_80122B74)
    {
    }
    temp_s1 = arg0 * 3 << 5;
    off = temp_s1 + 0x2EF4;
    func_800B2844(0, D_80122B74 + off, 0x15);
    addr = D_80122B74 + off;
    field_run_name_entry(addr, addr, 3, *(u8 *)(D_80122B74 + temp_s1 + 0x2F09), 0);
}
