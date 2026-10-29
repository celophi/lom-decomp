#include "common.h"
#include <abs.h>

#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_pathfinding.h"
#include "common/saved_game.h"
#include "main/field_entry.h"

/**
 * @file wmap_pathfinding.c
 * @brief Land encyclopedia unlocks, the land entered on leaving the world map,
 *        and BFS route building.
 */

#define WMAP_NODE_RECORD_SIZE 12

#define WMAP_ROUTE_NODE_COUNT 64
#define WMAP_ROUTE_UNVISITED_DISTANCE 100
#define WMAP_NODE_X(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] & 0xF))
#define WMAP_NODE_Y(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] >> 4))

/** @brief Game flag whose event moves the Flames land record into the Underworld record. */
#define WMAP_FLAG_FLAMES_TO_UNDERWORLD 12
/** @brief Game flag whose event moves the Underworld land record back into the Flames record. */
#define WMAP_FLAG_UNDERWORLD_TO_FLAMES 13

extern u8 D_800432C9[];
/** @brief WMAP land id to enter when each game flag is pending (only some flag ids are used). */
extern u8 g_wmap_flag_event_lands[SAVED_GAME_FLAG_COUNT];
/** @brief Music track index to play when each game flag's event land is entered. */
extern u8 g_wmap_flag_event_music_tracks[SAVED_GAME_FLAG_COUNT];
extern s16 g_music_track_index;

/**
 * @brief Unlock the encyclopedia entries of the lands and their artifacts from the land records.
 * @note Lucemia's entries are unlocked from FIELD_LAND_LUCEMIA_2 once it has FIELD_LAND_FLAG_04.
 */
void wmap_unlock_land_encyclopedia_entries(void)
{
    s32 i;

    for (i = 0; i < FIELD_ENCYCLOPEDIA_LAND_COUNT; i++)
    {
        if ((g_saved_lands[i].record.flags >> 1) & 1)
        {
            SAVED_BIT_SET(g_saved_encyclopedia_bits, i + FIELD_ENCYCLOPEDIA_LAND_BIT);
        }
    }

    if (g_saved_lands[FIELD_LAND_LUCEMIA_2].word & FIELD_LAND_FLAG_04)
    {
        SAVED_BIT_SET(g_saved_encyclopedia_bits, FIELD_ENCYCLOPEDIA_LAND_BIT + FIELD_LAND_LUCEMIA);
    }

    for (i = 0; i < FIELD_ENCYCLOPEDIA_LAND_COUNT; i++)
    {
        if (g_saved_lands[i].record.flags & FIELD_LAND_PLACED)
        {
            SAVED_BIT_SET(g_saved_encyclopedia_bits, i + FIELD_ENCYCLOPEDIA_ARTIFACT_BIT);
        }

        if ((g_saved_lands[i].record.flags >> 1) & 1)
        {
            SAVED_BIT_SET(g_saved_encyclopedia_bits, i + FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT);
        }
    }

    if (g_saved_lands[FIELD_LAND_LUCEMIA_2].word & FIELD_LAND_FLAG_04)
    {
        SAVED_BIT_SET(g_saved_encyclopedia_bits, FIELD_ENCYCLOPEDIA_ARTIFACT_FLAG_02_BIT + FIELD_LAND_LUCEMIA);
    }
}

/**
 * @brief Set up the field entry for leaving the world map; a pending event flag redirects it to its event land.
 * @param cell_x World-map grid X of the lead traveler.
 * @param cell_y World-map grid Y of the lead traveler.
 */
