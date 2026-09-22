#include "wmap_map_display.h"
#include "wmap_resource_support.h"
#include "sdk/libgpu.h"
#include "wmap_main.h"
#include "wmap_map_labels.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_sequence_runtime.h"

/**
 * @brief Update world-map state and draw the local map and status sprites.
 */
void func_80054A2C(void)
{
typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

typedef struct
{
    s32 x;
    s32 y;
} WmapPosition;

typedef struct
{
    u8 pad0[0x74];
    u_long ot_entry;
    u8 pad78[0x33C - 0x78];
    SPRT *prim_cursor;
} WmapRenderContext;

extern SPRT D_800C45B0;
extern SPRT D_800C45C4;
extern SPRT D_800C45D8;
extern SPRT D_800C45EC;
extern s32 D_800D7CC0;
extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern s32 D_800D7CCC;
extern s32 D_800D921C;
extern s32 D_800D9220;
extern s32 D_800DBE78;
extern s32 D_800DCEC0;
extern s32 D_8011CF18;
extern s32 D_8011CF7C;
extern s32 D_8011D4FC;
extern s32 D_8013922C;
extern s32 D_80139230;
extern WmapCell D_80139290[][6];
extern s32 D_8013986C;
extern s32 D_80139880;
extern s32 D_801398C0;
extern WmapRenderContext *D_801398EC;
extern WmapPosition D_80139950;
extern s32 D_8013B258;
extern s32 D_8013B25C;
extern s32 D_8013B268;
extern s32 D_8013B298;
extern s32 D_80182230;
extern s32 D_80182238;
extern s32 D_801ADAEC;

    s32 timer;
    s32 valid;
    s32 scan_x;
    s32 scan_y;
    s32 render_x;
    s32 render_y;
    SPRT *sprite;

    if (D_8013B25C != 0)
    {
        if ((D_801398C0 & 4) != 0)
        {
            timer = D_8013B25C - 1;
            D_8013B25C = timer;
            if (timer == 0)
            {
                valid = 1;
                D_800D7CC8 = D_80139950.y / 48;
                D_800D7CC4 = D_80139950.x / 48;

                for (scan_y = D_800D7CC8; scan_y < D_800D7CC8 + 3; scan_y++)
                {
                    for (scan_x = D_800D7CC4; scan_x < D_800D7CC4 + 3; scan_x++)
                    {
                        if (D_80139290[scan_x][scan_y].value == 0xFF)
                        {
                            valid = 0;
                        }
                    }
                }

                if ((D_800DBE78 != 0) || (D_8013986C != 0) || (D_8011CF18 != 0) || (D_80139230 != 0) || (D_8011D4FC != -1))
                {
                    valid = 0;
                }

                if (valid == 0)
                {
                    D_8013B25C = 60;
                    return;
                }

                D_800DCEC0 = 0;
                func_80064F64(0x1154);
                D_80182238 = 0;
                D_800D7CC0 = 0;
                D_8011CF7C = 0;
                D_80139880 = 0;
                D_80182230 = 0x708;
                D_800DBE78 = 1;
                D_801ADAEC = 0;
                D_8013B268 = 0;
                D_8013B258 = 1;
                D_8013B298 = 0;
                D_800D7CCC = 15;
                D_800D9220 = 0xF0E0;
            }
        }
        else
        {
            D_8013B25C = 60;
        }
        return;
    }

    switch (D_80139880)
    {
    case 0:
        func_800551A8();
        break;
    case 1:
        func_8005536C();
        break;
    case 2:
        func_8005556C();
        break;
    }

    for (render_y = D_800D7CC8; render_y < D_800D7CC8 + 3; render_y++)
    {
        for (render_x = D_800D7CC4; render_x < D_800D7CC4 + 3; render_x++)
        {
            func_800581A0(render_x, render_y, D_80139290[render_x][render_y].value);
        }
    }

    render_y = D_800D7CC0;
    render_x = 0xF0;
    if (D_80182238 == 0)
    {
        sprite = D_801398EC->prim_cursor;
        *sprite = D_800C45C4;
        addPrim(&D_801398EC->ot_entry, sprite);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->prim_cursor++;
        }
    }
    else if (render_y != 0)
    {
        do
        {
            sprite = D_801398EC->prim_cursor;
            *sprite = D_800C45C4;
            sprite->x0 = render_x;
            sprite->u0 = (render_y % 10) * 16;
            render_y /= 10;
            addPrim(&D_801398EC->ot_entry, sprite);
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += sizeof(SPRT);
                D_801398EC->prim_cursor++;
            }
            render_x -= 16;
        } while (render_y != 0);
    }

    sprite = D_801398EC->prim_cursor;
    *sprite = D_800C45B0;
    addPrim(&D_801398EC->ot_entry, sprite);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor++;
    }

    sprite = D_801398EC->prim_cursor;
    *sprite = D_800C45EC;
    sprite->u0 = D_8013B298 * 16;
    addPrim(&D_801398EC->ot_entry, sprite);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor++;
    }

    sprite = D_801398EC->prim_cursor;
    *sprite = D_800C45D8;
    addPrim(&D_801398EC->ot_entry, sprite);
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor++;
    }

    func_8006534C(0x55, 1);

    if ((D_800D7CC0 - D_80182238) >= 0)
    {
        if ((D_800D7CC0 - D_80182238) < 30)
        {
            goto move_small;
        }
        goto move_large;
    }

    if ((D_80182238 - D_800D7CC0) < 30)
    {
move_small:
        if (D_800D7CC0 < D_80182238)
        {
            D_800D7CC0++;
        }
        if (D_80182238 < D_800D7CC0)
        {
            D_800D7CC0--;
        }
    }
    else
    {
move_large:
        if (D_800D7CC0 < D_80182238)
        {
            D_800D7CC0 += 15;
        }
        if (D_80182238 < D_800D7CC0)
        {
            D_800D7CC0 -= 15;
        }
    }

    D_801398C0 = 0;
    D_8013922C = 0;
}

/**
 * @brief Update and enqueue the world-map sprite for the current state.
 */
void func_800551A8(void)
{
typedef struct
{
    u8 pad_000[0x74];
    u_long ordering_table_entry;
    u8 pad_078[0x2C4];
    void* prim_cursor;
} WmapRenderState;

extern SPRT D_800C4588;
extern SPRT D_800C459C;
extern s32 D_800D7CCC;
extern s32 D_800D921C;
extern s32 D_800DBE78;
extern s32 D_800DCEC0;
extern s32 D_8011CF7C;
extern s32 D_8013922C;
extern s32 D_80139880;
extern WmapRenderState* D_801398EC;
extern s32 D_8013B25C;
extern s32 D_8013B298;
extern s32 D_80182230;

    SPRT* sprite;

    sprite = D_801398EC->prim_cursor;

    if (D_8013B298 < 8 && D_800D7CCC >= 12)
    {
        s32 state_flags;

        state_flags = D_8013922C;
        *sprite = D_800C4588;

        if (state_flags & 0x80)
        {
            D_80139880 = 1;
            D_80182230 = 0x78;
            func_800652A8(0x3C, 0x80);
            D_800D7CCC = 0;
            D_8013B298++;
        }
        else if (state_flags & 0x20)
        {
            D_8013B25C = 0x3C;
            D_80139880 = 0;
            D_8011CF7C = 1;
            D_800DCEC0 = 1;
            D_800DBE78 = 2;
            func_80064094();
        }
    }
    else
    {
        s32 state_flags;

        state_flags = D_8013922C;
        *sprite = D_800C459C;

        if (state_flags & 0x20)
        {
            D_8013B25C = 0x3C;
            D_80139880 = 0;
            D_8011CF7C = 1;
            D_800DCEC0 = 1;
            D_800DBE78 = 2;
            func_80064094();
        }
    }

    addPrim(&D_801398EC->ordering_table_entry, sprite);

    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
    }

    func_8006534C(0x55, 1);
}

