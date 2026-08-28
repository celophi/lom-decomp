#include "common.h"
#include "psyq_compat/memory.h"

extern u8 D_800EE2D8;
extern u8 D_800EE4D8;

/**
 * @brief Copies a 32-byte entry from one of two field-data tables.
 *
 * @param destination Buffer that receives the selected entry.
 * @param index Combined index across the two 16-entry tables.
 */
void func_800A55E4(unsigned char *destination, s32 index)
{
    if (index < 0x10)
    {
        bcopy(&D_800EE2D8 + (index << 5), destination, 0x20);
    }
    else
    {
        bcopy(&D_800EE4D8 + ((index - 0x10) << 5), destination, 0x20);
    }
}
