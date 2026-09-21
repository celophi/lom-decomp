#include "common.h"

extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A798(void);
extern void func_8009A7D0(void);
extern s32 D_801B2C50;

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A75C(void)
{
    if (D_801398AC == 0)
    {
        D_801B2C4C += 1;
        func_8009A798();
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009A798(void)
{
    D_801B2C50 = 0x12C;
    D_801B2C4C += 1;
    func_8009A7D0();
}
