#include "common.h"

typedef struct
{
    u8 pad0[8];
    s32 unk8;    /* 0x08 */
    u8 padC[0x58 - 0xC];
    s32 unk58;   /* 0x58 */
} StructFC4;

extern StructFC4 *D_80123FC4;

s32 func_800BF9F0(s32 arg0)
{
    StructFC4 *p;

    p = D_80123FC4;
    if ((p->unk58 & 0xF) == 0)
    {
        arg0 = 0;
    }
    if (p->unk8 < arg0)
    {
        return 0;
    }
    return -1;
}
