#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
} StructB3580;

/** @brief View of the D_80122B74 block: a byte at 0x2E5 and 0xC-byte rows at 0x2F4. */
typedef struct
{
    u8 pad[0x2E5];
    u8 unk2E5;
    u8 pad2E6[0x2F4 - 0x2E6];
    u8 unk2F4[1][0xC];
} StructB74;

#define FIELD_B74 ((StructB74 *)D_80122B74)

void akao_set_song_params(s32, s32, s32, s32);
void func_800BD520(s32, s32, s32);
s32 func_800B37D4(void);
s32 func_800B3DF4(s32);
void func_800B4390(void);
void func_800C1EC8(s32, void *, s32);
u8 *func_800C1E40(s32);
u32 func_800BD414(s32, s32);
s32 func_800C3688(s32);
void func_800B3580(void);
s32 func_800B3670(s32);

extern u8 *D_80122B74;
extern s32 D_8010D020;
extern u8 D_800EF8C0[];
extern u8 D_800F0B48[];
extern u8 D_800F0AE8[];
extern StructB3580 D_80123B08;
extern u8 *D_80123FAC;
extern StructB3580 *D_80123FB0;
extern u16 g_music_track_index;

/**
 * @brief Rebuild the D_80123B08 block and write script variables 0x4280 and 0x4284, or call func_800B4390 when arg0 is 0.
 *
 * With D_8010D020 set both variables are written as 1 instead of the
 * computed values. 0x4280 and 0x4284 are the counter pair that func_800B48B8
 * increments and func_800B62D8 tests for zero.
 *
 * @param arg0 Nonzero selects the rebuild path and is forwarded to func_800B3DF4.
 * @see decomp.me (100%)
 */
void func_800B34D0(s32 arg0)
{
    s32 value;

    if (arg0 != 0)
    {
        func_800B3580();
        value = func_800B37D4();
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4280, 1);
        }
        else
        {
            func_800BD520(0, 0x4280, value);
        }
        value = func_800B3DF4(arg0);
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4284, 1);
        }
        else
        {
            func_800BD520(0, 0x4284, value);
        }
    }
    else
    {
        func_800B4390();
    }
}

/**
 * @brief Zero the D_80123B08 block, then fill it from the current track's 0xC-byte layout record and resource 1.
 * @see decomp.me (100%)
 */
void func_800B3580(void)
{
    s32 i;
    u8 *p;

    D_80123FAC = D_800EF8C0;
    D_80123FB0 = &D_80123B08;
    func_800C1EC8(0, &D_80123B08, 0x4A4);
    D_80123FB0->unk18 = 0;
    D_80123FB0->unk0 = func_800B3670(0);

    for (i = 0; i < 8; i++)
    {
        D_80123FB0->unkC[i] = D_800F0B48[FIELD_B74->unk2F4[g_music_track_index][i]];
    }

    p = func_800C1E40(1);
    D_80123FB0->unk4 = p + *(s32 *)(p + 4);
    D_80123FB0->unk8 = p + *(s32 *)(p + 8);
    func_800BD520(0, 0x428C, -1);
}

/**
 * @brief Look up D_800F0AE8 by a 0..0x3F index and clamp the result to script variables 0x52E0..0x52E8 and 0x63.
 *
 * The index comes from the byte at 0x2E5 of the layout buffer when @p arg0 or
 * bit 7 of script variable 0x52F0 is set, otherwise from func_800C3688 for the
 * current track. Script variable 0x2938 adds 0x14 (mode 1) or forces 0x3F
 * (mode 2).
 *
 * @param arg0 Nonzero selects the byte-at-0x2E5 index.
 * @return Value in 0..0x63.
 * @see decomp.me (100%)
 */
s32 func_800B3670(s32 arg0)
{
    s32 flag;
    s32 mode;
    s32 index;
    s32 value;
    u32 lo;
    u32 hi;

    flag = arg0;
    if (func_800BD414(0, 0x52F0) & 0x80)
    {
        flag = 1;
    }
    mode = func_800BD414(0, 0x2938);

    if (flag != 0)
    {
        switch (mode)
        {
            case 1:
                index = FIELD_B74->unk2E5 + 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
            default:
                index = FIELD_B74->unk2E5;
                break;
        }
        index = (index * 3) / 2;
    }
    else
    {
        index = func_800C3688(g_music_track_index);
        switch (mode)
        {
            case 1:
                index += 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
        }
    }

    if (index >= 0x40)
    {
        index = 0x3F;
    }

    value = D_800F0AE8[index];
    lo = func_800BD414(0, 0x52E0);
    hi = func_800BD414(0, 0x52E8);
    if (value < lo)
    {
        value = lo;
    }
    else if (value > hi)
    {
        value = hi;
    }

    if (value >= 0x64)
    {
        value = 0x63;
    }
    return value;
}