/**
 * @brief Advance the active world-map transition and refresh nearby object timers.
 */
void func_8005536C(void)
{
typedef struct
{
    u8 _pad00[0x74];
    u_long ordering_table_tag;
    u8 _pad78[0x2C4];
    u8* packet_cursor;
} WmapRenderContext;

typedef struct
{
    s32 object_id;
    u8 _pad04[0x24];
} WmapCell;

typedef struct
{
    u8 _pad00[0x28];
    s16 timer;
    u8 _pad2A[2];
} WmapObject;

extern SPRT D_800C4600;
extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern s32 D_800D921C;
extern WmapCell D_80139290[][6];
extern s32 D_80139880;
extern WmapRenderContext* D_801398EC;
extern s32 D_80139948;
extern s32 D_80182230;
extern WmapObject D_80182248[];

    SPRT* sprite;
    s32 i;
    s32 j;
    s32 object_id;

    sprite = (SPRT*)D_801398EC->packet_cursor;
    *sprite = D_800C4600;
    addPrim(&D_801398EC->ordering_table_tag, sprite);

    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor += sizeof(SPRT);
    }

    D_80182230--;
    if (D_80182230 == 60)
    {
        for (i = D_800D7CC8; i < D_800D7CC8 + 3; i++)
        {
            for (j = D_800D7CC4; j < D_800D7CC4 + 3; j++)
            {
                object_id = D_80139290[j][i].object_id;
                func_80058014(object_id, 1);
                D_80182248[object_id].timer = 0;
            }
        }
    }

    if (D_80182230 == 0)
    {
        D_80182230 = 0x384;
        D_80139880 = 2;
        D_80139948 = 30;
    }

    func_8006534C(0x55, 1);
    func_8005FF88(-1);
}

/**
 * @brief Update world-map object transitions in the active cell neighborhood.
 */
void func_8005556C(void)
{
typedef struct
{
    s32 object_id;
    u8 pad04[0x24];
} WmapGridCell;

typedef struct
{
    u8 pad00[5];
    u8 state;
    s8 transition;
    u8 pad07[0x21];
    s16 timer;
    u8 pad2A[2];
} WmapObject;

extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern s32 D_800D7CCC;
extern s32 D_800D922C;
extern s32 D_800D9234;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8013922C;
extern WmapGridCell D_80139290[][6];
extern s32 D_80139880;
extern s32 D_801398C0;
extern s32 D_8013B298;
extern s32 D_80182230;
extern s32 D_80182238;
extern WmapObject D_80182248[];

    s32 x;
    s32 y;
    s32 object_id;
    s32 step;
    WmapObject* object;

    D_80182230--;
    if (D_80182230 == 0)
    {
        D_80139880 = 0;
    }

    D_800D9234 = D_801398C0;
    func_80055AF4();

    y = D_800D7CC8 + D_800DCEF0;
    x = D_800D7CC4 + D_800DCEEC;
    object_id = D_80139290[x][y].object_id;
    object = &D_80182248[object_id];

    if (D_8013922C & 0x40)
    {
        if ((u32)(object->state - 2) < 2)
        {
            step = (D_8013B298 + 1) * 5;
            D_80182238 += step;
            D_800D7CCC++;
            if (object_id == 0x1F)
            {
                D_80182238 += step;
            }
            D_800D922C = D_80182238;
            func_80058014(object_id, 1);
            object->transition = 1;
            func_800652A8(0x3E, 0x80);
        }
        else
        {
            D_80182238 -= (D_8013B298 + 1) * 2;
            D_800D7CCC--;
            if (D_80182238 < 0)
            {
                D_80182238 = 0;
            }
            func_800652A8(0x3F, 0x80);
        }
    }

    for (y = D_800D7CC8; y < D_800D7CC8 + 3; y++)
    {
        for (x = D_800D7CC4; x < D_800D7CC4 + 3; x++)
        {
            object_id = D_80139290[x][y].object_id;
            object = &D_80182248[object_id];
            if (object->timer != 0)
            {
                object->timer--;
                if ((object->timer == 0) && (object->state != 1))
                {
                    func_80058014(D_80139290[x][y].object_id, 1);
                    object->transition = 4;
                    func_800652A8(2, 0x80);
                }
            }
        }
    }
}

/**
 * @brief Schedule randomized timer updates for nearby world-map objects.
 * @param delay_min Base value for the next update delay.
 * @param delay_range Randomized span added to the next update delay.
 * @param timer_min Base value assigned to selected object timers.
 * @param timer_range Randomized span added to selected object timers.
 */
void func_80055830(s32 delay_min, s32 delay_range, s32 timer_min, s32 timer_range)
{
typedef struct
{
    s32 object_id;
    u8 _pad04[0x24];
} WmapCell;

typedef struct
{
    u8 _pad00[5];
    u8 state;
    u8 _pad06[0x22];
    s16 timer;
    u8 _pad2A[2];
} WmapObject;

extern u8 D_800C4614[][9];
extern s32 D_800D7CC4;
extern s32 D_800D7CC8;
extern WmapCell D_80139290[][6];
extern s32 D_80139948;
extern s32 D_8013B298;
extern WmapObject D_80182248[];

extern s32 rand(void);

    s32 row;
    s32 column;
    s32 pattern_index;
    s32 object_id;
    WmapObject* object;

    if (D_80139948-- > 0)
    {
        return;
    }

    D_80139948 = ((rand() * delay_range) >> 15) + delay_min;
    column = (rand() % 3) + D_800D7CC4;
    row = (rand() % 3) + D_800D7CC8;
    object_id = D_80139290[column][row].object_id;

    if (rand() & 0xFF)
    {
        WmapObject* object_base;

        object_base = D_80182248;
        object = &object_base[object_id];
        if (object->state != 1)
        {
            return;
        }

        object->timer = ((rand() * timer_range) >> 15) + timer_min;
        func_80058014(object_id, 2);
    }
    else
    {
        pattern_index = 0;
        for (row = D_800D7CC8; row < D_800D7CC8 + 3; row++)
        {
            for (column = D_800D7CC4; column < D_800D7CC4 + 3; column++)
            {
                object_id = D_80139290[column][row].object_id;
                if (D_800C4614[D_8013B298][pattern_index])
                {
                    WmapObject* object_base;

                    object_base = D_80182248;
                    object = &object_base[object_id];
                    if (object->state == 1)
                    {
                        object->timer = ((rand() * timer_range) >> 15) + timer_min;
                        func_80058014(object_id, 2);
                    }
                }
                pattern_index++;
            }
        }
    }

    func_800652A8(0x3D, 0x80);
}

