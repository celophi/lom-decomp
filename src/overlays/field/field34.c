#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

extern Struct_D80105AE0 D_80105AE0[];

Struct_D80105AE0 *func_80087F0C(s32 arg0)
{
    Struct_D80105AE0 *rec;
    s32 i;

    rec = D_80105AE0;
    for (i = 0; i < 0xD; i++)
    {
        if (rec->unk14 == arg0)
        {
            return rec;
        }
        rec++;
    }
    return (Struct_D80105AE0 *) -1;
}
