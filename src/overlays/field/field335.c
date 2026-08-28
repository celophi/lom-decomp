#include "common.h"

typedef struct {
    u32 unk0;
    s32 unk4;   /* 0x4 */
    u8 *unk8;   /* 0x8 */
} SeqRec;

extern SeqRec *D_80123FB8;

/**
 * @note NOT YET MATCHED (97.97%). residue is a 6-row ALLOC-43 base/index register swap on the 0xC-stride record address; not source-fixable (mult stride, not a shift).
 * @see decomp.me (97.97%) TODO
 */
void func_800BA154(void)
{
    s32 i;

    i = D_80123FB8->unk4;
    func_800B4410(D_80123FB8[i].unk8[1]);
    i = D_80123FB8->unk4;
    D_80123FB8[i].unk8 += 2;
}
