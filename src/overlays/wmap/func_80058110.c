#include "common.h"
#include "sdk/libgpu.h"

/** @brief Resource identifier and cache-slot state for a world-map object. */
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

/** @brief Cache entry storing a resource identifier, age, and image pointer. */
typedef struct
{
    s32 unk0;
    s32 resource_id;
    u32 age;
    s32 busy;
    u8 *image;
} WmapCacheEntry;

/** @brief Upload rectangle in the 28-byte world-map image table. */
typedef struct
{
    RECT rect;
    u8 pad8[0x14];
} WmapImageRect;

extern WmapImageRect D_800CBBE8[];
extern s32 D_8011CF74;
extern WmapCacheEntry D_8011CF88[];
extern s32 D_8013C628[];

/**
 * @brief Upload a resource into a cache slot and record its image pointer and age.
 * @param resource Resource selecting the packed image data.
 * @param slot Destination cache slot.
 */
void func_80058110(WmapResource *resource, s32 slot)
{
    WmapCacheEntry *cache;
    WmapCacheEntry *base;
    s16 resource_id;
    u8 *image;

    base = D_8011CF88;
    cache = &base[slot];
    resource_id = resource->resource_id;
    cache->resource_id = resource_id;
    image = (u8 *)D_8013C628 + D_8013C628[resource_id];
    LoadImage(&D_800CBBE8[slot].rect, (u_long *)image);
    cache->image = image + 0x2004;
    cache->age = D_8011CF74;
}