/**
 * @brief Update state-dependent world-map effect timing.
 */
void func_80055AF4(void)
{
extern s32 D_8013B298;

    switch (D_8013B298)
    {
    case 1:
        func_80055830(40, 20, 25, 30);
        break;
    case 2:
        func_80055830(30, 20, 20, 25);
        break;
    case 3:
        func_80055830(10, 20, 20, 20);
        break;
    case 4:
        func_80055830(5, 25, 15, 20);
        break;
    default:
        func_80055830(5, 20, 8, 15);
        break;
    }
}

/**
 * @brief Resolve or load the cache slot used by a world-map resource.
 * @param resource World-map resource whose cache slot is being resolved.
 * @return Cache slot index, or -1 when the resource cannot be loaded.
 */
s32 func_80055BB0(WmapResource *resource)
{
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

/**
 * @brief Build and enqueue a world-map marker sprite.
 * @param map_x Map-grid X coordinate.
 * @param map_y Map-grid Y coordinate.
 * @param marker Marker record containing the texture selection.
 */
void func_80055E5C(s32 map_x, s32 map_y, WmapMarker* marker)
{
typedef struct
{
    u8 pad_000[0x70];
    u_long ordering_table[179];
    void* prim_cursor;
} WmapRenderState;

typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjectionState;

typedef struct
{
    u16 x;
    u16 y;
} WmapMapPoint;



extern WmapMapPoint D_8004FD04[];
extern s16 D_800D036C[];
extern s32 D_800D921C;
extern s32 D_8013986C;
extern WmapRenderState* D_801398EC;
extern WmapProjectionState D_80139950;

    SPRT* sprite;
    s32 depth;
    s32 type;
    SVECTOR position;
    u32 screen;
    u16* screen_ptr;
    s32 projected_z;

    sprite = D_801398EC->prim_cursor;

    switch (D_8013986C)
    {
    case 0:
    {
        s32 scale;
        s32 z_bucket;
        s32 sprite_y;

        scale = D_80139950.scale;
        position.vx = ((((map_x - 1) * 160) - ((D_80139950.x * 0x14000) / scale)) * 0x6000) / scale;
        position.vy = ((((map_y - 1) * 160) - ((D_80139950.y * 0x14000) / scale)) * 0x6000) / scale;
        position.vz = 0;

        gte_ldv0(&position);
        gte_rtps();
        gte_stsxy(&screen);
        gte_stszotz(&projected_z);
        screen_ptr = (u16*)&screen;

        if ((s32)(screen_ptr[1] << 16) < 0)
        {
            return;
        }

        z_bucket = (0x1B91 - projected_z) / 4;
        depth = z_bucket + 0x2F;
        if (depth < 0x2E || depth > 0xAE)
        {
            depth = 0x2E;
        }

        sprite->x0 = screen_ptr[0] - 8;
        sprite_y = screen_ptr[1] - 22;
        sprite->y0 = sprite_y;
        break;
    }
    case 1:
    {
        s32 sprite_x;
        s32 sprite_y;

        sprite_x = D_8004FD04[map_x + map_y * 6].x;
        sprite_y = D_8004FD04[map_x + map_y * 6].y;
        depth = 0xAE - map_y;
        sprite->x0 = sprite_x;
        sprite->y0 = sprite_y;
        break;
    }
    default:
        return;
    }

    *(u32*)&sprite->r0 = 0x808080;

    {
        s32 texture;
        s32 u_index;
        s32 v_index;

        texture = D_800D036C[marker->texture_index];
        u_index = texture & 7;
        sprite->u0 = u_index << 5;
        v_index = texture / 8;
        sprite->v0 = v_index << 5;

        switch (v_index)
        {
        case 0:
            sprite->clut = (u_index << 6) | 0x582E;
            type = 12;
            break;
        case 1:
            sprite->clut = (u_index << 6) | 0x5A2E;
            type = 12;
            break;
        case 2:
            sprite->clut = (u_index << 6) | 0x5C2E;
            type = 12;
            break;
        case 3:
            sprite->clut = (u_index << 6) | 0x5E2E;
            type = 12;
            break;
        }
    }

    *(u32*)&sprite->w = PACK_U16_PAIR(32, 32);
    setSprt(sprite);
    setSemiTrans(sprite, 1);

    addPrim(&D_801398EC->ordering_table[depth], sprite);

    if (D_800D921C < 0x7D00)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
    }

    func_8006534C(type, depth);
}

/**
 * @brief Update and render an animated world-map sprite.
 * @param x World-map X cell coordinate.
 * @param y World-map Y cell coordinate.
 * @param state Animation and rendering state for the sprite.
 * @param resource_index Index of the sprite resource and texture set.
 */
void func_800561F8(s32 x, s32 y, WmapSpriteState* state, s32 resource_index)
{
typedef struct
{
    u8 _pad00[0x70];
    u_long ordering_table[0xB3];
    u8* packet_cursor;
} WmapRenderContext;

typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
} WmapProjection;

typedef struct
{
    u8 _pad00[0x10];
    u8* animation_data;
} WmapSpriteResource;



typedef struct
{
    u8 sequence_id;
    u8 duration;
    u8 _pad02[2];
} WmapAnimationEntry;

typedef struct
{
    s8 x;
    s8 y;
    u8 u;
    u8 v;
    u8 width;
    u8 height;
    s8 texture_index;
    u8 _pad07[2];
    u8 blend_mode;
    u8 _pad0A[2];
} WmapQuadData;

typedef struct
{
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
    s16 x3;
    s16 y3;
} WmapQuadScale;

typedef struct
{
    u8 _pad00[2];
    u8 v_offset;
    u8 _pad03[5];
    u16 clut[8];
    u16 tpage_x;
    u16 tpage_y;
} WmapTextureInfo;

extern s32 D_800500B4[];
extern WmapTextureInfo D_800CBBE8[];
extern WmapQuadScale D_800CBDC4[];
extern s32 D_800D921C;
extern WmapSpriteResource D_8011CF88[];
extern WmapRenderContext* D_801398EC;
extern WmapProjection D_80139950;


