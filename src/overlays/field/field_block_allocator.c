/** @file field_block_allocator.c
 * @brief Packed FIELD block-list initialization, allocation and release.
 *
 * A pool is a chain of blocks, each a one-word header followed by its
 * payload. The last word of the pool is a used block with the end tag.
 */

#include "common.h"
#include "field_calls.h"

/** @brief Tag of the terminal block that ends a pool. */
#define FIELD_BLOCK_TAG_END 0x7FF

/** @brief Header word in front of every block of a pool. */
typedef struct
{
    u32 size : 20; /**< Payload size in bytes, a multiple of four. */
    u32 tag : 11;  /**< Owner tag of a used block. */
    u32 used : 1;
} FieldBlockHeader;

/**
 * @brief Initialize a packed block pool and its terminal marker.
 * @param pool Word-aligned pool buffer.
 * @param size Pool size in bytes, rounded down to a multiple of four.
 */
void func_8009CA08(u32* pool, u32 size)
{
    FieldBlockHeader* end;
    s32 unused[6]; /* never used; the original stack frame reserves it */

    size &= 0xFFFFC;
    pool[0] = (size - 8) & 0xFFFFF;
    end = (FieldBlockHeader*)((u8*)pool + size) - 1;
    end->used = 1;
    end->tag = FIELD_BLOCK_TAG_END;
}

/**
 * @brief Allocate a tagged block from a packed block pool.
 *
 * Takes the first free block that is large enough. A block with at most four
 * spare bytes is used whole; a larger one is split and its remainder becomes
 * a new free block.
 *
 * @param block First block header of the pool.
 * @param size Requested payload size in bytes.
 * @param tag Owner tag stored in the allocated block header.
 * @return Pointer to the allocated payload, or NULL when no suitable block exists.
 */
void* func_8009CA54(FieldBlockHeader* block, s32 size, s32 tag)
{
    u32 need;
    FieldBlockHeader* next;

    need = (size + 3) & 0xFFFFFC;
    while (1)
    {
        if (!block->used)
        {
            if (block->size >= need)
            {
                if (block->size == need || block->size == need + 4)
                {
                    block->used = 1;
                    block->tag = tag;
                    return block + 1;
                }
                next = (FieldBlockHeader*)((u8*)block + need) + 1;
                next->used = 0;
                next->size = block->size - need - 4;
                next->tag = 0;
                block->used = 1;
                block->tag = tag;
                block->size = need;
                return block + 1;
            }
        }
        else if (block->tag == FIELD_BLOCK_TAG_END)
        {
            return NULL;
        }
        block = (FieldBlockHeader*)((u8*)block + block->size + 4);
    }
}

/**
 * @brief Free every block with the requested tag and merge adjacent free blocks.
 * @param first_block First block header of the pool, viewed as raw header words.
 * @param requested_tag Owner tag of the blocks to free.
 * @note Written with header masks in locals rather than FieldBlockHeader
 *       bitfields; the bitfield form does not reproduce the original code.
 */
void func_8009CB64(u32* first_block, s32 requested_tag)
{
    u32* previous_block;
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
                    *previous_block = (previous_header & flags_mask) | (((previous_header & size_mask) + (header & size_mask) + 4) & size_mask);
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
                    *previous_block =
                        (coalesce_previous_header & flags_mask) | (((coalesce_previous_header & size_mask) + (coalesce_header & size_mask) + 4) & size_mask);
                    first_block = previous_block;
                }
            }
        }

        previous_block = first_block;
        first_block = (u32*)((u8*)first_block + (*first_block & size_mask) + 4);
    } while (1);
}
