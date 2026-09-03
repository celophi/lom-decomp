#include "common.h"

typedef struct ActorB4934
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[5];
    u16 unkA;
} ActorB4934;

extern u8 *D_80122B74;
void func_800B4934(ActorB4934 *arg0);

void func_800B6744(ActorB4934 *arg0)
{
    s32 i;
    s32 off;
    s32 j;
    u8 *rec;
    u8 *p;

    for (i = 0; i < 4; i++)
    {
        off = 0x50 + i * 0x40;
        rec = D_80122B74 + (arg0->unk4 * 0x250 + 0x5F0) + off;
        if (rec[0] != 0 && (*(u16 *)(rec + 0x2E) & 2))
        {
            for (j = 0; j < 4; j++)
            {
                p = rec + j;
                if (p[0x20] == 0x58)
                {
                    p[0x20] = 0xFF;
                    *(u16 *)(rec + 0x2E) = 0;
                    func_800B4934(arg0);
                    return;
                }
            }
        }
    }
}
