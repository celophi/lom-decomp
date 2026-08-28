#include "common.h"

typedef struct { u32 unk0; s32 unk4; u8 *unk8; } SeqRec;
extern SeqRec *D_80123FB8;
extern u8 *D_80122B78;

/**
 * @note NOT YET MATCHED (96.67%). residue is a 13-row ALLOC-ORDER register permutation (the same 0xC-stride base/index shape as its siblings, plus the D_80122B78+0x24 arg adds pressure); not source-fixable.
 * @see decomp.me (96.67%) TODO
 */
void func_800B87D4(void)
{
    s32 i;
    i = D_80123FB8->unk4;
    func_800BD6F4(D_80123FB8[i].unk8[1], D_80122B78 + 0x24);
    i = D_80123FB8->unk4;
    D_80123FB8[i].unk8 += 2;
}
