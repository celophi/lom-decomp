#include "common.h"

extern u8 *g_field_script;

void func_800BD128(s32 arg0);

/**
 * @brief Advances the active field record's timer or triggers a reset.
 *
 * Selects the record at @c g_field_script offset by its stored index (field 0x4,
 * stride 12); when the record's 0xC flags have bit 0 set, adds 3 to its 0x8
 * timer, otherwise calls func_800BD128(1).
 */
void func_800B88C4(void)
{
    u8 *rec = g_field_script;

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
