#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"

/**
 * @brief Add four black, one-pixel-offset copies of a sprite group.
 * @param ordering_table Ordering-table entry receiving the copied sprites.
 * @param primitive_buffer First free primitive address after the template group.
 * @param sprite_count Number of sprites in the fixed template group.
 * @return First free primitive address after the four copied groups.
 * @note Match: 99.505160% gcc272_cdk; six independent prologue rows differ.
 * @see decomp.me WIP
 */
s32 *func_800AD658(s32 *ordering_table, s32 *primitive_buffer, s32 sprite_count)
{
    SPRT *cur;
    SPRT *base;
    s32 dir;
    u32 mask_hi;
    s32 stride;
    s32 count;
    u16 val;
    SPRT *rec;

    cur = (SPRT *)primitive_buffer;
    /* The template remains relative to the original buffer position. */
    base = cur;
    dir = 0;
    stride = sprite_count * 0x14;
    do
    {
        bcopy((u8 *)base - stride, (u8 *)cur, stride);
        count = 0;
        if (sprite_count > 0)
        {
            rec = cur;
            do
            {
                rec->b0 = 0;
                rec->g0 = 0;
                rec->r0 = 0;
                switch (dir)
                {
                    case 0:
                        rec->x0 = rec->x0 + 1;
                        break;
                    case 1:
                        rec->x0 = rec->x0 - 1;
                        break;
                    case 2:
                        val = rec->y0 + 1;
                        goto store_y;
                    default:
                        val = rec->y0 - 1;
                    store_y:
                        rec->y0 = val;
                        break;
                }
                /* Keep this mask local to the inner loop across bcopy calls. */
                mask_hi = 0xFF000000;
                rec = (SPRT *)((u8 *)rec + 0x14);
                count++;
                cur->tag = (cur->tag & mask_hi) | (*ordering_table & 0xFFFFFF);
                *ordering_table = (*ordering_table & mask_hi) | ((s32)cur & 0xFFFFFF);
                cur = (SPRT *)((u8 *)cur + 0x14);
            } while (count < sprite_count);
        }
        dir++;
    } while (dir < 4);

    return (s32 *)cur;
}
