#include "wmap_land_transition.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_backdrop.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

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
void func_800593D4__for_func_800594D8(void) __asm__("func_800593D4");

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
                func_8006CBD8(func_800593D4__for_func_800594D8);
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
