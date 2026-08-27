#include "common.h"

typedef struct FieldState800BA1D4
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} FieldState800BA1D4;

extern FieldState800BA1D4 *D_80123FB8;
extern void func_800B4584(void);

/**
 * @brief Updates field state and increments the active record counter.
 */
void func_800BA1D4(void)
{
    FieldState800BA1D4 *entry;

    func_800B4584();
    entry = D_80123FB8 + D_80123FB8->unk4;
    entry->unk8 = entry->unk8 + 1;
}
