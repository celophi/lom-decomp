#include "common.h"

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
