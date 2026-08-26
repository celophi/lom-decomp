#include "common.h"

typedef struct
{
    u32 word;
    u8 pad[0x14 - 4];
} UnkEntry80122828;

extern s32 D_8012298C;
extern UnkEntry80122828 D_80122828[];

void func_800ADEB0(void)
{
    UnkEntry80122828 *p;
    s32 i;

    D_8012298C = 0;
    p = D_80122828;
    for (i = 0; i < 8; i++)
    {
        p->word &= ~7;
        p++;
    }
}
