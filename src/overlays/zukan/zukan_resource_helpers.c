#include "common.h"

typedef struct
{
    u16 field_0;
    u16 field_2;
} ZukanResourceEntry;

extern s32 D_80157520;
extern ZukanResourceEntry D_80157530[];

void func_80142884(s32 index)
{
    func_800141EC((D_80157530[index].field_2 & 0x7FFF) + 0xBFC, D_80157520);
}

void func_801428C4(s32 value)
{
    s32 resource = D_80157520;

    func_800141EC((value + 0xBFC) & 0xFFFF, resource);
}