typedef struct
{
    SVECTOR position;
    volatile s32 blend_mode;
    DVECTOR screen;
    s32 depth;
} WmapSpriteLocals;

    WmapSpriteResource* resource;
    WmapQuadScale* scale;
    WmapTextureInfo* texture;
    WmapTextureInfo* texture_base;
    WmapAnimationEntry* animation;
    WmapQuadData* quad;
    POLY_FT4* packet;
    WmapSpriteLocals locals;
    DVECTOR* screen_ptr;
    s32 depth_index;
    s32 ot_depth;
    s32 count;
    s32 sequence_id;
    s32 sequence_end;
    s32 phase_delta;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
    u8* animation_data;
    u32 address_mask;
    u32 tag_mask;

    phase_delta = D_800500B4[state->phase_mode];
    state->phase += phase_delta;
    resource = &D_8011CF88[resource_index];

    if ((s8)state->phase >= 15)
    {
        state->phase = 15;
        state->phase_mode = 2;
    }

    if ((s8)state->phase <= 0)
    {
        state->phase_mode = 1;
        return;
    }

    projection_scale = D_80139950.projection_scale;
    locals.position.vx = (((x - 1) * 0xA0 - (D_80139950.x * 0x14000) / projection_scale) * 0x6000) / projection_scale;
    locals.position.vy = (((y - 1) * 0xA0 - (D_80139950.y * 0x14000) / projection_scale) * 0x6000) / projection_scale;
    locals.position.vz = 0;

    gte_ldv0(&locals.position);
    gte_rtps();
    {
        DVECTOR* projected_screen;

        projected_screen = &locals.screen;
        gte_stsxy(projected_screen);
        gte_stszotz(&locals.depth);

        if (((u16)projected_screen->vy << 16) < 0)
        {
            return;
        }
    }

    animation_data = resource->animation_data;
    if (state->previous_frame_index != state->frame_index)
    {
        state->previous_frame_index = state->frame_index;
        animation_offset = *(s16*)(animation_data + state->frame_index * 2);
        state->frame_timer = 1;
        state->animation_start = animation_data + animation_offset;
        state->animation_cursor = state->animation_start;
    }

    sequence_end = 0xFF;
    if (state->frame_timer != sequence_end)
    {
        state->frame_timer--;
    }

    if (state->frame_timer == 0)
    {
        animation = (WmapAnimationEntry*)state->animation_cursor;
        sequence_id = animation->sequence_id;
        state->frame_timer = animation->duration;
        if (sequence_id == sequence_end)
        {
            animation = (WmapAnimationEntry*)state->animation_start;
            state->animation_cursor = (u8*)animation;
            sequence_id = animation->sequence_id;
            state->frame_timer = animation->duration;
        }
        state->animation_cursor += sizeof(WmapAnimationEntry);
        state->quad_data = (s8*)(animation_data + *(s16*)(animation_data + 0x40 + sequence_id * 2));
    }

    quad = (WmapQuadData*)state->quad_data;
    count = *(s8*)quad;
    quad = (WmapQuadData*)((s8*)quad + 1);
    if ((u32)(count - 1) >= 32)
    {
        func_80064F14(animation_data, sequence_end, projection_scale, resource);
        return;
    }

    depth_index = (0x1B91 - locals.depth) / 4;
    ot_depth = depth_index + 0x2E;
    if ((u32)depth_index >= 0x81)
    {
        ot_depth = 0x2E;
    }

    do
    {
        packet = (POLY_FT4*)D_801398EC->packet_cursor;
        screen_ptr = &locals.screen;
        texture_base = D_800CBBE8;
        address_mask = 0x00FFFFFF;
        tag_mask = 0xFF000000;
        scale = &D_800CBDC4[(s8)state->phase];

        packet->x0 = locals.screen.vx + ((quad->x * scale->x0) >> 8);
        packet->x1 = locals.screen.vx + (((quad->x + (s8)quad->width) * scale->x1) >> 8);
        packet->x2 = locals.screen.vx + ((quad->x * scale->x2) >> 8);
        packet->x3 = locals.screen.vx + (((quad->x + (s8)quad->width) * scale->x3) >> 8);
        packet->y0 = screen_ptr->vy + ((quad->y * scale->y0) >> 8);
        packet->y1 = screen_ptr->vy + ((quad->y * scale->y1) >> 8);
        packet->y2 = screen_ptr->vy + (((quad->y + (s8)quad->height) * scale->y2) >> 8);
        packet->y3 = screen_ptr->vy + (((quad->y + (s8)quad->height) * scale->y3) >> 8);

        texture_offset = resource_index * sizeof(WmapTextureInfo);
        texture = (WmapTextureInfo*)((u8*)texture_base + texture_offset);
        packet->u0 = quad->u;
        packet->u1 = quad->u + quad->width;
        packet->u2 = quad->u;
        packet->u3 = quad->u + quad->width;
        packet->v0 = quad->v + texture->v_offset;
        packet->v1 = quad->v + texture->v_offset;
        packet->v2 = quad->v + quad->height + texture->v_offset;
        packet->v3 = quad->v + quad->height + texture->v_offset;
        *(u32*)&packet->r0 = 0x80808080;
        locals.blend_mode = (s8)quad->blend_mode;
        packet->tpage = getTPage(0, quad->blend_mode & 3, texture->tpage_x, texture->tpage_y);
        packet->clut = *(u16*)((u8*)texture_base + (quad->texture_index * 2 + texture_offset) + 8);
        setPolyFT4(packet);
        setSemiTrans(packet, 1);
        packet->tag = (packet->tag & tag_mask) | (ot_depth[D_801398EC->ordering_table] & address_mask);
        ot_depth[D_801398EC->ordering_table] = (ot_depth[D_801398EC->ordering_table] & tag_mask) | ((u32)packet & address_mask);

        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(POLY_FT4);
            D_801398EC->packet_cursor += sizeof(POLY_FT4);
        }

        quad++;
        count--;
    } while (count != 0);
}

/**
 * @brief Update world-map object display states for the visible map grid.
 */