extern u8 *D_80122B74;
extern s32 D_8010D020;
extern u8 *func_80087F0C(s32);
extern void func_800B3D84(void);
extern void func_800B4934(u8 *);
extern s32 func_800B7EE8(u8 *, s32);

/** @brief Initialize the three party actor records and their derived attributes. */
s32 func_800B37D4(void)
{
    s32 sp14;
    s32 sp10;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a1;
    s32 temp_v1;
    s32 var_a1;
    s32 var_a3;
    s32 var_fp;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s0_3;
    s32 var_s2;
    s32 var_s4;
    s32 var_t0;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t3;
    s32 var_t5;
    s32 var_t6;
    s32 var_v0_2;
    s8 temp_v0_2;
    s8 var_s7;
    s8 var_v0;
    u16 var_a0;
    u32 var_s1;
    u32 var_s1_2;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a0_5;
    u8 *temp_a0_6;
    u8 *temp_a0_7;
    u8 *temp_a0_8;
    u8 *temp_a1_2;
    u8 *temp_a1_3;
    u8 *temp_a1_4;
    u8 *temp_a1_5;
    u8 *temp_a2;
    u8 *temp_a2_2;
    u8 *temp_a3;
    u8 *temp_v0;
    u8 *temp_v1_2;
    u8 *temp_v1_3;
    u8 *temp_v1_4;
    u8 *temp_v1_5;
    u8 *var_v1;

    var_s7 = 0;
    var_s4 = 0;
    var_s2 = 0;
    var_fp = 0x28;
    sp10 = 0;
    sp14 = 0x5F0;
    do
    {
        if (*(u8 *)((u8 *)((D_80122B74 + var_s4)) + 0x5F0) != 0)
        {
            if ((D_8010D020 != 0) && (var_s7 == 0))
            {
                var_v1 = ((u8 *)D_80123FB0);
                var_v0 = *(u8 *)((u8 *)(var_v1) + 0x28) | 0x40;
            }
            else
            {
                var_v1 = ((u8 *)D_80123FB0) + var_s2;
                var_v0 = *(u8 *)((u8 *)(var_v1) + 0x28) | 0x80;
            }
            *(u8 *)((u8 *)(var_v1) + 0x28) = var_v0;
            *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_s2)) + 0x2B) = 0xF;
            *(u8 *)(((u8 *)D_80123FB0) + var_s2 + 0x2C) = var_s7;
            temp_a0 = ((u8 *)D_80123FB0) + var_s2;
            temp_v1 = *(u32 *)((u8 *)(temp_a0) + 0x2C);
            temp_a1 = temp_v1 | 0x100;
            *(u32 *)((u8 *)(temp_a0) + 0x2C) = temp_a1;
            if (D_8010D020 != 0)
            {
                var_v0_2 = 0;
                if (var_s7 == 0)
                {
                    var_v0_2 = 0xFF;
                }
                *(u32 *)((u8 *)(temp_a0) + 0x2C) = (s32) ((temp_a1 & ~0x200) | ((var_v0_2 & 1) << 9));
            }
            else
            {
                *(u32 *)((u8 *)(temp_a0) + 0x2C) = (s32) (temp_v1 | 0x300);
            }
            temp_a1_2 = ((u8 *)D_80123FB0) + var_s2;
            *(u8 *)((u8 *)(temp_a1_2) + 0x30) = 5;
            *(u32 *)((u8 *)(temp_a1_2) + 0x2C) = (s32) ((*(u32 *)((u8 *)(temp_a1_2) + 0x2C) & 0xFFFF03FF) | ((*(u8 *)((u8 *)((D_80122B74 + var_s4)) + 0x608) & 0x3F) << 0xA));
            *(u16 *)((u8 *)(temp_a1_2) + 0x2E) = 0;
            *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_s2)) + 0x31) = 5;
            temp_v0 = ((u8 *)D_80123FB0) + var_s2;
            *(u16 *)((u8 *)(temp_v0) + 0x32) = 0;
            *(u32 *)((u8 *)(temp_v0) + 0x34) = 0;
            var_t1 = 0;
            var_t3 = var_s2;
            var_t6 = 0;
            var_t5 = var_s2;
            temp_a0_2 = ((u8 *)D_80123FB0) + var_s2;
            *(u8 * *)((u8 *)(temp_a0_2) + 0x38) = func_80087F0C(var_s7);
            *(u8 *)((u8 *)(temp_a0_2) + 0x42) = 0x19;
            *(u16 *)((u8 *)(temp_a0_2) + 0x40) = (u16) *(u16 *)((u8 *)((var_s4 + D_80122B74)) + 0x664);
            do
            {
                var_s0 = 1;
                var_t0 = var_s4 + 0x40;
                *(u16 *)((u8 *)((((u8 *)D_80123FB0) + var_t5)) + 0x44) = 0;
                *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_t3)) + 0x4C) = (u8) *(u8 *)((u8 *)((var_s4 + D_80122B74 + var_t1)) + 0x670);
