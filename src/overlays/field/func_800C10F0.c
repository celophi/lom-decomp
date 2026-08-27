#include "common.h"

extern u8 *D_80122B74;

/**
 * @brief Adds to a party record's counter field and clamps it to 0x98967F.
 *
 * Selects the record at @c D_80122B74 offset (base[0x859] + 0x68) * 4 + 0xE4
 * (a 32-bit counter), adds @p arg0 to it, then re-reads the same record and
 * clamps the counter to a maximum of 0x98967F (9,999,999).
 *
 * @param arg0 Amount to add to the counter.
 */
void func_800C10F0(s32 arg0)
{
    u8 *base;
    s32 *rec;
    s32 idx;
    s32 idx2;

    base = D_80122B74;

    idx = base[0x859] + 0x68;
    rec = (s32 *)(base + idx * 4 + 0xE4);
    *rec += arg0;

    idx2 = base[0x859] + 0x68;
    rec = (s32 *)(base + idx2 * 4 + 0xE4);
    if ((u32)*rec > 0x98967F)
    {
        *rec = 0x98967F;
    }
}
