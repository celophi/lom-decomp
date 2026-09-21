#include "common.h"

typedef struct { unsigned char pad[0x14]; s32 unk14; } WmapObj;

extern void func_80084FE0(void);
extern WmapObj* D_80139280;
extern s32 D_801B2898;
extern s32 D_801B289C;

void func_80084F98(void)
{
    D_801B289C = 0x20;
    D_80139280->unk14 = -1;
    D_801B2898 += 1;
    func_80084FE0();
}
