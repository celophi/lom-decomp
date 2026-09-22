#include "wmap_land_selection.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "cdrom.h"
#include "wmap_view_effects.h"
#include "wmap_main.h"
#include "wmap_effect_backdrop.h"
#include "wmap_map_labels.h"
#include "wmap_effect_resources.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/libgpu.h"

/** @brief Clear the candidate list and search for an available map entry. */
void func_8005909C(void)
{
/** @brief Palette selection with its stored padding. */
typedef struct
{
    u16 value;
    u16 pad;
} WmapPaletteEntry;

/** @brief Active palette state; only the second halfword changes here. */
typedef struct
{
    u16 field_00;
    u16 value;
} WmapPaletteState;

extern WmapPaletteEntry D_80051198[];
extern WmapPaletteState D_800CC774;
extern s32 D_800CC130;
extern s32 D_80139270;
extern s32 D_80139838[];
extern s32 func_8005CC50(s32);
extern s32 func_8005D8FC(void);
extern void func_8005CA3C(s32, s32 *);

    s32 i;

    D_80139270 = func_8005CC50(D_800CC130 / 4);
    i = 0;
    do
    {
        D_80139838[i] = -1;
        i++;
    } while ((u32)i < 12U);
    i = 12;
    do
    {
        if (func_8005D8FC() != -1)
        {
            break;
        }
        D_800CC130 += 4;
        func_8005CA3C(1, D_80139838);
        if (D_800CC130 < 0)
        {
            D_800CC130 = 47;
        }
        else if (D_800CC130 >= 48)
        {
            D_800CC130 = 0;
        }
        i--;
        D_800CC774.value = D_80051198[D_800CC130].value;
    } while (i != -1);
}

/** @brief Upload the selected effect resources and register its callback. */
void func_800591A8(u32 selection)
{
typedef struct
{
    u8 pad0[2];
    s16 resource_id;
    u8 pad4;
    u8 state;
    u8 pad6[0xA];
    s16 slot;
    u8 pad12[2];
    s32 busy;
    u8 pad18[0x14];
} WmapResource;

extern WmapResource D_80182248[];
extern WmapResource D_80182508;
extern s32 D_801ADAF8;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 func_80055BB0(WmapResource *);
extern s32 func_800706B0(s32);
extern s32 func_800712C0(s32);
extern s32 func_8007369C(s32);
extern s32 func_80074FB4(s32);
extern s32 func_80077560(s32);
extern s32 func_8007998C(s32);
extern s32 func_8007B910(s32);
extern s32 func_8007DC14(s32);
extern s32 func_8007F398(s32);
extern s32 func_80081BA0(s32);
extern s32 func_8008437C(s32);
extern s32 func_80085A70(s32);
extern s32 func_8008759C(s32);
extern s32 func_80089134(s32);
extern s32 func_8008B25C(s32);
extern s32 func_8008D4B4(s32);
extern s32 func_8008FD1C(s32);
extern s32 func_80091C0C(s32);
extern s32 func_80093E98(s32);
extern s32 func_80095F34(s32);
extern s32 func_80097C50(s32);
extern s32 func_8009B8E4(s32);
extern s32 func_8009E250(s32);
extern s32 func_800A1604(s32);
extern s32 func_800A3CC8(s32);
extern s32 func_800AEAB8(s32);
extern s32 func_800B7290(s32);
extern s32 func_800BAE24(s32);
extern s32 func_800BD83C(s32);


    D_801ADAF8 = 0;
    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    if (selection != 2)
    {
        func_800651B4(D_80193640);
    }
    if (selection == 0x16)
    {
        func_80055BB0(&D_80182508);
    }
    else
    {
        func_80055BB0(&D_80182248[selection]);
    }
    switch (selection)
    {
    case 33:
        func_8006CAC0(func_800BD83C);
        break;
    case 24:
        func_8006CAC0(func_800BAE24);
        break;
    case 31:
        func_8006CAC0(func_800B7290);
        break;
    case 25:
        func_8006CAC0(func_800AEAB8);
        break;
    case 22:
        func_8006CAC0(func_800A1604);
        break;
    case 19:
        func_8006CAC0(func_800A3CC8);
        break;
    case 23:
        func_8006CAC0(func_8009E250);
        break;
    case 27:
        func_8006CAC0(func_8009B8E4);
        break;
    case 16:
        func_8006CAC0(func_80097C50);
        break;
    case 0:
        func_8006CAC0(func_80091C0C);
        break;
    case 9:
        func_8006CAC0(func_80095F34);
        break;
    case 8:
        func_8006CAC0(func_80093E98);
        break;
    case 10:
        func_8006CAC0(func_8008FD1C);
        break;
    case 5:
        func_8006CAC0(func_8008D4B4);
        break;
    case 17:
        func_8006CAC0(func_8008B25C);
        break;
    case 21:
        func_8006CAC0(func_80089134);
        break;
    case 3:
        func_8006CAC0(func_8008759C);
        break;
    case 11:
        func_8006CAC0(func_80085A70);
        break;
    case 12:
        func_8006CAC0(func_8008437C);
        break;
    case 30:
        func_8006CAC0(func_80081BA0);
        break;
    case 7:
        func_8006CAC0(func_8007F398);
        break;
    case 2:
        func_8006CAC0(func_8007DC14);
        break;
    case 15:
        func_8006CAC0(func_8007B910);
        break;
    case 32:
        func_8006CAC0(func_8007998C);
        break;
    case 26:
        func_8006CAC0(func_80077560);
        break;
    case 1:
        func_8006CAC0(func_8007369C);
        break;
    case 18:
        func_8006CAC0(func_800706B0);
        break;
    case 4:
        func_8006CAC0(func_800712C0);
        break;
    case 13:
        func_8006CAC0(func_80074FB4);
        break;
    default:
        func_8006CAC0(func_8006D244);
        break;
    }
}

