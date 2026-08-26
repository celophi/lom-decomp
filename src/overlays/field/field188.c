#include "main.h"

extern s16 D_80122C10;

void func_800C752C(void)
{
    D_80122C10 = g_menuLayoutBuffer[(D_80122C10 * 0x60) + 0x2F09];
}
