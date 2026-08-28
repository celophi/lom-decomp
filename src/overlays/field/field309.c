#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
} StructB2A9C;

extern s32 *D_80123FB0;
StructB2A9C *func_800B2A9C(void);
s32 func_800BD414(s32 arg0, s32 arg1);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

/**
 * @see decomp.me (100%)
 */
void func_800B48B8(void)
{
    s32 var_v0;

    if ((D_80123FB0 != NULL) && (*D_80123FB0 >= 0))
    {
        if (func_800B2A9C()->unk4 & 0x200)
        {
            var_v0 = func_800BD414(0, 0x4280);
            func_800BD520(0, 0x4280, var_v0 + 1);
        }
        else
        {
            var_v0 = func_800BD414(0, 0x4284);
            func_800BD520(0, 0x4284, var_v0 + 1);
        }
    }
}
