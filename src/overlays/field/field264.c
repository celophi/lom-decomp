#include "common.h"

typedef struct
{
    u8 pad0[0x34];
    u8 unk34; /* 0x34 */
    u8 pad35[0x4C - 0x35];
    u8 unk4C; /* 0x4C */
} StructFC4;

extern StructFC4 *D_80123FC4;

void func_800BF8E0(void)
{
    s32 mask;
    s32 i;
    u8 *e;

    i = 0;
    mask = 1;
    do
    {
        if (mask & D_80123FC4->unk4C)
        {
            e = (u8 *)D_80123FC4 + (i * 2);
            if (e[0xD] != 0)
            {
                D_80123FC4->unk34 |= mask;
            }
        }
        i++;
        mask *= 2;
    } while (i < 8);
}
