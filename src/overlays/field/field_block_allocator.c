/** @file
 * @brief Packed FIELD block-list initialization, allocation and release.
 */

#include "common.h"

extern void func_dead6(s32, s32, s32, s32, s32, s32);

/**
 * @brief Initialize a packed block pool and its terminal marker.
 * @param ptr Word-aligned pool buffer.
 * @param size Pool size in bytes, rounded down to a multiple of four.
 */
void func_8009CA08(u32 *ptr, u32 size)
{
    u8 *end;

    if (0)
    {
        func_dead6(0, 0, 0, 0, 0, 0);
    }
    size &= 0xFFFFC;
    ptr[0] = (size - 8) & 0xFFFFF;
    end = (u8 *) ptr + size;
    *(u32 *) (end - 4) |= 0x80000000;
    *(u32 *) (end - 4) |= 0x7FF00000;
}


/**
 * @brief Allocate and mark a block from a packed FIELD block list.
 * @param arg0 First block header to inspect.
 * @param arg1 Requested payload size in bytes.
 * @param arg2 Tag stored in the allocated block header.
 * @return Pointer to the allocated payload, or NULL when no suitable block exists.
 */
void *func_8009CA54(u8 *arg0, s32 arg1, s32 arg2)
{
    u8 *block;
    s32 header;
    u32 size;
    s32 flags_or;
    u32 need;
    u32 need_masked;
    u8 *next;
    s32 next_header;
    s32 align_mask;
    s32 size_mask;
    s32 used_flag;
    s32 keep_mask;
    s32 hi_mask;

    block = arg0;
    align_mask = 0xFFFFFC;
    size_mask = 0xFFFFF;
    used_flag = 0x80000000;
    flags_or = (arg2 & 0x7FF) << 20;
    keep_mask = 0x800FFFFF;
    hi_mask = 0xFFF00000;
    need = (arg1 + 3) & align_mask;
    need_masked = need & size_mask;
loop:
    header = *(s32 *)block;
    size = (u32)header & size_mask;
    if (header >= 0)
    {
        if (size >= need)
        {
            if (size == need)
            {
                *(s32 *)block = ((header | used_flag) & keep_mask) | flags_or;
                return block + 4;
            }
            if (size == need + 4)
            {
                *(s32 *)block = ((header | used_flag) & keep_mask) | flags_or;
                return block + 4;
            }
            next = block + need;
            next_header = *(s32 *)(next + 4) & 0x7FFFFFFF;
            *(s32 *)(next + 4) = next_header;
            *(s32 *)(next + 4) = (((next_header & hi_mask) & hi_mask) | (((*(s32 *)block & size_mask) - need - 4) & size_mask)) & keep_mask;
            *(s32 *)block = ((((*(s32 *)block | used_flag) & keep_mask) | flags_or) & hi_mask) | need_masked;
            return block + 4;
        }
        goto advance;
    }
    if (((u32)header >> 20 & 0x7FF) != 0x7FF)
    {
advance:
        block = block + (*(s32 *)block & size_mask) + 4;
        goto loop;
    }
    return NULL;
}


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
