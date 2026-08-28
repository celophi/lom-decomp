#include "common.h"

typedef struct { u32 unk0; s32 unk4; u8 *unk8; } SeqRec;
extern SeqRec *D_80123FB8;
extern u8 *D_80122B78;

/**
 * @brief Dispatches the next two-byte sequence operand and advances its PC.
 *
 * @note gcc280_g0, 100% match. The pre-call and post-call record/index values
 *       are separate source value webs, matching the target register lifetimes.
 */
void func_800B87D4(void)
{
    s32 i;
    s32 j;
    SeqRec *rec;
    SeqRec *rec2;

    rec = D_80123FB8;
    i = rec->unk4;
    rec += i;
    func_800BD6F4(rec->unk8[1], D_80122B78 + 0x24);

    rec2 = D_80123FB8;
    j = rec2->unk4;
    rec2 += j;
    rec2->unk8 += 2;
}
