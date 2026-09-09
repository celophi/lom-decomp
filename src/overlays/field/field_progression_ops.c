#include "common.h"

extern u8 *D_80122B74;

/**
 * @brief Computes a scaled progression value from a field record's state byte.
 *
 * Reads the state byte at offset 0x610 of the record selected by @p index
 * (records are 0x250 bytes apart in the table at *D_80122B74). Below the
 * threshold 0x63 the return is a scaled progression of that byte,
 * (state - 1) * (state * 20) + state * 10; at or above the threshold the packed
 * 32-bit field at the same offset is returned shifted right by 8 instead.
 *
 * @param index Record index into the table at *D_80122B74.
 * @return The scaled progression below the threshold, otherwise the packed
 *         counter bits of the 32-bit field at offset 0x610.
 */
s32 func_800B607C(s32 index)
{
    u8 *record = D_80122B74 + index * 0x250;
    s32 value = record[0x610];

    if (value < 0x63)
    {
        s32 previous = value - 1;
        s32 scaled = value * 5;
        s32 scaled_x4 = scaled * 4;
        return previous * scaled_x4 + scaled * 2;
    }

    return *(u32 *)(record + 0x610) >> 8;
}


extern u8 *D_80122B74;

s32 func_800C14A4(s32 arg0, s32 arg1);

/**
 * @brief Initialize three field records and wait for each record to become ready.
 * @param arg0 Progression level used to initialize the records.
 */
void func_800B60DC(s32 arg0)
{
    s32 previous;
    s32 scaled;
    s32 scaled_x4;
    s32 record_index;
    s32 base;
    s32 sub_offset;
    s32 count;
    u8 *raw;
    u8 *record;
    u8 *slot;
    s32 progression;

    arg0 = arg0 - 1;
    previous = arg0 - 1;
    scaled = arg0 * 5;
    scaled_x4 = scaled * 4;
    record_index = 0;
    progression = (previous * scaled_x4 + scaled * 2) << 8;
    do
    {
        count = 0;
        base = record_index * 0x250;
        (D_80122B74 + base)[0x610] = 1;
        sub_offset = base;
        record = D_80122B74 + base;
        raw = D_80122B74;
        *(u16 *)(record + 0x614) = 0x32;
        *(u32 *)(record + 0x610) = record[0x610] | progression;

        do
        {
            slot = raw + sub_offset;
            *(u16 *)(slot + 0x620) = (*(u16 *)(slot + 0x620) & 0xFE00) | 0x14;
            sub_offset += 2;
        } while (++count < 8);

        while (func_800C14A4(record_index, 1) != 0)
        {
        }

        record_index += 1;
    } while (record_index < 3);
}
