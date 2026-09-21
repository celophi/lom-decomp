#include "common.h"

#include "sdk/libgpu.h"

extern RECT D_80051A88;

/** @brief Clear the configured image rectangle to black. */
void func_800652F8(void)
{
    RECT rectangle = D_80051A88;
    ClearImage(&rectangle, 0, 0, 0);
}
