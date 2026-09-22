#include "wmap_land_selection.h"

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
