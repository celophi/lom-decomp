#include "common.h"

extern s32 D_801609A4;
extern void *D_80165488;
extern s32 D_80160574;
extern s32 D_80160580;
extern s32 D_80160588;
extern s32 D_801609B4;
extern s32 D_80122988;
extern s32 D_8016093C;

s32 func_80144C28();

/** @see decomp.me (100%) */
s32 func_80140790(void)
{
    s32 result;

    if (D_801609A4 >= 0x10)
    {
        if (D_80165488 == 0)
        {
            D_80165488 = &D_80160574;
        }
    }

    do
    {
        result = func_80144C28();
    } while (result == 3);

    if ((D_801609B4 != 0) && (D_80122988 & 0x220))
    {
        if (D_8016093C == 0)
        {
            D_801609A4 = 0xF9;
        }
        else
        {
            D_801609A4 = 0xF8;
        }
        D_80165488 = &D_80160588;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            D_80165488 = &D_80160580;
            D_801609B4 = 0;
            break;
        case 5:
            if (D_8016093C == 0)
            {
                D_801609A4 = 0xF9;
            }
            else
            {
                D_801609A4 = 0xF8;
            }
            /* fallthrough */
        case 2:
            D_80165488 = &D_80160588;
            break;
        }
    }
}
