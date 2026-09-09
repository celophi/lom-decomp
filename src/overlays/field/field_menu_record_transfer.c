#include "common.h"

extern u8 g_menuLayoutBuffer[], D_80122A08[], D_800F0E98[];
extern u8 D_80122C02, D_80122C03, D_80122C04, D_80122C0C;
void func_800B2844();

/**
 * @brief Validate a shared equipment record and prepare its display parameters.
 * @note Nonmatching m2c translation; retains byte, halfword, and word views
 * of the packed record, and the extra fourth argument at two dispatch sites.
 */
void func_800C8A2C(void)
{
    u8 *var_a0;
    u8 *var_a0_2;
    u8 *var_a0_3;
    s32 temp_s5;
    s32 temp_v0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_s0;
    s32 var_t0;
    s8 temp_s2;
    s8 var_s3;
    s8 var_v1;
    u16 var_s1;
    u32 temp_a0;
    u32 temp_a1;
    u32 temp_v0;
    u8 var_s4;
    u8 *temp_a2;
    u8 *temp_a2_2;
    u8 *temp_a2_3;
    u8 *temp_v1;
    u8 *temp_v1_2;

    (&D_80122C02)[1] = 0;
    temp_v1 = (D_80122C02 << 6) + D_80122A08;
    var_t0 = 0;
    if (*(u8 *)(temp_v1 + 0x0) == 0)
    {
        (&D_80122C02)[1] = 1;
        return;
    }
    var_a1 = 0;
    var_a0 = g_menuLayoutBuffer;
loop_6:
    if ((*(u8 *)(var_a0 + 0xCE0) == 0) || (*(u32 *)(var_a0 + 0xD18) != *(u32 *)(temp_v1 + 0x38)) || (*(u32 *)(var_a0 + 0xD1C) != *(u32 *)(temp_v1 + 0x3C)))
    {
        var_a1 += 1;
        var_a0 += 0x40;
        if (var_a1 < 0x64)
        {
            goto loop_6;
        }
    }
    else
    {
        var_t0 = 1;
    }
    var_a1_2 = 0;
    temp_a2 = (D_80122C02 << 6) + D_80122A08;
    var_a0_2 = g_menuLayoutBuffer;
loop_11:
    if ((*(u8 *)(var_a0_2 + 0x640) == 0) || (*(u32 *)(var_a0_2 + 0x678) != *(u32 *)(temp_a2 + 0x38)) || (*(u32 *)(var_a0_2 + 0x67C) != *(u32 *)(temp_a2 + 0x3C)))
    {
        var_a1_2 += 1;
        var_a0_2 += 0x40;
        if (var_a1_2 < 8)
        {
            goto loop_11;
        }
    }
    else
    {
        var_t0 = 1;
    }
    var_a1_3 = 0;
    temp_a2_2 = (D_80122C02 << 6) + D_80122A08;
    var_a0_3 = g_menuLayoutBuffer;
loop_16:
    if ((*(u8 *)(var_a0_3 + 0x3160) == 0) || (*(u32 *)(var_a0_3 + 0x3198) != *(u32 *)(temp_a2_2 + 0x38)) || (*(u32 *)(var_a0_3 + 0x319C) != *(u32 *)(temp_a2_2 + 0x3C)))
    {
        var_a1_3 += 1;
        var_a0_3 += 0x40;
        if (var_a1_3 < 4)
        {
            goto loop_16;
        }
    }
    else
    {
        var_t0 = 1;
    }
    if (var_t0 == 0)
    {
        temp_v0 = *(u32 *)((D_80122C02 << 6) + D_80122A08 + 0x14);
        temp_s2 = (temp_v0 >> 8) & 3;
        var_s3 = (temp_v0 >> 0xA) & 0x3F;
        if (temp_s2 == 1)
        {
            var_s3 += 0xB;
        }
    else if (temp_s2 == 2)
    {
            var_s3 += 0x17;
        }
        temp_v1_2 = (D_80122C02 << 6) + D_80122A08;
        temp_s5 = *(u16 *)(temp_v1_2 + 0x16) & 0x3F;
        if (temp_s2 == 0)
        {
            var_s1 = *(u16 *)(temp_v1_2 + 0x24);
        }
    else if (temp_s2 == 1)
    {
            var_s1 = *(u16 *)(temp_v1_2 + 0x24) + *(u16 *)(temp_v1_2 + 0x26) + *(u16 *)(temp_v1_2 + 0x28) + *(u16 *)(temp_v1_2 + 0x2A);
        }
    else
    {
            var_s1 = (u16) temp_v1_2[0x26];
        }
        temp_a2_3 = (D_80122C02 << 6) + D_80122A08;
        temp_a0 = *(u32 *)(temp_a2_3 + 0x18);
        temp_a1 = *(u32 *)(temp_a2_3 + 0x1C);
        var_s4 = ((temp_a0 & 0xF) + ((temp_a0 >> 4) & 0xF) + ((temp_a0 >> 8) & 0xF) + ((temp_a0 >> 0xC) & 0xF) + ((temp_a0 >> 0x10) & 0xF) + ((temp_a0 >> 0x14) & 0xF) + ((temp_a0 >> 0x18) & 0xF) + (temp_a0 >> 0x1C) + (temp_a1 & 0xF) + ((temp_a1 >> 4) & 0xF) + ((temp_a1 >> 8) & 0xF) + ((temp_a1 >> 0xC) & 0xF) + ((temp_a1 >> 0x10) & 0xF) + ((temp_a1 >> 0x14) & 0xF) + ((temp_a1 >> 0x18) & 0xF) + (temp_a1 >> 0x1C)) >= 0x29;
        if (temp_s2 == 2)
        {
            var_s4 = temp_a2_3[0x24];
        }
        var_s0 = *(s32 *)(temp_a2_3 + 0x34);
        func_800B2844(0, temp_a2_3, 0xFF, D_80122C02);
        *(u8 *)((u8 *)&D_80122C04 + 0) = temp_s2;
        temp_v0_2 = temp_s5 * 2;
        *(u8 *)((u8 *)&D_80122C04 + 1) = var_s3;
        func_800B2844(1, *(temp_v0_2 + D_800F0E98) + (*(temp_v0_2 + 1 + D_800F0E98) << 8) + D_800F0E98, 0xFF);
        *(u16 *)((u8 *)&D_80122C04 + 2) = var_s1;
        if (var_s4 != 0)
        {
            *(u16 *)((u8 *)&D_80122C04 + 2) = (u16) (var_s1 - 0x8000);
        }
        *(s32 *)((u8 *)&D_80122C04 + 4) = var_s0;
        var_v1 = 0;
        do

{
            var_s0 /= 0xA;
            var_v1 += 1;
        } while (var_s0 != 0);
        D_80122C0C = var_v1;
        return;
    }
    D_80122C03 = 2;
    func_800B2844(0, (D_80122C02 << 6) + D_80122A08, 0xFF, D_80122C02);
}


