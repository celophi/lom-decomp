#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009FBD0(void);
extern void func_8009F420(void);
extern void func_8009EE1C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E58C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009E5C0(void)
{
    func_8006CAC0(func_8009FBD0);
    func_8006CAC0(func_8009F420);
    func_8006CAC0(func_8009EE1C);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}
