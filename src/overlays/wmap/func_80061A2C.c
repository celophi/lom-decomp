/* Partial WMAP decompilation: 73.200000% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

#include "sdk/libgpu.h"
M2C_UNK akao_play_sfx_from_buffer(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
void cdrom_queue_read(s32, void *);
M2C_UNK cdrom_wait_queue_empty();                   /* extern */
void func_8006534C(s32, s32);
extern s32 D_800CB204;
extern s32 D_800CB224;
extern s32 D_800CB23C;
extern s32 D_800CB254;
extern u8 D_800D06E4;
extern u8 D_800D06F8;
extern s32 D_800D9210;
extern s32 D_800D921C;
extern s32 D_800DCEA0;
extern s32 D_8013922C;
extern s32 D_801398C0;
extern s32 D_801398C4;
extern s32 D_801398D4;
extern void *D_801398EC;
extern s32 D_801398FC;
extern u8 D_8019D6E0;
extern s32 D_801ADAFC;

void func_80061A2C(void)
{
    RECT sp10;
    void *var_a3;
    s32 temp_a0;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 var_a0;
    s32 var_t1;
    void *temp_a0_2;
    void *temp_a0_3;

    if (D_8013922C & 0x100)
    {
        if (D_801398FC == 0)
        {
            temp_v0 = D_801398D4 == 0;
            D_801398D4 = temp_v0;
            if (temp_v0 != 0)
            {
                var_a0 = D_800CB224;
            }
            else
            {
                var_a0 = D_800CB23C;
            }
        }
        else
        {
            var_a0 = D_800CB224;
            D_801398FC = 0;
        }
        akao_play_sfx_from_buffer(var_a0, 0, 0x80, 0x7F);
        D_801398C0 = 0;
        D_8013922C = 0;
    }
    if (D_801398D4 == 0)
    {
        temp_a0 = D_800D9210 - 1;
        temp_v0_2 = temp_a0 * 0x10;
        if (temp_v0_2 != D_801398C4)
        {
            if (temp_v0_2 < D_801398C4)
            {
                D_801398C4 -= 4;
            }
            else
            {
                D_801398C4 += 4;
            }
            goto block_30;
        }
        if (D_801398FC == 0)
        {
            if (D_8013922C & 0x1000)
            {
                D_800D9210 = temp_a0;
                if (temp_a0 <= 0)
                {
                    D_800D9210 = 6;
                }
                akao_play_sfx_from_buffer(D_800CB204, 0, 0x80, 0x7F);
            }
            if (D_8013922C & 0x4000)
            {
                temp_v0_3 = D_800D9210 + 1;
                D_800D9210 = temp_v0_3;
                if (temp_v0_3 >= 7)
                {
                    D_800D9210 = 1;
                }
                akao_play_sfx_from_buffer(D_800CB204, 0, 0x80, 0x7F);
            }
        }
        if (D_8013922C & 0x40)
        {
            if (D_801398FC == 0)
            {
                D_801398FC = D_800D9210;
                akao_play_sfx_from_buffer(D_800CB254, 0, 0x80, 0x7F);
            }
        }
        if (D_8013922C & 0x20)
        {
            if (D_801398FC == 0)
            {
                D_801398D4 = D_801398D4 == 0;
                akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
                D_801398C0 = 0;
                D_8013922C = 0;
                return;
            }
            D_801398FC = 0;
            akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
            goto block_30;
        }
block_30:
        if (D_800DCEA0 != D_801398FC)
        {
            D_800DCEA0 = D_801398FC;
            cdrom_queue_read(((u16) D_801398FC + 0x114B) & 0xFFFF, &D_8019D6E0);
            cdrom_wait_queue_empty();
            sp10 = *(RECT *)((u8 *)&D_8019D6E0 + 12);
            LoadImage(&sp10, (u8 *)&D_8019D6E0 + 0x14);
            sp10 = *(RECT *)((u8 *)&D_8019D6E0 + 12 + M2C_FIELD(&D_8019D6E0, s32 *, 8));
            LoadImage(&sp10, M2C_FIELD(&D_8019D6E0, s32 *, 8) + ((u8 *)&D_8019D6E0 + 8) + 0xC);
            DrawSync(0);
            D_801ADAFC = 1;
        }
        if (D_801398FC == 0)
        {
            var_t1 = 0;
            var_a3 = &D_800D06F8;
            do
            {
                temp_a0_2 = M2C_FIELD(D_801398EC, void **, 0x33C);
                M2C_FIELD(temp_a0_2, s32 *, 0) = (s32) M2C_FIELD(var_a3, s32 *, 0);
                M2C_FIELD(temp_a0_2, s32 *, 4) = (s32) M2C_FIELD(var_a3, s32 *, 4);
                M2C_FIELD(temp_a0_2, s32 *, 8) = (s32) M2C_FIELD(var_a3, s32 *, 8);
                M2C_FIELD(temp_a0_2, s32 *, 0xC) = (s32) M2C_FIELD(var_a3, s32 *, 0xC);
                M2C_FIELD(temp_a0_2, s32 *, 0x10) = (s32) M2C_FIELD(var_a3, s32 *, 0x10);
                M2C_FIELD(temp_a0_2, s32 *, 0x14) = (s32) M2C_FIELD(var_a3, s32 *, 0x14);
                M2C_FIELD(temp_a0_2, s32 *, 0x18) = (s32) M2C_FIELD(var_a3, s32 *, 0x18);
                M2C_FIELD(temp_a0_2, u16 *, 0xA) = (u16) (M2C_FIELD(temp_a0_2, u16 *, 0xA) + (u16) D_801398C4);
                M2C_FIELD(temp_a0_2, u16 *, 0x1A) = (u16) (M2C_FIELD(temp_a0_2, u16 *, 0x1A) + (u16) D_801398C4);
                M2C_FIELD(temp_a0_2, u16 *, 0x12) = (u16) (M2C_FIELD(temp_a0_2, u16 *, 0x12) + (u16) D_801398C4);
                M2C_FIELD(temp_a0_2, s32 *, 0) = (s32) ((M2C_FIELD(temp_a0_2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFFFFFF));
                M2C_FIELD(D_801398EC, s32 *, 0x74) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFF000000) | ((s32) temp_a0_2 & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x1C;
                    M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x1C);
                }
                var_t1 += 1;
                var_a3 += 0x1C;
            } while (var_t1 < 0x1C);
        }
        func_8006534C(0xB5, 1);
        temp_a0_3 = M2C_FIELD(D_801398EC, void **, 0x33C);
        M2C_FIELD(temp_a0_3, s32 *, 0) = (s32) M2C_FIELD(&D_800D06E4, s32 *, 0);
        M2C_FIELD(temp_a0_3, s32 *, 4) = (s32) M2C_FIELD(&D_800D06E4, s32 *, 4);
        M2C_FIELD(temp_a0_3, s32 *, 8) = (s32) M2C_FIELD(&D_800D06E4, s32 *, 8);
        M2C_FIELD(temp_a0_3, s32 *, 0xC) = (s32) M2C_FIELD(&D_800D06E4, s32 *, 0xC);
        M2C_FIELD(temp_a0_3, s32 *, 0x10) = (s32) M2C_FIELD(&D_800D06E4, s32 *, 0x10);
        M2C_FIELD(temp_a0_3, s32 *, 0) = (s32) ((M2C_FIELD(temp_a0_3, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFFFFFF));
        M2C_FIELD(D_801398EC, s32 *, 0x74) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFF000000) | ((s32) temp_a0_3 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x14);
        }
        func_8006534C(0xD5, 1);
        D_801398C0 = 0;
        D_8013922C = 0;
    }
}
