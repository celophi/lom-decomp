#include "common.h"

extern s32 D_80122698;
extern u8 *D_801228F8[];
extern u8 *D_80122B74;
u8 *func_800C1E40(s32 arg0);

void func_800B661C(void)
{
    s32 off;
    s32 i;
    u8 *base;
    u8 *p;

    D_80122698 = 0;
    off = 0;
    if (D_80122B74[0x840] != 0 && ((*(u32 *)(D_80122B74 + 0x858) & 0x7F) == 2))
    {
        D_80122698 = 1;
        off = (D_80122B74[0x859] + 1) * 3;
    }

    base = func_800C1E40(9);
    i = 0;
    if (base != NULL)
    {
        u8 **out;
        out = D_801228F8;
        p = (u8 *)((off * 2) + (s32)base);
        do
        {
            *out = base + (*(u16 *)(p + 4) + 4);
            p += 2;
            i++;
            out++;
        } while ((u32)i < 3);
    }
    else
    {
        u8 **out;
        out = D_801228F8;
        do
        {
            *out = NULL;
            i++;
            out++;
        } while ((u32)i < 3);
    }
}
