#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_8006CAC0(void (*step)(void));
extern void func_80077E8C(void);
extern void func_80078C2C(void);
extern void func_80078A90(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077B14(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_80077B48(void)
{
    func_8006CAC0(func_80077E8C);
    func_8006CAC0(func_80078C2C);
    func_8006CAC0(func_80078A90);
    D_801B25F4 = 0xAC;
    D_801B25F0 += 1;
}
