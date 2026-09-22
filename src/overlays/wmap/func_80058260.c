#include "common.h"

/** @brief World-map resource cache entry. */
typedef struct
{
    s32 slot_index;
    s32 resource_id;
    u32 age;
    s32 busy;
    u8 *image;
} WmapCacheEntry;

extern WmapCacheEntry D_8011CF88[];

/** @brief Initialize the 16 cache slots while preserving their image pointers. */
void func_80058260(void)
{
    u32 slot_index;
    for (slot_index = 0; slot_index < 16U; slot_index++)
    {
        D_8011CF88[slot_index].slot_index = slot_index;
        D_8011CF88[slot_index].resource_id = -1;
        D_8011CF88[slot_index].age = 0;
        D_8011CF88[slot_index].busy = 0;
    }
}
