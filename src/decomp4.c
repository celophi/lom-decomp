#include "decomp4.h"

/**
 * decomp.me (100%) https://decomp.me/scratch/hjYpL
 */
void func_800299EC(void)
{
    s32 val = (s32)D_8003EC6A;
    *(s16*)0x1F801DB0 = (s16)val;
    *(s16*)0x1F801DB2 = (s16)val;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/TxNq3
 */
void func_80029A0C(s32* arg0, s32* arg1, u32 arg2)
{

    arg2 >>= 2;

    while ((arg2 >> 2) != 0)
    {

        s32 second = arg0[1];
        s32 third = arg0[2];
        s32 fourth = arg0[3];
        s32 first = arg0[0];

        arg1[0] = first;
        arg1[1] = second;
        arg1[2] = third;
        arg1[3] = fourth;

        arg0 += 4;
        arg1 += 4;
        arg2 -= 4;
    }

    while (arg2 != 0)
    {
        *arg1 = *arg0;
        arg0++;
        arg1++;
        arg2--;
    }
}