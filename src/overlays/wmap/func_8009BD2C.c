#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009CC64(void);
extern void func_8009C694(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BD2C(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009BD60(void)
{
    func_8006CAC0(func_8009CC64);
    func_8006CAC0(func_8009C694);
    D_801B2C6C = 0x28;
    D_801B2C68 += 1;
}
