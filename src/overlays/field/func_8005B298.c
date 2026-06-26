#include "common.h"

extern u8 D_800CBF44[];

/**
 * @brief Apply a color-correction lookup to a range of 16-bit pixels.
 *
 * For each pixel in @p arg0[0..arg1-1], extracts the maximum of the three
 * 5-bit color components (R: bits 0-4, G: bits 5-9, B: bits 10-14), uses
 * that maximum as an index into a 64-entry lookup table selected by @p arg2,
 * adds the looked-up value to bit 15 of the original pixel, and writes the
 * result back.
 *
 * @param arg0 Pointer to an array of 16-bit pixel values.
 * @param arg1 Number of pixels to process (0 = no-op).
 * @param arg2 Lookup-table selector; table used is D_800CBF44[arg2 * 64 ..].
 * @param arg3 Unused (FieldObject* in caller).
 *
 * @note Called by field_load_map with arg1 = object->unk2A and
 *       arg2 = D_801ED490 - 1.
 * @note Matches 100% with both gcc272_cdk and gcc280_g4.
 * @see decomp.me (100%) TODO
 */
void func_8005B298(u16* arg0, s32 arg1, s32 arg2, void* arg3)
{
    u16* ptr;
    s32 count;
    u16 pixel;
    u32 r;
    u32 g;
    u32 b;
    u32 max_component;
    u32 table_base;

    ptr = arg0;
    count = arg1 - 1;
    table_base = (u32)&D_800CBF44[arg2 * 64];
    if (arg1 != 0)
    {
        do
        {
            pixel = *ptr;
            max_component = pixel & 0x1F;
            g = (pixel >> 5) & 0x1F;
            if (max_component < g)
            {
                max_component = g;
            }
            b = (pixel >> 10) & 0x1F;
            if (max_component < b)
            {
                max_component = b;
            }
            count -= 1;
            *ptr = *(u16*)(table_base + max_component * 2) + (pixel & 0x8000);
            ptr += 1;
        }
        while (count != -1);
    }
}
