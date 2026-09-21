#include "common.h"

extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A90C(void);
extern void func_8009A944(void);
extern s32 D_801B2C50;

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A8D0(void)
{
    if (D_801398AC == 0)
    {
        D_801B2C4C += 1;
        func_8009A90C();
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009A90C(void)
{
    D_801B2C50 = 0x78;
    D_801B2C4C += 1;
    func_8009A944();
}
