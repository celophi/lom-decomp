#include "common.h"

typedef struct {
    u8 _pad[0x4B];
    u8 unk4B;
} ArrEntry2;

/**
 * @brief Read byte at offset 0x4B of the array entry at index arg0 in the
 *        array at 0x801ED000 (element stride 0x98 bytes).
 * @param arg0 Array index (low 16 bits used).
 * @return The u8 value at entry->unk4B.
 * @see decomp.me (100%) TODO
 */
u8 func_80067598(s32 arg0) {
    ArrEntry2 *entry = (ArrEntry2 *)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED000);
    return entry->unk4B;
}
