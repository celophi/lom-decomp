#include "common.h"

/**
 * WIP matching reconstruction of func_801477CC.
 * Best exact-size verified checkpoint: 99.250000% (GCC 2.7.2 CDK).
 * A 99.42% weighted experimental basin exists but emits one extra instruction,
 * so this exact-size checkpoint is the version wired into the overlay.
 */
extern s32 D_80165FEC;
extern s32 D_801660A0;
extern s32 D_801663A8[];
extern s32 D_80166438;
extern char D_80166440[];
extern s32 D_80166A80[];
extern s32 D_80166AD8;
extern s32 D_80166AE8[];
extern char D_800ECF7C[];
extern char D_800ECFC4[];

s32 func_8001714C();
s32 func_80147490();
s32 func_80147588();
void func_80147C5C();
void func_8014A1C4();

s32 func_801477CC(void)
{
    s32 used[10];
    s32 *zero_ptr;
    s32 *scan_ptr;
    s32 *rank_ptr;
    s32 *base_rank;
    s32 *row;
    s32 *elem;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *field_base;
    s32 *field1;
    s32 slot;
    s32 count;
    s32 max_count;
    s32 loop_count;
    s32 final_count;
    s32 less_count;
    s32 j;
    s32 t0v;
    s32 i;
    s32 shared3;
    s32 shared4;
    s32 shared1;
    s32 shared2;
    s32 entry_off;
    s32 *fields;
    char *entries;
    u8 *parse_base;
    u8 *call_ptr;
    u8 *p;
    s32 n;
    s32 acc;
    u32 tmp0, tmp1, tmp2;

    func_80147588();
    i = 9;
    func_8014A1C4();
    zero_ptr = &used[9];
    do {
        *zero_ptr = 0;
        i--;
        zero_ptr--;
    } while (i >= 0);

    shared4 = 0;
    i = shared4;
    if (D_80165FEC > 0) {
        entries = D_80166440;
        parse_base = (u8 *)entries + 0xC;
        fields = D_80166AE8;
        shared1 = (s32)D_80166A80;
        shared3 = (s32)entries;
        entry_off = i;
        shared2 = i;
        do {
            if (func_8001714C(D_800ECF7C,
                    (void *)(D_801660A0 * 0x320 + shared3), 0xC) == 0) {
                n = 5;
                p = (u8 *)(D_801660A0 * 0x320 + entry_off + (s32)parse_base);
                acc = 0;
                while (((u8)(*p - '0') < 10) || ((u8)(*p - 'a') < 6) || ((u8)(*p - 'A') < 6)) {
                    if (n == 0)
                        break;
                    acc <<= 4;
                    if ((u8)(*p - '0') < 10) {
                        tmp0 = acc - 0x30;
                        acc = tmp0 + *p;
                    } else if ((u8)(*p - 'A') < 6) {
                        tmp1 = acc - 0x37;
                        acc = tmp1 + *p;
                    } else if ((u8)(*p - 'a') < 6) {
                        tmp2 = acc - 0x57;
                        acc = tmp2 + *p;
                    }
                    p++;
                    n--;
                }
                call_ptr = (u8 *)(D_801660A0 * 0x320 + shared3 + 0xC);
                *(s32 *)(shared2 + (D_801660A0 * 0x50 + (s32)fields)) = acc;
                *(s32 *)shared1 = func_80147490(call_ptr, acc, n);
                used[*(s32 *)shared1] = 1;
                if (shared4 < *(s32 *)shared1)
                    shared4 = *(s32 *)shared1;
            } else {
                *(s32 *)(shared2 + (D_801660A0 * 0x50 + (s32)fields)) = -1;
                *(s32 *)shared1 = 0;
            }
            do { shared1 += 4; } while (0);
            shared3 += 0x28;
            do { entry_off += 0x28; } while (0);
            loop_count = D_80165FEC;
            i++;
            do { shared2 += 4; } while (0);
        } while (i < loop_count);
    }

    shared3 = -1;
    func_80147C5C();
    t0v = 1;
    i = 0;
    if (D_80165FEC > 0)
    {
        count = D_80165FEC;
        base_rank = &D_801663A8[0];
        rank_ptr = base_rank;
        slot = D_801660A0;
        field1 = D_80166AE8;
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
                if (*elem >= shared3)
                {
                    *rank_ptr = t0v;
                    shared3 = *elem;
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
    D_80166438 = t0v;
    t0v = -1;
    i = 0;
    shared3 = 0;
    if (D_80165FEC > 0)
    {
        max_count = D_80165FEC;
        slot = D_801660A0;
        field_base = D_80166AE8;
        max_ptr = (s32 *)((slot * 0x50) + (s32)field_base);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                shared3 = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < max_count);
    }
    D_80166AD8 = t0v + 1;

    shared4 = 1;
    scan_ptr = &used[1];
scan_used:
    if (*scan_ptr != 0) {
        do { do { shared4++; } while (0); } while (0);
        scan_ptr++;
        if (shared4 < 9)
            goto scan_used;
    }

    i = 0;
    if (D_80165FEC > 0) {
        shared2 = (s32)D_80166A80;
        shared1 = (s32)D_80166440;
loop_prefix:
        if (func_8001714C(D_800ECFC4,
                (void *)(D_801660A0 * 0x320 + shared1), 8) == 0) {
            *(s32 *)shared2 = shared4;
        } else {
            do { shared2 += 4; } while (0);
            final_count = D_80165FEC;
            i++;
            do { shared1 += 0x28; } while (0);
            if (i < final_count)
                goto loop_prefix;
        }
    }
    return shared3;
}
