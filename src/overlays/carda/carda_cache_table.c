#include "common.h"

extern s32 D_80166BE8[];
extern void *D_80166FFC;
extern u8 D_80167000[];

/** @see decomp.me (100.00%) */
void func_8014ADF8(void)
{
    s32 i;
    s32 *p;

    D_80166FFC = D_80167000;
    i = 0;
    p = D_80166BE8;
    do
    {
        *p = (u16)*p;
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_8014AE34(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = D_80166BE8;
    do
    {
        if (!(*p & flag))
        {
            *p = 0;
        }
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_8014AE74(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = D_80166BE8;
    p += 0xFF;
    do
    {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    q = D_80167000;
    do
    {
        *(u8 *)(i + (s32)q) = 0;
        i++;
    } while (i <= 0x7FFF);
}
