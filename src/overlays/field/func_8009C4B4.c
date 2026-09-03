#include "common.h"
#include "sdk/libgpu.h"

extern s32 D_801178B8;
extern u16 D_800EB2D4[];
extern u16 D_800EBAD4[];

void func_8009C4B4(void)
{
    RECT rect;
    u16 *src;

    rect.x = 0x120;
    rect.y = 0xB4;
    rect.w = 0x20;
    rect.h = 0x20;
    if (D_801178B8 != 0)
        src = D_800EBAD4;
    else
        src = D_800EB2D4;
    LoadImage(&rect, (u_long *)src);

    rect.x = 0x120;
    rect.y = 0x19C;
    rect.w = 0x20;
    rect.h = 0x20;
    if (D_801178B8 != 0)
        src = D_800EBAD4;
    else
        src = D_800EB2D4;
    LoadImage(&rect, (u_long *)src);

    DrawSync(0);
    D_801178B8 = D_801178B8 == 0;
}
