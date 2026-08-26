#include "common.h"

typedef struct
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5[0x23];
} FieldUnkRecord_80086F20;

extern FieldUnkRecord_80086F20 D_80107800[];

/**
 * @brief Clear the unk4 flag byte across all 256 records of the D_80107800
 *        table.
 */
void func_80086F20(void)
{
    s32 i;

    for (i = 0xFF; i >= 0; i--)
    {
        D_80107800[i].unk4 = 0;
    }
}
