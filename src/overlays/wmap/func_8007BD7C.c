#include "common.h"

extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8006CAC0(void (*step)(void));
extern void func_8007D1B8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BD7C(void)
{
    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007BDB0(void)
{
    func_8006CAC0(func_8007D1B8);
    D_801B26EC = 0xC;
    D_801B26E8 += 1;
}
