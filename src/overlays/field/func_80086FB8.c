#include "common.h"

/** @brief Horizontal and vertical packet translation. */
typedef struct
{
    u16 x, y;
} Motion;
extern u8 D_80107800[];
extern Motion D_801077FC;
extern u16 D_801058E0[];
extern void bcopy(void *, void *, s32);
/**
 * @brief Fade, translate, and enqueue active packets from the 256-entry effect pool.
 * @param buffer Ordering table and packet-buffer state, with the write cursor at 0x40B8.
 */
void func_80086FB8(u8 *buffer)
{
    u8 *source, *cursor, *output;
    u32 *ordering, *link, address, mask;
    u16 *depth;
    s32 i;
    u8 color;
    ordering = (u32 *)buffer;
    source = D_80107800;
    i = 0;
    mask = 0xFF000000;
    cursor = source + 0x22;
    depth = D_801058E0;
    output = *(u8 **)(buffer + 0x40B8);
    do
    {
        color = cursor[-0x1E];
        if (color != 0)
        {
            if (color < 0x10)
            {
                color = 0;
            }
            else
            {
                color -= 0x10;
            }
            cursor[-0x1C] = color;
            cursor[-0x1D] = color;
            cursor[-0x1E] = color;
            cursor[-0x1B] |= 2;
            *(u16 *)(cursor - 26) += D_801077FC.x;
            *(u16 *)(cursor - 18) += D_801077FC.x;
            *(u16 *)(cursor - 10) += D_801077FC.x;
            *(u16 *)(cursor - 2) += D_801077FC.x;
            *(u16 *)(cursor - 24) += D_801077FC.y;
            *(u16 *)(cursor - 16) += D_801077FC.y;
            *(u16 *)(cursor - 8) += D_801077FC.y;
            {
                u16 *last_y = (u16 *)(cursor - 8);
                last_y[4] += D_801077FC.y;
            }
            bcopy(source, output, 0x28);
            *(u32 *)output = (*(u32 *)output & mask) | (ordering[*depth] & 0xFFFFFF);
            address = (u32)output & 0xFFFFFF;
            link = &ordering[*depth];
            output += 0x28;
            *link = (*link & mask) | address;
        }
        depth++;
        i++;
        cursor += 0x28;
        source += 0x28;
    } while (i < 256);
    *(u8 **)(buffer + 0x40B8) = output;
}
