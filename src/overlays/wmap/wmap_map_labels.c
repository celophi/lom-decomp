#include "wmap_party_travel.h"
#include "wmap_resource_support.h"
#include "wmap_map_labels.h"
#include "sdk/libgpu.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))

/** @brief Screen position of a map label at a particular map scale. */
typedef struct
{
    s16 x, y;
} WmapPoint;

extern u8 D_800515F4;
extern u8 D_80051A58;
extern s32 D_800D921C;
extern SPRT D_800DBE80;
extern s32 D_800DBE84;
extern s32 D_8011CF80;
extern SPRT D_801391E8;
extern SPRT D_801398D8;
extern u8 D_801398DC;
extern void* D_801398EC;
extern SPRT D_80182DA0;
extern s32 D_80182DA4;
extern s32 D_80182E34;
extern WmapPoint D_800519E4[];
extern SPRT D_80051A08;
extern SPRT D_80051A1C;
extern s32 D_800D0230[];
extern s32 D_800D0254[];
extern s32 D_800D0368;
extern u16 D_800D036C[];
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern u16 D_8011CF78;
extern SPRT D_8011D518;
extern s32 D_801398D0;
extern u16 D_80182DD0;
extern s32 D_80182E04;
extern SPRT D_80182E08;
extern s32 D_801ADAE4;
extern s32 func_8005D528(s32);
extern u8 D_80051A6C;
extern u8 D_800D03EC;
extern u8 D_800D040C;
extern s32 D_801398BC;
extern s32 D_8013B258;
extern s32 D_8013B28C;
extern SPRT D_80051A30;
extern SPRT D_80051A44;
extern s8 D_80182E0C;

void func_80060230(void);

/**
 * @brief Append map label and decorative packets with selection fades.
 */
