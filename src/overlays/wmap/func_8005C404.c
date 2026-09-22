/* Partial WMAP decompilation: 94.825584% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

extern u8 D_800CFDCC[][12];
extern s32 D_800D00CC[];
extern s32 D_800D8318[][8];
extern u8 *D_800D8B3C;
extern s32 func_8005D670(void);
extern void func_8005BD54(s32, s32, s32, s32 (*)[8]);

/** @brief Calculate eight map attributes after placing a proposed land. */
void func_8005C404(u32 x, s32 y, u32 land, u32 proposed_x, s32 proposed_y, s32 *output)
{
    s32 current_land;
    s32 cell;
    s32 i;
    s32 j;
    s32 component;
    s32 level;
    s32 row;
    s32 value;
    s32 fallback;

    if (x < 6U && y >= 0 && y < 6 && land < 64U)
    {
        if (proposed_x < 6U && proposed_y >= 0 && proposed_y < 6)
        {
            cell = x + y * 6;
            current_land = func_8005D670();
            for (i = 0; i < 64; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    D_800D8318[i][j] = ((SavedGame *)(g_saved_game.bytes + (j + i * 12)))->bytes[0x2F4];
                }
            }
            for (j = 0; j < 8; j++)
            {
                D_800D8318[land][j] = D_800CFDCC[land][j];
            }
            func_8005BD54(land, proposed_x, proposed_y, D_800D8318);
            for (i = 0; i < 8; i++)
            {
                value = 0;
                if (current_land == 255)
                {
                    if (proposed_x == x && proposed_y == y)
                    {
                        component = D_800CFDCC[land][i];
                        row = land * 32;
                        goto selected_land;
                    }
                    component = (D_800D8B3C + (i + cell * 12))[8];
                    fallback = 0;
                    if (component >= 0)
                    {
                        fallback = D_800D00CC[component * 8];
                    }
                    output[i] = fallback;
                }
                else
                {
                    component = ((SavedGame *)(g_saved_game.bytes + (i + current_land * 12)))->bytes[0x2F4];
                    row = current_land * 32;
selected_land:
                    level = *(s32 *)((u8 *)D_800D8318 + (i * 4 + row));
                    if (component >= 0 && level >= 0)
                    {
                        value = D_800D00CC[component * 7 + level];
                    }
                    output[i] = value;
                }
            }
        }
        else
        {
            for (i = 0; i < 8; i++)
            {
                output[i] = D_800D00CC[0];
            }
        }
    }
}
