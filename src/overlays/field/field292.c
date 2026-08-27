#include "common.h"

typedef struct
{
    u8 flags;
    u8 pad1[2];
    u8 count;
    u8 pad4[8];
} FieldRec;

typedef struct
{
    u8 pad0[0x2E4];
    u8 counter;
    u8 pad2E5[11];
    FieldRec recs[64];
} FieldBig;

extern FieldBig *D_80122B74;

s32 func_800C3518(s32 arg0)
{
    u8 flags;

    if (arg0 < 0x40)
    {
        flags = D_80122B74->recs[arg0].flags;
        if ((flags & 1) == 0)
        {
            if ((flags >> 3) & 1)
            {
                goto do_stuff;
            }
        }
        return -1;
    do_stuff:
        D_80122B74->counter += 1;
        D_80122B74->recs[arg0].flags |= 1;
        D_80122B74->recs[arg0].count = D_80122B74->counter;
        return arg0;
    }
    return -1;
}