/** @brief Start the world-map transition when idle.
 * @return Zero when started, or one while another transition is active.
 */
s32 func_800593D4(void)

{
/* Partial WMAP decompilation: 82.769230% (gcc280_g0). */

/** @brief Signed map-screen coordinate pair. */
typedef struct
{
    s16 x;
    s16 y;
} WmapPoint;

extern s32 func_8005D4A4(void);
extern WmapPoint D_80054944[];
extern s16 D_800D928A;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF44;
extern s32 D_80139244;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern s32 D_8013B208;
extern s32 D_8013B254;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182E34;
extern s32 D_801ADAF4;

    WmapPoint *point;

    if (D_8011CF44 != 0)
    {
        return 1;
    }
    func_80064F64(func_8005D4A4() + 0x10CE);
    func_8006683C(0x808080);
    D_800DBE70 = 2;
    D_8013B254 = 1;
    D_800DBE78 = 2;
    D_80139244 = 0;
    D_80182E34 = 2;
    D_801ADAF4 = 0x10;
    func_8006D870(0);
    D_801398D0 = 2;
    D_8013986C = 0;
    D_8013B208 = 0;
    D_800D928A = 0x80;
    point = &D_80054944[D_800DCEF0 * 3 + D_800DCEEC];
    D_80182D68 = (s32) -point->x;
    D_80182D78 = (s32) -point->y;
    func_80064094();
    return 0;
}

