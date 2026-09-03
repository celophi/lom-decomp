#include "common.h"

typedef struct { u8 data[0x28]; } NikiEntry28;

extern s32 D_80164B70;
extern s32 D_80164B78;
extern char D_800ECF7C[];
extern NikiEntry28 D_80165018[][20];
extern s32 D_80164F20[];
extern s32 D_80164EB8[];

s32 func_8001714C();
s32 func_80144648();

/**
 * @brief Parse the hex rank value out of each NIKI entry whose name matches the
 *        D_800ECF7C prefix, store it, and return the maximum.
 * @note NON-MATCHING (99.82%). Sibling of addhero func_80144570 (same body).
 *       The lone residue is a sched2 arg-order swap at the func_8001714C call:
 *       the target emits `addiu a0, %lo(D_800ECF7C)` before `li a2, 0xC`, ours
 *       after. sched_oracle classifies it as a post-allocation (sched2) reorder,
 *       not an emit-order fix; the do/while(0) fence on `pattern` is the best of
 *       the forms tried (direct-pass regresses to 98.62%).
 *       TODO: recover the exact sched2 ordering.
 * @see (99.82%)
 */
s32 func_80144740(void)
{
    s32 i;
    s32 max;
    u8 *p;
    u8 *field;
    s32 count;
    s32 acc;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;
    s32 r;

    i = 0;
    max = i;
    while (i < D_80164B78)
    {
        u8 *pattern;
        do { pattern = (u8 *)&D_800ECF7C; } while (0);
        if (func_8001714C(pattern, (u8 *)&D_80165018[D_80164B70][i], 0xC) == 0)
        {
            count = 5;
            p = (u8 *)(D_80164B70 * 0x320 + ((i << 4) + (i << 4) + (i << 3)) + (s32)D_80165018 + 0xC);
            acc = 0;
            while (((u8)(*p - '0') < 10) || ((u8)(*p - 'a') < 6) || ((u8)(*p - 'A') < 6))
            {
                if (count == 0)
                    break;
                acc <<= 4;
                if ((u8)(*p - '0') < 10)
                {
                    tmp0 = acc - 0x30;
                    acc = tmp0 + *p;
                }
                else if ((u8)(*p - 'A') < 6)
                {
                    tmp1 = acc - 0x37;
                    acc = tmp1 + *p;
                }
                else if ((u8)(*p - 'a') < 6)
                {
                    tmp2 = acc - 0x57;
                    acc = tmp2 + *p;
                }
                p++;
                count--;
            }
            field = &D_80165018[D_80164B70][i].data[0xC];
            {
                s32 addr;
                addr = D_80164B70 * 0x50 + (s32)D_80164F20;
                *(s32 *)(addr + i * 4) = acc;
            }
            r = func_80144648(field, acc, count);
            D_80164EB8[i] = r;
            if (max < r)
                max = r;
        }
        else
        {
            s32 addr;
            addr = D_80164B70 * 0x50 + (s32)D_80164F20;
            *(s32 *)(addr + i * 4) = -1;
            D_80164EB8[i] = 0;
        }
        i++;
    }
    return max;
}