void func_800A8F8C(void *, void *);
extern u8 g_menuLayoutBuffer[];

/** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void)
{
    s32 i;
    u8 *rec;
    s32 outer_i;
    s32 src_off;
    u8 *scan;
    u8 *dest_base;
    s32 inner_i;
    s32 off;
    u8 *a1;
    u8 *s0;

    i = 0;
    rec = g_menuLayoutBuffer;
    do

{
        if (rec[0x3160] != 0 && *(s32 *)(rec + 0x3194) == 0)
        {
            rec[0x3160] = 0;
        }
        i += 1;
        rec += 0x40;
    } while (i < 4);

    outer_i = 0;
    scan = g_menuLayoutBuffer;
    dest_base = scan + 0x3160;
    src_off = 0;
    do

{
        if (scan[0x3160] == 0 && *(s32 *)(scan + 0x3194) == 0)
        {
            inner_i = outer_i + 1;
            off = inner_i << 6;
            if (inner_i < 4)
            {
                a1 = off + dest_base;
                s0 = off + g_menuLayoutBuffer;
            loop_10:
                inner_i += 1;
                if (s0[0x3160] != 0)
                {
                    func_800A8F8C(src_off + dest_base, a1);
                    s0[0x3160] = 0;
                    *(s32 *)(s0 + 0x3194) = 0;
                }
                else
                {
                    a1 += 0x40;
                    s0 += 0x40;
                    if (inner_i < 4)
                    {
                        goto loop_10;
                    }
                }
            }
        }
        src_off += 0x40;
        outer_i += 1;
        scan += 0x40;
    } while (outer_i < 4);
}


typedef struct
{
    u8 pad0[0x3160];
    u8 unk3160;   /* 0x3160 */
    u8 pad3161[0x3194 - 0x3161];
    s32 unk3194;  /* 0x3194 */
} BigStruct;

extern u8 D_80122C02;
extern u8 D_80046138[];


extern u8 *func_800A9060(void);
extern /** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void);

/** @brief Copy the selected pending record out and compact the table. */
void func_800C8F4C(void)
{
    s32 idx;
    s32 offset;
    u8 *handle;
    BigStruct *rec;

    idx = D_80122C02;
    handle = func_800A9060();
    offset = idx << 6;
    func_800A8F8C(handle, &D_80046138[offset]);
    rec = (BigStruct *) (D_80046138 - 0x3160 + offset);
    rec->unk3160 = 0;
    rec->unk3194 = 0;
    func_800C8E2C();
}


extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C02;
extern s32 D_80122C08;

extern /** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void);

/** @brief Restore the first pending result record and expose its result. */
void func_800C8FA8(void)
{
    s32 i;
    s32 result;
    u8 *base;
    u8 *arg;
    u8 *p;

    result = 0;
    D_80122C02 = 0xFF;
    i = 0;
    p = g_menuLayoutBuffer;
    arg = p + 0x3160;
    base = p;

loop:
    if (base[0x3160] == 0 && *(s32 *)&base[0x3194] != 0)
    {
        base[0x3160] = base[0x3180];
        func_800B2844(0, arg, 0xFF);
        result = *(s32 *)&base[0x3194];
        *(s32 *)&base[0x3194] = 0;
        D_80122C02 = i;
    }
    else
    {
        arg += 0x40;
        i++;
        base += 0x40;
        if (i < 4)
        {
            goto loop;
        }
    }

    D_80122C08 = result;
    if (result == 0)
    {
        func_800C8E2C();
    }
}

