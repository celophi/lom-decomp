#include "common.h"

extern u32 D_800D82F8;
extern s32 D_800D82FC;
extern s32 D_800D8300;
extern s32 D_800D8304;
extern s32 D_800D8308;
extern s32 D_800D9000[];
extern s32 func_8005D948(s32, s32, s32);

/** @brief Scroll the save list and rebuild its twelve visible entries. */
void func_8005CA3C(s32 forward, s32 *visible_entries)
{
    s32 row;
    s32 entry;
    s32 i;

    if (forward != 0)
    {
        if (D_800D9000[D_800D82F8] == -1)
        {
            if (D_800D82F8 >= 12U)
            {
                if (D_800D9000[D_800D82FC] == -1)
                {
                    D_800D82F8--;
                    D_800D82FC--;
                }
            }
        }
        D_800D8300++;
        D_800D82FC++;
    }
    else
    {
        if (D_800D9000[D_800D82F8] == -1 && D_800D82F8 >= 12U && D_800D9000[D_800D8304] == -1)
        {
            D_800D82F8--;
        }
        D_800D8300--;
        D_800D82FC--;
    }
    D_800D8300 = func_8005D948(D_800D8300, 0, 11);
    D_800D82FC = func_8005D948(D_800D82FC, 0, D_800D82F8);
    D_800D8308 = func_8005D948(D_800D8300 + 11, 0, 11);
    D_800D8304 = func_8005D948(D_800D82FC + 11, 0, D_800D82F8);
    for (i = 0; i < 12; i++)
    {
        row = func_8005D948(D_800D8300 + i, 0, 11);
        entry = func_8005D948(D_800D82FC + i, 0, D_800D82F8);
        visible_entries[row] = D_800D9000[entry];
    }
}