void wmap_commit_land_entry(s32 cell_x, s32 cell_y)
{
    s32 flag;
    s32 column;
    s32 row;
    s32 neighbor;
    s32 land;
    s32 scene_index;

    wmap_unlock_land_encyclopedia_entries();
    land = wmap_get_land_at_cell(cell_x, cell_y);
    scene_index = (land * WMAP_LAND_SCENE_VARIANTS) + ((g_saved_lands[land].record.flags >> 4) & 3);
    g_field_entry_scene.id = g_wmap_land_scenes[scene_index];
    g_field_music_id = g_wmap_land_music_ids[scene_index];
    g_music_track_index = wmap_get_land_at_cell(cell_x, cell_y);

    for (flag = 0; flag < SAVED_GAME_FLAG_COUNT; flag++)
    {
        if (SAVED_BIT_IS_SET(g_saved_flag_bits, flag))
        {
            if (flag == 9 || flag == 10 || flag == 11 || flag == WMAP_FLAG_FLAMES_TO_UNDERWORLD || flag == WMAP_FLAG_UNDERWORLD_TO_FLAMES || flag == 14 ||
                flag == 24 || flag == 0 || flag == 27 || flag == 28)
            {
                SAVED_BIT_CLEAR(g_saved_flag_bits, flag);
                g_music_track_index = g_wmap_flag_event_music_tracks[flag];
                land = g_wmap_flag_event_lands[flag];
                scene_index = (land * WMAP_LAND_SCENE_VARIANTS) + ((g_saved_lands[land].record.flags >> 4) & 3);
                g_field_entry_scene.id = g_wmap_land_scenes[scene_index];
                g_field_music_id = g_wmap_land_music_ids[scene_index];

                if (flag == WMAP_FLAG_FLAMES_TO_UNDERWORLD)
                {
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[4] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[0];
                    column = (u8)FIELD_LAND_WORD_X(g_saved_lands[FIELD_LAND_FLAMES].word);
                    row = (u8)FIELD_LAND_WORD_Z(g_saved_lands[FIELD_LAND_FLAMES].word);
                    neighbor = wmap_get_land_at_cell(column - 1, row);
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.placed = (u8)(g_saved_lands[FIELD_LAND_FLAMES].record.flags & FIELD_LAND_PLACED);
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.flag_02 = (u8)((g_saved_lands[FIELD_LAND_FLAMES].word >> 1) & 1);
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.flag_04 = 1;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.x = (u8)FIELD_LAND_WORD_X(g_saved_lands[FIELD_LAND_FLAMES].word);
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.z = (u8)FIELD_LAND_WORD_Z(g_saved_lands[FIELD_LAND_FLAMES].word);
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.unk2 = g_saved_lands[FIELD_LAND_FLAMES].record.unk2;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.count = g_saved_lands[FIELD_LAND_FLAMES].record.count;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[0] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[0];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[1] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[1];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[2] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[2];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[3] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[3];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[5] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[5];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[6] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[6];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[7] = g_saved_lands[FIELD_LAND_FLAMES].record.levels[7];
                    g_saved_lands[FIELD_LAND_FLAMES].bits.placed = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].bits.flag_02 = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].bits.flag_04 = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].bits.x = FIELD_LAND_CELL_NONE;
                    g_saved_lands[FIELD_LAND_FLAMES].bits.z = FIELD_LAND_CELL_NONE;
                    g_saved_lands[FIELD_LAND_FLAMES].record.unk2 = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.count = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[0] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[1] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[2] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[3] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[4] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[5] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[6] = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[7] = 0;
                }

                if (flag == WMAP_FLAG_UNDERWORLD_TO_FLAMES)
                {
                    g_saved_lands[FIELD_LAND_FLAMES].bits.placed = (u8)(g_saved_lands[FIELD_LAND_UNDERWORLD].record.flags & FIELD_LAND_PLACED);
                    g_saved_lands[FIELD_LAND_FLAMES].bits.flag_02 = (u8)((g_saved_lands[FIELD_LAND_UNDERWORLD].word >> 1) & 1);
                    g_saved_lands[FIELD_LAND_FLAMES].bits.flag_04 = 0;
                    g_saved_lands[FIELD_LAND_FLAMES].bits.x = (u8)FIELD_LAND_WORD_X(g_saved_lands[FIELD_LAND_UNDERWORLD].word);
                    g_saved_lands[FIELD_LAND_FLAMES].bits.z = (u8)FIELD_LAND_WORD_Z(g_saved_lands[FIELD_LAND_UNDERWORLD].word);
                    g_saved_lands[FIELD_LAND_FLAMES].record.unk2 = g_saved_lands[FIELD_LAND_UNDERWORLD].record.unk2;
                    g_saved_lands[FIELD_LAND_FLAMES].record.count = g_saved_lands[FIELD_LAND_UNDERWORLD].record.count;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[0] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[0];
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[1] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[1];
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[2] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[2];
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[3] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[3];
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[4] = FIELD_LAND_SPIRIT_MAX;
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[5] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[5];
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[6] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[6];
                    g_saved_lands[FIELD_LAND_FLAMES].record.levels[7] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[7];
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[0] = g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[4];
                    column = (u8)FIELD_LAND_WORD_X(g_saved_lands[FIELD_LAND_UNDERWORLD].word);
                    row = (u8)FIELD_LAND_WORD_Z(g_saved_lands[FIELD_LAND_UNDERWORLD].word);

                    neighbor = wmap_get_land_at_cell(column - 1, row);
                    if (neighbor != WMAP_NO_LAND)
                    {
                        g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[1] = g_saved_lands[neighbor].record.levels[4];
                        if (g_saved_lands[neighbor].record.levels[4] < FIELD_LAND_SPIRIT_MAX)
                        {
                            g_saved_lands[neighbor].record.levels[4] = 5;
                        }
                    }

                    neighbor = wmap_get_land_at_cell(column + 1, row);
                    if (neighbor != WMAP_NO_LAND)
                    {
                        g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[2] = g_saved_lands[neighbor].record.levels[4];
                        if (g_saved_lands[neighbor].record.levels[4] < FIELD_LAND_SPIRIT_MAX)
                        {
                            g_saved_lands[neighbor].record.levels[4] = 5;
                        }
                    }

                    neighbor = wmap_get_land_at_cell(column, row - 1);
                    if (neighbor != WMAP_NO_LAND)
                    {
                        g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[3] = g_saved_lands[neighbor].record.levels[4];
                        if (g_saved_lands[neighbor].record.levels[4] < FIELD_LAND_SPIRIT_MAX)
                        {
                            g_saved_lands[neighbor].record.levels[4] = 5;
                        }
                    }

                    neighbor = wmap_get_land_at_cell(column, row + 1);
                    if (neighbor != WMAP_NO_LAND)
                    {
                        g_saved_lands[FIELD_LAND_UNDERWORLD].record.levels[4] = g_saved_lands[neighbor].record.levels[4];
                        if (g_saved_lands[neighbor].record.levels[4] < FIELD_LAND_SPIRIT_MAX)
                        {
                            g_saved_lands[neighbor].record.levels[4] = 5;
                        }
                    }

                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.placed = 0;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.flag_02 = 0;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.flag_04 = 0;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.x = FIELD_LAND_CELL_NONE;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].bits.z = FIELD_LAND_CELL_NONE;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.unk2 = 0;
                    g_saved_lands[FIELD_LAND_UNDERWORLD].record.count = 0;
                }
            }
        }
    }

    g_saved_game.layout.control.word |= FIELD_CONTROL_RESET_WORDS;
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

    if (wmap_get_land_at_cell(start_x, start_y) == WMAP_NO_LAND)
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
                if (node_index != WMAP_NO_LAND)
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
                if (node_index != WMAP_NO_LAND)
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
                if (node_index != WMAP_NO_LAND)
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
                if (node_index != WMAP_NO_LAND)
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
