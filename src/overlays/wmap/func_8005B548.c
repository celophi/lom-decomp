#include "common.h"

#include "sdk/libgpu.h"

extern u32 D_800D81FC[];

/** @brief Initialize 16 black, semitransparent two-point line packets. */
void func_8005B548(void)
{
    s32 index;
    for (index = 0; index < 16; index++)
    {
        D_800D81FC[index * 4] = 0;
        setlen((LINE_F2 *)((u8 *)D_800D81FC - 4) + index, 3);
        setcode((LINE_F2 *)((u8 *)D_800D81FC - 4) + index, 0x42);
    }
}
