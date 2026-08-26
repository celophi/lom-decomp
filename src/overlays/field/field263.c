#include "common.h"

typedef struct
{
    /** 0x00 horizontal position. */
    s16 x;
    /** 0x02 vertical position. */
    s16 y;
    /** 0x04 depth. */
    s16 z;
} FieldPos;

typedef struct
{
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} UnkStruct800BDC40;

extern void func_8005AF5C(s32 obj_index, s32 part_index, FieldPos *out);

void func_800BDC40(s32 arg0, UnkStruct800BDC40 *arg1)
{
    FieldPos pos;
    s32 var_a1;

    var_a1 = -1;
    if (arg1->unk14 != 0xFF)
    {
        var_a1 = arg1->unk14;
    }

    func_8005AF5C(arg1->unk10, var_a1, &pos);

    arg1->unk4 = pos.x;
    arg1->unk8 = pos.y;
    arg1->unkC = pos.z;
}
