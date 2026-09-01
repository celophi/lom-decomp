#include "common.h"

typedef struct
{
    u32 flags;
    u32 state;
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u8 padE[6];
} FieldADF84Rec;

extern FieldADF84Rec D_80122828[];

FieldADF84Rec *func_800ADF84(void)
{
    FieldADF84Rec *rec;
    s32 i;

    rec = D_80122828;
    for (i = 0; i < 8; i++, rec++)
    {
        if ((rec->flags & 7) == 0)
        {
            rec->flags = (rec->flags & ~7) | 1;
            rec->unk8 = 0;
            rec->unkA = 0;
            rec->state &= ~0x200;
            rec->state &= ~0xC00;
            ((u16 *)&rec->state)[1] = 0;
            rec->unkC = 0;
            return rec;
        }
    }
    return D_80122828;
}
