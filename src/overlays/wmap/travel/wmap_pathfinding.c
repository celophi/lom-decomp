#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_pathfinding.h"
#include "common/saved_game.h"
#include "main/field_entry.h"
#include <abs.h>

/**
 * @file wmap_pathfinding.c
 * @brief World-map node reachability, queued node-swap state updates, and BFS
 *        route building.
 */

#define WMAP_NODE_RECORD_SIZE 12

#define WMAP_ROUTE_NODE_COUNT 64
#define WMAP_ROUTE_UNVISITED_DISTANCE 100
#define WMAP_INVALID_NODE 0xFF
#define WMAP_NODE_X(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] & 0xF))
#define WMAP_NODE_Y(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] >> 4))

/**
 * @brief Packed world-map slot record: a 32-bit word whose low bits/nibbles hold
 *        flags and grid coordinates, aliased byte-wise and as bitfields.
 */
typedef union {
    u32 word;
    struct { u8 b0; u8 b1; u8 unk2; u8 unk3; } b;
    struct {
        u32 bit0 : 1;
        u32 bit1 : 1;
        u32 bit2 : 1;
        u32 pad3 : 5;
        u32 nib8 : 4;
        u32 nibC : 4;
        u32 hi : 16;
    } f;
} WmapSlot;

typedef struct { u8 unk0; u8 unk1; u8 unk2; u8 unk3; } WmapQuad;
typedef struct { u8 unk0; u8 unk1; } WmapPair;

extern s32 D_800432BC;
extern u32 D_800432C0[];
extern u8 D_800432C8[];
extern u8 D_800432C9[];
extern u8 D_800432D0[];
extern WmapSlot D_800433DC;
extern WmapSlot D_80043388;
extern WmapQuad D_8004338C;
extern WmapQuad D_800433E0;
extern WmapPair D_80043392;
extern WmapPair D_800433E6;
extern u8 D_80043390;
extern u8 D_80043391;
extern s8 D_800433E4;
extern u8 D_800433E5;
extern s32 D_800460EC;
extern s32 D_800460FC;
extern u8 D_800D01B0[];
extern u8 D_800D01F0[];
extern s16 g_music_track_index;

/**
 * @brief Unlock the encyclopedia entries of the lands and their artifacts from the land records.
 */
void wmap_unlock_land_encyclopedia_entries(void)
{
    s32 i;

    for (i = 0; i < FIELD_ENCYCLOPEDIA_LAND_COUNT; i++)
    {
        if ((D_800432C8[i * WMAP_NODE_RECORD_SIZE] >> 1) & 1)
        {
            SAVED_BIT_SET(g_saved_encyclopedia_bits, i + FIELD_ENCYCLOPEDIA_LAND_BIT);
        }
    }
    if (g_wmap_replacement_land_status & 4)
    {
        D_800460EC |= 0x01000000;
    }
    for (i = 0; i < FIELD_ENCYCLOPEDIA_LAND_COUNT; i++)
    {
        if (D_800432C8[i * WMAP_NODE_RECORD_SIZE] & FIELD_LAND_PLACED)
        {
            SAVED_BIT_SET(g_saved_encyclopedia_bits, i + FIELD_ENCYCLOPEDIA_ARTIFACT_BIT);
        }
        if ((D_800432C8[i * WMAP_NODE_RECORD_SIZE] >> 1) & 1)
        {
            SAVED_BIT_SET(g_saved_encyclopedia_bits, i + FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT);
        }
    }
    if (g_wmap_replacement_land_status & 4)
    {
        D_800460FC |= 0x01000000;
    }
}

/**
 * @brief Commit the land being entered as the world map exits: publish its
 *        field scene and music, then apply the queued land events.
 *
 * Looks up the land at (cell_x, cell_y), derives a slot from the land's
 * packed flag byte, and stores the field entry scene, field music and music
 * track for that slot. It then scans the 0x40-bit pending mask; for each queued
 * event id in the accepted set it clears the pending bit, republishes that
 * event's land scene data, and for ids 0xC / 0xD performs the swap of the two
 * packed slot records (D_800433DC and D_80043388) plus the four-neighbour
 * reachability update.
 *
 * @param cell_x World-map grid X of the lead traveler.
 * @param cell_y World-map grid Y of the lead traveler.
 */