void func_8005F9BC(void)
{
    void* var_a2;
    void* var_a3;
    s32* temp_a0;
    s32* temp_a0_3;
    s32* temp_a0_4;
    s32* temp_a0_5;
    s32* temp_a1;
    s32* temp_a1_2;
    s32* temp_a1_3;
    s32* temp_a1_4;
    s32* temp_a1_5;
    s32 var_t1;
    void* temp_a0_2;

    if (D_80182E34 != 3)
    {
        temp_a1 = M2C_FIELD(D_801398EC, s32**, 0x33C);
        M2C_FIELD(temp_a1, s32*, 0) = M2C_FIELD(&D_80051A58, s32*, 0);
        M2C_FIELD(temp_a1, s32*, 4) = (s32)M2C_FIELD(&D_80051A58, s32*, 4);
        M2C_FIELD(temp_a1, s32*, 8) = (s32)M2C_FIELD(&D_80051A58, s32*, 8);
        M2C_FIELD(temp_a1, s32*, 0xC) = (s32)M2C_FIELD(&D_80051A58, s32*, 0xC);
        M2C_FIELD(temp_a1, s32*, 0x10) = (s32)M2C_FIELD(&D_80051A58, s32*, 0x10);
        M2C_FIELD(temp_a1, s32*, 0) = (M2C_FIELD(temp_a1, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32*, 0x8C) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFF000000) | ((s32)temp_a1 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x14);
        }
        if (g_wmap_travel_day != D_8011CF80)
        {
            D_8011CF80 = g_wmap_travel_day;
            M2C_FIELD(&D_801391E8, s32*, 0) = (s32)M2C_FIELD(&D_80182DA0, s32*, 0);
            M2C_FIELD(&D_801391E8, s32*, 4) = (s32)M2C_FIELD(&D_80182DA0, s32*, 4);
            M2C_FIELD(&D_801391E8, s32*, 8) = (s32)M2C_FIELD(&D_80182DA0, s32*, 8);
            M2C_FIELD(&D_801391E8, s32*, 0xC) = (s32)M2C_FIELD(&D_80182DA0, s32*, 0xC);
            M2C_FIELD(&D_801391E8, s32*, 0x10) = (s32)M2C_FIELD(&D_80182DA0, s32*, 0x10);
            M2C_FIELD(&D_80182DA0, s32*, 4) = 0;
            M2C_FIELD(&D_800DBE80, s32*, 0) = (s32)M2C_FIELD(&D_801398D8, s32*, 0);
            M2C_FIELD(&D_800DBE80, s32*, 4) = (s32)M2C_FIELD(&D_801398D8, s32*, 4);
            M2C_FIELD(&D_800DBE80, s32*, 8) = (s32)M2C_FIELD(&D_801398D8, s32*, 8);
            M2C_FIELD(&D_800DBE80, s32*, 0xC) = (s32)M2C_FIELD(&D_801398D8, s32*, 0xC);
            M2C_FIELD(&D_800DBE80, s32*, 0x10) = (s32)M2C_FIELD(&D_801398D8, s32*, 0x10);
            M2C_FIELD(&D_801398D8, s32*, 0xC) = 0;
            M2C_FIELD(&D_801391E8, u8*, 7) = (u8)(M2C_FIELD(&D_801391E8, u8*, 7) | 2);
            M2C_FIELD(&D_80182DA0, s32*, 0xC) = (s8)(((g_wmap_travel_day & 1) * 0x30) + 8);
            M2C_FIELD(&D_80182DA0, s8*, 0xD) = (s8)((((s32)g_wmap_travel_day / 2) * 0x38) + 0xE);
            M2C_FIELD(&D_800DBE80, u8*, 7) = (u8)(M2C_FIELD(&D_800DBE80, u8*, 7) | 2);
            M2C_FIELD(&D_801398D8, s8*, 0xD) = (s8)(g_wmap_travel_day << 5);
        }
        var_a2 = &D_80182DA0;
        M2C_FIELD(&D_80182DA0, u8*, 5) = (u8)M2C_FIELD(&D_80182DA0, s32*, 4);
        M2C_FIELD(&D_80182DA0, u8*, 6) = (u8)M2C_FIELD(&D_80182DA0, s32*, 4);
        temp_a1_2 = M2C_FIELD(D_801398EC, s32**, 0x33C);
        M2C_FIELD(temp_a1_2, s32*, 0) = M2C_FIELD(&D_80182DA0, s32*, 0);
        M2C_FIELD(temp_a1_2, s32*, 4) = (s32)M2C_FIELD(&D_80182DA0, s32*, 4);
        M2C_FIELD(temp_a1_2, s32*, 8) = (s32)M2C_FIELD(&D_80182DA0, s32*, 8);
        M2C_FIELD(temp_a1_2, s32*, 0xC) = (s32)M2C_FIELD(&D_80182DA0, s32*, 0xC);
        M2C_FIELD(temp_a1_2, s32*, 0x10) = (s32)M2C_FIELD(&D_80182DA0, s32*, 0x10);
        if ((s8)M2C_FIELD(&D_80182DA0, s32*, 4) >= 0)
        {
            M2C_FIELD(&D_80182DA0, s32*, 4) = (s8)((u8)M2C_FIELD(&D_80182DA0, s32*, 4) + 8);
            M2C_FIELD(temp_a1_2, u8*, 7) = (u8)(M2C_FIELD(temp_a1_2, u8*, 7) | 2);
        }
        temp_a0 = M2C_FIELD(D_801398EC, s32**, 0x33C);
        *temp_a0 = (*temp_a0 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32*, 0x8C) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x14);
        }
        if ((u8)M2C_FIELD(&D_801391E8, s32*, 4) != 0)
        {
            M2C_FIELD(&D_801391E8, u8*, 5) = (u8)M2C_FIELD(&D_801391E8, s32*, 4);
            M2C_FIELD(&D_801391E8, u8*, 6) = (u8)M2C_FIELD(&D_801391E8, s32*, 4);
            var_a2 = D_801398EC;
            temp_a1_3 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            M2C_FIELD(temp_a1_3, s32*, 0) = M2C_FIELD(&D_801391E8, s32*, 0);
            M2C_FIELD(temp_a1_3, s32*, 4) = (s32)M2C_FIELD(&D_801391E8, s32*, 4);
            M2C_FIELD(temp_a1_3, s32*, 8) = (s32)M2C_FIELD(&D_801391E8, s32*, 8);
            M2C_FIELD(temp_a1_3, s32*, 0xC) = (s32)M2C_FIELD(&D_801391E8, s32*, 0xC);
            M2C_FIELD(temp_a1_3, s32*, 0x10) = (s32)M2C_FIELD(&D_801391E8, s32*, 0x10);
            M2C_FIELD(temp_a1_3, s32*, 0) = (M2C_FIELD(temp_a1_3, s32*, 0) & 0xFF000000) | (M2C_FIELD(var_a2, s32*, 0x8C) & 0xFFFFFF);
            D_800DBE84 = M2C_FIELD(&D_801391E8, s32*, 4);
            M2C_FIELD(var_a2, s32*, 0x8C) = (s32)((M2C_FIELD(var_a2, s32*, 0x8C) & 0xFF000000) | ((s32)temp_a1_3 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(var_a2, s32**, 0x33C) = (s32*)(M2C_FIELD(var_a2, s32**, 0x33C) + 0x14);
            }
            M2C_FIELD(&D_801391E8, s32*, 4) = (s8)((u8)M2C_FIELD(&D_801391E8, s32*, 4) - 8);
        }
        func_8006534C(0xBC, 7, (s32)var_a2, &D_801391E8);
        func_80060230();
        temp_a1_4 = M2C_FIELD(D_801398EC, s32**, 0x33C);
        M2C_FIELD(&D_801398DC, s32*, 0) = (s32)D_80182DA4;
        temp_a0_2 = &D_801398DC - 4;
        M2C_FIELD(temp_a0_2, u8*, 7) = (u8)(M2C_FIELD(temp_a0_2, u8*, 7) | 2);
        M2C_FIELD(temp_a1_4, s32*, 0) = M2C_FIELD(&D_801398DC, s32*, -4);
        M2C_FIELD(temp_a1_4, s32*, 4) = (s32)M2C_FIELD(&D_801398DC, s32*, 0);
        M2C_FIELD(temp_a1_4, s32*, 8) = (s32)M2C_FIELD(&D_801398DC, s32*, 4);
        M2C_FIELD(temp_a1_4, s32*, 0xC) = (s32)M2C_FIELD(&D_801398DC, s32*, 8);
        M2C_FIELD(temp_a1_4, s32*, 0x10) = (s32)M2C_FIELD(&D_801398DC, s32*, 0xC);
        temp_a0_3 = M2C_FIELD(D_801398EC, s32**, 0x33C);
        *temp_a0_3 = (*temp_a0_3 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32*, 0x8C) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x14);
        }
        if ((u8)M2C_FIELD(&D_800DBE80, s32*, 4) >= 9U)
        {
            temp_a1_5 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            M2C_FIELD(temp_a1_5, s32*, 0) = M2C_FIELD(&D_800DBE80, s32*, 0);
            M2C_FIELD(temp_a1_5, s32*, 4) = (s32)M2C_FIELD(&D_800DBE80, s32*, 4);
            M2C_FIELD(temp_a1_5, s32*, 8) = (s32)M2C_FIELD(&D_800DBE80, s32*, 8);
            M2C_FIELD(temp_a1_5, s32*, 0xC) = (s32)M2C_FIELD(&D_800DBE80, s32*, 0xC);
            M2C_FIELD(temp_a1_5, s32*, 0x10) = (s32)M2C_FIELD(&D_800DBE80, s32*, 0x10);
            temp_a0_4 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            *temp_a0_4 = (*temp_a0_4 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32*, 0x8C) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x8C) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x14);
            }
        }
        func_8006534C(0x3E, 7, (s32)D_801398EC, (void*)0xFFFFFF);
        var_t1 = 0;
        var_a3 = &D_800515F4;
        do
        {
            temp_a0_5 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            M2C_FIELD(temp_a0_5, s32*, 0) = M2C_FIELD(var_a3, s32*, 0);
            M2C_FIELD(temp_a0_5, s32*, 4) = (s32)M2C_FIELD(var_a3, s32*, 4);
            M2C_FIELD(temp_a0_5, s32*, 8) = (s32)M2C_FIELD(var_a3, s32*, 8);
            M2C_FIELD(temp_a0_5, s32*, 0xC) = (s32)M2C_FIELD(var_a3, s32*, 0xC);
            M2C_FIELD(temp_a0_5, s32*, 0x10) = (s32)M2C_FIELD(var_a3, s32*, 0x10);
            M2C_FIELD(temp_a0_5, s32*, 0x14) = (s32)M2C_FIELD(var_a3, s32*, 0x14);
            M2C_FIELD(temp_a0_5, s32*, 0x18) = (s32)M2C_FIELD(var_a3, s32*, 0x18);
            M2C_FIELD(temp_a0_5, s8*, 3) = 6;
            M2C_FIELD(temp_a0_5, s8*, 7) = 0x32;
            M2C_FIELD(temp_a0_5, s32*, 0) = (M2C_FIELD(temp_a0_5, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x90) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32*, 0x90) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x90) & 0xFF000000) | ((s32)temp_a0_5 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x1C;
                M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x1C);
            }
            var_t1 += 1;
            var_a3 += 0x1C;
        } while (var_t1 < 0x24);
        func_8006534C(0x20, 8, D_800D921C, var_a3);
    }
}

