#include "common.h"

extern s32 D_801609A0;
extern s32 D_801609C4;

extern void field_text_reset_scratch(void);
extern void func_80146D64(void);
extern void func_801406A8(s32);
extern void func_80146DA0(void);
extern void func_80063194(void);
extern void func_80145A9C(void);
extern void field_text_reset_windows(void);
extern void func_80019788(s32);

/** @see decomp.me (100%) */
s32 func_801401F8(s32 arg0)
{
    if (D_801609A0 != 0)
    {
        func_80145A9C();
        field_text_reset_windows();
        func_80019788(0);
        return D_801609A0;
    }
    field_text_reset_scratch();
    func_80146D64();
    func_801406A8(arg0);
    func_80146DA0();
    func_80063194();
    D_801609C4 ^= 1;
    return 0;
}
