#include "common.h"

void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Finds an offset-table record with the requested byte identifier.
 *
 * Searches the records referenced by the table following its count word. A
 * failed search issues an AKAO diagnostic command and returns null.
 *
 * @param base Base of the count and record-offset table.
 * @param value Byte identifier to find at record offset 0x18.
 * @return Pointer to the matching record, or null when absent.
 */
u8 *func_800B4844(u32 *base, s32 value)
{
    u32 index;
    u32 count;
    u32 loaded_count;
    u32 *offset;
    u8 *record;
    s32 target;

    index = 0;
    loaded_count = *base;
    target = value & 0xFF;
    if (loaded_count != 0)
    {
        count = loaded_count;
        offset = base;
        do
        {
            record = (u8 *)base + offset[1];
            if (record[0x18] != target)
            {
                index++;
                offset++;
            }
            else
            {
                return record;
            }
        } while (index < count);
    }

    akao_set_song_params(0x8001, 0x67, target, -1);
    return 0;
}
