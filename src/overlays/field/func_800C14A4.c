#include "common.h"

extern u8 *D_80122B74;
void func_800C1658(u8 *record, u8 *base, u8 *current, s32 offset);
void func_800C15AC(u8 *record, u8 *base, u8 *current, s32 offset);
void func_800B7C58(s32 index);
s32 func_8008B500(s32 index, s32 value);

/**
 * @brief Advance the indexed field record when its packed progression reaches the next threshold.
 * @param index Index of the 0x250-byte field record to update.
 * @param notify Nonzero to emit update code 0x23 after advancing the record.
 * @return -1 when the record advances, otherwise 0.
 */
s32 func_800C14A4(s32 index, s32 notify)
{
    u8 *initial_base;
    u8 *record;
    u8 *current;
    u_long level_or_base;
    u32 packed;
    s32 previous_level;
    s32 scaled_level;
    s32 call_offset;

    initial_base = D_80122B74;
    record = initial_base + index * 0x250;
    level_or_base = record[0x610];
    packed = *(u32 *)(record + 0x610);
    previous_level = level_or_base - 1;
    scaled_level = (level_or_base << 2) + level_or_base;
    if ((s32)(packed >> 8) >= previous_level * (scaled_level << 2) + (scaled_level << 1))
    {
        record[0x610] = (u8)(level_or_base + 1);
        level_or_base = (u_long)D_80122B74;
        current = (u8 *)level_or_base + index * 0x250;
        if (current[0x610] >= 0x64)
        {
            current[0x610] = 0x63;
            return 0;
        }

        call_offset = index * 0x250 + 0x5F0;
        if ((current[0x608] & 0x7F) == 3)
        {
            func_800C1658((u8 *)level_or_base + call_offset, (u8 *)level_or_base, record, index * 0x250);
        }
        else
        {
            func_800C15AC((u8 *)level_or_base + call_offset, (u8 *)level_or_base, record, index * 0x250);
        }

        func_800B7C58(index);
        if (notify != 0)
        {
            func_8008B500(index, 0x23);
        }
        return -1;
    }
    return 0;
}
