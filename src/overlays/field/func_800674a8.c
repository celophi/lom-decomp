#include "common.h"

/** @brief Bytes of the 32-bit flags word at 0x10. */
typedef struct
{
    u8 unk10;               // 0x10
    u8 unk11;               // 0x11
    u8 unk12;               // 0x12
    u8 unk13;               // 0x13
} FlagBytes;

/** @brief The flags word at 0x10, addressed either whole or by byte. */
typedef union
{
    u32 flags;              // 0x10
    FlagBytes b;
} FlagWord;

typedef struct {
    u32 unk0;               // 0x00
    u8 _pad4[0x10 - 4];     // 0x04
    FlagWord unk10;         // 0x10
    u8 unk14;               // 0x14
    u8 _pad15[0x4E - 0x15]; // 0x15
    s16 unk4E;              // 0x4E
    s16 unk50;              // 0x50
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

/**
 * @see decomp.me (100%) TODO
 */
s32 func_8006751C(s32 arg0)
{
    ArrEntry* entry = (ArrEntry*)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED034);

    if (((entry->unk10.flags & 7) != 0) && ((entry->unk10.b.unk10 & 7) < 4))
    {
        if (entry->unk14 != 0)
        {
            return 2;
        }
        return entry->unk0 != 0;
    }
    return -1;
}
