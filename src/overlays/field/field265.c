#include "common.h"

typedef struct
{
    u8 pad0[0x35];
    u8 unk35; /* 0x35 */
    u8 pad36[0x4D - 0x36];
    u8 unk4D; /* 0x4D */
} StructFC4;

extern StructFC4 *D_80123FC4;

void func_800BF944(void)
{
    s32 mask;
    s32 i;

    i = 0;
    mask = 1;
    D_80123FC4->unk35 = 0;
    do
    {
        if (D_80123FC4->unk4D & mask)
        {
            D_80123FC4->unk35 |= mask;
        }
        i++;
        mask *= 2;
    } while (i < 8);
}
