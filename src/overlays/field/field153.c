#include "common.h"

typedef struct
{
    s32 unk0; /* 0x00 */
    u8 pad4[0x8 - 0x4];
    s32 unk8; /* 0x08 */
} Struct_UnkVec8;

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800C1FBC(Struct_UnkVec8 *arg0, Struct_UnkVec8 *arg1)
{
    s32 dx;
    s32 dz;

    dx = arg0->unk0 - arg1->unk0;
    if (dx < 0)
    {
        dx = -dx;
    }

    dz = arg0->unk8 - arg1->unk8;
    if (dz < 0)
    {
        dz = -dz;
    }

    return dx + dz;
}
