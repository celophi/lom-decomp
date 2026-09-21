#include "common.h"
#include "cdrom.h"

extern u8 D_80193640[];

/**
 * @brief Queue a CD read of the given resource into the world-map image buffer.
 * @param resource_index CD resource index to fetch.
 */
void func_800A8B38(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, D_80193640);
}
