#include "common.h"

extern u8 D_800F0E98[];
extern s16 D_80122C14;
extern void func_800B2844(s32 arg0, void *arg1, s32 arg2);

/**
 * @brief Decodes a little-endian offset from the field table and dispatches
 *        the referenced entry.
 */
void func_800C7D5C(void)
{
    s32 offset = (D_80122C14 + 0x58) * 2;

    func_800B2844(4, D_800F0E98[offset] +
                         (D_800F0E98[offset + 1] << 8) + D_800F0E98, 0xFF);
}
