#include "common.h"

extern u8 *D_80122B78;
extern s32 g_layout_option;

void func_80087FC0(s32 arg0, s32 arg1);
void func_8009AFBC(s32 arg0);
s32 akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Advance the field transition state and update its fade and audio state.
 */
void func_800B1BBC(void)
{
    u8 *ptr = D_80122B78;
    u32 val = *(u32 *)(ptr + 0x418);
    s32 i;
    u32 flags;
    u16 half;
    u32 packed;
    s32 v0;

    if ((val >> 29) & 1)
    {
        goto no_loop;
    }

    for (i = 0; i < 3; i++)
    {
        func_80087FC0(i, 2);
    }

    flags = *(u32 *)(D_80122B78 + 0x418) | 0x20000000;
    *(u32 *)(D_80122B78 + 0x418) = flags;
    if (*(s32 *)(D_80122B78 + 0x414) != 0xFF)
    {
        half = *(u16 *)(D_80122B78 + 0x418);
        if ((u16)(half + 2) >= 2)
        {
            func_8009AFBC(half & 0x7FFF);
        }

        packed = *(u32 *)(D_80122B78 + 0x410);
        field_set_fade_target(packed & 0x3FF, (packed >> 10) & 0x3FF, (packed >> 20) & 0x3FF, *(s32 *)(D_80122B78 + 0x414));

        if (*(u16 *)(D_80122B78 + 0x418) == 0xFFFF)
        {
            s32 shift = *(s32 *)(D_80122B78 + 0x414) << 2;
            g_layout_option = -1;
            akao_cmd_c1(0, shift, 0);
        }

        ptr = D_80122B78;
        v0 = *(s32 *)(ptr + 0x414);
        v0 = v0 + 1;
        goto tail_write;
    }
    *(u32 *)(D_80122B78 + 0x418) = flags | 0x80000000;
    return;

no_loop:
    if (*(s32 *)(ptr + 0x414) <= 0)
    {
        *(u32 *)(ptr + 0x418) = val | 0x80000000;
    }

    ptr = D_80122B78;
    v0 = *(s32 *)(ptr + 0x414) - 1;

tail_write:
    *(s32 *)(ptr + 0x414) = v0;
}
