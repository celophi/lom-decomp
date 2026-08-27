#include "common.h"

extern u8 *g_pad_ctx;
extern char D_800ECFDC[][0xC];

void func_800B0170(char *name)
{
    s32 i;

    for (i = 0; i < 0xB; i++)
    {
        if (strncmp(name, D_800ECFDC[i], 0xC) == 0)
        {
            *(u32 *)(g_pad_ctx + 0x204) |= 1 << i;
        }
    }
}
