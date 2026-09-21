#include "common.h"
#include "sdk/libgpu.h"

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

typedef struct
{
    s32 unk0;
    s32 resource_id;
    u32 age;
    s32 busy;
    u8 *image;
} WmapCacheEntry;

typedef struct
{
    RECT rect;
    u8 pad8[0x14];
} WmapImageRect;

extern s32 D_800500DC[];
extern WmapImageRect D_800CBBE8[];
extern s32 D_800DBE74;
extern s32 D_8011CF74;
extern WmapCacheEntry D_8011CF88[];
extern s32 D_8013C628[];
extern WmapResource D_80182248[];
extern s32 D_8019D6D8;
extern s32 D_801ADAF8;

/**
 * @brief Resolve or load the cache slot used by a world-map resource.
 * @param resource World-map resource whose cache slot is being resolved.
 * @return Cache slot index, or -1 when the resource cannot be loaded.
 */
s32 func_80055BB0(WmapResource *resource)
{
    s32 i;
    s32 slot;
    s32 oldest_slot;
    u32 oldest_age;
    WmapCacheEntry *cache;
    WmapCacheEntry *search_cache;
    static void *const keep[] __attribute__((section(".discard"))) = { &&count_fail };

    {
        s32 resource_id;

        resource_id = resource->resource_id;
        if (resource_id == 0x1F)
        {
            return 0x10;
        }
        if (D_800500DC[resource_id] == -1)
        {
            return -1;
        }

        i = 0;
        search_cache = D_8011CF88;
search_resource:
        slot = i;
        if (search_cache->resource_id != resource_id)
        {
            i++;
            if (i < 0x10)
            {
                search_cache++;
                goto search_resource;
            }
            slot = -1;
        }
    }

    if (slot < 0)
    {
        D_801ADAF8 = 1;
        if (D_8019D6D8 != 0)
        {
            goto fail;
        }

        i = 0;
        if (D_800DBE74 >= 10)
        {
count_fail:
            return -1;
        }

        {
            s32 empty_id;

            empty_id = -1;
            search_cache = D_8011CF88;
search_empty:
            slot = i;
            if (search_cache->resource_id != empty_id)
            {
                i++;
                if (i < 0x10)
                {
                    search_cache++;
                    goto search_empty;
                }
                slot = -1;
            }
        }

        if (slot < 0)
        {
            goto replace_oldest;
        }

        {
            WmapCacheEntry *free_entry;
            WmapCacheEntry *cache_base;
            u8 *image;
            s16 resource_id;

            cache_base = D_8011CF88;
            free_entry = &cache_base[slot];
            resource_id = resource->resource_id;
            free_entry->resource_id = resource_id;
            image = (u8 *)D_8013C628 + D_8013C628[resource_id];
            LoadImage(&D_800CBBE8[slot].rect, (u_long *)image);
            free_entry->image = image + 0x2004;
            free_entry->age = D_8011CF74;
        }
    }

    if (resource->busy == 0)
    {
        resource->slot = -1;
    }
    return slot;

replace_oldest:
    oldest_slot = -1;
    oldest_age = 0x7FFFFFFF;
    slot = 0;
    do
    {
        cache = &D_8011CF88[slot];
        if (D_80182248[cache->resource_id].state == 0)
        {
            if (cache->age < oldest_age)
            {
                oldest_age = cache->age;
                oldest_slot = slot;
            }
        }
        slot++;
    } while (slot < 0x10);

    {
        WmapCacheEntry *oldest_entry;
        WmapCacheEntry *cache_base;
        u8 *image;
        s16 resource_id;

        cache_base = D_8011CF88;
        oldest_entry = &cache_base[oldest_slot];
        if (oldest_entry->busy != 0)
        {
            return -1;
        }

        i = -1;
        oldest_entry->resource_id = i;
        resource_id = resource->resource_id;
        oldest_entry->resource_id = resource_id;
        image = (u8 *)D_8013C628 + D_8013C628[resource_id];
        LoadImage(&D_800CBBE8[oldest_slot].rect, (u_long *)image);
        image += 0x2004;
        oldest_entry->image = image;
        oldest_entry->age = D_8011CF74;
        resource->busy = 0;
        resource->slot = i;
        return oldest_slot;
    }

fail:
    return -1;
}
