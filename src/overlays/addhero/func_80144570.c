#include "common.h"

extern s32 D_801609A8;
extern s32 D_801609A4;
extern s32 D_801651B0[];
extern s32 D_80164A68[];
extern u8 D_80164B60[];
extern char D_800ECF7C[];
s32 func_8001714C();
s32 func_80144478(u8 *text, s32 unused1, s32 unused2);

typedef struct { u8 data[0x28]; } AddheroEntry28;

/**
 * @brief Parse the hex rank value out of each ADDHERO entry whose name matches
 *        the D_800ECF7C prefix, store it, and return the maximum.
 * @note NON-MATCHING (99.82%). Sibling of niki func_80144740 (same body).
 *       The lone residue is a sched2 arg-order swap at the func_8001714C call:
 *       the target emits `addiu a0, %lo(D_800ECF7C)` before `li a2, 0xC`, ours
 *       after. sched_oracle classifies it as a post-allocation (sched2) reorder,
 *       not an emit-order fix; the do/while(0) fence on `pattern` is the best of
 *       the forms tried (direct-pass regresses to 98.62%).
 *       TODO: recover the exact sched2 ordering.
 * @see (99.82%)
 */
s32 func_80144570(void)
{
    s32 i; s32 max; u8 *p; u8 *field; s32 count; s32 acc;
    u32 tmp0, tmp1, tmp2; s32 r; u8 *pattern;
    i = 0; max = i;
    while (i < D_801609A4) {
        do { pattern = (u8 *)&D_800ECF7C; } while (0);
        if (func_8001714C(pattern, (u8 *)&((AddheroEntry28 (*)[20])D_80164B60)[D_801609A8][i], 0xC) == 0) {
            count = 5;
            p = (u8 *)(D_801609A8 * 0x320 + ((i << 4) + (i << 4) + (i << 3)) + (s32)D_80164B60 + 0xC);
            acc = 0;
            while (((u8)(*p-'0') < 10) || ((u8)(*p-'a') < 6) || ((u8)(*p-'A') < 6)) {
                if (count == 0) break;
                acc <<= 4;
                if ((u8)(*p-'0') < 10) { tmp0=acc-0x30; acc=tmp0+*p; }
                else if ((u8)(*p-'A') < 6) { tmp1=acc-0x37; acc=tmp1+*p; }
                else if ((u8)(*p-'a') < 6) { tmp2=acc-0x57; acc=tmp2+*p; }
                p++; count--;
            }
            field = (u8 *)&((AddheroEntry28 (*)[20])D_80164B60)[D_801609A8][i].data[0xC];
            { s32 addr; addr = D_801609A8 * 0x50 + (s32)D_80164A68; *(s32 *)(addr + i*4) = acc; }
            r = func_80144478(field, acc, count);
            D_801651B0[i] = r;
            if (max < r) max = r;
        } else {
            { s32 addr; addr = D_801609A8 * 0x50 + (s32)D_80164A68; *(s32 *)(addr + i*4) = -1; }
            D_801651B0[i] = 0;
        }
        i++;
    }
    return max;
}