/** @brief Update world-map selection and route state. */
void func_800594D8(void)
{
typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjectionState;

typedef struct
{
    u16 value;
    u16 pad;
} WmapPaletteEntry;

typedef struct
{
    u8 pad_00[4];
    s16 enabled;
    u8 pad_06[0x22];
} WmapCell;

typedef struct
{
    u8 pad_00[2];
    s16 state;
    u8 pad_04[0x18];
} WmapRouteCell;

extern WmapPaletteEntry D_80051198[];
extern s32 D_8005135C[];
extern s32 D_800CC128;
extern s32 D_800CC12C;
extern s32 D_800CC130;
extern u16 D_800CC776;
extern s32 D_800CCBF4[];
extern s32 D_800D9168;
extern s32 D_800D9218;
extern s32 D_800D9220;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCF0C;
extern s32 D_8011CF18;
extern s32 D_8011CF50;
extern WmapRouteCell D_8011D108[6][6];
extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D52C;
extern s32 D_8011D530;
extern s32 D_80129550;
extern s32 D_8013922C;
extern s32 D_80139230;
extern WmapCell D_80139290[6][6];
extern s32 D_80139838[12];
extern s32 D_801398C0;
extern s32 D_801398F4;
extern WmapProjectionState D_80139950;
extern s32 D_8013B208;
extern s32 D_8013B230;
extern s32 D_80182DDC;
extern s32 D_80182DE0;
extern s32 D_80182E24;

s32 akao_cmd_c2(s32 value0, s32 value1, s32 value2, s32 value3);
void func_800591A8(s32 value);
s16 func_8005B8C8(s32 x, s32 y, s32 selection);
void func_8005BBC8(s32 x, s32 y, s32 selection);
void func_8005CA3C(s32 direction, s32* values);
s32 func_8005D8FC();
void func_800593D4(void);

    s32 map_x;
    s32 map_y;
    s32 table_index;
    map_x = D_80139950.x / 48 + D_800DCEEC;
    map_y = D_80139950.y / 48 + D_800DCEF0;

    if (D_800CC128 != 0)
    {
        s32 palette_index;

        palette_index = D_800CC130 + D_800CC12C;
        D_800CC130 = palette_index;
        if (palette_index < 0)
        {
            D_800CC130 = 47;
        }
        else if (palette_index >= 48)
        {
            D_800CC130 = 0;
        }

        D_800CC128--;
        D_800CC776 = D_80051198[D_800CC130].value;
        if (D_800CC128 == 0)
        {
            D_8011CF50 = 0;
            D_8013922C = 0;
        }
    }
    else if (D_8011CF18 == 0)
    {
        s32 input_mask;
        s32 invalid;

        invalid = -1;
        input_mask = 0x80;
        if (D_8011D4FC != invalid)
        {
            input_mask = 0x20;
        }

        if ((D_8013922C & input_mask) != 0)
        {
            if (D_80182DE0 == 0 && D_80139230 == 0)
            {
                s32 index;
                s32 data;

                D_8011CF18 = 3;
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
                func_8005FF88(-1);

                index = D_800DCEEC + D_800DCEF0 * 3;
                data = D_8005135C[index];
                D_80182DDC = data;
                D_800D9218 = D_8005135C[index + 1] - 1;
                func_8006D8F0(0, data, D_8005135C);
                func_8006D870(1);
                D_80129550 = 0;
                D_800D9168 = invalid;
                func_800652A8(4, 0x80);
            }
        }

        if (D_80129550 == 1 && (D_8013922C & 0x40) != 0 && D_8011D4FC != -1 && D_80139290[map_x][map_y].enabled != 0 && D_80182DE0 == 0)
        {
            D_80182DE0 = D_80129550;
            func_800652A8(0x16, 0x80);
            D_801398C0 = 0;
            D_8013922C = 0;
            D_800D9220 = 0x20;
            D_801398C0 = 0;
            D_8013922C = 0;
        }

        if ((D_8013922C & 0x20) != 0)
        {
            D_80182DE0 = 0;
            D_800D9220 = -1;
            cdrom_wait_queue_empty();
        }

        if (D_80139290[map_x][map_y].enabled != 0 && D_80182DE0 != 0)
        {
            if (D_80182DE0 < 0x1F)
            {
                D_80182DE0++;
            }

            if (D_80182DE0 == 0x1E)
            {
                s32 selection;

                D_8011D510 = map_x;
                D_8011D530 = map_y;
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8011D52C = 1;
                D_8013922C = 0;
                D_8013B208 = 1;
                akao_cmd_c2(0, 0x3C, 0x7F, 1);
                func_800591A8(D_8011D4FC);
                selection = D_8011D4FC;
                if (selection == 0x16)
                {
                    func_8005BBC8(D_8011D510, D_8011D530, 0x10);
                }
                else
                {
                    func_8005BBC8(D_8011D510, D_8011D530, selection);
                }
                func_8006CBD8(func_800593D4);
            }
        }
    }
    else
    {
        do
        {
        if ((D_8013922C & 0xA0) != 0)
        {
            s32 index;

            func_8005FF88(-1);
            D_8011CF18 = 4;
            D_8011CF50 = 1;
            index = D_800DCEEC + D_800DCEF0 * 3;
            D_80182DDC = D_8005135C[index + 1] - 1;
            D_800D9218 = D_8005135C[index];
            func_8006D870(0);
            D_8011D4FC = -1;
            D_80182E24 = 0;
            func_800652A8(0x18, 0x80);
            D_8011CF50 = 1;
            D_801398C0 = 0;
            D_8013922C = 0;
        }

        if ((D_8013922C & 0x2000) != 0)
        {
            D_8011CF50 = 1;
            D_801398C0 = 0;
            D_8013922C = 0;
            func_800652A8(8, 0x8F);
            D_800CC12C = 1;
            D_800CC128 = 4;
            func_8005CA3C(1, D_80139838);
        }

        if ((D_8013922C & 0x8000) != 0)
        {
            D_8011CF50 = 1;
            D_801398C0 = 0;
            D_8013922C = 0;
            func_800652A8(9, 0x8F);
            D_800CC12C = -1;
            D_800CC128 = 4;
            func_8005CA3C(0, D_80139838);
        }

        if ((D_8013922C & 0x40) != 0)
        {
            s32 selected;

            selected = func_8005D8FC();
            D_8011D4FC = selected;
            D_8013B230 = 0;
            if (selected != -1)
            {
                D_80129550 = 1;
                for (map_y = 0; map_y < 6; map_y++)
                {
                    for (map_x = 0; map_x < 6; map_x++)
                    {
                        ((volatile WmapCell*)&D_80139290[map_x][map_y])->enabled = func_8005B8C8(map_x, map_y, D_8011D4FC);
                        D_8011D108[map_x][map_y].state = 0;
                    }
                }

                D_8011CF18 = 4;
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
                D_800D9168 = D_800CCBF4[D_8011D4FC];
                D_800DCF0C = D_800CCBF4[D_8011D4FC + 1] - 1;
                table_index = D_800DCEEC + D_800DCEF0 * 3;
                D_80182DDC = D_8005135C[table_index + 1] - 1;
                D_800D9218 = D_8005135C[table_index];
                func_8006D870(0);
                func_800A89DC(D_8011D4FC);
            }
        }

        if (D_8011CF18 == 2)
        {
            s32* entries;
            s32* entry;
            s32 value;

            entries = D_80139838;
            entry = &entries[(D_801398F4 + 0x7FFF) % 12];
            if (*entry != 0xFF)
            {
                value = func_8005D8FC(entry);
                if (value != -1)
                {
                    value += 0x40;
                }
                func_8005FF88(value);
            }
        }
        } while (0);
    }
}

