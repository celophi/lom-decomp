#include "common.h"

typedef struct { u32 unk0; s32 unk4; u8 *unk8; } SeqRec;
extern SeqRec *D_80123FB8;

/**
 * @note NOT YET MATCHED (98.03%). residue is a 6-row ALLOC-43 base/index register swap on the 0xC-stride record address (D_80123FB8[i]); the struct-array mult fixes operand order at expand and 0xC is not a shift, so it is not source-fixable.
 * @see decomp.me (98.03%) TODO
 */
void func_800B9CBC(void)
{
    s32 i;
    i = D_80123FB8->unk4;
    func_800A3938(D_80123FB8[i].unk8[1], D_80123FB8[i].unk8[2]);
    i = D_80123FB8->unk4;
    D_80123FB8[i].unk8 += 3;
}
