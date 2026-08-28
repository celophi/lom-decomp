#include "common.h"

extern void *D_80165674;
extern u8 D_80165678[];
extern s32 D_8016D678[];

/** @see decomp.me (100.00%) */
void func_80146EB8(void)
{
    s32 i;
    s32 *p;

    D_80165674 = D_80165678;
    i = 0;
    p = D_8016D678;
    do
    {
        *p = (u16)*p;
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_80146EF4(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = D_8016D678;
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
void func_80146F34(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = D_8016D678;
    p += 0xFF;
    do
    {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    q = D_80165678;
    do
    {
        *(u8 *)(i + (s32)q) = 0;
        i++;
    } while (i <= 0x7FFF);
}
