#include "common.h"
#include "sdk/libgpu.h"

extern s16 D_80105758[];
extern u8 D_80104B58[];
extern u8 D_80105358[];

void *memcpy(void *palette_dst, const void *src, s32 n);

/**
 * @brief Upload a field texture and its optional palette to the selected slot.
 * @param resource Pointer to the image resource header and payload blocks.
 * @param slot Upload slot; values of three or more share the third flags entry.
 * @note GCC 2.7.2 CDK currently matches 81.690720 percent of the target.
 */
void func_8009AE38(u8 *resource, s32 slot)
{
    s32 original_slot;
    u8 *cursor;
    s32 block_size;
    s32 copy_size;
    u8 *palette_dst;
    RECT rect;
    s32 width;
    s32 height;

    original_slot = slot;
    resource += 4;
    cursor = resource;
    if (original_slot >= 3)
    {
        slot = 2;
    }
    D_80105758[slot] = *cursor;
    cursor += 4;
    /* Preserve the pointer-valued flags index used by the original code. */
    resource = (u8 *)original_slot;
    if (original_slot >= 3)
    {
        resource = (u8 *)2;
    }
    if (D_80105758[(s32)resource] & 8)
    {
        block_size = *(s32 *)cursor;
        cursor += 0xC;
        if (original_slot < 2)
        {
            copy_size = 0x200;
            rect.y = (original_slot * 2) + 0x1EE;
            rect.w = 0x100;
            rect.h = 1;
            palette_dst = (u8 *)(original_slot << 10);
            palette_dst += (s32)D_80104B58;
            /* Reusing slot here keeps the two palette paths separate. */
            slot = block_size - 0xC;
            rect.x = 0;
            if (slot < 0x201)
            {
                copy_size = slot;
            }
        }
        else
        {
            copy_size = 0x200;
            rect.y = 0x1F2;
            rect.w = 0x100;
            rect.h = 1;
            palette_dst = D_80105358;
            rect.x = 0;
            if (block_size - 0xC < 0x201)
            {
                copy_size = block_size - 0xC;
            }
        }
        memcpy(palette_dst, cursor, copy_size);
        LoadImage(&rect, (u_long *)cursor);
        cursor = cursor + block_size - 0xC;
    }
    cursor += 8;
    width = *(u16 *)cursor;
    cursor += 2;
    height = *(u16 *)cursor;
    cursor += 2;
    if (original_slot < 2)
    {
        rect.x = (original_slot << 6) + 0x340;
        rect.y = 0x100;
    }
    else
    {
        rect.x = 0x140;
        rect.y = 0;
    }
    rect.w = width;
    rect.h = height;
    LoadImage(&rect, (u_long *)cursor);
}
