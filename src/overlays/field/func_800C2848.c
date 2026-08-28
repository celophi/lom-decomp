#include "common.h"

/**
 * @brief Active field record touched by the dispatch below.
 */
typedef struct
{
    u8 unk0;
    u8 pad1[0x8F];
    u32 unk90;
} StructC2848;

extern StructC2848 *func_800C1B98(void);
extern void func_800C1D14(s32 arg0, s32 arg1);
extern void func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Marks the active record and issues its follow-up actions.
 *
 * When a record is active, sets its 0x40000000 flag, notifies func_800C1D14
 * with the record's leading byte, and - if @p arg1 bit 1 is set - fires
 * func_80087D8C for @p arg0.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 94.29%
 * result was caused by GCC 2.7.2 CDK epilogue scheduling rather than the C
 * source shape.
 */
void func_800C2848(s32 arg0, s32 arg1)
{
    StructC2848 *r = func_800C1B98();

    if (r != NULL)
    {
        r->unk90 |= 0x40000000;
        func_800C1D14(r->unk0, arg1);
        if (arg1 & 2)
        {
            func_80087D8C(arg0, -0x400, 0, 0);
        }
    }
}
