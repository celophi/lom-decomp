#include "common.h"

/** @brief World-map resource and its cache state. */
typedef struct
{
    u8 pad0[2];
    s16 resource_id;
    u8 pad4;
    u8 state;
    u8 pad6[0xA];
    s16 slot;
    u8 pad12[2];
    s32 busy;
    u8 pad18[0x14];
} WmapResource;

/** @brief Cached world-map image and loading state. */
typedef struct
{
    s32 unk0;
    s32 resource_id;
    u32 age;
    s32 busy;
    u8 *image;
} WmapCacheEntry;

extern WmapResource D_80182248[];
extern WmapCacheEntry D_8011CF88[];
extern s32 func_80055BB0(WmapResource *);
extern void func_800561F8(s32, s32, WmapResource *, s32);
extern void func_80055E5C(s32, s32, WmapResource *);

/**
 * @brief Draw a cached map resource, or its marker while the image is unavailable.
 * @param map_x Map-grid X coordinate.
 * @param map_y Map-grid Y coordinate.
 * @param resource_index Index of the world-map resource to draw.
 */
void func_800581A0(s32 map_x, s32 map_y, s32 resource_index)
{
    WmapResource *resource;
    s32 slot;

    resource = &D_80182248[resource_index];
    if (resource->state == 1)
    {
        func_80055E5C(map_x, map_y, resource);
    }
    else
    {
        slot = func_80055BB0(resource);
        if (slot != -1 && D_8011CF88[slot].busy == 0)
        {
            func_800561F8(map_x, map_y, resource, slot);
        }
        else
        {
            func_80055E5C(map_x, map_y, resource);
        }
    }
}