void func_80059C78(void)
{
/* Partial WMAP decompilation: 71.188680% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern u16 D_80050170;
extern u8 D_80050AA0;
extern u8 D_80050DA8;
extern s32 D_800CC130;
extern u8 D_800CC134;
extern u8 D_800CC164;
extern u8 D_800CC764;
extern u8 D_800CC774;
extern s32 D_800D921C;
extern s32 D_800DBE78;
extern u8 D_80139838;
extern void *D_801398EC;
extern s32 D_801ADAFC;

    SVECTOR position;
    u32 sp3C;
    s32 sp38;
    MATRIX sp10;
    s32 *temp_a1;
    s32 *temp_a1_2;
    s32 *var_a2;
    s32 temp_a0;
    s32 var_a3;
    s32 var_s0;
    s32 var_t2;
    s32 var_t5;
    s32 var_t6;
    s32 var_t7;
    s32 var_t8;
    s32 var_t9;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    u16 *var_t4;
    void *temp_v0;

    if (D_800DBE78 != 3)
    {
        if (D_800DBE78 == 2)
        {
            if (M2C_FIELD(&D_800CC764, s32 *, 0) >= 0x65)
            {
                M2C_FIELD(&D_800CC764, s32 *, 0) = (s32) (M2C_FIELD(&D_800CC764, s32 *, 0) - 6);
                var_v0 = M2C_FIELD(&D_800CC764, s32 *, 4) - 6;
                goto block_8;
            }
            D_800DBE78 = 0;
            goto block_9;
        }
        if (D_800DBE78 == 1)
        {
            if (M2C_FIELD(&D_800CC764, s32 *, 0) < 0x94)
            {
                M2C_FIELD(&D_800CC764, s32 *, 0) = (s32) (M2C_FIELD(&D_800CC764, s32 *, 0) + 6);
                var_v0 = M2C_FIELD(&D_800CC764, s32 *, 4) + 6;
block_8:
                M2C_FIELD(&D_800CC764, s32 *, 4) = var_v0;
block_9:
                goto block_10;
            }
        }
        else
        {
block_10:
            TransMatrix(&sp10, &D_800CC764);
            RotMatrix(&D_800CC774, &sp10);
            SetRotMatrix(&sp10);
            SetTransMatrix(&sp10);
            var_a3 = 0;
            var_s0 = 0xC;
            var_t9 = 0xA;
            var_t8 = 8;
            var_t7 = 6;
            var_t6 = 4;
            var_t5 = 2;
            var_t4 = &D_80050170;
            do
            {
                var_a2 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                gte_ldv3((u8 *)&D_80050AA0 + M2C_FIELD(&D_80050DA8, s16 *, var_a3 * 6) * 8,
                    (u8 *)&D_80050AA0 + M2C_FIELD(&D_80050DA8, s16 *, var_a3 * 6 + 2) * 8,
                    (u8 *)&D_80050AA0 + M2C_FIELD(&D_80050DA8, s16 *, var_a3 * 6 + 4) * 8);
                gte_rtpt();
                M2C_FIELD(var_a2, u16 *, 0xE) = (u16) *var_t4;
                M2C_FIELD(var_a2, u8 *, 0xD) = M2C_FIELD(&D_80050170, u8 *, var_t6);
                M2C_FIELD(var_a2, u8 *, 0xC) = M2C_FIELD(&D_80050170, u8 *, var_t5);
                M2C_FIELD(var_a2, s32 *, 4) = 0x24808080;
                gte_stsxy3((u8 *)var_a2 + 8, (u8 *)var_a2 + 16, (u8 *)var_a2 + 24);
                gte_nclip();
                M2C_FIELD(var_a2, u8 *, 0x14) = M2C_FIELD(&D_80050170, u8 *, var_t7);
                M2C_FIELD(var_a2, u8 *, 0x15) = M2C_FIELD(&D_80050170, u8 *, var_t8);
                gte_stopz(&sp38);
                if (sp38 > 0)
                {
                    M2C_FIELD(var_a2, s32 *, 0) = 0x07000000;
                    M2C_FIELD(var_a2, s16 *, 0x16) = 0x1B;
                    M2C_FIELD(var_a2, u8 *, 0x1C) = M2C_FIELD(&D_80050170, u8 *, var_t9);
                    M2C_FIELD(var_a2, u8 *, 0x1D) = M2C_FIELD(&D_80050170, u8 *, var_s0);
                    M2C_FIELD(var_a2, s32 *, 0) = (M2C_FIELD(var_a2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0xE8) & 0xFFFFFF);
                    M2C_FIELD(D_801398EC, s32 *, 0xE8) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0xE8) & 0xFF000000) | ((s32) var_a2 & 0xFFFFFF));
                    if (D_800D921C < 0x7D00)
                    {
                        D_800D921C += 0x20;
                        M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) ((u8 *)M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x20);
                    }
                }
                var_s0 += 0xE;
                var_t9 += 0xE;
                var_t8 += 0xE;
                var_t7 += 0xE;
                var_t6 += 0xE;
                var_t5 += 0xE;
                var_t4 = (u16 *)((u8 *)var_t4 + 0xE);
                var_a3 += 1;
            } while (var_a3 < 0xA8);
            var_v0_2 = D_800CC130;
            if (var_v0_2 < 0)
            {
                var_v0_2 += 3;
            }
            var_t2 = (var_v0_2 >> 2) + 0xB;
loop_18:
            var_v0_3 = D_800CC130;
            if (var_v0_3 < 0)
            {
                var_v0_3 += 3;
            }
            if (((var_v0_3 >> 2) - 1) < var_t2)
            {
                var_a3 = (var_t2 + 0xC) % 12;
                temp_a0 = var_a3 * 4;
                var_a2 = temp_a0 + (u8 *)&D_80139838;
                if (*var_a2 != -1)
                {
                    temp_v0 = temp_a0 + (u8 *)&D_800CC134;
                    temp_a1 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                    position.vx = M2C_FIELD(temp_v0, u16 *, 0);
                    position.vy = 0;
                    position.vz = M2C_FIELD(temp_v0, u16 *, 2);
                    gte_ldv0(&position);
    gte_rtps();
                    M2C_FIELD(temp_a1, s32 *, 4) = 0x80808080;
                    var_a2 = (*var_a2 * 0x18) + (u8 *)&D_800CC164;
                    gte_stsxy(&sp3C);
                    M2C_FIELD(temp_a1, s16 *, 8) = (s16) (sp3C + M2C_FIELD(var_a2, u16 *, 8));
                    M2C_FIELD(temp_a1, s16 *, 0xA) = (s16) (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xA));
                    M2C_FIELD(temp_a1, u16 *, 0x10) = (u16) M2C_FIELD(var_a2, u16 *, 4);
                    M2C_FIELD(temp_a1, u16 *, 0x12) = (u16) M2C_FIELD(var_a2, u16 *, 6);
                    M2C_FIELD(temp_a1, u8 *, 0xC) = (u8) M2C_FIELD(var_a2, u8 *, 0);
                    M2C_FIELD(temp_a1, s16 *, 0xE) = 0x7FEC;
                    M2C_FIELD(temp_a1, s8 *, 3) = 4;
                    M2C_FIELD(temp_a1, s8 *, 7) = 0x66;
                    M2C_FIELD(temp_a1, u8 *, 0xD) = (u8) M2C_FIELD(var_a2, u8 *, 2);
                    var_a3 = D_800D921C;
                    M2C_FIELD(temp_a1, s32 *, 0) = (M2C_FIELD(temp_a1, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x9C) & 0xFFFFFF);
                    M2C_FIELD(D_801398EC, s32 *, 0x9C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x9C) & 0xFF000000) | ((s32) temp_a1 & 0xFFFFFF));
                    if (var_a3 < 0x7D00)
                    {
                        D_800D921C = var_a3 + 0x14;
                        M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) ((u8 *)M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x14);
                    }
                    temp_a1_2 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                    M2C_FIELD(temp_a1_2, s32 *, 4) = 0x404040;
                    M2C_FIELD(temp_a1_2, s16 *, 8) = (s16) (sp3C + M2C_FIELD(var_a2, u16 *, 0xC) + 0xA);
                    M2C_FIELD(temp_a1_2, s16 *, 0xA) = (s16) (M2C_FIELD(var_a2, u16 *, 6) + (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE)));
                    M2C_FIELD(temp_a1_2, s16 *, 0x10) = (s16) (M2C_FIELD(var_a2, u16 *, 4) + (sp3C + M2C_FIELD(var_a2, u16 *, 0xC)) + 0xA);
                    M2C_FIELD(temp_a1_2, s16 *, 0x12) = (s16) (M2C_FIELD(var_a2, u16 *, 6) + (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE)));
                    M2C_FIELD(temp_a1_2, s16 *, 0x18) = (s16) (sp3C + M2C_FIELD(var_a2, u16 *, 0xC));
                    M2C_FIELD(temp_a1_2, s16 *, 0x1A) = (s16) (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE));
                    M2C_FIELD(temp_a1_2, s16 *, 0x20) = (s16) (M2C_FIELD(var_a2, u16 *, 4) + (sp3C + M2C_FIELD(var_a2, u16 *, 0xC)));
                    M2C_FIELD(temp_a1_2, s16 *, 0x22) = (s16) (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE));
                    M2C_FIELD(temp_a1_2, u8 *, 0xC) = (u8) M2C_FIELD(var_a2, u8 *, 0);
                    M2C_FIELD(temp_a1_2, u8 *, 0xD) = (u8) M2C_FIELD(var_a2, u8 *, 2);
                    M2C_FIELD(temp_a1_2, s8 *, 0x14) = (s8) (M2C_FIELD(var_a2, u8 *, 0) + (u8) M2C_FIELD(var_a2, u16 *, 4));
                    M2C_FIELD(temp_a1_2, u8 *, 0x15) = (u8) M2C_FIELD(var_a2, u8 *, 2);
                    M2C_FIELD(temp_a1_2, u8 *, 0x1C) = (u8) M2C_FIELD(var_a2, u8 *, 0);
                    M2C_FIELD(temp_a1_2, s8 *, 0x1D) = (s8) (M2C_FIELD(var_a2, u8 *, 2) + (u8) M2C_FIELD(var_a2, u16 *, 6));
                    M2C_FIELD(temp_a1_2, s8 *, 0x24) = (s8) (M2C_FIELD(var_a2, u8 *, 0) + (u8) M2C_FIELD(var_a2, u16 *, 4));
                    M2C_FIELD(temp_a1_2, s16 *, 0xE) = 0x7FC0;
                    M2C_FIELD(temp_a1_2, s8 *, 3) = 9;
                    M2C_FIELD(temp_a1_2, s8 *, 7) = 0x2E;
                    M2C_FIELD(temp_a1_2, s8 *, 0x25) = (s8) (M2C_FIELD(var_a2, u8 *, 2) + (u8) M2C_FIELD(var_a2, u16 *, 6));
                    M2C_FIELD(temp_a1_2, s16 *, 0x16) = 0xAE;
                    if (D_801ADAFC != 0)
                    {
                        M2C_FIELD(temp_a1_2, s32 *, 0) = (M2C_FIELD(temp_a1_2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0xE4) & 0xFFFFFF);
                        M2C_FIELD(D_801398EC, s32 *, 0xE4) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0xE4) & 0xFF000000) | ((s32) temp_a1_2 & 0xFFFFFF));
                        if (D_800D921C < 0x7D00)
                        {
                            D_800D921C += 0x28;
                            M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) ((u8 *)M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x28);
                        }
                    }
                }
                var_t2 -= 1;
                goto loop_18;
            }
            func_8006534C(0xAE, 0x1D);
            func_8006534C(0xAE, 0xB);
        }
    }
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

void func_8005A318(void)
{
/* Partial WMAP decompilation: 95.887260% (gcc280_g0). */

typedef struct
{
    u8 _pad00[0x70];
    u_long ordering_table[0xB3];
    u8* packet_cursor;
} WmapRenderContext;

typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
} WmapProjection;