extern void func_800B2844(s32, u8 *, s32);

/** @brief Layout buffer view for active records, identity words, and packed metadata. */
typedef struct Layout
{
    u8 pad0[0x640];
    u8 unk640;
    u8 pad641[0x37];
    u32 unk678, unk67C;
    u8 pad680[0x660];
    u8 unkCE0;
    u8 padCE1[0x37];
    u32 unkD18, unkD1C;
    u8 padD20[0x2440];
    u8 unk3160;
    u8 pad3161[0x13];
    union

{
        u32 word;
        u16 halves[2];
    } packed;
    u8 pad3178[0x20];
    u32 unk3198, unk319C;
} Layout;
extern u8 D_80046138[], D_800F0E98[], g_menuLayoutBuffer[];
extern u8 D_80122C02, D_80122C03, D_80122C04, D_80122C06;
/**
 * @brief Count available records, detect duplicate identities, and prepare the selected display.
 * @note Status is 1 for an empty selection and 2 for an identity found in either table.
 */
void func_800C905C(void)
{

    s32 var_v1;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_v0;
    s32 temp_v0_3;
    s32 var_a1;
    s32 var_a3;
    s32 temp_s2;
    s32 var_a0;
    s32 var_s1;
    u32 temp_v0_2;
    u8 *temp_a2;
    u8 *temp_v1;

    s32 selected;
    selected = D_80122C02;
    var_a0 = 0;
    var_a1 = var_a0;
    var_v1 = (s32)g_menuLayoutBuffer + var_a1 * 0x40;
    do

{
        if (((Layout *)var_v1)->unk3160 != 0)
        {
            var_a0 += 1;
        }
        var_a1 += 1;
        var_v1 = (s32)g_menuLayoutBuffer + var_a1 * 0x40;
    } while (var_a1 < 4);
    var_a1 = (s32)&D_80122C06;
    *(u8 *)var_a1 = var_a0;
    var_a0 = (s32)g_menuLayoutBuffer;
    temp_v1 = (selected << 6) + g_menuLayoutBuffer;
    ((u8 *)var_a1)[-3] = 0;
    var_a3 = 0;
    if (((Layout *)temp_v1)->unk3160 == 0)
    {
        ((u8 *)var_a1)[-3] = 1;
        return;
    }
    goto search_first;
found_first:
    var_a3 = 1;
    goto search_second;
found_second:
    var_a3 = 1;
    goto searches_done;
search_first:
    var_a1 = var_a3;
    temp_a2 = temp_v1;
first_loop:
    if (((Layout *)var_a0)->unkCE0 != 0 &&
        ((Layout *)var_a0)->unkD18 == ((Layout *)temp_a2)->unk3198 &&
        ((Layout *)var_a0)->unkD1C == ((Layout *)temp_a2)->unk319C)

{
        goto found_first;
    }
    var_a1++;
    var_a0 += 0x40;
    if (var_a1 < 100)
    {
        goto first_loop;
    }
search_second:
    var_a1 = 0;
    var_v1 = (s32)g_menuLayoutBuffer;
    temp_a2 = (u8 *)((selected << 6) + var_v1);
    var_a0 = var_v1;
second_loop:
    if (((Layout *)var_a0)->unk640 != 0 &&
        ((Layout *)var_a0)->unk678 == ((Layout *)temp_a2)->unk3198 &&
        ((Layout *)var_a0)->unk67C == ((Layout *)temp_a2)->unk319C)

{
        goto found_second;
    }
    var_a1++;
    var_a0 += 0x40;
    if (var_a1 < 8)
    {
        goto second_loop;
    }
searches_done:
    temp_v0 = selected << 6;
    if (var_a3 == 0)
    {
        var_v1 = (s32)g_menuLayoutBuffer;
        temp_v0_2 = ((Layout *)(temp_v0 + var_v1))->packed.word;
        temp_s2 = (temp_v0_2 >> 8) & 3;
        var_s1 = (temp_v0_2 >> 0xA) & 0x3F;
        if (temp_s2 == 1)
        {
            var_s1 += 0xB;
        }
        else if (temp_s2 == 2)
        {
            var_s1 += 0x17;
        }
        var_v1 = (s32)g_menuLayoutBuffer;
        temp_v0_3 = selected << 6;
        temp_s0 = ((Layout *)(temp_v0_3 + var_v1))->packed.halves[1] & 0x3F;
        func_800B2844(0, temp_v0_3 + ((u8 *)var_v1 + 0x3160), 0xFF);
        D_80122C04 = temp_s2;
        temp_s0_2 = temp_s0 * 2;
        (&D_80122C04)[1] = var_s1;
        func_800B2844(1, D_800F0E98[temp_s0_2] + (D_800F0E98[temp_s0_2 + 1] << 8) + D_800F0E98,
                      0xFF);
        return;
    }
    func_800B2844(0, temp_v0 + D_80046138, 0xFF);
    D_80122C03 = 2;
}
