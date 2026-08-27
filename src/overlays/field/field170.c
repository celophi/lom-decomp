#include "common.h"

extern s32 D_800F19B8;

/**
 * @brief Thin stack-frame wrapper around func_800AAFEC passing &D_800F19B8.
 */
void func_800C5AA8(void)
{
    func_800AAFEC(&D_800F19B8);
}

extern s32 D_800F19C4;

void func_800C5ACC(void)
{
    func_800AAFEC(&D_800F19C4);
}

void func_800C5AF0(void)
{
    func_800AD0C8();
}
