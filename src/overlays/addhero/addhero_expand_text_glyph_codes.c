#include "common.h"

extern u8 D_8015D3B4[];
extern u8 D_801605A8[];

void func_80146E30(u8 *out, u8 *in)
{
    u32 c;
    s32 index;
    s16 lead;

    for (;;)
    {
        c = *in;
        if ((u8)c == 0)
        {
            goto done;
        }
        if ((u32)(c - 0x19) < 7)
        {
            u32 b1;
            s32 off;
            u8 *pa;
            u8 *pb;

            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pa = D_8015D3B4 + b1 * 2;
            pa += off * 33;
            lead = *(volatile u8 *)in;
            pa += lead * 528;
            *out = *pa;
            out++;
            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pb = D_8015D3B4 + 1 + b1 * 2;
            pb += off * 33;
            lead = *(volatile u8 *)in;
            pb += lead * 528;
            *out = *pb;
            out++;
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            lead = *(volatile u8 *)in;
            index = lead - 0x20;
            *out = D_801605A8[(index / 16) * 33 + (index & 0xF) * 2];
            out++;
            lead = *(volatile u8 *)in;
            index = lead - 0x20;
            *out = D_801605A8[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = D_801605A8[0];
            out++;
            *out = D_801605A8[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
