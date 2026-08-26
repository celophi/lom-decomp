#include "common.h"

extern s32 D_8011F428;
extern u8 D_80122C1E;

void func_800C8938(void)
{
    D_8011F428 = (s32) D_80122C1E;
    func_800AD120(D_80122C1E);
}
