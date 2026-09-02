#include "common.h"

typedef struct
{
    char name[20];
    s32 attr;
    s32 size;
    void *next;
    s32 head;
    char system[4];
} Entry;

extern s32 D_80165FEC;
extern s32 D_801660A0;
extern s32 D_80166078;
extern char D_80166440[];

s32 func_80147DCC(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (D_80165FEC > 0)
    {
        offset = D_801660A0 * 0x320;
        do
        {
            do
            {
                sum += ((Entry *)((u8 *)D_80166440 + offset))->size / 8192;
            } while (0);
            i++;
            offset += 0x28;
        } while (i < D_80165FEC);
    }
    if (sum >= 14 || D_80166078 == 2)
    {
        if (sum < 10)
        {
            if (D_80166078 != 2)
                goto one;
            goto zero;
        }
        goto one;
    }
zero:
    return 0;
one:
    return 1;
}