loop_13:
                if (*(u8 *)((u8 *)((D_80122B74 + var_t0)) + 0x640) != 0)
                {
                    temp_a0_3 = (s32)(var_s4 + D_80122B74 + (var_s0 << 6) + 0x640);
                    temp_a1_3 = ((u8 *)D_80123FB0) + (var_t6 + var_s2);
                    temp_a2 = ((u8 *)D_80123FB0) + var_t3;
                    *(u16 *)((u8 *)(temp_a1_3) + 0x44) = (u16) (*(u16 *)((u8 *)(temp_a1_3) + 0x44) + *(u16 *)((u8 *)((temp_a0_3 + var_t6)) + 0x24));
                    *(u8 *)((u8 *)(temp_a2) + 0x4C) = (u8) (*(u8 *)((u8 *)(temp_a2) + 0x4C) + *(u8 *)((u8 *)((temp_a0_3 + var_t1)) + 0x30));
                }
                var_s0 += 1;
                var_t0 += 0x40;
                if (var_s0 < 4)
                {
                    goto loop_13;
                }
                var_t3 += 1;
                var_t6 += 2;
                var_t1 += 1;
                var_t5 += 2;
            } while (var_t1 < 4);
            var_s0_2 = 0;
            func_800B4934(((u8 *)D_80123FB0) + var_fp);
            var_s1 = *(u32 *)((u8 *)((var_s4 + D_80122B74)) + 0x658);
            do
            {
                temp_v0_2 = func_800B7EE8(D_80122B74 + sp14, var_s0_2);
                temp_a0_4 = var_s0_2 + var_s2;
                temp_v1_2 = ((u8 *)D_80123FB0) + temp_a0_4;
                *(u8 *)((u8 *)(temp_v1_2) + 0x58) = temp_v0_2;
                *(u8 *)((u8 *)(temp_v1_2) + 0x50) = temp_v0_2;
                *(u8 *)((u8 *)((((u8 *)D_80123FB0) + temp_a0_4)) + 0x64) = (s8) (var_s1 & 0xF);
                var_s1 = var_s1 >> 4;
                temp_a0_5 = ((u8 *)D_80123FB0) + temp_a0_4;
                temp_v0_3 = *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_s0_2)) + 0xC);
                var_s0_2 += 1;
                *(u8 *)((u8 *)(temp_a0_5) + 0x64) = (u8) (*(u8 *)((u8 *)(temp_a0_5) + 0x64) + temp_v0_3);
            } while (var_s0_2 < 8);
            var_t1_2 = 1;
            *(u8 *)((u8 *)((((u8 *)D_80123FB0) + (var_s0_2 + var_s2))) + 0x6C) = 0;
            *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_s2)) + 0x60) = 0;
            var_a1 = 0x40;
            *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_s2)) + 0x61) = 0;
            var_a3 = var_s4 + 0x40;
            *(u8 *)((u8 *)((((u8 *)D_80123FB0) + var_s2)) + 0x62) = 0;
            do
            {
                if (*(u8 *)((u8 *)((D_80122B74 + var_a3)) + 0x640) != 0)
                {
                    var_s1_2 = *(u32 *)((u8 *)((var_s4 + D_80122B74 + var_a1)) + 0x658);
                    var_s0_3 = 0;
                    do
                    {
                        temp_v1_3 = ((u8 *)D_80123FB0) + (var_s0_3 + var_s2);
                        temp_v0_4 = *(u8 *)((u8 *)(temp_v1_3) + 0x6C);
                        var_s0_3 += 1;
                        *(u8 *)((u8 *)(temp_v1_3) + 0x6C) = (s8) (temp_v0_4 + (var_s1_2 & 0xF));
                        var_s1_2 = var_s1_2 >> 4;
                    } while (var_s0_3 < 8);
                    temp_a0_6 = ((u8 *)D_80123FB0) + var_s2;
                    *(u8 *)((u8 *)(temp_a0_6) + 0x60) = (u8) (*(u8 *)((u8 *)(temp_a0_6) + 0x60) | *(u8 *)((u8 *)((var_s4 + D_80122B74 + var_a1)) + 0x66C));
                    temp_a0_7 = ((u8 *)D_80123FB0) + var_s2;
                    *(u8 *)((u8 *)(temp_a0_7) + 0x62) = (u8) (*(u8 *)((u8 *)(temp_a0_7) + 0x62) | *(u8 *)((u8 *)((var_s4 + D_80122B74 + var_a1)) + 0x66D));
                }
                var_a1 += 0x40;
                var_t1_2 += 1;
                var_a3 += 0x40;
            } while (var_t1_2 < 4);
            temp_v1_4 = ((u8 *)D_80123FB0) + var_s2;
            *(u32 *)((u8 *)(temp_v1_4) + 0x3C) = 0;
            *(u8 *)((u8 *)(temp_v1_4) + 0x74) = (u8) *(u8 *)((u8 *)((D_80122B74 + var_s4)) + 0x633);
            if (D_8010D020 != 0)
            {
                temp_a1_4 = D_80122B74 + var_s4;
                temp_a2_2 = ((u8 *)D_80123FB0) + var_s2;
                *(s32 *)(*(u8 **)(temp_a2_2 + 0x38)) = (s32) (*(u16 *)((u8 *)(temp_a1_4) + 0x614) * 3);
                *(s32 *)(*(u8 **)(temp_a2_2 + 0x38) + 4) = (s32) (*(u16 *)((u8 *)(temp_a1_4) + 0x614) * 3);
                temp_a3 = *(u8 * *)((u8 *)(temp_a2_2) + 0x38);
                *(s32 *)((u8 *)(temp_a3) + 0x8) = (s32) ((*(s32 *)((u8 *)(temp_a3) + 0x8) & 0xFF000000) | (*(u16 *)((u8 *)(temp_a1_4) + 0x614) * 3));
                temp_a0_8 = *(u8 * *)((u8 *)(temp_a2_2) + 0x38);
                *(s32 *)((u8 *)(temp_a0_8) + 0x8) = (s32) (*(s32 *)((u8 *)(temp_a0_8) + 0x8) & 0x80FFFFFF);
            }
            else
            {
                *(s32 *)(*(u8 **)(((u8 *)D_80123FB0) + var_s2 + 0x38)) = (s32) *(u16 *)((u8 *)((D_80122B74 + var_s4)) + 0x614);
            }
            temp_a1_5 = ((u8 *)D_80123FB0) + var_s2;
            var_a0 = *(u8 *)((u8 *)(temp_a1_5) + 0x53) * 2;
            if ((((u32) *(u32 *)((u8 *)((D_80122B74 + var_s4)) + 0x654) >> 0xA) & 0x3F) == 7)
            {
                var_a0 += 0x80;
            }
            temp_v1_5 = *(u8 * *)((u8 *)(temp_a1_5) + 0x38);
            if (var_a0 < 0x100U)
            {
                *(u16 *)((u8 *)(temp_v1_5) + 0x68) = var_a0;
            }
            else
            {
                *(u16 *)((u8 *)(temp_v1_5) + 0x68) = 0xFFU;
            }
            if ((*(u8 *)((u8 *)((D_80122B74 + var_s4)) + 0x608) & 0x7F) == 3)
            {
                func_800B3D84();
            }
            sp10 += 1;
        }
        var_s4 += 0x250;
        var_s2 += 0x68;
        var_fp += 0x68;
        var_s7 += 1;
        sp14 += 0x250;
    } while (var_s7 < 3);
    return sp10;
}


extern u8 *D_80122B74;
/* No prototype is in scope at the original call site, so the arguments pass as
 * plain int (the target does not truncate the field to s16). */
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);


/**
 * @brief Restarts field music when the active scene index is out of range.
 *
 * Reads the active scene index at offset 0x2EF0 of the D_80122B74 buffer; if it
 * is 5 or greater it re-arms akao_set_song_params, then forwards the scene
 * entry's 0x2F3C word (stride 0x60) to func_800BD520.
 *
 * Matches under GCC 2.8.0. The pre-diagnostic scene index and the index
 * reloaded afterward are distinct value webs; materializing the second
 * index's 0x60-byte offset reproduces the target allocation exactly.
 */
void func_800B3D84(void)
{
    s32 idx1;
    s32 idx2;
    s32 off;

    idx1 = *(s32 *)(D_80122B74 + 0x2EF0);
    if ((u32)idx1 >= 5)
    {
        akao_set_song_params(0x8001, 0x75, idx1, 0);
    }

    idx2 = *(s32 *)(D_80122B74 + 0x2EF0);
    off = idx2 * 0x60;
    func_800BD520(2, 0xF020, *(s32 *)(D_80122B74 + off + 0x2F3C));
}
