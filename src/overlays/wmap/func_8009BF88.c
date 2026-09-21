#include "common.h"

extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8006CAC0(void (*step)(void));
extern void func_8009D274(void);
extern void func_8009D06C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BF88(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009BFBC(void)
{
    func_8006CAC0(func_8009D274);
    func_8006CAC0(func_8009D06C);
    D_801B2C6C = 0xAC;
    D_801B2C68 += 1;
}
