#include "common.h"

extern void func_8005B58C(void);
extern s32 func_8005D494(void);
extern s32 D_8013B274;

/** @brief Refresh world-map state and cache its saved seven-bit value. */
void func_80059070(void)
{
    func_8005B58C();
    D_8013B274 = func_8005D494();
}
