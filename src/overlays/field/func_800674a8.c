#include "common.h"

typedef struct {
    u8 _pad[0x4E];
    s16 unk4E;
    s16 unk50;
} ArrEntry;

void func_8006700C(ArrEntry* entry, s32 arg1);

/**
 * @brief Write two s16 values into the entry at index arg0 of the array at 0x801ED034
 *        (element stride 0x98 bytes).
 * @param arg0 Array index (low 16 bits used).
 * @param arg1 Value written to entry->unk4E.
 * @param arg2 Value written to entry->unk50.
 * @see decomp.me (100%) TODO
 */
void func_800674A8(s32 arg0, s16 arg1, s16 arg2) {
    ArrEntry *entry = (ArrEntry *)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED034);
    entry->unk4E = arg1;
    entry->unk50 = arg2;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_800674D8(s32 arg0)
{
    ArrEntry* entry = (ArrEntry*)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED034);
    func_8006700C(entry, 1);
}
