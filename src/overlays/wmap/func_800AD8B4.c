#include "common.h"

extern void func_800AD8EC(void);
extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800AD8B4(void)
{
    D_801B2EEC = 0x58;
    D_801B2EE8 += 1;
    func_800AD8EC();
}
