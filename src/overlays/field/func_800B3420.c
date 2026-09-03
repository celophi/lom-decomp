#include "common.h"

typedef struct
{
    u8 pad[0x48];
    u16 unk48;
} StructB3420Sub;

typedef struct
{
    u8 pad[0x10];
    StructB3420Sub *unk10;
} StructB3420;

StructB3420 *func_800B2A9C(s32 value);

/**
 * @see decomp.me (100%)
 */
void func_800B3420(s32 arg0)
{
    StructB3420 *rec;
    StructB3420Sub *sub;
    u32 value;

    rec = func_800B2A9C(2);
    sub = rec->unk10;
    value = sub->unk48;

    switch (value >> 6)
    {
        case 0:
            sub->unk48 = value + (arg0 * 4);
            break;
        case 1:
            sub->unk48 = value + (arg0 * 3);
            break;
        case 2:
            sub->unk48 = value + (arg0 * 2);
            break;
        case 3:
            sub->unk48 = value + arg0;
            break;
    }

    if (rec->unk10->unk48 >= 0x100)
    {
        rec->unk10->unk48 = 0;
    }
}
