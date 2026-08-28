#include "common.h"

extern u8 *D_80123FB0;
extern s32 func_800B2D34(u8 *arg0, s32 arg1);

/**
 * @brief Scales a per-field chance value into an output slot.
 *
 * Rolls func_800B2D34 with the field's 0x20 pointer, biases the result by 50,
 * multiplies by the field's 0x4A0 half-word, divides by 50, and stores the
 * quotient through @p out.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 93.93%
 * result was a compiler-routing mismatch; GCC 2.8.0 reproduces the target
 * allocation and epilogue exactly.
 */
void func_800B70F4(s32 arg0, s32 *out)
{
    s32 v;
    u32 prod;

    v = func_800B2D34(*(u8 **)(D_80123FB0 + 0x20), arg0);
    prod = *(u16 *)(D_80123FB0 + 0x4A0) * (v + 0x32);
    *out = prod / 50;
}
