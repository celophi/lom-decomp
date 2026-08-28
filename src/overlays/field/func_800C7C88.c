#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C1F;

/**
 * @brief Clears the active menu entry's high flag nibble and status bytes.
 *
 * For the entry selected by D_80122C1F (stride 0x8C in g_menuLayoutBuffer),
 * masks off bits 12-15 of its 0x26E4 word and writes 0xFF to the four status
 * bytes at 0x26EC.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 97.68%
 * result was a compiler-routing mismatch; GCC 2.8.0 reproduces the loop
 * scheduling and register allocation exactly.
 */
void func_800C7C88(void)
{
    u8 *base = g_menuLayoutBuffer;
    u8 *base2;
    s32 offset = D_80122C1F * 0x8C;
    s32 offset2;
    s32 i;

    *(u32 *)(base + offset + 0x26E4) &= 0xFFFF0FFF;
    i = 0;
    base2 = base;
    offset2 = offset;
    for (; i < 4; i++)
    {
        base2[i + offset2 + 0x26EC] = 0xFF;
    }
}
