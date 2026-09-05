#include "common.h"

extern s32 D_800F19B8;

/**
 * @brief Thin stack-frame wrapper around field_open_gosub_screen_sequence passing &D_800F19B8.
 */
void func_800C5AA8(void)
{
    field_open_gosub_screen_sequence(&D_800F19B8);
}

extern s32 D_800F19C4;

void func_800C5ACC(void)
{
    field_open_gosub_screen_sequence(&D_800F19C4);
}

void func_800C5AF0(void)
{
    func_800AD0C8();
}
