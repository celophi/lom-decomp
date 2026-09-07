#include "common.h"

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