/**
 * @brief Change the map label sprite while preserving its outgoing fade.
 * @param selection Label identifier, or -1 to hide the current label.
 */
void func_8005FF88(s32 selection)
{
    s32 position;
    s32 previous_selection;
    s32 width;
    s32* scale;

    if (selection == 33)
    {
        selection = 24;
    }
    if (D_801398D0 != 0)
    {
        selection = -1;
    }
    previous_selection = D_801ADAE4;
    if (previous_selection != selection && (D_80182E08.r0 == 0 || selection == -1))
    {
        D_80182E04 = previous_selection;
        D_80182E08 = D_8011D518;
        D_801ADAE4 = selection;
        D_80182DD0 = D_8011CF78;
        D_80182E08.code |= 2;
        D_80182E08.clut = (D_800D0368 << 6) | 47;
        if (selection == -1)
        {
            D_8011D518.r0 = 0;
            return;
        }
        position = D_800DCEEC + D_800DCEF0 * 3;
        if (selection >= 64)
        {
            D_8011CF78 = 45;
            D_8011D518 = D_80051A08;
            D_800D0368 = 506;
        }
        else
        {
            D_8011CF78 = 43;
            D_8011D518 = D_80051A1C;
            if (func_8005D528(selection) == 0)
            {
                D_800D0368 = 508;
            }
            else
            {
                D_800D0368 = 506;
            }
            scale = &D_800D0230[position];
            width = *scale * D_800D0254[selection];
            D_8011D518.y0 = D_800519E4[position].y;
            D_8011D518.x0 = D_800519E4[position].x - width / 2;
        }
        selection &= 63;
        selection = (s16)D_800D036C[selection];
        D_8011D518.r0 = 8;
        D_8011D518.u0 = (selection / 14) * 112;
        D_8011D518.v0 = (selection % 14) * 16;
    }
}

