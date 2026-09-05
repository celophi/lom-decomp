#include "common.h"

/**
 * @brief Field state header; only its leading byte is referenced here.
 */
typedef struct StateBCEFC
{
    u8 unk0;
} StateBCEFC;

extern StateBCEFC *g_field_script;
void func_800B28E0(s32, s32, s32);
void func_800B286C(s32, s32, s32);

/**
 * @brief Dispatches to one of two field handlers based on a selector.
 *
 * If @p p1 is the 0xFF sentinel it is replaced by the active state's leading
 * byte. Selector 0 forwards to func_800B28E0 and selector 1 to func_800B286C,
 * each with the resolved value and the low bytes of @p p2 and @p p3.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 95.56%
 * result was caused by GCC 2.7.2 CDK epilogue scheduling rather than the C
 * source shape.
 */
void func_800BCEFC(s32 p0, s32 p1, s32 p2, s32 p3)
{
    if (p1 == 0xFF)
    {
        p1 = g_field_script->unk0;
    }
    switch (p0)
    {
    case 0:
        func_800B28E0(p1, p2 & 0xFF, p3 & 0xFF);
        break;
    case 1:
        func_800B286C(p1, p2 & 0xFF, p3 & 0xFF);
        break;
    }
}