void wmap_commit_land_entry(s32 cell_x, s32 cell_y)
{
    s32 event_id;
    s32 record_x;
    s32 record_y;
    s32 neighbor_land;
    s32 land;
    s32 land_slot;

    wmap_unlock_land_encyclopedia_entries();
    land = wmap_get_land_at_cell(cell_x, cell_y);
    land_slot = (land * 4) + ((D_800432C8[land * WMAP_NODE_RECORD_SIZE] >> 4) & 3);
    g_field_entry_scene.id = g_wmap_land_scenes[land_slot];
    g_field_music_id = g_wmap_land_entry_flags[land_slot];
    g_music_track_index = wmap_get_land_at_cell(cell_x, cell_y);

    for (event_id = 0; event_id < 0x40; event_id++)
    {
        if (SAVED_BIT_IS_SET(D_800432C0, event_id))
        {
            if (event_id == 9 || event_id == 0xA || event_id == 0xB || event_id == 0xC || event_id == 0xD || event_id == 0xE || event_id == 0x18 || event_id == 0 || event_id == 0x1B || event_id == 0x1C)
            {
                D_800432C0[event_id / SAVED_BITS_PER_WORD] &= ~(1 << (event_id % SAVED_BITS_PER_WORD));
                g_music_track_index = D_800D01F0[event_id];
                land = D_800D01B0[event_id];
                land_slot = (land * 4) + ((D_800432C8[land * WMAP_NODE_RECORD_SIZE] >> 4) & 3);
                g_field_entry_scene.id = g_wmap_land_scenes[land_slot];
                g_field_music_id = g_wmap_land_entry_flags[land_slot];
                if (event_id == 0xC)
                {
                    D_80043390 = D_8004338C.unk0;
                    record_x = (u8)((D_800433DC.word >> 8) & 0xF);
                    record_y = (u8)((D_800433DC.word >> 0xC) & 0xF);
                    neighbor_land = wmap_get_land_at_cell(record_x - 1, record_y);
                    D_80043388.f.bit0 = (u8)(D_800433DC.b.b0 & 1);
                    D_80043388.f.bit1 = (u8)((D_800433DC.word >> 1) & 1);
                    D_80043388.word = D_80043388.word | 4;
                    D_80043388.f.nib8 = (u8)((D_800433DC.word >> 8) & 0xF);
                    D_80043388.f.nibC = (u8)((D_800433DC.word >> 0xC) & 0xF);
                    D_80043388.b.unk2 = D_800433DC.b.unk2;
                    D_80043388.b.unk3 = D_800433DC.b.unk3;
                    D_8004338C.unk0 = D_800433E0.unk0;
                    D_8004338C.unk1 = D_800433E0.unk1;
                    D_8004338C.unk2 = D_800433E0.unk2;
                    D_8004338C.unk3 = D_800433E0.unk3;
                    D_80043391 = D_800433E5;
                    D_80043392.unk0 = D_800433E6.unk0;
                    D_80043392.unk1 = D_800433E6.unk1;
                    D_800433DC.word = D_800433DC.word & ~1;
                    D_800433DC.word = D_800433DC.word & ~2;
                    D_800433DC.word = D_800433DC.word & ~4;
                    D_800433DC.word = D_800433DC.word | 0xF00;
                    D_800433DC.word = D_800433DC.word | 0xF000;
                    D_800433DC.b.unk2 = 0;
                    D_800433DC.b.unk3 = 0;
                    D_800433E0.unk0 = 0;
                    D_800433E0.unk1 = 0;
                    D_800433E0.unk2 = 0;
                    D_800433E0.unk3 = 0;
                    D_800433E4 = 0;
                    D_800433E5 = 0;
                    D_800433E6.unk0 = 0;
                    D_800433E6.unk1 = 0;
                }
                if (event_id == 0xD)
                {
                    D_800433DC.f.bit0 = (u8)(D_80043388.b.b0 & 1);
                    D_800433DC.f.bit1 = (u8)((D_80043388.word >> 1) & 1);
                    D_800433DC.word = D_800433DC.word & ~4;
                    D_800433DC.f.nib8 = (u8)((D_80043388.word >> 8) & 0xF);
                    D_800433DC.f.nibC = (u8)((D_80043388.word >> 0xC) & 0xF);
                    D_800433DC.b.unk2 = D_80043388.b.unk2;
                    D_800433DC.b.unk3 = D_80043388.b.unk3;
                    D_800433E0.unk0 = D_8004338C.unk0;
                    D_800433E0.unk1 = D_8004338C.unk1;
                    D_800433E0.unk2 = D_8004338C.unk2;
                    D_800433E0.unk3 = D_8004338C.unk3;
                    D_800433E4 = 6;
                    D_800433E5 = D_80043391;
                    D_800433E6.unk0 = D_80043392.unk0;
                    D_800433E6.unk1 = D_80043392.unk1;
                    D_8004338C.unk0 = D_80043390;
                    record_x = (u8)((D_80043388.word >> 8) & 0xF);
                    record_y = (u8)((D_80043388.word >> 0xC) & 0xF);
                    neighbor_land = wmap_get_land_at_cell(record_x - 1, record_y);
                    if (neighbor_land != 0xFF)
                    {
                        D_8004338C.unk1 = D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    neighbor_land = wmap_get_land_at_cell(record_x + 1, record_y);
                    if (neighbor_land != 0xFF)
                    {
                        D_8004338C.unk2 = D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    neighbor_land = wmap_get_land_at_cell(record_x, record_y - 1);
                    if (neighbor_land != 0xFF)
                    {
                        D_8004338C.unk3 = D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    neighbor_land = wmap_get_land_at_cell(record_x, record_y + 1);
                    if (neighbor_land != 0xFF)
                    {
                        D_80043390 = D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[neighbor_land * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    D_80043388.word = D_80043388.word & ~1;
                    D_80043388.word = D_80043388.word & ~2;
                    D_80043388.word = D_80043388.word & ~4;
                    D_80043388.word = D_80043388.word | 0xF00;
                    D_80043388.word = D_80043388.word | 0xF000;
                    D_80043388.b.unk2 = 0;
                    D_80043388.b.unk3 = 0;
                }
            }
        }
    }
    D_800432BC |= 0x800000;
}

/**
 * @brief Build a world-map route between two grid coordinates.
 * @param start_x Starting world-map grid X coordinate.
 * @param start_y Starting world-map grid Y coordinate.
 * @param end_x Destination world-map grid X coordinate.
 * @param end_y Destination world-map grid Y coordinate.
 * @param out_x Output array receiving route X coordinates.
 * @param out_y Output array receiving route Y coordinates.
 */
void func_8005EB68(s32 start_x, s32 start_y, s32 end_x, s32 end_y, s32 *out_x, s32 *out_y)
{
    s32 i;
    s32 path_found;
    s32 current_x;
    s32 current_y;
    s32 distance;
    s32 node_index;
    s32 distances[WMAP_ROUTE_NODE_COUNT];
    s32 route[WMAP_ROUTE_NODE_COUNT];

    if (wmap_get_land_at_cell(start_x, start_y) == WMAP_INVALID_NODE)
    {
        current_x = start_x;
        current_y = start_y;
        i = 0;
        while (current_x != end_x || current_y != end_y)
        {
            out_x[i] = current_x;
            out_y[i] = current_y;

            if (abs(end_x - current_x) > abs(end_y - current_y))
            {
                current_x += (end_x - current_x) / abs(end_x - current_x);
            }
            else
            {
                current_y += (end_y - current_y) / abs(end_y - current_y);
            }

            i++;
        }

        out_x[i] = end_x;
        out_y[i] = end_y;
        return;
    }

    i = 0;
    while (i < WMAP_ROUTE_NODE_COUNT)
    {
        distances[i] = WMAP_ROUTE_UNVISITED_DISTANCE;
        i++;
    }

    distances[wmap_get_land_at_cell(start_x, start_y)] = 0;
    distance = 0;
    path_found = 0;

    while (path_found == 0)
    {
        i = 0;
        while (i < WMAP_ROUTE_NODE_COUNT)
        {
            if (distances[i] == distance)
            {
                node_index = wmap_get_land_at_cell(WMAP_NODE_X(i) - 1, WMAP_NODE_Y(i));
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == wmap_get_land_at_cell(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = wmap_get_land_at_cell(WMAP_NODE_X(i) + 1, WMAP_NODE_Y(i));
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == wmap_get_land_at_cell(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = wmap_get_land_at_cell(WMAP_NODE_X(i), WMAP_NODE_Y(i) - 1);
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == wmap_get_land_at_cell(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = wmap_get_land_at_cell(WMAP_NODE_X(i), WMAP_NODE_Y(i) + 1);
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == wmap_get_land_at_cell(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }
            }
            i++;
        }
        distance++;
    }

    distance = distances[wmap_get_land_at_cell(end_x, end_y)];
    route[distance] = wmap_get_land_at_cell(end_x, end_y);
    node_index = wmap_get_land_at_cell(end_x, end_y);

    while (distance >= 0)
    {
        i = 0;
        while (i < WMAP_ROUTE_NODE_COUNT)
        {
            if (distances[i] == distance - 1)
            {
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index) - 1) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index)))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index) + 1) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index)))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index)) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index) - 1))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index)) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index) + 1))
                {
                    route[distance - 1] = i;
                    break;
                }
            }
            i++;
        }
        distance--;
        node_index = route[distance];
    }

    i = 0;
    while (i <= distances[wmap_get_land_at_cell(end_x, end_y)])
    {
        out_x[i] = WMAP_NODE_X(route[i]);
        out_y[i] = WMAP_NODE_Y(route[i]);
        i++;
    }
}
