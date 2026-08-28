#include "common.h"

extern u8 D_80165528[];
extern s32 D_8016D528[];
extern void *D_8016D93C;

/** @see decomp.me (100.00%) */
void func_80146D64(void)
{
    s32 i;
    s32 *p;

    D_8016D93C = D_80165528;
    i = 0;
    p = D_8016D528;
    do
    {
        *p = (u16)*p;
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void func_80146DA0(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = D_8016D528;
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
void func_80146DE0(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = D_8016D528;
    p += 0xFF;
    do
    {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    q = D_80165528;
    do
    {
        *(u8 *)(i + (s32)q) = 0;
        i++;
    } while (i <= 0x7FFF);
}