typedef struct
{
    s32 object_id;
    u8 _pad04[0x24];
} WmapCell;

typedef struct
{
    s16 x;
    s16 y;
    u8 _pad04[4];
} WmapPath8;

typedef struct
{
    s16 x;
    s16 y;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
} WmapPath12;

typedef struct
{
    u8 unk0;
    u8 _pad01;
    u8 unk2;
    u8 _pad03;
    u16 unk4;
    u16 unk6;
    u8 _pad08[8];
    s16 offset_x;
    s16 offset_y;
    s16 anchor_x;
    s16 anchor_y;
} WmapMarker;

extern POLY_FT4 D_800512E0;
extern u8 D_80051308[];
extern s16 D_80051338[3][3][2];
extern s16 D_80051384[3][3][2];
extern u8 D_800513A8[];
extern s8 D_80051B4C[];
extern s32 D_800CC130;
extern WmapMarker D_800CC164[];
extern WmapPath8 D_800CC804[];
extern WmapPath12 D_800CCCF8[];
extern s32 D_800CEFA8[];
extern WmapPath12 D_800CF0AC[];
extern s32 D_800D9168;
extern POLY_FT4 D_800D9170[];
extern s32 D_800D9214;
extern s32 D_800D9218;
extern s32 D_800D921C;
extern s32 D_800DCEC0;
extern s32 D_800DCEE0;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCF08;
extern s32 D_800DCF0C;
extern s32 D_800DCF10;
extern s32 D_8011CF18;
extern s32 D_8011CF48;
extern s32 D_8011CF50;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_8011D504;
extern s32 D_8011D508;
extern s32 D_8011D52C;
extern s32 D_80129558;
extern s32 D_801391E0;
extern SVECTOR D_80139278;
extern WmapCell D_80139290[][6];
extern s32 D_80139838[];
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapRenderContext* D_801398EC;
extern WmapProjection D_80139950;
extern s32 D_8013B208;
extern s32 D_8013B230;
extern s32 D_8013B248;
extern s32 D_8013B250;
extern s32 D_80182234;
extern s32 D_8018223C;
extern VECTOR D_80182DC0;
extern s32 D_80182DDC;
extern s32 D_80182DE0;
extern s32 D_80182E24;
extern s32 D_80182E30;
extern s32 D_801ADAE0;

