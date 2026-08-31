#include "common.h"

extern s32 D_80166078;
extern void *D_8012271C;

s32 func_80143380(void)
{
    if (D_80166078 == 0 && ((*(u32 *)((u8 *)D_8012271C + 0x28) >> 2) & 1))
        return 1;
    return 0;
}
