/** @file field_block_allocator.c
 * @brief Tagged block pool used for the FIELD actor heap.
 *
 * A pool is a chain of blocks, each a one-word header followed by its
 * payload. Blocks carry an owner tag so that every block of one owner can be
 * released in one call. The last word of the pool is a used block with the
 * end tag.
 */

#include "common.h"
#include "field_calls.h"

/** @brief Tag of the terminal block that ends a pool. */
#define FIELD_BLOCK_TAG_END 0x7FF

/** @brief Mask of the size field in a block header word. */
#define FIELD_BLOCK_SIZE_MASK 0xFFFFF

/** @brief Header word in front of every block of a pool. */
typedef struct
{
    u32 size : 20; /**< Payload size in bytes, a multiple of four. */
    u32 tag : 11;  /**< Owner tag of a used block. */
    u32 used : 1;
} FieldBlockHeader;

/**
 * @brief Return the header of the block that follows a block.
 * @param block Block header.
 * @return Header right after the payload of block.
 */
static inline FieldBlockHeader* field_block_next(FieldBlockHeader* block)
{
    return (FieldBlockHeader*)((u8*)block + block->size + sizeof(FieldBlockHeader));
}

/**
 * @brief Grow a free block over the block that follows it.
 * @param into Free block that absorbs the next one.
 * @param block Block right after into, header included.
 */
static inline void field_block_absorb(FieldBlockHeader* into, FieldBlockHeader* block)
{
    into->size = into->size + block->size + sizeof(FieldBlockHeader);
}

/**
 * @brief Mark a block free and clear its owner tag.
 * @param block Block header.
 */
static inline void field_block_release(FieldBlockHeader* block)
{
    block->used = 0;
    block->tag = 0;
}

/**
 * @brief Initialize a pool as one free block plus the terminal marker.
 * @param pool Word-aligned pool buffer.
 * @param size Pool size in bytes, rounded down to a multiple of four.
 */
void field_block_pool_init(void* pool, u32 size)
{
    FieldBlockHeader* end;
    s32 unused[6]; /* never used; the original stack frame reserves it */

    size &= FIELD_BLOCK_SIZE_MASK & ~3;
    /* One word store: a free block with tag 0 spanning all but two headers. */
    *(u32*)pool = (size - 2 * sizeof(FieldBlockHeader)) & FIELD_BLOCK_SIZE_MASK;
    end = (FieldBlockHeader*)((u8*)pool + size) - 1;
    end->used = 1;
    end->tag = FIELD_BLOCK_TAG_END;
}

/**
 * @brief Allocate a tagged block from a pool.
 *
 * Takes the first free block that is large enough. A block with at most four
 * spare bytes is used whole; a larger one is split and its remainder becomes
 * a new free block.
 *
 * @param pool Pool buffer set up by field_block_pool_init().
 * @param size Requested payload size in bytes.
 * @param tag Owner tag stored in the allocated block header.
 * @return Pointer to the allocated payload, or NULL when no block fits.
 */
void* field_block_alloc(void* pool, s32 size, s32 tag)
{
    FieldBlockHeader* block;
    u32 need;
    FieldBlockHeader* next;

    block = pool;
    /* Round up to whole words (the original mask keeps 24 bits). */
    need = (size + 3) & 0xFFFFFC;
    while (1)
    {
        if (!block->used)
        {
            if (block->size >= need)
            {
                if (block->size == need || block->size == need + sizeof(FieldBlockHeader))
                {
                    block->used = 1;
                    block->tag = tag;
                    return block + 1;
                }
                next = (FieldBlockHeader*)((u8*)block + need) + 1;
                next->used = 0;
                next->size = block->size - need - sizeof(FieldBlockHeader);
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
        block = field_block_next(block);
    }
}

/**
 * @brief Free every block with a tag and merge adjacent free blocks.
 * @param pool Pool buffer set up by field_block_pool_init().
 * @param tag Owner tag of the blocks to free.
 */
void field_block_free_tag(void* pool, s32 tag)
{
    FieldBlockHeader* block;
    FieldBlockHeader* previous;

    block = pool;
    previous = block;
    while (1)
    {
        if (block->tag == FIELD_BLOCK_TAG_END)
        {
            return;
        }
        if (block->used && block->tag == tag)
        {
            if (previous == block)
            {
                field_block_release(block);
            }
            else if (!previous->used)
            {
                field_block_absorb(previous, block);
                block = previous;
            }
            else
            {
                field_block_release(block);
            }
        }
        if (previous != block && !block->used && !previous->used)
        {
            field_block_absorb(previous, block);
            block = previous;
        }
        previous = block;
        block = field_block_next(block);
    }
}
