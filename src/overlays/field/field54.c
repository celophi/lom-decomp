#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80117ED0;

extern UnkStruct80117ED0 D_80117ED0;
extern s32 D_80117EC0;
extern s32 D_80117EC4;
extern u8 D_80117EC8[];

void func_800A2DFC(void)
{
    D_80117ED0.unk8 = -2;
    D_80117ED0.unk4 = -2;
    D_80117ED0.unk0 = -2;
    D_80117EC0 = 0;
    D_80117EC8[0] = 0xFF;
    D_80117EC4 = 0;
}
