typedef signed int s32; typedef unsigned int u32; typedef unsigned char u8;
typedef struct { char name[20]; s32 attr; s32 size; void *next; s32 head; char system[4]; } CardaDirEntry;
extern s32 D_80165FEC;
extern s32 D_801660A0;
extern CardaDirEntry D_80166440[][20];
extern s32 D_80166AE8[];
extern s32 D_80166A80[];
extern char D_800ECF7C[];
s32 func_80147490(u8 *, s32, s32);
s32 func_8001714C();

/**
 * @brief Scan directory entries for records whose name matches D_800ECF7C, parse a
 *        hex rank suffix from each, store it, and return the maximum rank seen.
 * @return Highest rank value produced by func_80147490 across matching entries.
 * @note WIP - not yet byte-matching. Currently 97.82% (residual: saved-register
 *       allocation for the max/index accumulators plus two scheduling swaps in the
 *       name-compare call and the shift-amount reuse).
 */
s32 func_80147588(void)
{
    s32 i; s32 max; u8 *p; u8 *field; s32 count; s32 acc;
    u32 tmp0, tmp1, tmp2; s32 r; u8 *pattern;
    i = 0; max = i;
    while (i < D_80165FEC) {
        do { pattern = (u8 *)&D_800ECF7C; } while (0);
        if (func_8001714C(pattern, (u8 *)&((CardaDirEntry (*)[20])D_80166440)[D_801660A0][i], 0xC) == 0) {
            count = 5;
            p = (u8 *)(D_801660A0 * 0x320 + ((i << 4) + (i << 4) + (i << 3)) + (s32)D_80166440 + 0xC);
            acc = 0;
            while (((u8)(*p-'0') < 10) || ((u8)(*p-'a') < 6) || ((u8)(*p-'A') < 6)) {
                if (count == 0) break;
                acc <<= 4;
                if ((u8)(*p-'0') < 10) { tmp0=acc-0x30; acc=tmp0+*p; }
                else if ((u8)(*p-'A') < 6) { tmp1=acc-0x37; acc=tmp1+*p; }
                else if ((u8)(*p-'a') < 6) { tmp2=acc-0x57; acc=tmp2+*p; }
                p++; count--;
            }
            field = (u8 *)&((CardaDirEntry (*)[20])D_80166440)[D_801660A0][i].name[0xC];
            { s32 addr; addr = D_801660A0 * 0x50 + (s32)D_80166AE8; *(s32 *)(addr + i*4) = acc; }
            r = func_80147490(field, acc, count);
            D_80166A80[i] = r;
            if (max < r) max = r;
        } else {
            { s32 addr; addr = D_801660A0 * 0x50 + (s32)D_80166AE8; *(s32 *)(addr + i*4) = -1; }
            D_80166A80[i] = 0;
        }
        i++;
    }
    return max;
}
