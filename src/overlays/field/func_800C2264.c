#include "common.h"

#define FIELD_SLOT_COUNT 5
#define FIELD_SLOT_SIZE 0x60
#define FIELD_SLOT_TABLE_OFFSET 0x2EF4
#define FIELD_SLOT_VALUE_OFFSET 0x2F50
#define FIELD_RANDOM_VALUE_MASK 0xFF00FF00
#define FIELD_FIXED_VALUE_MASK 0x00FF00FF

extern u8 *D_80122B74;
extern u16 g_scene_mode;

extern u8 *func_800C1E40(s32 arg0);
extern void func_800B2844(s32 arg0, u8 *arg1, s32 arg2);
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