/**
 * @brief Append the current and outgoing label sprites and auxiliary labels.
 */
void func_80060230(void)
{
    s32 var_t0;
    u16 var_v0;
    u8 temp_v1;
    void* temp_a0;
    void* temp_a2;
    void* temp_a2_2;
    void* temp_v1_2;
    void* var_a3;

    if (M2C_FIELD(&D_8011D518, u8*, 4) != 0)
    {
        M2C_FIELD(&D_8011D518, u8*, 6) = (u8)M2C_FIELD(&D_8011D518, u8*, 4);
        M2C_FIELD(&D_8011D518, u8*, 5) = (u8)M2C_FIELD(&D_8011D518, u8*, 4);
        temp_a2 = M2C_FIELD(D_801398EC, void**, 0x33C);
        if (!(M2C_FIELD(&D_8011D518, u8*, 4) & 0x80))
        {
            var_v0 = *(u16*)&D_800D0368;
            M2C_FIELD(&D_8011D518, u8*, 4) = (u8)(M2C_FIELD(&D_8011D518, u8*, 4) + 8);
        }
        else
        {
            var_v0 = *(u16*)&D_800D0368 + 1;
        }
        M2C_FIELD(&D_8011D518, s16*, 0xE) = (s16)((var_v0 << 6) | 0x2F);
        M2C_FIELD(temp_a2, s32*, 0) = (s32)M2C_FIELD(&D_8011D518, s32*, 0);
        M2C_FIELD(temp_a2, s32*, 4) = (s32)M2C_FIELD(&D_8011D518, u8*, 4);
        M2C_FIELD(temp_a2, s32*, 8) = (s32)M2C_FIELD(&D_8011D518, s32*, 8);
        M2C_FIELD(temp_a2, s32*, 0xC) = (s32)M2C_FIELD(&D_8011D518, s32*, 0xC);
        M2C_FIELD(temp_a2, s32*, 0x10) = (s32)M2C_FIELD(&D_8011D518, s32*, 0x10);
        M2C_FIELD(temp_a2, u8*, 7) = (u8)(M2C_FIELD(temp_a2, u8*, 7) | 2);
        M2C_FIELD(temp_a2, s32*, 0) = (s32)((M2C_FIELD(temp_a2, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x7C) & 0xFFFFFF));
        M2C_FIELD(D_801398EC, s32*, 0x7C) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x7C) & 0xFF000000) | ((s32)temp_a2 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, void**, 0x33C) = (void*)(M2C_FIELD(D_801398EC, void**, 0x33C) + 0x14);
        }
        func_8006534C(D_8011CF78, 3, temp_a2, D_801398EC);
    }
    temp_v1 = M2C_FIELD(&D_80182E08, u8*, 4);
    if (temp_v1 != 0)
    {
        temp_a2_2 = M2C_FIELD(D_801398EC, void**, 0x33C);
        M2C_FIELD(&D_80182E08, u8*, 4) = (u8)(temp_v1 - 8);
        M2C_FIELD(&D_80182E08, u8*, 6) = temp_v1;
        M2C_FIELD(&D_80182E08, u8*, 5) = temp_v1;
        M2C_FIELD(temp_a2_2, s32*, 0) = (s32)M2C_FIELD(&D_80182E08, s32*, 0);
        M2C_FIELD(temp_a2_2, s32*, 4) = (s32)M2C_FIELD(&D_80182E08, u8*, 4);
        M2C_FIELD(temp_a2_2, s32*, 8) = (s32)M2C_FIELD(&D_80182E08, s32*, 8);
        M2C_FIELD(temp_a2_2, s32*, 0xC) = (s32)M2C_FIELD(&D_80182E08, s32*, 0xC);
        M2C_FIELD(temp_a2_2, s32*, 0x10) = (s32)M2C_FIELD(&D_80182E08, s32*, 0x10);
        if (D_80182E04 != -1)
        {
            M2C_FIELD(temp_a2_2, s32*, 0) = (s32)((M2C_FIELD(temp_a2_2, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x7C) & 0xFFFFFF));
            M2C_FIELD(D_801398EC, s32*, 0x7C) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x7C) & 0xFF000000) | ((s32)temp_a2_2 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(D_801398EC, void**, 0x33C) = (void*)(M2C_FIELD(D_801398EC, void**, 0x33C) + 0x14);
            }
            func_8006534C(D_80182DD0, 3, temp_a2_2, D_801398EC);
        }
    }
    if (D_8013B28C != D_801398BC)
    {
        D_8013B28C = D_801398BC;
    }
    if (D_8013B258 == 0)
    {
        var_t0 = *((D_8013B28C * 4) + (u8*)&D_80051A6C);
        var_a3 = (var_t0 * 6) + (u8*)&D_800D040C;
        do
        {
            temp_v1_2 = (M2C_FIELD(var_a3, u8*, 0) * 8) + (u8*)&D_800D03EC;
            temp_a0 = M2C_FIELD(D_801398EC, void**, 0x33C);
            M2C_FIELD(temp_a0, u8*, 0xC) = (u8)M2C_FIELD(temp_v1_2, u8*, 0);
            M2C_FIELD(temp_a0, s8*, 0xD) = (s8)(M2C_FIELD(temp_v1_2, u8*, 2) - 0x20);
            M2C_FIELD(temp_a0, u16*, 0x10) = (u16)M2C_FIELD(temp_v1_2, u16*, 4);
            M2C_FIELD(temp_a0, u16*, 0x12) = (u16)M2C_FIELD(temp_v1_2, u16*, 6);
            M2C_FIELD(temp_a0, u16*, 8) = (u16)M2C_FIELD(var_a3, u16*, 2);
            M2C_FIELD(temp_a0, s16*, 0xE) = 0x7F2E;
            M2C_FIELD(temp_a0, s32*, 4) = 0x808080;
            M2C_FIELD(temp_a0, s8*, 3) = 4;
            M2C_FIELD(temp_a0, s8*, 7) = 0x64;
            M2C_FIELD(temp_a0, s16*, 0xA) = (s16)M2C_FIELD(var_a3, u8*, 4);
            M2C_FIELD(temp_a0, s32*, 0) = (s32)((M2C_FIELD(temp_a0, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x74) & 0xFFFFFF));
            M2C_FIELD(D_801398EC, s32*, 0x74) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x74) & 0xFF000000) | ((s32)temp_a0 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(D_801398EC, void**, 0x33C) = (void*)(M2C_FIELD(D_801398EC, void**, 0x33C) + 0x14);
            }
            var_t0 += 1;
            var_a3 += 6;
        } while (*(((D_8013B28C + 1) * 4) + (u8*)&D_80051A6C) != var_t0);
        func_8006534C(0xBU, 1, (void*)D_800D921C, var_a3);
    }
}

/**
 * @brief Restore map sprite templates and reset the display selection.
 */
void func_800605B4(void)
{
    D_8011D518 = D_80051A1C;
    D_80182DA0 = D_80051A44;
    D_801391E8 = D_80051A44;
    D_801398D8 = D_80051A30;
    D_800DBE80 = D_80051A30;
    D_801391E8.b0 = 0;
    D_801391E8.g0 = 0;
    D_801391E8.r0 = 0;
    D_800DBE80.b0 = 0;
    D_800DBE80.g0 = 0;
    D_800DBE80.r0 = 0;
    D_80182E0C = 0;
    D_8011D518.r0 = 0;
    D_801ADAE4 = -1;
    D_8011CF80 = -1;
}
