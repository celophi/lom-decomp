#include "common.h"

extern u8 *D_80123FB8;

void func_800BD128(s32 arg0);

/**
 * @brief Advances the active field record's timer or triggers a reset.
 *
 * Selects the record at @c D_80123FB8 offset by its stored index (field 0x4,
 * stride 12); when the record's 0xC flags have bit 0 set, adds 3 to its 0x8
 * timer, otherwise calls func_800BD128(1).
 */
void func_800B88C4(void)
{
    u8 *rec = D_80123FB8;

    rec += *(s32 *)(rec + 4) * 12;
    if (*(s32 *)(rec + 0xC) & 1)
    {
        *(s32 *)(rec + 8) += 3;
    }
    else
    {
        func_800BD128(1);
    }
}
