/* Partial WMAP decompilation: 66.991840% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

#include "sdk/libgpu.h"
void cdrom_queue_read(s32, void *);
M2C_UNK cdrom_wait_queue_empty();                   /* extern */
s32 func_80065428(s32, s32 *);                      /* extern */
extern u8 D_800D0544;
extern s32 D_800D916C;
extern s32 D_8011CF50;
extern s32 D_8011D0DC;
extern s32 D_8013922C;
extern u8 D_80139290;
extern s32 D_80139868;
extern s32 D_801398A8;
extern s32 D_801398B4;
extern s32 D_801398C0;
extern s32 D_8013B258;
extern s32 D_80182DB4;
extern u8 D_8019D248;
extern u8 D_8019D6E0;
extern s32 D_801ADAFC;

void func_80061FF8(void)
{
    RECT sp10;
    s16 temp_v1;
    s16 temp_v1_2;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 var_a1;
    s16 var_a2;
    s16 var_a3;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1_3;
    s32 var_t1;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    void *temp_s1;
    void *temp_s1_2;

    temp_s1 = *(((D_8011D0DC - 1) * 4) + (u8 *)&D_800D0544) + (D_801398A8 * 2);
    temp_v1 = M2C_FIELD(temp_s1, s16 *, 0);
    temp_s1_2 = temp_s1 + 2;
    D_801398B4 = (s32) temp_v1;
    if (temp_v1 == -1)
    {
        D_8011CF50 = 0;
        goto block_39;
    }
    if (temp_v1 == -2)
    {
        temp_v1_2 = M2C_FIELD(temp_s1, s16 *, 2);
        switch (temp_v1_2)
        {
        case 0:
            if (M2C_FIELD(temp_s1_2, s16 *, 2) > 0)
            {
                cdrom_queue_read((u16) M2C_FIELD(temp_s1_2, s16 *, 2), &D_8019D6E0);
                cdrom_wait_queue_empty();
                sp10 = *(RECT *)((u8 *)&D_8019D6E0 + 12);
                LoadImage(&sp10, (u8 *)&D_8019D6E0 + 0x14);
                sp10 = *(RECT *)((u8 *)&D_8019D6E0 + 12 + M2C_FIELD(&D_8019D6E0, s32 *, 8));
                LoadImage(&sp10, M2C_FIELD(&D_8019D6E0, s32 *, 8) + ((u8 *)&D_8019D6E0 + 8) + 0xC);
                DrawSync(0);
                D_801ADAFC = 1;
                D_800D916C = 1;
            }
            else
            {
                D_800D916C = 0;
            }
            var_v0 = D_801398A8 + 3;
block_27:
            D_801398A8 = var_v0;
            break;
        case 1:
            D_80139868 = (s32) M2C_FIELD(temp_s1_2, s16 *, 2);
            D_801398A8 += 3;
            break;
        case 2:
            if (M2C_FIELD(temp_s1_2, s16 *, 2) != 0)
            {
                D_8013B258 = 0;
            }
            else
            {
                D_8013B258 = 1;
            }
            var_v0 = D_801398A8 + 3;
            goto block_27;
        case 3:
            var_v1 = 3;
block_19:
            D_80182DB4 = var_v1;
            D_801398A8 += 2;
            break;
        case 4:
            var_v1 = 4;
            goto block_19;
        case 5:
            var_v1 = 5;
            goto block_19;
        case 6:
            var_v1 = 6;
            goto block_19;
        case 7:
            var_v1 = 7;
            goto block_19;
        case 8:
            var_a1 = 0;
            var_a2 = 0;
            do
            {
                var_a3 = 0;
                var_t1 = 0;
loop_22:
                temp_v0 = var_a2 << 0x10;
                if (*((var_a1 * 0x28) + var_t1 + (u8 *)&D_80139290) == 0)
                {
                    M2C_FIELD(&D_8019D248, s16 *, 0x10) = var_a3;
                    M2C_FIELD(&D_8019D248, s16 *, 0xC) = var_a3;
                    var_a3 = 0x120;
                    var_t1 = 0x5A0;
                    M2C_FIELD(&D_8019D248, s16 *, 8) = var_a2;
                    var_a2 = 6;
                    temp_v1_3 = var_a1 * 3;
                    temp_a0 = var_a1 << 0x10;
                    M2C_FIELD(&D_8019D248, s16 *, 0xA) = var_a1;
                    var_a1 = 6;
                    temp_v1_4 = temp_v1_3 * 0x10;
                    temp_v0_2 = temp_v0 >> 0x10;
                    temp_a0_2 = temp_a0 >> 0x10;
                    M2C_FIELD(&D_8019D248, s16 *, 0x12) = temp_v1_4;
                    M2C_FIELD(&D_8019D248, s16 *, 0xE) = temp_v1_4;
                    M2C_FIELD(&D_8019D248, s32 *, 0x18) = temp_v0_2;
                    M2C_FIELD(&D_8019D248, s32 *, 0) = temp_v0_2;
                    M2C_FIELD(&D_8019D248, s32 *, 0x1C) = temp_a0_2;
                    M2C_FIELD(&D_8019D248, s32 *, 4) = temp_a0_2;
                    M2C_FIELD(&D_8019D248, s32 *, 0x14) = 0;
                }
                var_a3 += 0x30;
                var_a2 += 1;
                var_t1 += 0xF0;
                if (var_a2 < 6)
                {
                    goto loop_22;
                }
                var_a1 += 1;
                var_a2 = 0;
            } while (var_a1 < 6);
            var_v0 = D_801398A8 + 2;
            goto block_27;
        }
        D_801398B4 = 1;
        return;
    }
    if (temp_v1 != 0)
    {
        temp_v1_5 = M2C_FIELD(temp_s1, s16 *, 2);
        D_801398A8 += 2;
        D_801398C0 = (s32) temp_v1_5;
        D_8013922C = (s32) temp_v1_5;
        return;
    }
    if (M2C_FIELD(temp_s1, s16 *, 2) == 0)
    {
        var_v0_2 = func_80065428(D_801398A8, &D_8011D0DC);
        goto block_35;
    }
    if ((u16) M2C_FIELD(temp_s1, s16 *, 2) & 0x800)
    {
        var_v0_2 = func_80065428(D_801398A8, &D_8011D0DC) & (s16) ((u16) M2C_FIELD(temp_s1, s16 *, 2) & 0xF7FF);
block_35:
        if (var_v0_2 != 0)
        {
            D_801398A8 += 2;
        }
        D_801398B4 = 1;
        D_801398C0 = 0;
        D_8013922C = 0;
        return;
    }
block_39:
    D_8011D0DC = 0;
    D_801398B4 = 1;
}