void func_80056824(void)
{
typedef struct
{
    s32 object_id;
    s16 flag;
    u8 pad06[0x22];
} WmapGridCell;

typedef struct
{
    u8 pad00[4];
    u8 state;
    u8 phase;
    s8 transition;
    u8 pad07[0x25];
} WmapObject;

typedef struct
{
    u8 pad00[0xC];
    s32 active;
    u8 pad10[4];
} WmapPartRecord;

extern s32 D_800DBE70;
extern WmapPartRecord D_8011CF88[];
extern s32 D_8011D4FC;
extern SVECTOR D_80139278;
extern WmapGridCell D_80139290[][6];
extern s32 D_8013986C;
extern SVECTOR D_801398C8;
extern s32 D_80139958;
extern s32 D_80139978;
extern WmapObject D_80182248[];
extern VECTOR D_80182D48;
extern VECTOR D_80182DC0;
extern s32 D_801ADAE0;

    MATRIX matrix;
    VECTOR translation;
    SVECTOR rotation;
    VECTOR world_position;
    s32 x;
    s32 y;
    s32 object_id;
    s32 state;
    s32 part_index;
    s32 distance_state;

    rotation.vx = D_80139278.vx + D_801398C8.vx;
    rotation.vy = D_80139278.vy + D_801398C8.vy;
    rotation.vz = D_80139278.vz + D_801398C8.vz;

    world_position.vx = D_80182DC0.vx + D_80182D48.vx;
    world_position.vy = D_80182DC0.vy + D_80182D48.vy;
    world_position.vz = D_80182DC0.vz + D_80182D48.vz;

    translation = world_position;
    translation.vz = (translation.vz * D_80139958) / 0x6000;

    RotMatrix(&rotation, &matrix);
    TransMatrix(&matrix, &translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);

    for (y = 0; y < 6; y++)
    {
        for (x = 0; x < 6; x++)
        {
            object_id = D_80139290[x][y].object_id;

            if (D_8013986C == 1)
            {
                state = 1;
            }
            else if (object_id == D_80139978)
            {
                state = 2;
            }
            else
            {
                s32 base_state;

                distance_state = func_80058400(x * 0x30, y * 0x30);
                base_state = D_800DBE70;
                if (base_state != 2)
                {
                    state = base_state;
                }
                if ((base_state == 2) || (distance_state < state))
                {
                    state = distance_state;
                }
            }

            if (object_id >= 0x100)
            {
                state = 5;
                object_id &= 0xFF;
            }

            if (object_id != 0xFF)
            {
                WmapObject* current_object;

                current_object = &D_80182248[object_id];
                if (current_object->transition >= 0x10)
                {
                    current_object->transition = 0xF;
                }
                else if (current_object->transition < 0)
                {
                    current_object->transition = 0;
                }

                if (state == 5)
                {
                    current_object->state = 2;
                    current_object->phase = 2;
                    current_object->transition = 0xF;
                }
                else if (state != current_object->state)
                {
                    switch (current_object->state)
                    {
                    case 0:
                        current_object->state = state;
                        if (state == 1)
                        {
                            current_object->phase = state;
                        }
                        else
                        {
                            current_object->phase = 3;
                        }
                        break;
                    case 1:
                        current_object->state = state;
                        if (state != 0)
                        {
                            current_object->phase = 3;
                        }
                        else
                        {
                            current_object->phase = 0;
                            current_object->transition = 0;
                        }
                        break;
                    case 2:
                        current_object->state = state;
                        if (state != 0)
                        {
                            current_object->phase = 4;
                        }
                        else
                        {
                            current_object->phase = 0;
                            current_object->transition = 0;
                        }
                        break;
                    }
                }

                if (state != 0)
                {
                    WmapObject* active_object;

                    active_object = &D_80182248[object_id];
                    if (active_object->phase != 1)
                    {
                        part_index = func_80055BB0(active_object);
                        if ((part_index != -1) && (D_8011CF88[part_index].active == 0))
                        {
                            func_800561F8(x, y, active_object, part_index);
                        }
                        else
                        {
                            func_80055E5C(x, y, active_object);
                        }
                    }
                    else
                    {
                        func_80055E5C(x, y, active_object);
                    }
                }
            }

            if ((D_8011D4FC == -1) || (D_801ADAE0 != 0))
            {
                state = 0;
            }
            func_8005833C(x, y, state);

            if ((D_80139290[x][y].flag != 0) && (D_8013986C == 0))
            {
                func_80056C30(x, y);
            }
        }
    }
}

/**
 * @brief Render the animated world-map quad set for one map cell.
 * @param x World-map grid X coordinate.
 * @param y World-map grid Y coordinate.
 */
void func_80056C30(s32 x, s32 y)
{
typedef struct
{
    u8 pad00[4];
    s16 state;
    s16 frame;
    u8 pad08[0x14];
} WmapEffectCell;

typedef struct
{
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
} WmapQuadVertex;

typedef struct
{
    WmapQuadVertex vertices[4];
    CVECTOR colors[4];
} WmapQuadTemplate;

typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjection;

typedef struct
{
    u8 pad000[0x328];
    u_long ot_entry;
    u8 pad32C[0x10];
    POLY_G4* prim_cursor;
} WmapRenderState;

extern s32 D_800500C8[];
extern WmapQuadTemplate D_800CB8E8[16];
extern s32 D_800D921C;
extern WmapEffectCell D_8011D108[6][6];
extern s32 D_8011D4FC;
extern WmapRenderState* D_801398EC;
extern WmapProjection D_80139950;

    WmapEffectCell* cell;
    WmapQuadVertex base;
    WmapQuadVertex transformed[4];
    s32 sxy0;
    s32 sxy1;
    s32 sxy2;
    s32 sxy3;
    s32 projected_x;
    s32 projected_y;
    s32 i;
    s32 color_frame;
    s32 frame_delta;

    cell = &D_8011D108[x][y];
    frame_delta = D_800500C8[cell->state];
    cell->frame += frame_delta;
    if (cell->frame >= 0x11)
    {
        cell->frame = 0x10;
    }
    else if (cell->frame < 0)
    {
        cell->frame = 0;
    }

    if (cell->frame >= 0xF)
    {
        cell->state = 2;
    }

    if (cell->frame <= 0)
    {
        cell->state = 1;
        return;
    }

    projected_x = (D_80139950.x * 0x14000) / D_80139950.scale;
    projected_y = (D_80139950.y * 0x14000) / D_80139950.scale;
    base.x = ((((x - 1) * 0xA0) - projected_x) * 0x6000) / D_80139950.scale + 0xA;
    base.y = ((((y - 1) * 0xA0) - projected_y) * 0x6000) / D_80139950.scale + 0xC;
    base.z = 0;

    color_frame = cell->frame * 7;

    for (i = 0; i < 16; i++)
    {
        WmapQuadTemplate* source;
        POLY_G4* poly;

        source = &D_800CB8E8[i];
        poly = D_801398EC->prim_cursor;

        transformed[0].x = base.x + source->vertices[0].x;
        transformed[0].y = base.y + source->vertices[0].y;
        transformed[0].z = base.z + source->vertices[0].z;
        transformed[1].x = base.x + source->vertices[1].x;
        transformed[1].y = base.y + source->vertices[1].y;
        transformed[1].z = base.z + source->vertices[1].z;
        transformed[2].x = base.x + source->vertices[2].x;
        transformed[2].y = base.y + source->vertices[2].y;
        transformed[2].z = base.z + source->vertices[2].z;
        transformed[3].x = base.x + source->vertices[3].x;
        transformed[3].y = base.y + source->vertices[3].y;
        transformed[3].z = base.z + source->vertices[3].z;

        gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
        gte_rtpt();

        *(u32*)&poly->r0 = func_8006CF40(source->colors[0], color_frame);
        *(u32*)&poly->r1 = func_8006CF40(source->colors[1], color_frame);
        *(u32*)&poly->r2 = func_8006CF40(source->colors[2], color_frame);
        *(u32*)&poly->r3 = func_8006CF40(source->colors[3], color_frame);

        if ((D_8011D4FC == 3) || (D_8011D4FC == 7) || (D_8011D4FC == 0x1B))
        {
            poly->r0 = poly->r1 = poly->r2 = poly->r3 = 0;
        }

        gte_stsxy3(&sxy0, &sxy1, &sxy2);
        gte_ldv0(&transformed[3]);
        gte_rtps();

        *(u32*)&poly->x0 = sxy0;
        *(u32*)&poly->x1 = sxy1;
        *(u32*)&poly->x2 = sxy2;
        gte_stsxy(&sxy3);
        *(u32*)&poly->x3 = sxy3;

        setlen(poly, 8);
        setcode(poly, 0x3A);
        addPrim(&D_801398EC->ot_entry, poly);

        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(POLY_G4);
            D_801398EC->prim_cursor++;
        }
    }

    func_8006534C(0xAE, 0xAE);
}

