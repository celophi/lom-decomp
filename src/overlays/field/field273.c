#include "common.h"

void cdrom_queue_seek(s32);

/**
 * @brief Queue a CD seek for a masked field resource entry index.
 * @param arg0 Resource selector; the low 15 bits index into the field
 *             resource table that begins at logical block 0x60C.
 * @return None.
 * @note Masks @c arg0 to 15 bits and biases by 0x60C before dispatching to
 *       @c cdrom_queue_seek.
 * @see decomp.me (100.00%)
 */
void func_8009AFBC(s32 arg0)
{
    cdrom_queue_seek((arg0 & 0x7FFF) + 0x60C);
}
