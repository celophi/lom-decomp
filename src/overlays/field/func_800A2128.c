#include "common.h"

extern s32 D_80117E68;
extern s32 D_80117E70;
extern s32 D_80117E74;
extern s32 D_80117E80;

/**
 * @brief Combine and copy selected source words into an output array in three passes.
 * @param arg0 Source records at an eight-byte stride.
 * @param arg1 Destination words; pass ranges come from the shared counters.
 * @note The shared parity selector chooses one of the two words in each source record.
 * @note WIP: loop address calculations and register allocation differ from the target.
 */
void func_800A2128(u8 *arg0, s32 *arg1)
{
    s32 mask;
    s32 i;
    s32 count;
    s32 e70;
    u8 *out;
    u8 *base;
    s32 v0, v1;

    count = D_80117E74;
    if (count > 0)
    {
        out = (u8 *)arg1;
        e70 = D_80117E70;
        mask = ((D_80117E68 - 1) & 1) * 4;
        i = 0;
        do
        {
            v1 = *(s32 *)(arg0 + mask + (i + count) * 8 - 8);
            v0 = *(s32 *)(arg0 + mask + (i + e70 + count) * 8 - 8);
            i++;
            *(s32 *)out = v1 + v0;
            out += 4;
        } while (i < count);
    }

    count = D_80117E70 - D_80117E80;
    if (count > 0)
    {
        s32 e74 = D_80117E74;
        s32 e80 = D_80117E80;
        mask = ((D_80117E68 - 1) & 1) * 4;
        i = 0;
        do
        {
            s32 idx = i + e74;
            s32 src_index = i + e80;
            i++;
            *(s32 *)((u8 *)arg1 + idx * 4) = *(s32 *)(arg0 + mask + src_index * 8);
        } while (i < count);
    }

    count = D_80117E74 - 1;
    if (count > 0)
    {
        base = arg0;
        e70 = D_80117E70;
        mask = ((D_80117E68 - 1) & 1) * 4;
        i = 0;
        do
        {
            s32 src_index = i + e70;
            i++;
            *(s32 *)((u8 *)arg1 + (src_index - D_80117E74) * 4 + 4) =
                *(s32 *)(arg0 + mask + src_index * 8) + *(s32 *)(mask + base);
            base += 8;
        } while (i < count);
    }
}
