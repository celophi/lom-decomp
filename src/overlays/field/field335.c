#include "common.h"

typedef struct {
    u32 unk0;
    s32 unk4;   /* 0x4 */
    u8 *unk8;   /* 0x8 */
} SeqRec;

extern SeqRec *D_80123FB8;

/**
 * @brief Dispatches the next two-byte sequence operand and advances its PC.
 *
 * @note gcc280_g0, 100% match. Advancing a record pointer by the active index
 *       reproduces the target's base-register destination for the 0xC stride.
 */
void func_800BA154(void)
{
    s32 i;
    SeqRec *rec;

    rec = D_80123FB8;
    i = rec->unk4;
    rec += i;
    func_800B4410(rec->unk8[1]);

    rec = D_80123FB8;
    i = rec->unk4;
    rec += i;
    rec->unk8 += 2;
}
