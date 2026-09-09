#include "common.h"

/** @brief Partial Rec layout used by func_800C0A38. */
typedef struct Rec
{
    u8 pad0[0xC];
    s32 unkC;
    u8 *unk10;
    u8 *unk14;
} Rec;

extern u8 D_800F18C4[];
extern s32 (*D_800F18B4[])(s32, s32, Rec *);

s32 rand(void);

/**
 * @brief Select a bounded random entry and invoke its dispatch handler.
 * @param arg0 Actor record containing flags and selection tables.
 * @return The handler result, or -1 for a null record or invalid dispatch entry.
 * @note WIP: load scheduling and temporary-register differences remain.
 * @see decomp.me (91.29%) WIP
 */
s32 func_800C0A38(Rec *arg0)
{
    s32 result;
    s32 count;
    s32 mask;
    s32 shift_count;
    s32 within_range;
    u8 *entry;
    u8 dispatch_idx;

    result = -1;
    if (arg0 != NULL)
    {
        count = D_800F18C4[(u8)(arg0->unk10[0x4C]) >> 5];
        within_range = count < 8;
        if (arg0->unkC & 0x04000000)
        {
            count += 2;
            within_range = count < 8;
        }
        if (!within_range)
        {
            count = 7;
        }
        shift_count = rand();
        mask = shift_count & 0xFFFF;
        if (arg0->unkC & 0x08000000)
        {
            mask = shift_count & 0xFFFC;
        }
        shift_count = 0;
        if (count != 0)
        {
            while (!(mask & 1))
            {
                shift_count += 1;
                mask >>= 1;
                if (shift_count >= count)
                {
                    break;
                }
            }
        }
        entry = arg0->unk14 + (shift_count * 2);
        if (*(volatile u8 *)(entry + 0x40) >= 4)
        {
            return -1;
        }
        dispatch_idx = entry[0x40];
        result = D_800F18B4[dispatch_idx](dispatch_idx, entry[0x41], arg0);
    }
    return result;
}
