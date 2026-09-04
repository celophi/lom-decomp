#include "common.h"

typedef struct {
    u8 pad0[0x179];
    s8 unk179;
    u8 pad17A[0x23C - 0x17A];
} Rec23C;

extern Rec23C D_80105AE0[];

s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC(s32 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);
s32 func_8009104C(void);

s32 func_80090F80(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 v;

    if (arg3 & 0x8000)
        return func_8009104C();

    v = func_800839F8(arg0, 0);
    if ((v != -1) && (func_80083EEC(arg0, v, arg3 & 0x3FF) != 0))
    {
        field_start_actor_animation(v, arg1, arg2);
        D_80105AE0[arg0].unk179 = v;
    }
    return 1;
}
