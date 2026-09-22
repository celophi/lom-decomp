/* Partial WMAP decompilation: 89.292305% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

#include "sdk/libgpu.h"
#include "sdk/libetc.h"
M2C_UNK akao_cmd_f0();                              /* extern */
M2C_UNK akao_cmd_f1();                              /* extern */
M2C_UNK func_80060720();                            /* extern */
s32 func_800623CC();                                /* extern */
extern u8 D_80051A80;
extern s32 D_800D0550;
extern u8 D_80129560;
extern s32 D_80182228;
extern s32 D_80182240;

s32 run_world_map(void)
{
    RECT sp10;
    s32 var_v0;

    sp10 = *(RECT *)&D_80051A80;
    akao_cmd_f0();
    akao_cmd_f1();
    func_80060720();
    M2C_FIELD(&D_80129560, s8 *, 0x18) = 0;
    M2C_FIELD(&D_80129560, s8 *, 0x7E58) = 0;
    D_800D0550 = func_800623CC();
    akao_cmd_f0();
    akao_cmd_f1();
    if (D_800D0550 == 2)
    {
        DrawSync(0);
        VSync(0);
        SetDispMask(0);
        ResetGraph(0);
        ClearImage(&sp10, 0, 0, 0);
        return 2;
    }
    if (D_80182228 != 0)
    {
        return 9;
    }
    var_v0 = 0xA;
    if (D_80182240 == 0)
    {
        var_v0 = D_800D0550;
    }
    return var_v0;
}
