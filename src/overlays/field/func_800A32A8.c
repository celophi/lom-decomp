#include "common.h"

extern u8 D_800EC33C[];
extern u8 D_800EC37C[];
extern u8 D_800EC388[];
extern s32 D_80117ED0;
extern s32 D_801227C8;

/**
 * @brief Emit a fading animated textured quad for an active slot.
 * @param slot Quad layout index; zero and one share the first frame counter.
 * @param buffer Render buffer with an ordering table and cursor at offset 0x40B8.
 * @note Use the frame value read before incrementing its counter for this quad.
 */
void func_800A32A8(s32 slot, u8 *buffer)
{
    s32 *write_counter;
    s32 *read_counter;
    s32 *reset_counter;
    s32 *increment_counter;
    s32 uv_offset;
    s32 frame;
    s32 brightness;
    u8 swap_value;
    u8 uv_flags;
    u8 *primitive;
    u32 *ordering_table;
    s32 *counters;

    primitive = *(u8 **)(buffer + 0x40B8);
    ordering_table = (u32 *)buffer;
    counters = &D_80117ED0;
    read_counter = counters;
    if (slot != 0)
    {
        read_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
    }
    frame = *read_counter;
    if ((u32)(frame + 2) >= 2U)
    {
        if (frame >= 0x180)
        {
            reset_counter = counters;
            if (slot != 0)
            {
                reset_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
            }
            *reset_counter = -1;
            return;
        }
        if (D_801227C8 == 0)
        {
            write_counter = counters;
            if (slot != 0)
            {
                write_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
            }
            increment_counter = counters;
            if (slot != 0)
            {
                increment_counter = (s32 *)((slot - 1) * 4 + (u32)counters);
            }
            *write_counter = *increment_counter + 1;
        }
        if (frame > 0x100)
        {
            brightness = 0xFF - ((frame - 0x100) * 2);
        }
        else
        {
            brightness = 255;
        }
        if (brightness < 0)
        {
            brightness = 0;
        }
        if (brightness > 0xFF)
        {
            brightness = 0x100;
        }
        /* POLY_FT4 packet length, monochrome color and command. */
        *(u8 *)(primitive + 0x3) = 9;
        *(u8 *)(primitive + 0x6) = brightness;
        *(u8 *)(primitive + 0x5) = brightness;
        *(u8 *)(primitive + 0x4) = brightness;
        *(u8 *)(primitive + 0x7) = 0x2E;
        /* Preserve packed XY word accesses and their address grouping. */
        *(u32 *)(primitive + 0x8) = *(s32 *)(slot * 0x10 + D_800EC33C);
        *(u32 *)(primitive + 0x10) = *(s32 *)(D_800EC33C + slot * 0x10 + 0x4);
        *(u32 *)(primitive + 0x18) = *(s32 *)(D_800EC33C + slot * 0x10 + 0x8);
        *(u32 *)(primitive + 0x20) = *(s32 *)(D_800EC33C + slot * 0x10 + 0xC);
        /* Each animation frame selects UV coordinates and optional flips. */
        uv_flags = *((frame % 12) + D_800EC37C);
        uv_offset = (uv_flags & 1) * 8;
        *(u16 *)(primitive + 0xC) = *(u16 *)(uv_offset + D_800EC388);
        *(u16 *)(primitive + 0x14) = *(u16 *)(uv_offset + (D_800EC388 + 2));
        *(u16 *)(primitive + 0x1C) = *(u16 *)(uv_offset + (D_800EC388 + 4));
        *(u16 *)(primitive + 0x24) = *(u16 *)(uv_offset + (D_800EC388 + 6));
        if (uv_flags & 0x80)
        {
            swap_value = *(u8 *)(primitive + 0xC);
            *(u8 *)(primitive + 0xC) = *(u8 *)(primitive + 0x14);
            *(u8 *)(primitive + 0x14) = swap_value;
            swap_value = *(u8 *)(primitive + 0x1C);
            *(u8 *)(primitive + 0x1C) = *(u8 *)(primitive + 0x24);
            *(u8 *)(primitive + 0x24) = swap_value;
        }
        if (uv_flags & 0x40)
        {
            swap_value = *(u8 *)(primitive + 0xD);
            *(u8 *)(primitive + 0xD) = *(u8 *)(primitive + 0x1D);
            *(u8 *)(primitive + 0x1D) = swap_value;
            swap_value = *(u8 *)(primitive + 0x15);
            *(u8 *)(primitive + 0x15) = *(u8 *)(primitive + 0x25);
            *(u8 *)(primitive + 0x25) = swap_value;
        }
        /* Texture page, CLUT and ordering-table linkage. */
        *(u16 *)(primitive + 0x16) = 0x27;
        *(u16 *)(primitive + 0xE) = 0x7B05;
        *(u32 *)(primitive + 0x0) =
            (s32)((*(u32 *)(primitive + 0x0) & 0xFF000000) | (ordering_table[3] & 0xFFFFFF));
        ordering_table[3] = (s32)((ordering_table[3] & 0xFF000000) | ((s32)primitive & 0xFFFFFF));
        primitive += 0x28;
        *(u8 **)(buffer + 0x40B8) = primitive;
    }
}
