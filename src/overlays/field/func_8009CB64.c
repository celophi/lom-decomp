#include "common.h"

/**
 * @brief Free blocks with the requested tag and coalesce adjacent free blocks.
 * @param first_block First packed block header to inspect.
 * @param requested_tag Tag identifying blocks to free.
 */
void func_8009CB64(u32 *first_block, s32 requested_tag)
{
    u32 *previous_block;
    u32 header;
    u32 coalesce_header;
    s32 block_tag;
    s32 previous_header;
    s32 coalesce_previous_header;
    u32 tag_bits;
    u32 cleared_header;
    u32 clear_used_mask;
    u32 keep_header_mask;
    u32 size_mask;
    u32 flags_mask;

    previous_block = first_block;
    clear_used_mask = 0x7FFFFFFF;
    keep_header_mask = 0x800FFFFF;
    size_mask = 0xFFFFF;
    flags_mask = 0xFFF00000;
    do
    {
        header = *first_block;
        tag_bits = header >> 20;
        block_tag = tag_bits & 0x7FF;
        tag_bits = 0x7FF;
        if (block_tag == tag_bits)
        {
            return;
        }

        if ((s32)header < 0 && block_tag == requested_tag)
        {
            if (previous_block != first_block)
            {
                previous_header = *previous_block;
                if (previous_header >= 0)
                {
                    *previous_block = (previous_header & flags_mask) |
                                      (((previous_header & size_mask) + (header & size_mask) + 4) & size_mask);
                    first_block = previous_block;
                }
                else
                {
                    cleared_header = header & clear_used_mask;
                    *first_block = cleared_header & keep_header_mask;
                }
            }
            else
            {
                cleared_header = header & clear_used_mask;
                *first_block = cleared_header & keep_header_mask;
            }
        }

        if (previous_block != first_block)
        {
            coalesce_header = *first_block;
            if ((s32)coalesce_header >= 0)
            {
                coalesce_previous_header = *previous_block;
                if (coalesce_previous_header >= 0)
                {
                    *previous_block = (coalesce_previous_header & flags_mask) |
                                      (((coalesce_previous_header & size_mask) + (coalesce_header & size_mask) + 4) & size_mask);
                    first_block = previous_block;
                }
            }
        }

        previous_block = first_block;
        first_block = (u32 *)((u8 *)first_block + (*first_block & size_mask) + 4);
    } while (1);
}
