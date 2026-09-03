#include "common.h"

extern u8 D_800F18CC[];
extern s32 func_800C19D0(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Advance eight packed 9-bit counters by a per-slot nibble delta, clamped.
 *
 * Looks up a 32-bit table word in @c D_800F18CC indexed by
 * @c ((*(u32 *)(arg0 + 0x64) >> 8) & 0xFC), treating its low 32 bits as eight
 * 4-bit deltas. For each of the eight u16 values at @c arg0+0x30, adds the next
 * nibble to the low 9 bits (mod 0x200), preserving the upper 7 bits, and clamps
 * the result to 0x18C when it reaches or exceeds 0x18D. Finally updates the u16
 * at @c arg0+0x24 from the fifth counter (at @c arg0+0x38) via @c func_800C19D0.
 *
 * @param arg0 Pointer to the record holding the packed counters and control fields.
 */
void func_800C15AC(u8 *arg0)
{
    s32 i;
    u32 bits;
    u16 x;
    u32 low;
    u8 *table;

    i = 0;
    table = D_800F18CC;
    bits = *(u32 *)(table + ((*(u32 *)(arg0 + 0x64) >> 8) & 0xFC));
    do
    {
        x = *(u16 *)(arg0 + 0x30 + i * 2);
        low = (x & 0x1FF) + (bits & 0xF);
        low &= 0x1FF;
        x = (x & 0xFE00) | low;
        *(u16 *)(arg0 + 0x30 + i * 2) = x;
        if ((u32)(x & 0x1FF) >= 0x18D)
        {
            *(u16 *)(arg0 + 0x30 + i * 2) = (x & 0xFE00) | 0x18C;
        }
        bits >>= 4;
        i++;
    } while (i < 8);
    *(u16 *)(arg0 + 0x24) = func_800C19D0(*(u16 *)(arg0 + 0x24), ((u32)(*(u16 *)(arg0 + 0x38) & 0x1FF)) >> 2, 3);
}
