#include "common.h"

extern s32 func_800A4744(void);
extern s32 func_800A4778(void);
extern u16 D_80122C16;
extern s32 D_800F19CC;

void func_800C6850(void)
{
    s32 result = func_800A4744();

    if (result < 0)
    {
        D_80122C16 = 1;
        *(&D_80122C16 - 1) = func_800A4778();
    }
    else
    {
        D_80122C16 = 0;
        *(&D_80122C16 - 1) = result;
    }
}

/**
 * @brief Thin stack-frame wrapper around field_open_gosub_screen_sequence passing &D_800F19CC.
 */
void func_800C68A4(void)
{
    field_open_gosub_screen_sequence(&D_800F19CC);
}
