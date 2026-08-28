#include "common.h"

extern s32 D_801227EC;
extern s32 D_80122908;
extern u16 D_80122998;

extern void func_800ADEB0(void);
extern void func_800AA02C(void);
extern void func_800A71CC(void);
extern void func_800A764C(void);
extern void func_800A7724(void);
extern void func_800B0A08(s32 arg0);

/**
 * @brief Reset field sub-state and dispatch to one of three handlers.
 *
 * Runs the shared reset (func_800ADEB0, then latches @c D_801227EC to 4 and
 * calls func_800AA02C), then selects a handler by state: func_800A71CC when
 * @c D_80122998 is set, else func_800A764C when @c D_80122908 is set, else
 * func_800A7724. Finishes with func_800B0A08(0).
 *
 * @see decomp.me (100%) TODO
 */
void func_800A7434(void)
{
    func_800ADEB0();
    D_801227EC = 4;
    func_800AA02C();
    if (D_80122998 != 0)
    {
        func_800A71CC();
    }
    else if (D_80122908 != 0)
    {
        func_800A764C();
    }
    else
    {
        func_800A7724();
    }
    func_800B0A08(0);
}
