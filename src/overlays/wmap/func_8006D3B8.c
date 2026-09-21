#include "common.h"

extern s32 D_801B10A8;
extern void func_8006D3E4(void);

/** @brief World-map step handler: bump the step counter and run the next step. */
void func_8006D3B8(void)
{
    D_801B10A8 += 1;
    func_8006D3E4();
}
