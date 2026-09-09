#include "common.h"

void func_800CA1E0(void);
void func_800CA1A0(s32 arg0);

/** @brief Reset layout slots and activate the default set in order. */
void func_800C8830(void)
{
    func_800CA1E0();
    func_800CA1A0(0);
    func_800CA1A0(1);
    func_800CA1A0(2);
    func_800CA1A0(3);
    func_800CA1A0(4);
    func_800CA1A0(5);
    func_800CA1A0(7);
    func_800CA1A0(8);
    func_800CA1A0(9);
    func_800CA1A0(0xA);
    func_800CA1A0(0xB);
    func_800CA1A0(0xC);
    func_800CA1A0(0xD);
    func_800CA1A0(0xF);
    func_800CA1A0(0x10);
    func_800CA1A0(0x11);
    func_800CA1A0(0x12);
    func_800CA1A0(0x13);
    func_800CA1A0(0x15);
    func_800CA1A0(0x16);
    func_800CA1A0(0x18);
    func_800CA1A0(0x19);
    func_800CA1A0(0x1A);
    func_800CA1A0(0x1B);
    func_800CA1A0(0x1E);
    func_800CA1A0(0x1F);
    func_800CA1A0(0x20);
    func_800CA1A0(0x20);
    func_800CA1A0(0x17);
}

extern s32 D_8011F428;
extern u8 D_80122C1E;

/** @brief Restore the saved field mode through the mode dispatcher. */
void func_800C8938(void)
{
    D_8011F428 = (s32) D_80122C1E;
    func_800AD120(D_80122C1E);
}

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
