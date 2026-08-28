#include "common.h"

extern s32 D_80165FE0;
extern s32 D_80166004;

s32 func_80140370(s32 arg0)
{
    if (D_80165FE0 != 0)
    {
        func_8014986C();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    func_8014ADF8();
    func_80140830(arg0);
    func_8014AE34();
    func_80063194();
    D_80166004 ^= 1;
    return 0;
}
