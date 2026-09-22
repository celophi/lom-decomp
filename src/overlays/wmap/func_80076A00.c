#include "common.h"

extern void func_80076A38(void);
extern s32 D_801B25CC;
extern s32 D_801B25C8;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076A00(void)
{
    D_801B25CC = 0x10;
    D_801B25C8 += 1;
    func_80076A38();
}
