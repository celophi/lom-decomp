#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldObjState;

u8 *func_800C1E40(s32 arg0);

extern FieldObjState D_80122C0C;
extern u16 D_80122C0E;

/**
 * @see decomp.me (100%)
 */
void func_800C6F9C(void)
{
    s16 temp_s0;
    u8 *p;
    s32 offset;
    u16 result;

    temp_s0 = D_80122C0C.unk0;
    if (temp_s0 == 0xFF)
    {
        result = 0xFFFF;
    }
    else
    {
        if (D_80122C0C.unk2 == 0)
        {
            p = func_800C1E40(0x102);
            offset = temp_s0 * 4;
        }
        else
        {
            p = func_800C1E40(0x102);
            offset = temp_s0 * 4;
            offset = offset | 2;
        }
        result = *(u16 *)(p + offset + 4);
    }
    D_80122C0E = result;
}
