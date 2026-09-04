#include "common.h"

typedef struct
{
    u16 field_0;
    u16 field_2;
} ZukanResourceEntry;

extern ZukanResourceEntry D_80157530[];
extern s32 D_80157D3C;

void func_801427E0(s32 arg0)
{
    u32 first[0x200];
    u16 second[0x400];
    s32 count;
    s32 i;
    s32 limit;

    count = func_80142D08(arg0, first, second);
    D_80157D3C = count;
    i = 0;
    if (count > 0)
    {
        limit = count;
        do
        {
            if (first[i] != 0)
            {
                D_80157530[i].field_2 |= 0x8000;
                D_80157530[i].field_2 =
                    (D_80157530[i].field_2 & 0x8000) | ((u16)first[i] & 0x7FFF);
                D_80157530[i].field_0 = second[i * 2];
            }
            else
            {
                D_80157530[i].field_2 = 0;
                D_80157530[i].field_0 = 0;
            }
            i++;
        } while (i < limit);
    }
}
