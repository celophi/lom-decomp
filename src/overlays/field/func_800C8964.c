#include "common.h"

extern u8 D_80122C00[];
extern u8 g_menuLayoutBuffer[];
extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

/**
 * @brief Scan the five menu layout records and build a compacted index of the
 *        active entries whose flag bit 30 is set.
 * @note For each active record (offset 0x2EF4 non-zero) with bit 30 of the word
 *       at 0x2F38 set, appends its 0x2F09 id and slot index to the D_80122C00
 *       tables and clears the record's 0x2EF4 marker via func_800B2844.
 * @see decomp.me (100.00%)
 */
void func_800C8964(void)
{
    s32 count;
    s32 i;
    u8 *ids;
    u8 *slots;
    u8 *record;
    u8 *arg;
    u32 bit;

    count = 0;
    i = count;
    ids = D_80122C00;
    slots = ids + 0x1D;
    record = g_menuLayoutBuffer;
    arg = record + 0x2EF4;
    do
    {
        record = &g_menuLayoutBuffer[i * 0x60];
        if (record[0x2EF4] != 0)
        {
            bit = (*(u32 *)(record + 0x2F38) >> 30) & 1;
            if (bit == 1)
            {
                *(u8 *)((u32)count + (u32)ids) = record[0x2F09];
                *(u8 *)((u32)count + (u32)slots) = i;
                func_800B2844(count, arg, 0xFF);
                count++;
            }
        }
        arg += 0x60;
        i++;
    } while (i < 5);
}
