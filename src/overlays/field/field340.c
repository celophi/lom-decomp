#include "common.h"

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} Prim;

typedef struct {
    u8 pad[0xC];
    s32 unkC;
} OtLike;

/**
 * @brief Build a textured sprite primitive and link it into an ordering table.
 *
 * Fills the primitive at @p arg0 with fixed colour, code, texture, clut, and
 * size fields (the UV taken from @p arg2), copies the packed xy from @p arg3,
 * then splices the primitive ahead of the tag stored at @c arg1->unkC.
 *
 * @param arg0 Primitive packet to populate.
 * @param arg1 Object holding the ordering-table tag at @c unkC.
 * @param arg2 UV selector; scaled by 8 and biased by 0x1558.
 * @param arg3 Source of the packed xy word written to the primitive.
 * @return Pointer just past the emitted primitive (@p arg0 + 0x14).
 * @see decomp.me (100%) TODO
 */
void *func_80085FAC(void *arg0, void *arg1, s32 arg2, s32 *arg3)
{
    Prim *p;
    OtLike *ot;
    s32 temp;

    p = (Prim *)arg0;
    ot = (OtLike *)arg1;

    p->unk4 = 0x808080;
    ((u8 *)p)[3] = 4;
    ((u8 *)p)[7] = 0x64;
    temp = *arg3;
    p->unkC = (s16)((arg2 * 8) + 0x1558);
    p->unk10 = 0xB0008;
    p->unkE = 0x7810;
    p->unk8 = temp;
    p->unk0 = (p->unk0 & 0xFF000000) | (ot->unkC & 0xFFFFFF);
    ot->unkC = (ot->unkC & 0xFF000000) | ((s32)p & 0xFFFFFF);
    return (u8 *)arg0 + 0x14;
}
