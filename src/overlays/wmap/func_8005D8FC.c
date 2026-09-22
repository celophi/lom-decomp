#include "common.h"

extern s32 func_8005D948(s32, s32, s32);
extern s32 D_800D82F8;
extern s32 D_800D82FC;
extern s32 D_800D9000[];

/**
 * @brief Look up the entry four positions ahead, wrapping at the table limit.
 * @return Value at the wrapped index.
 */
s32 func_8005D8FC(void)
{
    return D_800D9000[func_8005D948(D_800D82FC + 4, 0, D_800D82F8)];
}
