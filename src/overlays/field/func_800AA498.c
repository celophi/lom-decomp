#include "common.h"

/** @brief Partial Struct_D80105AE0 layout used by func_800AA498. */
typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    u8 pad8[0x23C - 0x8];
} Struct_D80105AE0;

extern Struct_D80105AE0 D_80105AE0[];
extern s32 D_800FDFC8;
extern s32 D_800FE754;
extern s32 g_frame_counter;

void func_800A3938(s32, s32);

/**
 * @brief Check two frame-phased sound queues and submit their guarded sound command.
 * @note WIP: the second queue check retains load and delay-slot differences.
 */
void func_800AA498(void)
{
    if (D_800FE754 != 0)
    {
        if (!(g_frame_counter & 0x1F))
        {
            if ((D_80105AE0[0].unk4 != 0) && ((u32) (D_80105AE0[0].unk4 * 4) < (u32) D_80105AE0[0].unk0))
            {
                func_800A3938(0xA6, 0x80);
            }
        }

        if (!((g_frame_counter + 0x10) & 0x1F) && !(D_800FDFC8 & 0x1FF) &&
            (D_80105AE0[1].unk4 != 0) && ((u32) (D_80105AE0[1].unk4 * 4) < (u32) D_80105AE0[1].unk0))
        {
            func_800A3938(0xA6, 0x80);
        }
    }
}