/**
 * @brief Reset world-map objects and effect cells to their initial state.
 */
void func_800571A4(void)
{
typedef struct
{
    s16 unk00;
    s16 unk02;
    u8 state;
    u8 phase;
    s8 transition;
    u8 pad07[7];
    s16 unk0e;
    s16 unk10;
    u8 pad12[0xE];
    s16 unk20;
    u8 pad22[0xA];
} WmapObject;

typedef struct
{
    s16 unk00;
    s16 unk02;
    s16 state;
    s16 frame;
    s32 unk08;
    s32 unk0c;
    s16 unk10;
    u8 pad12[2];
    s32 unk14;
    s32 unk18;
} WmapEffectCell;

extern WmapObject D_80182248[64];
extern WmapEffectCell D_8011D108[6][6];

    s32 i;
    s32 x;
    s32 y;

    for (i = 0; i < 64; i++)
    {
        D_80182248[i].unk00 = i;
        D_80182248[i].unk02 = i;
        D_80182248[i].unk10 = 0;
        D_80182248[i].unk0e = 0;
        D_80182248[i].transition = 0;
        D_80182248[i].phase = 0;
        D_80182248[i].state = 0;
        D_80182248[i].unk20 = 0;
    }

    for (y = 0; y < 6; y++)
    {
        for (x = 0; x < 6; x++)
        {
            WmapEffectCell cell;

            D_8011D108[x][y].unk00 = i;
            D_8011D108[x][y].frame = 0;
            D_8011D108[x][y].state = 0;
            D_8011D108[x][y].unk02 = 0;
            D_8011D108[x][y].unk10 = 0;
            cell = D_8011D108[x][y];
        }
    }
}

/**
 * @brief Advance selected world-map animation timers and draw all eight sprites.
 * @param selected_index Animation index to advance, or -1 to advance every animation.
 */
void func_80057274(s32 selected_index)
{
typedef struct
{
    u8 pad_000[0x70];
    u_long ordering_table[179];
    void* prim_cursor;
} WmapRenderState;

extern SPRT D_8004FD94[];
extern u8 D_800CC0C4[];
extern u8 D_800CC0D0[];
extern u8 D_800D7CD0[];
extern u8 D_800D7CD8[];
extern s32 D_800D921C;
extern WmapRenderState* D_801398EC;
extern s32 D_8013B268;

    s32 i;

    for (i = 0; i < 8; i++)
    {
        SPRT* sprite;
        u8 frame;

        if (selected_index == -1 || selected_index == i)
        {
            D_800D7CD0[i]--;
        }

        if ((s8)D_800D7CD0[i] <= 0)
        {
            frame = D_800D7CD8[i] + 2;
            D_800D7CD8[i] = frame;
            if ((s8)frame >= D_800CC0C4[i + 1])
            {
                D_800D7CD8[i] = D_800CC0C4[i];
            }
            D_800D7CD0[i] = D_800CC0D0[(s8)D_800D7CD8[i] + 1];
        }

        sprite = D_801398EC->prim_cursor;
        *sprite = D_8004FD94[i];
        sprite->r0 = sprite->g0 = sprite->b0 = D_8013B268;
        sprite->u0 = (D_800CC0D0[(s8)D_800D7CD8[i]] % 5) * 24;
        sprite->v0 = (D_800CC0D0[(s8)D_800D7CD8[i]] / 5) * 24;

        if (D_8013B268 < 0x40)
        {
            setSemiTrans(sprite, 1);
        }

        addPrim(&D_801398EC->ordering_table[4], sprite);

        if (D_800D921C < 0x7D00)
        {
            D_801398EC->prim_cursor = (u8*)D_801398EC->prim_cursor + sizeof(SPRT);
            D_800D921C += sizeof(SPRT);
        }
    }

    func_8006534C(0x3D, 4);
}

/**
 * @brief Render the current world-map sprite set.
 */
void func_800574D0(void)
{
typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjection;

typedef struct
{
    u32 packed_xy;
    u8 pad04[0x10];
} WmapSpritePosition;

typedef struct
{
    u8 pad000[0x74];
    u_long ot_entry;
    u8 pad078[0x2C4];
    SPRT* prim_cursor;
} WmapRenderState;

extern WmapSpritePosition D_8004FD9C[8];
extern SPRT D_8004FE34[16];
extern SPRT D_8004FF74[16];
extern s32 D_800D921C;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_80129550;
extern WmapRenderState* D_801398EC;
extern WmapProjection D_80139950;
extern s32 D_8013B268;

void func_8005C404(s32 x, s32 y, s32 mode, s32 map_x, s32 map_y, s32* indices);
void func_8005D6B8(s32 x, s32 y, s32* indices);
void func_8005D7A0(s32 value, s32* indices);
s32 func_8005D8FC(void);

    s32 sprite_indices[8];
    s32 x;
    s32 y;
    s32 i;
    s32 color;

    if (D_8013B268 == 0)
    {
        return;
    }

    x = D_80139950.x / 48 + D_800DCEEC;
    y = D_80139950.y / 48 + D_800DCEF0;

    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    if (x >= 6)
    {
        x = 5;
    }
    if (y >= 6)
    {
        y = 5;
    }

    color = D_8013B268 + (D_8011CF74 & 8);
    if (color < 0)
    {
        color = 0;
    }

    if (D_8011CF18 == 2)
    {
        func_8005D7A0(func_8005D8FC(), sprite_indices);
    }
    else if (D_80129550 == 1)
    {
        func_8005C404(x, y, D_8011D4FC, x, y, sprite_indices);
    }
    else
    {
        func_8005D6B8(x, y, sprite_indices);
    }

    i = 0;
    do
    {
        SPRT* primary_sprite;
        SPRT* secondary_sprite;

        primary_sprite = D_801398EC->prim_cursor;
        *primary_sprite = D_8004FF74[sprite_indices[i]];
        primary_sprite->r0 = primary_sprite->g0 = primary_sprite->b0 = color;
        *(u32*)&primary_sprite->x0 = D_8004FD9C[i].packed_xy;
        primary_sprite->clut = getClut(0x2E0, i + 0x1A8);
        setSemiTrans(primary_sprite, 1);
        addPrim(&D_801398EC->ot_entry, primary_sprite);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->prim_cursor++;
        }

        secondary_sprite = D_801398EC->prim_cursor;
        *secondary_sprite = D_8004FE34[sprite_indices[i]];
        secondary_sprite->r0 = secondary_sprite->g0 = secondary_sprite->b0 = D_8013B268;
        *(u32*)&secondary_sprite->x0 = D_8004FD9C[i].packed_xy;
        secondary_sprite->clut = getClut(0x2E0, i + 0x1A0);
        secondary_sprite->x0 += 4;
        secondary_sprite->y0 += 0x14;
        if (D_8013B268 < 0x40)
        {
            setSemiTrans(secondary_sprite, 1);
        }
        addPrim(&D_801398EC->ot_entry, secondary_sprite);
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->prim_cursor++;
        }

        i++;
    } while (i < 8);

    func_8006534C(0x3D, 1);
}

