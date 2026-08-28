#include "common.h"
#include "cdrom.h"

extern s32 D_80122B68[];

/**
 * @brief Checks whether either active field resource is already queued.
 *
 * @return 1 if an active resource is already queued, otherwise 0.
 */
s32 func_800B0888(void)
{
    s32 *resource_index;
    s32 i;

    i = 0;
    resource_index = D_80122B68;
    do
    {
        if (*resource_index != 0)
        {
            if (cdrom_can_queue_resource((u16)*resource_index) == 0)
            {
                return 1;
            }
        }
        i++;
        resource_index++;
    } while (i < 2);

    return 0;
}
