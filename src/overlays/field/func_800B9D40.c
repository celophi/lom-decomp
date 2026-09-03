#include "common.h"

typedef struct { u8 unk0; u8 pad1[3]; s32 unk4; u8 *unk8; } SeqRec;
extern SeqRec *D_80123FB8;

extern void func_80087FC0(s32 arg0, u8 arg1, u8 *arg2);

/**
 * @brief Dispatches the current sequence command, defaulting to the running
 *        status byte, then advances the active record's PC by three bytes.
 *
 * @note gcc280_g0, 100% match. Reading the command byte through a truncating
 *       copy (cmd = (short)i) reproduces the target's separate register plus
 *       delay-slot move; the record stride is written as (i * 3 << 2) so the
 *       base+index addu keeps the base register first.
 */
void func_800B9D40(void)
{
    SeqRec *base;
    u8 *p;
    s32 i;
    s32 cmd;

    base = D_80123FB8;
    i = base->unk4;
    p = ((SeqRec *)((u8 *)base + (i * 3 << 2)))->unk8;
    i = p[1];
    cmd = (short) i;
    if (i == 0xFF)
    {
        cmd = base->unk0;
    }
    func_80087FC0(cmd & 0xFF, p[2], p);

    {
        SeqRec *rec;
        s32 j;
        rec = D_80123FB8;
        j = rec->unk4;
        rec += j;
        rec->unk8 += 3;
    }
}
