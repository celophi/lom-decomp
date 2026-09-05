#include "common.h"

s32 func_800A9060(void);
void func_800A8F8C(s32 arg0, void *arg1);
u8 *func_800C1E40(s32 arg0);
void func_800C57D4(void);
void func_800AD030(s32 arg0);

/*
 * Deliberately declared without a prototype: func_800C9C3C passes a fourth
 * argument to keep its cursor byte live in a3, while the other callers pass
 * three. A fixed-arity prototype would change codegen for one side or the
 * other.
 */
void func_800B2844();

extern u8 D_800459AC;
extern u8 D_80122C00;
extern u8 D_80122C01;
extern s8 D_80122C12;
extern u16 D_80122C16;
extern u8 D_80122C1C;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Dispatch the current menu record for the active cursor slot.
 * @see decomp.me (100%)
 */
void func_800C9BC4(void)
{
    s32 temp_s1;
    s32 temp_s0;
    s32 off;

    temp_s1 = D_80122C00;
    if (func_800A9060() != 0)
    {
        temp_s0 = func_800A9060();
        func_800A8F8C(temp_s0, func_800C1E40(5) + (off = (temp_s1 << 6) + 4));
        func_800B2844(0, func_800C1E40(5) + off, 0xFF);
    }
}

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
    mlb = g_menuLayoutBuffer;
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

/**
 * @brief Re-select the cursor slot's menu record and dispatch it.
 */
void func_800C9CE4(void)
{
    u8 *base;
    u8 *rec;
    s32 idx;

    idx = ((u8 *)&D_80122C1C)[D_80122C1C + 1] * 0x60;
    base = g_menuLayoutBuffer;
    rec = idx + base;
    D_80122C1C = rec[0x2F3C];
    base = base + 0x2EF4;
    func_800B2844(3, idx + base, 0xFF);
}

/**
 * @brief Count the active menu records and store the total in D_80122C16.
 */
void func_800C9D44(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 5; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x60];
        if (p[0x2EF4] != 0)
        {
            count++;
        }
    }
    D_80122C16 = (u16) count;
}

/**
 * @brief Forward D_80122C01 to func_800AD030 after the common menu prologue.
 */
void func_800C9D84(void)
{
    func_800C57D4();
    func_800AD030(D_80122C01);
}

/**
 * @brief Store the high nibble of D_800459AC into D_80122C12.
 */
void func_800C9DB4(void)
{
    D_80122C12 = (s8) ((u8) D_800459AC >> 4);
}
