/** @file field_record_position_queries.c
 * @brief Measure X/Z distance and compare stored object positions with live bounds.
 */

#include "common.h"

typedef struct
{
    s32 unk0; /* 0x00 */
    u8 pad4[0x8 - 0x4];
    s32 unk8; /* 0x08 */
} Struct_UnkVec8;

typedef struct
{
    u8 pad0[0x50];
    s32 unk50;
    u8 pad54[4];
    s32 unk58;
} Obj80087F0C;

/** @brief View of D_80122B74 exposing the 64-word bitset at 0x2E8. */
typedef struct
{
    u8 pad0[0x2E8];
    s32 arr2E8[0x40]; /* 0x2E8 */
} StructB74;

#define FLAG_BITSET ((StructB74 *)D_80122B74)

Obj80087F0C *func_80087F0C(s32 arg0);
s32 func_80087F44(s32 arg0, s32 *out);
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
void func_800B2844(s32, void *, s32);
void func_800C2228(s32 idx);

extern u8 *D_80122B74;
extern u16 D_800F0E98[];

/**
 * @brief Manhattan distance between two positions on the X and Z axes.
 * @param arg0 First position.
 * @param arg1 Second position.
 * @return |dx| + |dz|.
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

/**
 * @brief Test whether an object's stored position lies within a box around its live position.
 * @param arg0 Object id.
 * @param arg1 Half-width on X.
 * @param arg2 Half-width on Z.
 * @return -1 when inside the box, 0 otherwise.
 */
s32 func_800C1FFC(s32 arg0, s32 arg1, s32 arg2)
{
    Obj80087F0C *obj;
    s32 buf[4];
    s32 bx;
    s32 by;

    obj = func_80087F0C(arg0);
    func_80087F44(arg0, buf);
    bx = buf[0];
    if ((bx - arg1) < obj->unk50 && obj->unk50 < (bx + arg1))
    {
        by = buf[2];
        if ((by - arg2) < obj->unk58 && obj->unk58 < (by + arg2))
        {
            return -1;
        }
    }
    return 0;
}
