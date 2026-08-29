#include "common.h"

extern s32 D_801609A4;
extern s32 D_80165490[];
extern s32 D_801609A8;
extern s32 D_80164A68[];
extern s32 D_80165520;
extern s32 D_80164A5C;
extern s32 D_801651B0[];
extern char D_80164B60[];
extern char D_800ECFC4[];

s32 func_80144570();
s32 func_80146198();
void func_801449F0(void);
s32 func_8001714C();

s32 func_801447B4(s32 unused0, s32 unused1, s32 unused2)
{
    s32 *row;
    s32 *elem;
    s32 *rank_ptr;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *base_rank;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *field_base;
    s32 *field1;
    s32 slot;
    s32 *out_ptr;
    char *ent_ptr;
    s32 t0v;
    s32 i;
    s32 s3v;
    s32 count;
    s32 handle;
    s32 less_count;
    s32 j;

    func_80144570();
    s3v = -1;
    func_80146198();
    i = 0;
    handle = func_80144570();
    func_801449F0();
    t0v = 1;
    if (D_801609A4 > 0)
    {
        count = D_801609A4;
        base_rank = &D_80165490[0];
        rank_ptr = base_rank;
        slot = D_801609A8;
        field1 = D_80164A68;
        row = field1 + slot * 20;
        elem = row;
        do
        {
            if (*elem >= 0)
            {
                j = 0;
                if (i > 0)
                {
                    j += 1; j -= 1;
                }
                if (*elem >= s3v)
                {
                    *rank_ptr = t0v;
                    s3v = *elem;
                    t0v += 1;
                }
                else
                {
                    less_count = j;
                    if (i > 0)
                    {
                        ecopy = elem;
                        inc_ptr = base_rank;
                        cmp_ptr = row;
                        do
                        {
                            if (*ecopy < *cmp_ptr)
                            {
                                less_count += 1;
                                *inc_ptr += 1;
                            }
                            inc_ptr += 1;
                            j += 1;
                            cmp_ptr += 1;
                        } while (j < i);
                    }
                    {
                        s32 rank_value;
                        do { do { do { rank_value = t0v - less_count; } while (0); } while (0); } while (0);
                        *rank_ptr = rank_value;
                    }
                    t0v += 1;
                }
            }
            rank_ptr += 1;
            i += 1;
            elem += 1;
        } while (i < count);
    }
    cmp_ptr = base_rank;
    inc_ptr = row;
    D_80165520 = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (D_801609A4 > 0)
    {
        s32 max_count;
        max_count = D_801609A4;
        slot = D_801609A8;
        field_base = D_80164A68;
        max_ptr = (s32 *)((slot * 0x50) + (s32)field_base);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                s3v = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < max_count);
        i = 0;
    }
    D_80164A5C = t0v + 1;
    if (D_801609A4 > 0)
    {
        out_ptr = &D_801651B0[0];
        ent_ptr = &D_80164B60[0];
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], (void *)((D_801609A8 * 0x320) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += 0x28;
            i += 1;
            if (i < D_801609A4)
            {
                goto loop_20;
            }
        }
    }
    return s3v;
}
