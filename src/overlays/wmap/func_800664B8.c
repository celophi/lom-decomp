/* Partial WMAP decompilation: 70.612750% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_8005FF88(M2C_UNK);                     /* extern */
void func_800652A8(s32, s32);
extern s32 D_800D9220;
extern s16 D_800D928A;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern u8 D_800DCEC8;
extern s32 D_8011CF18;
extern s32 D_8011D4FC;
extern s32 D_8013922C;
extern s32 D_8013986C;
extern s32 D_801398C0;
extern u8 D_80139950;
extern u8 D_801AFBA8;
extern u8 D_801AFBB8;

void func_800664B8(void)
{
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_a3_3;
    s32 temp_t1;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;

    temp_t1 = D_8013986C;
    switch (temp_t1)                                /* irregular */
    {
    case 0:
        if ((D_8013922C & 0x10) && (D_8011CF18 == 0) && (D_8011D4FC == -1))
        {
            D_8013986C = 3;
            func_8005FF88(-1);
            M2C_FIELD(&D_800DCEC8, s32 *, 0) = (s32) M2C_FIELD(&D_80139950, s32 *, 0);
            M2C_FIELD(&D_800DCEC8, s32 *, 4) = (s32) M2C_FIELD(&D_80139950, s32 *, 4);
            M2C_FIELD(&D_800DCEC8, s32 *, 8) = (s32) M2C_FIELD(&D_80139950, s32 *, 8);
            M2C_FIELD(&D_800DCEC8, s32 *, 0xC) = (s32) M2C_FIELD(&D_80139950, s32 *, 0xC);
            M2C_FIELD(&D_801AFBB8, s32 *, 0) = (s32) (M2C_FIELD(&D_80139950, s32 *, 0) << 8);
            M2C_FIELD(&D_801AFBB8, s32 *, 4) = (s32) (M2C_FIELD(&D_80139950, s32 *, 4) << 8);
            M2C_FIELD(&D_801AFBB8, s32 *, 8) = (s32) (M2C_FIELD(&D_80139950, s32 *, 8) << 8);
            M2C_FIELD(&D_801AFBA8, s32 *, 0) = (s32) -(M2C_FIELD(&D_80139950, s32 *, 0) * 0x10);
            temp_a3 = -(M2C_FIELD(&D_80139950, s32 *, 4) * 0x10);
            M2C_FIELD(&D_801AFBA8, s32 *, 4) = temp_a3;
            M2C_FIELD(&D_801AFBA8, s32 *, 8) = (s32) (0xC0000 - (M2C_FIELD(&D_80139950, s32 *, 8) * 0x10));
            func_800652A8(1, 0x80);
            var_v0 = 0xA130;
block_13:
            D_800D9220 = var_v0;
            D_801398C0 = 0;
            D_8013922C = 0;
            return;
        }
        return;
    case 1:
        if (D_8013922C & 0x30)
        {
            D_8013986C = 2;
            D_800D928A = 0x80;
            func_800652A8(2, 0x80);
            var_v0 = -1;
            goto block_13;
        }
        break;
    case 2:
        D_8013922C = 0;
        var_t0 = M2C_FIELD(&D_801AFBB8, s32 *, 0) - M2C_FIELD(&D_801AFBA8, s32 *, 0);
        M2C_FIELD(&D_801AFBB8, s32 *, 0) = var_t0;
        temp_a2 = M2C_FIELD(&D_801AFBB8, s32 *, 4) - M2C_FIELD(&D_801AFBA8, s32 *, 4);
        temp_a3_2 = M2C_FIELD(&D_801AFBB8, s32 *, 8) - M2C_FIELD(&D_801AFBA8, s32 *, 8);
        M2C_FIELD(&D_801AFBB8, s32 *, 4) = temp_a2;
        M2C_FIELD(&D_801AFBB8, s32 *, 8) = temp_a3_2;
        if (var_t0 < 0)
        {
            var_t0 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32 *, 0) = (s32) (var_t0 >> 8);
        var_v0_2 = temp_a2;
        if (var_v0_2 < 0)
        {
            var_v0_2 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32 *, 4) = (s32) (var_v0_2 >> 8);
        var_v0_3 = temp_a3_2;
        if (temp_a3_2 < 0)
        {
            var_v0_3 = temp_a3_2 + 0xFF;
        }
        M2C_FIELD(&D_80139950, s32 *, 8) = (s32) (var_v0_3 >> 8);
        if (temp_a3_2 == 0x600000)
        {
            M2C_FIELD(&D_80139950, s32 *, 0) = (s32) M2C_FIELD(&D_800DCEC8, s32 *, 0);
            M2C_FIELD(&D_80139950, s32 *, 4) = (s32) M2C_FIELD(&D_800DCEC8, s32 *, 4);
            M2C_FIELD(&D_80139950, s32 *, 8) = (s32) M2C_FIELD(&D_800DCEC8, s32 *, 8);
            M2C_FIELD(&D_80139950, s32 *, 0xC) = (s32) M2C_FIELD(&D_800DCEC8, s32 *, 0xC);
            D_800DBE78 = 0;
            D_8013986C = 0;
            D_800DBE70 = temp_t1;
            return;
        }
        break;
    case 3:
        D_8013922C = 0;
        var_t0_2 = M2C_FIELD(&D_801AFBB8, s32 *, 0) + M2C_FIELD(&D_801AFBA8, s32 *, 0);
        M2C_FIELD(&D_801AFBB8, s32 *, 0) = var_t0_2;
        temp_a2_2 = M2C_FIELD(&D_801AFBB8, s32 *, 4) + M2C_FIELD(&D_801AFBA8, s32 *, 4);
        temp_a3_3 = M2C_FIELD(&D_801AFBB8, s32 *, 8) + M2C_FIELD(&D_801AFBA8, s32 *, 8);
        M2C_FIELD(&D_801AFBB8, s32 *, 4) = temp_a2_2;
        M2C_FIELD(&D_801AFBB8, s32 *, 8) = temp_a3_3;
        if (var_t0_2 < 0)
        {
            var_t0_2 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32 *, 0) = (s32) (var_t0_2 >> 8);
        var_v0_4 = temp_a2_2;
        if (var_v0_4 < 0)
        {
            var_v0_4 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32 *, 4) = (s32) (var_v0_4 >> 8);
        var_v0_5 = temp_a3_3;
        if (temp_a3_3 < 0)
        {
            var_v0_5 = temp_a3_3 + 0xFF;
        }
        M2C_FIELD(&D_80139950, s32 *, 8) = (s32) (var_v0_5 >> 8);
        if (temp_a3_3 == 0xC00000)
        {
            M2C_FIELD(&D_80139950, s32 *, 0) = 0;
            M2C_FIELD(&D_80139950, s32 *, 8) = 0xC000;
            D_800DBE78 = 0;
            D_8013986C = 1;
            M2C_FIELD(&D_80139950, s32 *, 4) = 0;
            D_800DBE70 = 1;
        }
        break;
    }
}