void func_8005784C(s32 arg0)
{
/* Partial WMAP decompilation: 86.301650% (gcc280_g0). */

typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapProjection;

typedef struct
{
    u8 pad00[4];
    s16 unk04;
    u8 pad06[0x22];
} WmapTileEntry;

typedef struct
{
    u8 pad000[0x74];
    u_long ot_entry;
    u8 pad078[0x2C4];
    SPRT* prim_cursor;
} WmapRenderState;

extern s32 D_8004FC74[];
extern SPRT D_8004FE34[16];
extern s32 D_800D921C;
extern WmapProjection D_800DCEC8;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_80129550;
extern WmapTileEntry D_80139290[][6];
extern WmapRenderState* D_801398EC;
extern s32 D_8013B268;

void func_8005C404(s32 x, s32 y, s32 mode, s32 map_x, s32 map_y, s32* indices);
void func_8005D6B8(s32 x, s32 y, s32* indices);

    s32 sprite_indices[8];
    s32 target_x;
    s32 target_y;
    s32 cur_x;
    s32 cur_y;
    s32 base_index;
    SPRT* sprite;

    if (D_8013B268 == 0)
    {
        return;
    }

    target_x = D_800DCEC8.x / 48 + D_800DCEEC;
    target_y = D_800DCEC8.y / 48 + D_800DCEF0;

    cur_y = 0;
    base_index = 0;
    do
    {
        cur_x = 0;
        do
        {
            if (D_80129550 == 1 && D_80139290[target_x][target_y].unk04 != 0)
            {
                func_8005C404(cur_x, cur_y, D_8011D4FC, target_x, target_y, sprite_indices);
            }
            else
            {
                func_8005D6B8(cur_x, cur_y, sprite_indices);
            }

            if (cur_x == target_x && cur_y == target_y && (D_8011CF74 & 4))
            {
                sprite = D_801398EC->prim_cursor;
                *sprite = D_8004FE34[sprite_indices[arg0]];
                *(u32*)&sprite->x0 = D_8004FC74[base_index + cur_x];
                sprite->u0 = 0xD0;
                sprite->v0 = 0;
                sprite->r0 = sprite->g0 = sprite->b0 = (u8)D_8013B268;
                sprite->clut = 0x6AAE;
                if (D_8013B268 < 0x40)
                {
                    setSemiTrans(sprite, 1);
                }
                addPrim(&D_801398EC->ot_entry, sprite);
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->prim_cursor++;
                }
            }

            sprite = D_801398EC->prim_cursor;
            *sprite = D_8004FE34[sprite_indices[arg0]];
            *(u32*)&sprite->x0 = D_8004FC74[base_index + cur_x];
            sprite->r0 = sprite->g0 = sprite->b0 = (u8)D_8013B268;
            sprite->clut = getClut(0x2E0, arg0 + 0x1A0);
            if (D_8013B268 < 0x40)
            {
                setSemiTrans(sprite, 1);
            }
            addPrim(&D_801398EC->ot_entry, sprite);
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += sizeof(SPRT);
                D_801398EC->prim_cursor++;
            }

            cur_x++;
        } while (cur_x < 6);

        cur_y++;
        base_index += 6;
    } while (cur_y < 6);

    func_8006534C(0x3D, 1);
}

/** @brief Update map rendering and approach the requested fade intensity. */
void func_80057C14(void)
{
extern s32 D_800DCF04;
extern s32 D_8011CF74;
extern s32 D_80139218;
extern s32 D_8013986C;
extern s32 D_8013B268;
extern s32 D_801ADAEC;

    if (D_8013B268 != 0)
    {
        switch (D_8013986C)
        {
        case 0:
            func_80057274(-1);
            func_800574D0();
            func_80057D2C();
            break;
        case 1:
            if (D_800DCF04 != 0)
            {
                func_80057274(D_800DCF04 - 1);
                func_8005784C(D_800DCF04 - 1);
            }
            break;
        }
    }
    if (D_8011CF74 & 1)
    {
        D_80139218 = (D_80139218 + 1) & 7;
    }
    if (D_8013B268 > D_801ADAEC)
    {
        D_8013B268 -= 8;
    }
    if (D_8013B268 < D_801ADAEC)
    {
        D_8013B268 += 8;
    }
    if (D_8013B268 >= 0x81)
    {
        D_801ADAEC = 0x80;
        D_8013B268 = 0x80;
    }
}

/** @brief Draw the enabled map information labels and substitute dynamic glyphs. */
void func_80057D2C(void)
{
/* Partial WMAP decompilation: 98.924730% (gcc280_g0). */

typedef struct
{
    u8 u, v, width, height;
    u16 palette;
    u8 pad_06[2];
} WmapGlyph;
typedef struct
{
    u16 x;
    u8 y, glyph;
} WmapGlyphPlacement;
typedef struct
{
    u8 pad_00[0x70];
    u32 ordering_table[179];
    u8 *packet_cursor;
} WmapRenderContext;
typedef struct
{
    s32 x, y;
} WmapPosition;
extern WmapGlyph D_800CB2F8[];
extern WmapGlyphPlacement D_800CB3D8[];
extern s32 D_800CB8BC[];
extern s32 D_800D7CE0[];
extern s32 D_800D9160;
extern s32 D_800D921C;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011D4FC;
extern s32 D_8013922C;
extern s32 D_801398C0;
extern s32 D_801398D0;
extern WmapRenderContext *D_801398EC;
extern WmapPosition D_80139950;
extern void func_8005D018(s32, s32, s32, s32 *, s32 *, s32);

    s32 dynamic_index;
    s32 group;
    s32 i;
    s32 end;
    s32 next_offset;
    s32 glyph;
    u16 palette;
    WmapGlyphPlacement *placement;
    WmapGlyph *image;
    SPRT *packet;

    if ((D_8013922C & 0xF000) || D_801398D0 != 0)
    {
        D_800D9160 = 0;
    }
    if (!(D_801398C0 & 0x10) || D_8011D4FC == -1)
    {
        D_800D9160 = 0;
        return;
    }
    if (D_800D9160 == 0)
    {
        func_8005D018(D_800DCEEC + D_800DCEF0 * 3, D_80139950.x / 48 + D_800DCEEC,
                     D_80139950.y / 48 + D_800DCEF0, &D_800D9160, D_800D7CE0, D_8011D4FC);
    }
    dynamic_index = 0;
    for (group = 0, next_offset = 4; group < 10; next_offset += 4, group++)
    {
        if ((D_800D9160 >> group) & 1)
        {
            i = D_800CB8BC[group];
            end = *(s32 *)((u8 *)D_800CB8BC + next_offset);
            for (; i < end; i++)
            {
                placement = &D_800CB3D8[i];
                glyph = placement->glyph;
                if (glyph == 255)
                {
                    glyph = D_800D7CE0[dynamic_index++];
                }
                packet = (SPRT *)D_801398EC->packet_cursor;
                packet->x0 = placement->x;
                image = &D_800CB2F8[glyph];
                packet->y0 = placement->y;
                packet->u0 = image->u + 192;
                packet->v0 = image->v + 96;
                packet->w = image->width;
                packet->h = image->height;
                palette = image->palette;
                setlen(packet, 4);
                packet->r0 = 128;
                packet->g0 = 128;
                packet->b0 = 128;
                packet->code = 100;
                packet->clut = ((palette + 432) << 6) | 46;
                addPrim(&D_801398EC->ordering_table[2], packet);
                if (D_800D921C < 32000)
                {
                    D_800D921C += 20;
                    D_801398EC->packet_cursor += 20;
                }
            }
        }
    }
    func_8006534C(59, 2);
}

