#include "wmap_land_preview_lines.h"
#include "sdk/libgpu.h"

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005B540(void)
{
}

/** @brief Initialize 16 black, semitransparent two-point line packets. */
void func_8005B548(void)
{
extern u32 D_800D81FC[];

    s32 index;
    for (index = 0; index < 16; index++)
    {
        D_800D81FC[index * 4] = 0;
        setlen((LINE_F2 *)((u8 *)D_800D81FC - 4) + index, 3);
        setcode((LINE_F2 *)((u8 *)D_800D81FC - 4) + index, 0x42);
    }
}
