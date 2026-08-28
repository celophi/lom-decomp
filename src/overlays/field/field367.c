#include "common.h"

extern u8 D_80122C1C;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern u8 g_menuLayoutBuffer;

/**
 * @brief Latch the current menu selection as the pending gosub result.
 *
 * Reads the active menu record's selection index from @c g_menuLayoutBuffer; if
 * it is in range (< 5), records it into the @c D_80122C1C cursor slot and the
 * pending-result globals, sets the record's 0x40000000 flag, and dispatches
 * func_800B2844 for it.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C9C3C(void)
{
    u8 *dbase;
    u8 d0;
    u8 *mlb;
    u8 *ptr;
    u8 *dp1;
    u8 *argp;
    s32 val;
    s32 sel;
    s32 off;
    u8 *rec;

    func_800C57D4();
    dbase = &D_80122C1C;
    d0 = D_80122C1C;
    dp1 = dbase + 1;
    ptr = d0 + dp1;
    val = *ptr;
    g_gosub_result_count = 1;
    mlb = &g_menuLayoutBuffer;
    sel = *(s32 *)(mlb + 0x2EF0);
    g_gosub_result_values = val;
    if (sel < 5)
    {
        *ptr = (u8)sel;
        off = sel * 0x60;
        rec = off + mlb;
        *(s16 *)(dbase - 8) = *(u8 *)(rec + 0x2F09);
        *(s32 *)(rec + 0x2F38) = *(s32 *)(rec + 0x2F38) | 0x40000000;
        argp = mlb + 0x2EF4;
        func_800B2844(d0, off + argp, 0xFF, d0);
    }
}