/**
 * @brief Clamp the actor intensity and select the requested mode transition.
 * @param index Actor index.
 * @param mode Requested mode; five forces mode two at full intensity.
 */
void func_80058014(s32 index, s32 mode)
{
/** @brief Actor mode, transition, and signed intensity within a 44-byte record. */
typedef struct
{
    u8 pad_00[4];
    u8 mode;
    u8 transition;
    s8 intensity;
    u8 pad_07[0x25];
} WmapActor;

extern WmapActor D_80182248[];

    s8 intensity;
    u8 old_mode;
    WmapActor *actor;

    actor = &D_80182248[index];
    intensity = actor->intensity;
    if (intensity >= 0x10)
    {
        actor->intensity = 0xF;
    }
    else if (intensity < 0)
    {
        actor->intensity = 0;
    }
    if (mode == 5)
    {
        actor->mode = 2U;
        actor->transition = 2U;
        actor->intensity = 0xF;
        return;
    }
    old_mode = actor->mode;
    if (mode != old_mode)
    {
        switch (old_mode)
        {                       
        case 0:
            actor->mode = mode;
            if (mode == 1)
            {
                actor->transition = mode;
                return;
            }
            actor->transition = 3U;
            return;
        case 1:
            actor->mode = mode;
            if (mode != 0)
            {
                actor->transition = 3;
                return;
            }
            actor->transition = 0;
            actor->intensity = 0;
            return;
        case 2:
            actor->mode = mode;
            if (mode != 0)
            {
                actor->transition = 4U;
                return;
            }
            actor->transition = 0U;
           
            actor->intensity = 0;
            return;
        }
    }
}

/**
 * @brief Upload a resource into a cache slot and record its image pointer and age.
 * @param resource Resource selecting the packed image data.
 * @param slot Destination cache slot.
 */
void func_80058110(WmapResource *resource, s32 slot)
{
/** @brief Resource identifier and cache-slot state for a world-map object. */


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

/**
 * @brief Draw a cached map resource, or its marker while the image is unavailable.
 * @param map_x Map-grid X coordinate.
 * @param map_y Map-grid Y coordinate.
 * @param resource_index Index of the world-map resource to draw.
 */
void func_800581A0(s32 map_x, s32 map_y, s32 resource_index)
{
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

/** @brief Initialize the 16 cache slots while preserving their image pointers. */
void func_80058260(void)
{
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

    u32 slot_index;
    for (slot_index = 0; slot_index < 16U; slot_index++)
    {
        D_8011CF88[slot_index].slot_index = slot_index;
        D_8011CF88[slot_index].resource_id = -1;
        D_8011CF88[slot_index].age = 0;
        D_8011CF88[slot_index].busy = 0;
    }
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_80058298(void)
{
}

/**
 * @brief Limit the current result by the world-map state unless that state is two.
 * @return The current result or its lower state limit.
 */
s32 func_800582A0(void)
{
extern s32 D_800DBE70;

    s32 current = func_80058400();
    if (D_800DBE70 != 2)
    {
        s32 limit = D_800DBE70;
        if (current >= limit)
        {
            return limit;
        }
    }
    return current;
}

/** @brief Enable eight slots and copy their initial table values. */
void func_800582E8(void)
{
extern u8 D_800CC0C4[];
extern s8 D_800D7CD0[];
extern u8 D_800D7CD8[];

    s32 index;
    for (index = 0; index < 8; index++)
    {
        D_800D7CD0[index] = 1;
        D_800D7CD8[index] = D_800CC0C4[index + 1];
    }
}

/**
 * @brief Select a cell effect transition when its requested mode changes.
 * @param x Map-grid X coordinate.
 * @param y Map-grid Y coordinate.
 * @param mode Requested mode: zero, one, or two.
 */
void func_8005833C(s32 x, s32 y, s32 mode)
{
/** @brief Per-cell animation state and frame counters. */
typedef struct
{
    s16 unk00;
    s16 unk02;
    s16 state;
    s16 frame;
    s32 unk08;
    s32 unk0c;
    s16 unk10;
    u8 pad12[2];
    s32 unk14;
    s32 unk18;
} WmapEffectCell;

extern WmapEffectCell D_8011D108[6][6];

    WmapEffectCell *cell;

    cell = &D_8011D108[x][y];
    if (mode != cell->unk02)
    {
        switch (cell->unk02)
        {
        case 0:
            cell->unk02 = mode;
            if (mode == 1)
            {
                cell->state = mode;
                return;
            }
            cell->state = 3;
            return;
        case 1:
            cell->unk02 = mode;
            if (mode != 0)
            {
                cell->state = 3;
                return;
            }
            cell->state = 0;
            cell->frame = 0;
            return;
        case 2:
            cell->unk02 = mode;
            if (mode != 0)
            {
                cell->state = 4;
                return;
            }
            cell->state = 0;
            cell->frame = 0;
            return;
        }
    }
}

/**
 * @brief Classify a point against two world-map coordinate regions.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @return Region code: zero, one, or two.
 */
s32 func_80058400(s32 x, s32 y)
{
extern s32 D_8013986C;
extern s32 D_80139950[];

    s32 dy;
    u32 dx;

    dx = x - D_80139950[0];
    dy = y - D_80139950[1];
    if ((dx < 0x61U) && (dy >= 0) && (dy < 0x61))
    {
        if ((D_8013986C == 3) || (D_8013986C == 1))
        {
            return 0;
        }
        return 2;
    }
    if ((u32)(dx - 16) < 97 && dy >= 16 && dy < 113)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Return the enabled result for this world-map callback.
 * @return Always one.
 */
s32 func_80058488(void)
{
    return 1;
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_80058490(void)
{
}

/**
 * @brief Clear three world-map state values.
 */
void func_80058498(void)
{
extern s32 D_800D7D60;
extern s32 D_800D7D64;
extern s32 D_800D7D68;

    D_800D7D60 = 0;
    D_800D7D64 = 0;
    D_800D7D68 = 0;
}