extern void func_8005B540(void);

    SVECTOR position;
    MATRIX matrix;
    DVECTOR screen;
    DVECTOR* screen_ptr;
    WmapPath8* path8;
    WmapPath12* path12_case3;
    WmapPath12* path12_case4;
    POLY_FT4* packet2;
    s32 counter;
    s32 ot_offset;
    s32 cell_x;
    s32 cell_y;
    s32 object_id;
    s32 screen_y;
    s32 screen_x;
    s32 value;
    s32 sound_id;
    s32 phase;
    s32 temp;

    func_8005B540();
    ot_offset = 0;

    if ((D_8013B208 == 0) || (D_801ADAE0 == 0))
    {
        counter = D_8013B230 + 1;
        D_8013B230 = counter;

        if (D_80182DE0 == 0)
        {
            phase = D_80051B4C[(counter & 0xFF) + 0x20];
            if (phase < 0)
            {
                phase += 0x1F;
            }
            phase >>= 5;
            D_8011CF48 = phase;
        }
        else
        {
            if (D_8011CF48 > 0)
            {
                D_8011CF48--;
            }
            if (D_8011CF48 < 0)
            {
                D_8011CF48++;
            }
        }

        if (D_8011CF18 >= 2)
        {
            D_8011CF48 = 0;
        }

        if (D_801398D0 == 2)
        {
            RotMatrix(&D_80139278, &matrix);
            TransMatrix(&matrix, &D_80182DC0);
            SetRotMatrix(&matrix);
            SetTransMatrix(&matrix);

            position.vz = 0;
            position.vx = ((D_80139950.x * 0x14000) / D_80139950.projection_scale * 0x6000) / D_80139950.projection_scale;
            position.vy = ((D_80139950.y * 0x14000) / D_80139950.projection_scale * 0x6000) / D_80139950.projection_scale;

            gte_ldv0(&position);
            gte_rtps();
            screen_ptr = &screen;
            gte_stsxy(screen_ptr);

            D_8013B248 = D_80182234 - screen.vx;
            D_8013B250 = D_8018223C - (s16)(u16)screen_ptr->vy;
        }
        else
        {
            D_8013B250 = 0;
            D_8013B248 = 0;
        }

        switch (D_8011CF18)
        {
            case 0:
                D_800DCF08 = D_80051338[D_800DCEF0][D_800DCEEC][0];
                D_800DCF10 = D_80051338[D_800DCEF0][D_800DCEEC][1];
                break;

            case 1:
                break;

            case 2:
                D_800DCF08 = D_800CC804[D_80182DDC].x;
                D_800DCF10 = D_800CC804[D_80182DDC].y;
                ot_offset = 7;
                break;

            case 3:
                if (D_800D9218 == D_80182DDC)
                {
                    temp = D_80182DDC;
                    D_800DCF08 = D_800CC804[temp].x;
                    D_800DCF10 = D_800CC804[temp].y;

                    if (D_8011D4FC == -1)
                    {
                        D_8011CF18 = 2;
                        D_8011CF50 = 0;
                        D_800D9168 = D_8011D4FC;
                        D_80182E30 = 0;
                        D_800DCEE0 = 0;
                    }
                    else if (D_800D9168 == -1)
                    {
                        D_800D9168 = D_800CEFA8[D_8011D4FC];
                        D_800DCF0C = D_800CEFA8[D_8011D4FC + 1] - 1;
                    }
                    else if (D_800D9168 == D_800DCF0C)
                    {
                        func_800652A8(D_800CF0AC[D_800DCF0C].unk8, 0x98, D_800DCF0C, D_8011D4FC);
                        D_8011CF18 = 2;
                        D_8011CF50 = 0;
                        D_80182E30 = 0;
                        D_800D9168 = -1;
                        D_800DCEE0 = 0;
                        D_8011D4FC = -1;
                    }
                    else
                    {
                        path12_case3 = D_800CF0AC;
                        path12_case3 += D_800D9168;
                        D_800DCF08 = path12_case3->x;
                        D_800DCF10 = path12_case3->y;
                        counter = D_800D9168 + 1;
                        D_800D9168 = counter;
                        D_800DCEE0 = path12_case3->unk4;
                        D_80182E30 = path12_case3->unk6;
                        D_800D9214 = path12_case3->unkA;

                        if (counter == D_800DCF0C)
                        {
                            s32* route_slots;
                            value = D_800CC130;
                            route_slots = D_80139838;
                            if (value < 0)
                            {
                                value += 3;
                            }
                            D_80182E24 = 0;
                            route_slots[((value >> 2) + 0x2B) % 12] = D_8011D4FC;
                        }
                    }
                }
                else
                {
                    temp = D_80182DDC;
                    D_80182DDC = temp + 1;
                    D_800DCF08 = D_800CC804[temp].x;
                    D_800DCF10 = D_800CC804[temp].y;
                }
                break;

            case 4:
            {
                WmapPath8* path_base;
                WmapPath12* route_base;
                s32 path_index;
                s32 next_index;

                path_base = D_800CC804;
                path_index = D_80182DDC;
                path8 = &path_base[path_index];
                D_800DCF08 = path8->x;
                D_800DCF10 = path8->y;

                if (D_8011D4FC == -1)
                {
                    D_80182E30 = 0;
                    D_80182E24 = 0;
                }

                if (D_800D9218 == path_index)
                {
                    D_8011CF18 = 0;
                    D_8011CF50 = 0;
                }
                else if (D_800D9168 == -1)
                {
                    D_80182E24 = 1;
                    D_80182DDC = path_index - 1;
                    D_800DCF08 = path8->x;
                    D_800DCF10 = path8->y;
                    if (D_8011D4FC != -1)
                    {
                        s32* route_slots;
                        value = D_800CC130;
                        route_slots = D_80139838;
                        if (value < 0)
                        {
                            value += 3;
                        }
                        route_slots[((value >> 2) + 0x2B) % 12] = -1;
                    }
                }
                else
                {
                    route_base = D_800CCCF8;
                    path12_case4 = &route_base[D_800D9168];
                    D_800DCF08 = path12_case4->x;
                    D_800DCF10 = path12_case4->y;
                    D_800DCEE0 = path12_case4->unk4;
                    D_80182E30 = path12_case4->unk6;
                    D_800D9214 = path12_case4->unkA;
                    sound_id = path12_case4->unk8;
                    if (sound_id != -1)
                    {
                        func_800652A8(sound_id, 0x8F, D_80182DDC, D_8011D4FC);
                    }
                    ot_offset = 7;
                    next_index = D_800D9168 + 1;
                    D_800D9168 = next_index;
                    if (D_800DCF0C == next_index)
                    {
                        D_800D9168 = -1;
                    }
                }
                break;
            }
        }

        D_80129558 = D_800DCF08;
        D_801391E0 = D_800DCF10;

        if ((D_8013B208 == 0) && (D_8013986C == 0))
        {
            POLY_FT4* packet;
            u8 x0_offset;
            u8 y0_offset;
            u8 x1_offset;
            u8 y1_offset;
            u8 x2_offset;
            u8 y2_offset;
            u8 x3_offset;
            u8 y3_offset;
            u16 u_base;
            u8 v_base;

            packet = (POLY_FT4*)D_801398EC->packet_cursor;
            *packet = D_800512E0;

            if (D_8011D4FC == -1)
            {
                screen_x = D_80129558;
                screen_y = D_801391E0;
            }
            else
            {
                screen_x = D_80129558 + D_800CC164[D_8011D4FC].offset_x;
                screen_y = D_801391E0 + D_800CC164[D_8011D4FC].offset_y;
            }

            screen_y += D_80182DE0 + D_8011CF48;
            screen_x += D_8013B248;
            screen_y += D_8013B250;

            x0_offset = D_800513A8[D_800DCEE0 * 8 + 0];
            y0_offset = D_800513A8[D_800DCEE0 * 8 + 1];
            x1_offset = D_800513A8[D_800DCEE0 * 8 + 2];
            y1_offset = D_800513A8[D_800DCEE0 * 8 + 3];
            x2_offset = D_800513A8[D_800DCEE0 * 8 + 4];
            y2_offset = D_800513A8[D_800DCEE0 * 8 + 5];
            x3_offset = D_800513A8[D_800DCEE0 * 8 + 6];
            y3_offset = D_800513A8[D_800DCEE0 * 8 + 7];

            packet->x0 = (s8)x0_offset + screen_x;
            packet->y0 = (s8)y0_offset + screen_y;
            packet->x1 = (s8)x1_offset + screen_x;

            *(u16*)&packet->u0 = *(u16*)&D_80051308[D_80182E30 * 8];
            u_base = packet->u0;
            v_base = packet->v0;
            packet->u1 = u_base + D_80051308[(D_80182E30 * 8) | 2];
            packet->v1 = v_base;
            packet->u2 = u_base;
            packet->v2 = v_base + D_80051308[(D_80182E30 * 8) | 3];

            packet->y1 = (s8)y1_offset + screen_y;
            packet->x2 = (s8)x2_offset + screen_x;
            packet->y2 = (s8)y2_offset + screen_y;
            packet->x3 = (s8)x3_offset + screen_x;
            packet->y3 = (s8)y3_offset + screen_y;

            packet->u3 = u_base + D_80051308[(D_80182E30 * 8) | 2];
            packet->v3 = v_base + D_80051308[(D_80182E30 * 8) | 3];

            if (D_80182E30 != 0)
            {
                if (D_800D9214 != 0)
                {
                    addPrim(&D_801398EC->ordering_table[5], packet);
                }
                else
                {
                    addPrim(&D_801398EC->ordering_table[6 + ot_offset], packet);
                }

                D_800D9170[(D_8011CF74 & 1) * 2] = *packet;

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }

                packet2 = (POLY_FT4*)D_801398EC->packet_cursor;
                *packet2 = *packet;
                packet2->v0 += 0x20;
                packet2->v1 += 0x20;
                packet2->v2 += 0x20;
                packet2->v3 += 0x20;
                addPrim(&D_801398EC->ordering_table[4 + ot_offset], packet2);
                D_800D9170[(D_8011CF74 & 1) * 2 + 1] = *packet2;

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
            else
            {
                addPrim(&D_801398EC->ordering_table[4 + ot_offset], packet);
                D_800D9170[(D_8011CF74 & 1) * 2] = *packet;
                D_800D9170[(D_8011CF74 & 1) * 2 + 1] = *packet;

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }

        if ((D_801ADAE0 == 0) && (D_8013986C == 0))
        {
            if (((D_8011D4FC != -1) && (D_80182E24 != 0)) || (D_8011D52C != 0))
            {
                SPRT* sprite;

                sprite = (SPRT*)D_801398EC->packet_cursor;
                if ((D_8011D52C != 0) && (D_801398D0 != 2))
                {
                    sprite->x0 = D_8011D504;
                    sprite->y0 = D_8011D508;
                }
                else
                {
                    sprite->x0 = ((u16)D_80129558 + D_800CC164[D_8011D4FC].anchor_x) + (u16)D_8013B248;
                    D_8011D504 = (s16)sprite->x0;
                    sprite->y0 = (((u16)D_801391E0 + D_800CC164[D_8011D4FC].anchor_y) + (u16)D_80182DE0) + (u16)D_8011CF48 + (u16)D_8013B250;
                    D_8011D508 = (s16)sprite->y0;
                }

                sprite->w = D_800CC164[D_8011D4FC].unk4;
                sprite->h = D_800CC164[D_8011D4FC].unk6;
                sprite->u0 = D_800CC164[D_8011D4FC].unk0;
                sprite->v0 = D_800CC164[D_8011D4FC].unk2;
                sprite->clut = 0x7FEC;
                *(u32*)&sprite->r0 = 0x80808080;
                setSprt(sprite);
                setSemiTrans(sprite, 1);
                addPrim(&D_801398EC->ordering_table[5], sprite);

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->packet_cursor += sizeof(SPRT);
                }
                func_8006534C(0xAE, 5, D_801398EC, sprite);
            }

            cell_x = (D_80139950.x / 48) + D_800DCEEC;
            cell_y = (D_80139950.y / 48) + D_800DCEF0;
            if ((D_8011CF18 == 0) && (D_800DCEC0 != 0))
            {
                object_id = D_80139290[cell_x][cell_y].object_id;
                if (object_id == 0xFF)
                {
                    object_id = -1;
                }
                func_8005FF88(object_id);
            }

            if ((D_80139290[cell_x][cell_y].object_id == 0xFF) && (D_8011CF18 == 0) && (D_801398D0 != 2) && (D_8013B208 == 0))
            {
                POLY_FT4* packet;
                s32 row;
                s32 column;
                packet = (POLY_FT4*)D_801398EC->packet_cursor;
                packet->clut = 0x7F2D;
                *(u32*)&packet->r0 = 0x00808080;
                row = D_800DCEF0;
                column = D_800DCEEC;
                packet->u3 = 0x20;
                packet->u1 = 0x20;
                packet->v1 = 0xC0;
                packet->v0 = 0xC0;
                packet->v3 = 0xE0;
                packet->v2 = 0xE0;
                packet->u2 = 0;
                packet->u0 = 0;
                packet->tpage = 0x5B;
                *(u32*)&packet->x0 = *(u32*)&D_80051384[row][column][0];
                setPolyFT4(packet);
                setSemiTrans(packet, 1);
                packet->x2 = packet->x0;
                packet->x3 = packet->x2 + 0x20;
                packet->x1 = packet->x3;
                packet->y1 = packet->y0;
                packet->y3 = packet->y1 + 0x20;
                packet->y2 = packet->y3;
                addPrim(&D_801398EC->ordering_table[0xAD], packet);

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
    }
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005B540(void)
{
}

/** @brief Initialize 16 black, semitransparent two-point line packets. */
void func_8005B548(void)
{
extern u32 D_800D81FC[];

    s32 index;
    for (index = 0; index < 16; index++)
    {
        D_800D81FC[index * 4] = 0;
        setlen((LINE_F2 *)((u8 *)D_800D81FC - 4) + index, 3);
        setcode((LINE_F2 *)((u8 *)D_800D81FC - 4) + index, 0x42);
    }
}
