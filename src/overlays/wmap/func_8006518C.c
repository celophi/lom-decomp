#include "common.h"

#include "cdrom.h"

extern u8 D_800DCF18[];

/**
 * @brief Queue a resource read into the world-map buffer.
 * @param resource_index Resource identifier; only the low 16 bits are used.
 */
void func_8006518C(s32 resource_index)
{
    cdrom_queue_read(resource_index & 0xFFFF, D_800DCF18);
}
