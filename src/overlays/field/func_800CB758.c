#include "common.h"
/** @brief Packed layout entry, including its active bit at bit sixteen. */
typedef union
{
    u32 word;
    struct
    {
        unsigned low : 16;
        unsigned active : 1;
        unsigned high : 15;
    } bits;
} Packed;
extern u8 D_800F1CD0[];
extern s32 D_80122C00;
extern u8 g_menuLayoutBuffer[];
/**
 * @brief Rebuild the menu grid's entry indices from the active composite layouts.
 * @return Grid bound stored in the selected layout's high nibble.
 * @note Empty cells receive 99; each layout part writes its owning entry index.
 * @note 100% match with GCC 2.7.2 CDK: 112 instructions, 448 bytes.
 */
u32 func_800CB758(void)
{
    s32 empty = 99;
    s32 index, part, count, base, table, x, z, clear_base, return_base;
    u32 packed, value;
    Packed bits;
    u8 *entry, *shape;
    index = 35;
    clear_base = (s32)g_menuLayoutBuffer;
    do
    {
        ((u8 *)(index * 4 + clear_base))[0x2A7F] = empty;
        index--;
    } while (index >= 0);
    index = 0;
    count = g_menuLayoutBuffer[0x29D6];
    if (count != 0)
    {
        table = (s32)D_800F1CD0;
        base = (s32)g_menuLayoutBuffer;
        do
        {
            entry = (u8 *)((s32)g_menuLayoutBuffer + index * 4);
            packed = *(u32 *)(entry + 0x29DC);
            bits.word = packed;
            if (bits.bits.active == 1 && (packed & 3) == ((u8 *)(D_80122C00 + base))[0x29D8])
            {
                part = 0;
                if (*(u8 *)((((packed >> 12) & 15) * 0x58) + table) != 0)
                {
                    do
                    {
                        value = *(u32 *)(entry + 0x29DC);
                        shape = (u8 *)(((((value >> 17) & 3) * 5 + part) * 4) +
                                       (((value >> 12) & 15) * 0x58) + table);
                        x = ((s32)(value << 8) >> 27) + (s8)shape[12];
                        z = ((s32)(value << 3) >> 27) + (s8)shape[13];
                        x += z * 6;
                        ((u8 *)(x * 4 + base))[0x2A7F] = index;
                        part++;
                    } while (part <
                             *(u8 *)((((*(u32 *)(entry + 0x29DC) >> 12) & 15) * 0x58) + table));
                }
            }
            index++;
        } while (index < count);
    }
    return_base = (s32)g_menuLayoutBuffer;
    return ((u8 *)(((u8 *)(D_80122C00 + return_base))[0x29D8] * 0x14C + return_base))[0x2B50] >> 4;
}
