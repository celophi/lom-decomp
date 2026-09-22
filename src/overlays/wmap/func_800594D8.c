#include "wmap_resource_support.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_backdrop.h"
#include "wmap_effect_resources.h"
#include "common.h"
#include "cdrom.h"

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

/** @brief Update world-map selection and route state. */
void func_800594D8(void)
{
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
