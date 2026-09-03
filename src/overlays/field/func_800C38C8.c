#include "common.h"

extern u8 *func_800C1E40(s32 arg0);

/**
 * @brief Compute the accumulated lookup-table value for a field record.
 *
 * @param arg0 Record containing the packed lookup selector at 0x14 and four
 *             table-entry bytes beginning at 0x20.
 * @return Product of the two packed-selector table entries plus each valid
 *         per-record table contribution.
 */
s32 func_800C38C8(u8 *arg0)
{
    u8 *base;
    u32 packed;
    s32 sum;
    u32 i;
    u8 id;
    u8 *p;

    base = func_800C1E40(0x11);
    i = 0;
    packed = *(u32 *)(arg0 + 0x14);
    sum = *(u16 *)(base + ((((packed >> 4) & 0x30) + ((packed >> 10) & 0x3F)) << 1) + 4) *
          *(u16 *)(base + ((packed >> 15) & 0x7E) + 0x4C);
    do
    {
        p = arg0 + i;
        id = p[0x20];
        if (id != 0xFF)
            sum += *(u16 *)(base + (p[0x20] << 1) + 0xCC);
        i++;
    } while (i < 4);
    return sum;
}
