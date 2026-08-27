#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80117ED0;

extern s32 D_80117E88[];

void func_800A2DD8(s32 arg0)
{
    if (arg0 < 2)
    {
        D_80117E88[arg0] = 0;
    }
}

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

extern u8 D_80117EC8[];

/**
 * @brief Return a pointer to the D_80117EC8 byte table.
 * @return Address of D_80117EC8.
 */
u8 *func_800A2E34(void)
{
    return D_80117EC8;
}
