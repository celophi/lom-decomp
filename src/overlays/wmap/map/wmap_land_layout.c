#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_land_transition.h"
#include "common/saved_game.h"
#include "main/cdrom.h"
#include "main/main.h"

/* WMAP interprets its portion of the resident save buffer through this view. */
/** @brief Event bits stored in each saved-game flag word. */
#define WMAP_EVENT_WORD_BITS 32
#define WMAP_DAY_MASK 0x7F
#define WMAP_LAYOUT_RESOURCE_BASE 0x12EA
/** @brief Map cell flag: no ordinary land may be placed here (every sea cell has it; Lucemia and the Tree of Mana ignore it). */
#define WMAP_TERRAIN_NO_LAND 0x01
/** @brief Map cell flag: coast, where WMAP_ARTIFACT_COAST_ONLY lands may go. */
#define WMAP_TERRAIN_COAST 0x02
/** @brief Map cell flag: sea, where the WMAP_ARTIFACT_SEA_ONLY ship goes without using up a land slot. */
#define WMAP_TERRAIN_SEA 0x04
/** @brief Artifact flag bit: the land goes only on WMAP_TERRAIN_SEA cells (only SS Buccaneer has it). */
#define WMAP_ARTIFACT_SEA_ONLY_SHIFT 10
/** @brief Artifact flag bit: after the first land, the land goes only on WMAP_TERRAIN_COAST cells (Polpota Harbor and Madora Beach). */
#define WMAP_ARTIFACT_COAST_ONLY_SHIFT 8
#define WMAP_SPIRIT_SPRITE_COLUMNS 7
#define WMAP_SPIRIT_SPRITE_DIAGONAL 8
#define WMAP_COORD_MASK 0x0F
#define WMAP_COORD_Y_SHIFT 4
#define WMAP_STATUS_X_MASK 0x00000F00
#define WMAP_STATUS_Y_MASK 0x0000F000
#define WMAP_LABEL_LEVEL_BASE 10
#define WMAP_LABEL_TERRAIN_BASE 13
#define WMAP_LABEL_DECREASE 17
#define WMAP_LABEL_INCREASE 18
#define WMAP_LABEL_CHANGE_BASE 13
#define WMAP_LABEL_PROPOSED_DECREASE 26
#define WMAP_LABEL_PROPOSED_INCREASE 27
#define WMAP_LABEL_PROPOSED_CHANGE_BASE 22
#define WMAP_LABEL_PROPOSED_GROUP 0x200

/** @brief Spirit strengths and placement restrictions for an artifact. */
typedef union
{
    u8 bytes[12];
    struct
    {
        u8 spirits[FIELD_LAND_SPIRIT_COUNT];
        u32 flags;
    } data;
} WmapLandAttributes;

/** @brief Terrain spirit bonuses and placement flags for one map cell. */
typedef struct
{
    u8 spirits[FIELD_LAND_SPIRIT_COUNT];
    u16 flags;
    u16 unused_0a; /**< 0-15 in the WM_D*.DAT layouts, tracing the edges of the placeable area; no code reads it. */
} WmapTerrainCell;

/** @brief Layout file header followed by the six-by-six terrain grid. */
typedef struct
{
    u16 starting_x;
    u16 starting_y;
    s32 land_count;
    WmapTerrainCell cells[WMAP_GRID_SIZE * WMAP_GRID_SIZE];
    u16 scenes[FIELD_LAND_COUNT * WMAP_LAND_ENTRY_SLOTS];
    u8 music_ids[FIELD_LAND_COUNT * WMAP_LAND_ENTRY_SLOTS];
} WmapLayout;

/** @brief Table of 64 world-map values copied to a local buffer. */
typedef struct
{
    s32 values[64];
} WmapValueTable;

/** @brief Byte 0 of the hero's FieldCharacterRecord::info in g_saved_game: the character type and the pad-controlled bit. */
extern u8 g_wmap_hero_character_info;
extern s32 g_wmap_layout_selection;
extern WmapLandAttributes g_wmap_land_attributes[FIELD_LAND_COUNT];
extern s32 g_wmap_spirit_sprites[];
extern s32 g_wmap_proposed_spirits[FIELD_LAND_COUNT][FIELD_LAND_SPIRIT_COUNT];
extern u8 g_wmap_land_lookup[WMAP_GRID_SIZE * WMAP_GRID_SIZE];
extern WmapLayout* g_wmap_layout;
extern WmapLayout g_wmap_layout_buffer;
extern WmapValueTable g_wmap_land_count_tiers;
extern s32 g_wmap_event_cursor;
extern u32 g_wmap_artifact_list_end;
extern s32 g_wmap_artifact_list_head;
extern s32 g_wmap_artifact_display_head;
extern s32 g_wmap_artifact_list_tail;
extern s32 g_wmap_artifact_display_tail;
extern s32 g_wmap_artifact_list[];

void wmap_apply_land_influence(s32 land, u32 x, s32 y, s32 (*spirits)[FIELD_LAND_SPIRIT_COUNT]);
void wmap_rebuild_land_lookup(void);
void wmap_append_land_spirit_labels(s32 mode, u32 x, s32 y, s32 mask, s32* count, s32* flags, s32* output, s32 unused,
                                    s32 (*comparison)[FIELD_LAND_SPIRIT_COUNT]);
s32 wmap_wrap_index(s32 value, s32 minimum, s32 maximum);

/**
 * @brief Test FIELD_LAND_CLEARED of the land occupying a known occupied cell.
 * @param x Cell column.
 * @param y Cell row.
 * @return One if the land is cleared, otherwise zero.
 */
static inline s32 wmap_cell_has_cleared_land(u32 x, s32 y)
{
    SavedGameLayout* save = &g_saved_game.layout;
    s32 land = wmap_get_land_at_cell(x, y);
    return (save->lands[land].flags >> 2) & 1;
}

/**
 * @brief Read one placement restriction from an artifact.
 * @param land Artifact index.
 * @param shift Bit position of the restriction.
 * @return The selected flag as zero or one.
 */
static inline s32 wmap_get_artifact_flag(s32 land, s32 shift)
{
    return (g_wmap_land_attributes[land].data.flags >> shift) & 1;
}

/**
 * @brief Add an artifact contribution to a neighboring land, clamping each spirit.
 * @param table_row Artifact index, or WMAP_NO_LAND to skip.
 * @param output_row Neighbor index, or WMAP_NO_LAND to skip.
 * @param output Table of proposed spirit levels.
 */
static inline void wmap_adjust_neighbor_spirits(s32 table_row, s32 output_row, s32 (*output)[FIELD_LAND_SPIRIT_COUNT])
{
    s32* entry;
    uintptr_t table;
    s32 row_offset;
    s32 adjustment;
    s32 value;
    s32 clamped;
    s32 index;

    if (output_row != WMAP_NO_LAND)
    {
        index = 0;

        if (table_row != WMAP_NO_LAND)
        {
            table = (uintptr_t)g_wmap_land_attributes;
            row_offset = table_row * sizeof(WmapLandAttributes);
            entry = (s32*)((output_row * sizeof(g_wmap_proposed_spirits[0])) + (uintptr_t)output);

            for (; index < FIELD_LAND_SPIRIT_COUNT; index++, entry++)
            {
                adjustment = *(u8*)(index + row_offset + table) - FIELD_LAND_SPIRIT_NEUTRAL;
                value = *entry + adjustment;

                *entry = value;
                if (value >= 0)
                {
                    clamped = FIELD_LAND_SPIRIT_MAX;

                    if (value < 7)
                    {
                        clamped = value;
                    }
                }
                else
                {
                    clamped = 0;
                }

                *entry = clamped;
            }
        }
    }
}

/**
 * @brief Accumulate a saved neighbor's influence relative to the neutral level.
 * @param record_index Saved neighbor index, or WMAP_NO_LAND to skip.
 * @param other_index Receiving land index, or WMAP_NO_LAND to skip.
 * @param unused Unused argument.
 * @param values Eight accumulated spirit contributions.
 */
static inline void wmap_accumulate_land_spirits(s32 record_index, s32 other_index, s32 unused, s32* values)
{
    s32 index;
    s32 value;

    if (other_index != WMAP_NO_LAND)
    {
        if (record_index != WMAP_NO_LAND)
        {
            for (index = 0; index < FIELD_LAND_SPIRIT_COUNT; index++)
            {
                value = values[index] - FIELD_LAND_SPIRIT_NEUTRAL;
                values[index] = value + g_saved_game.layout.lands[record_index].levels[index];
            }
        }
    }
}

/**
 * @brief Accumulate terrain spirits when the neighboring cell is on the map.
 * @param x Neighbor column.
 * @param y Neighbor row.
 * @param values Eight accumulated spirit contributions.
 */
static inline void wmap_accumulate_terrain_spirits(u32 x, s32 y, s32* values)
{
    s32 index;
    WmapLayout* layout;

    if (x < WMAP_GRID_SIZE && y >= 0 && y < WMAP_GRID_SIZE)
    {
        index = 0;
        layout = g_wmap_layout;

        for (; index < FIELD_LAND_SPIRIT_COUNT; index++)
        {
            values[index] += layout->cells[x + y * WMAP_GRID_SIZE].spirits[index];
        }
    }
}

/**
 * @brief Add an artifact to the selection list.
 * @param land Saved land index.
 * @return Position of the added artifact.
 */
static inline s32 wmap_append_artifact(s32 land)
{
    s32 position = g_wmap_artifact_list_end;
    g_wmap_artifact_list_end = position + 1;
    g_wmap_artifact_list[position] = land;
    return position;
}

/**
 * @brief Advance the six-day cycle and update timed and growth records.
 * @note The saved day must be in the range zero through five.
 */
void wmap_update_travel_growth(void)
{
    s32 growth[4][FIELD_LAND_SPIRIT_COUNT];
    s32 flag_word;
    s32 event;
    s32 blocked;
    s32 spirit;
    s32 day_bonus;
    s32 growth_slot;
    s32 growth_group;
    s32 total;
    s32 clamped;
    u16 age;
    u16 remaining_age;
    u32 day_flags;
    u32 day;
    s32* day_growth;

    blocked = 0;

    for (event = 0; event < FIELD_LAND_COUNT; event++)
    {
        flag_word = event / WMAP_EVENT_WORD_BITS;

        if ((g_saved_game.layout.flag_bits[flag_word] & (1 << (event - (flag_word << 5)))) &&
            (((u32)(event - 9) < (u32)WMAP_GRID_SIZE) || (event == 0x18) || (event == 0) || (event == 0x1B) || (event == 0x1C)))
        {
            blocked = 1;
        }
    }

    if (blocked == 0)
    {
        day_flags = (g_saved_game.layout.control.word & 0xFF80FFFF) |
                    ((((((u32)g_saved_game.layout.control.word >> 0x10) & WMAP_DAY_MASK) + 1) & WMAP_DAY_MASK) << 0x10);
        g_saved_game.layout.control.word = day_flags;

        if ((u32)((day_flags >> 0x10) & WMAP_DAY_MASK) >= 6U)
        {
            g_saved_game.layout.control.word = (u32)(day_flags & 0xFF80FFFF);
        }

        for (growth_group = 0; growth_group < PET_RECORD_COUNT; growth_group++)
        {
            if (g_saved_game.layout.pets[growth_group].name[0] != 0)
            {
                if (g_saved_game.layout.pets[growth_group].status.bits.egg)
                {
                    remaining_age = g_saved_game.layout.pets[growth_group].hatch_counter;

                    if (remaining_age != 0)
                    {
                        g_saved_game.layout.pets[growth_group].hatch_counter = (u16)(remaining_age - 1);
                    }
                }
                else
                {
                    age = g_saved_game.layout.pets[growth_group].hatch_counter;

                    if (age <= 0xFFFEU)
                    {
                        g_saved_game.layout.pets[growth_group].hatch_counter = (u16)(age + 1);
                    }
                }
            }
        }

        for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT; spirit++)
        {
            growth[0][spirit] = g_saved_game.layout.lands[FIELD_LAND_HOME].levels[spirit] - FIELD_LAND_SPIRIT_NEUTRAL;
            growth[1][spirit] = g_saved_game.layout.lands[FIELD_LAND_ORCHARDS].levels[spirit] - FIELD_LAND_SPIRIT_NEUTRAL;
            growth[2][spirit] = 0;
            growth[3][spirit] = 0;
        }

        day = g_saved_game.layout.control.fields.weekday & WMAP_DAY_MASK;

        switch (day)
        {
            case 0:
                day_bonus = 4;
                break;
            case 1:
                day_bonus = 3;
                break;
            case 2:
                day_bonus = 5;
                break;
            case 3:
                day_bonus = 6;
                break;
            case 4:
                day_bonus = 2;
                break;
            case 5:
                day_bonus = 7;
                break;
        }

        day_growth = &growth[0][day_bonus];
        day_growth[0] = (s32)(day_growth[0] + 1);
        day_growth[8] = (s32)(day_growth[8] + 1);
        day_growth[16] = (s32)(day_growth[16] + 1);
        day_growth[24] = (s32)(day_growth[24] + 1);
        growth[0][0]++;
        growth[0][1]++;
        growth[1][0]++;
        growth[1][1]++;
        growth[2][0]++;
        growth[2][1]++;
        growth[3][0]++;
        growth[3][1]++;

        for (growth_group = 0; growth_group < 4; growth_group++)
        {
            for (growth_slot = 0; growth_slot < FIELD_LAND_SPIRIT_COUNT; growth_slot++)
            {
                for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT; spirit++)
                {
                    total = g_saved_game.layout.menu_slots[growth_group].slots[growth_slot].counters[spirit];
                    total += growth[growth_group][spirit];

                    if (total >= 0)
                    {
                        clamped = 255;

                        if (total < 0x100)
                        {
                            clamped = total;
                        }
                    }
                    else
                    {
                        clamped = 0;
                    }

                    g_saved_game.layout.menu_slots[growth_group].slots[growth_slot].counters[spirit] = clamped;
                }
            }
        }
    }
}

/**
 * @brief Test terrain restrictions and adjacency for an artifact placement.
 * @param x Map column.
 * @param y Map row.
 * @param land Artifact index.
 * @return Nonzero when the placement is allowed.
 */
s32 wmap_can_place_land(u32 x, s32 y, s32 land)
{
    s32 cell;

    cell = x + y * WMAP_GRID_SIZE;

    if (x >= (u32)WMAP_GRID_SIZE || y < 0 || y >= WMAP_GRID_SIZE)
    {
        return 0;
    }

    if (wmap_get_land_at_cell(x, y) == WMAP_NO_LAND)
    {
        if (wmap_get_artifact_flag(land, WMAP_ARTIFACT_SEA_ONLY_SHIFT) == 1)
        {
            if (!(g_wmap_layout->cells[cell].flags & WMAP_TERRAIN_SEA))
            {
                return 0;
            }
        }
        else if ((u32)(land - FIELD_LAND_LUCEMIA) >= 2U && land != FIELD_LAND_TREE_OF_MANA && (g_wmap_layout->cells[cell].flags & WMAP_TERRAIN_NO_LAND))
        {
            return 0;
        }

        if (g_saved_game.layout.control.fields.placed_land_count == 0)
        {
            return 1;
        }

        if (!(g_wmap_layout->cells[cell].flags & WMAP_TERRAIN_COAST))
        {
            if (wmap_get_artifact_flag(land, WMAP_ARTIFACT_COAST_ONLY_SHIFT) == 1)
            {
                return 0;
            }
        }

        if (y > 0 && wmap_get_land_at_cell(x, y - 1) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_cleared_land(x, y - 1) == 1)
            {
                return 1;
            }
        }

        if (y < WMAP_GRID_SIZE - 1 && wmap_get_land_at_cell(x, y + 1) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_cleared_land(x, y + 1) == 1)
            {
                return 1;
            }
        }

        if ((s32)x > 0 && wmap_get_land_at_cell(x - 1, y) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_cleared_land(x - 1, y) == 1)
            {
                return 1;
            }
        }

        if ((s32)x < WMAP_GRID_SIZE - 1 && wmap_get_land_at_cell(x + 1, y) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_cleared_land(x + 1, y) == 1)
            {
                return 1;
            }
        }
    }

    return 0;
}

/** @brief Place a map record and update its neighboring layout data.
 * @param x Grid column.
 * @param y Grid row.
 * @param index Save record index.
 */
void wmap_place_land(s32 x, s32 y, s32 index)
{
    u8 unused_spirits[FIELD_LAND_SPIRIT_COUNT];
    s32 record;
    s32 field;
    s32 row_bits;
    SavedGame* shifted;
    u8* save_data;

    save_data = g_saved_game.bytes;
    shifted = (SavedGame*)(save_data + index * sizeof(FieldLandWords));
    row_bits = y * 16;
    shifted->land_words.lands[0].bytes.cell = (x & WMAP_COORD_MASK) | row_bits;
    shifted->land_words.lands[0].bytes.flags |= FIELD_LAND_PLACED;
    g_saved_game.layout.control.fields.placed_land_count++;
    shifted->land_words.lands[0].bytes.placement_order = g_saved_game.layout.control.fields.placed_land_count;

    if (g_saved_game.layout.control.fields.placed_land_count == 1)
    {
        g_current_land = index;
    }

    /* Apply placement in a word workspace before committing the byte-sized save levels. */
    for (record = 0; record < FIELD_LAND_COUNT; record++)
    {
        for (field = 0; field < FIELD_LAND_SPIRIT_COUNT; field++)
        {
            g_wmap_proposed_spirits[record][field] = g_saved_game.layout.lands[record].levels[field];
        }
    }

    for (field = 0; field < FIELD_LAND_SPIRIT_COUNT; field++)
    {
        g_wmap_proposed_spirits[index][field] = g_wmap_land_attributes[index].data.spirits[field];
    }

    wmap_apply_land_influence(index, x, y, g_wmap_proposed_spirits);

    for (record = 0; record < FIELD_LAND_COUNT; record++)
    {
        for (field = 0; field < FIELD_LAND_SPIRIT_COUNT; field++)
        {
            g_saved_game.layout.lands[record].levels[field] = g_wmap_proposed_spirits[record][field];
        }
    }

    wmap_rebuild_land_lookup();
}

/**
 * @brief Apply a proposed land and its neighbors to the spirit-level workspace.
 * @param land Artifact index.
 * @param x Placement column.
 * @param y Placement row.
 * @param spirits Spirit levels for all lands, updated in place.
 */
void wmap_apply_land_influence(s32 land, u32 x, s32 y, s32 (*spirits)[FIELD_LAND_SPIRIT_COUNT])
{
    s32 neighbor_influence[FIELD_LAND_SPIRIT_COUNT];
    s32* terrain_spirit;
    s32* land_spirit;
    s32* clear_entry;
    s32 cell_offset;
    s32 total;
    s32 clamped;
    s32 spirit;
    s32* influence;
    s32 neighbor_total;
    u8 terrain_bonus;
    WmapLayout* terrain;

    if ((x < WMAP_GRID_SIZE) && (y >= 0) && (y < WMAP_GRID_SIZE) && (wmap_can_place_land(x, y, land) != 0))
    {
        /* Neighbors receive the artifact contribution, clamped independently. */
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x - 1, y), spirits);
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x + 1, y), spirits);
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x, y - 1), spirits);
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x, y + 1), spirits);
        spirit = 0;
        terrain = g_wmap_layout;
        cell_offset = (x + y * WMAP_GRID_SIZE) * sizeof(WmapTerrainCell);
        terrain_spirit = (s32*)(land * sizeof(spirits[0]) + (uintptr_t)spirits);

        while (spirit < FIELD_LAND_SPIRIT_COUNT)
        {
            terrain_bonus = ((WmapLayout*)((u8*)terrain + (spirit + cell_offset)))->cells[0].spirits[0];
            spirit++;
            *terrain_spirit += terrain_bonus;
            terrain_spirit++;
        }

        /* The new land receives half the combined influence of its four neighbors. */
        spirit = FIELD_LAND_SPIRIT_COUNT - 1;
        clear_entry = &neighbor_influence[FIELD_LAND_SPIRIT_COUNT - 1];

        while (spirit >= 0)
        {
            *clear_entry = 0;
            spirit--;
            clear_entry--;
        }

        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x - 1, y), land, 0, neighbor_influence);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x + 1, y), land, 0, neighbor_influence);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x, y - 1), land, 0, neighbor_influence);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x, y + 1), land, 0, neighbor_influence);
        wmap_accumulate_terrain_spirits(x - 1, y, neighbor_influence);
        wmap_accumulate_terrain_spirits(x + 1, y, neighbor_influence);
        wmap_accumulate_terrain_spirits(x, y - 1, neighbor_influence);
        wmap_accumulate_terrain_spirits(x, y + 1, neighbor_influence);
        spirit = 0;
        land_spirit = (s32*)(land * sizeof(spirits[0]) + (uintptr_t)spirits);
        influence = neighbor_influence;

        while (spirit < FIELD_LAND_SPIRIT_COUNT)
        {
            neighbor_total = *influence;
            total = *land_spirit + (neighbor_total / 2);

            *land_spirit = total;
            if (total >= 0)
            {
                clamped = FIELD_LAND_SPIRIT_MAX;

                if (total < 7)
                {
                    clamped = total;
                }
            }
            else
            {
                clamped = 0;
            }

            *land_spirit = clamped;
            land_spirit++;
            spirit++;
            influence++;
        }
    }
}

/**
 * @brief Resolve spirit icons for a cell after a proposed land placement.
 * @param x Cell column.
 * @param y Cell row.
 * @param land Artifact being placed.
 * @param proposed_x Placement column.
 * @param proposed_y Placement row.
 * @param output Destination for eight sprite indices.
 */
void wmap_get_proposed_spirit_sprites(u32 x, s32 y, u32 land, u32 proposed_x, s32 proposed_y, s32* output)
{
    s32 current_land;
    s32 cell;
    s32 i;
    s32 j;
    s32 component;
    s32 level;
    s32 value;
    s32 fallback;

    if (x < WMAP_GRID_SIZE && y >= 0 && y < WMAP_GRID_SIZE && land < FIELD_LAND_COUNT)
    {
        if (proposed_x < WMAP_GRID_SIZE && proposed_y >= 0 && proposed_y < WMAP_GRID_SIZE)
        {
            cell = x + y * WMAP_GRID_SIZE;
            current_land = wmap_get_land_at_cell(x, y);

            for (i = 0; i < FIELD_LAND_COUNT; i++)
            {
                for (j = 0; j < FIELD_LAND_SPIRIT_COUNT; j++)
                {
                    g_wmap_proposed_spirits[i][j] = g_saved_game.layout.lands[i].levels[j];
                }
            }

            for (j = 0; j < FIELD_LAND_SPIRIT_COUNT; j++)
            {
                g_wmap_proposed_spirits[land][j] = g_wmap_land_attributes[land].data.spirits[j];
            }

            wmap_apply_land_influence(land, proposed_x, proposed_y, g_wmap_proposed_spirits);

            for (i = 0; i < FIELD_LAND_SPIRIT_COUNT; i++)
            {
                value = 0;

                if (current_land == WMAP_NO_LAND)
                {
                    if (proposed_x == x && proposed_y == y)
                    {
                        component = g_wmap_land_attributes[land].data.spirits[i];
                        level = g_wmap_proposed_spirits[land][i];

                        if (component >= 0 && level >= 0)
                        {
                            value = g_wmap_spirit_sprites[component * WMAP_SPIRIT_SPRITE_COLUMNS + level];
                        }

                        output[i] = value;
                    }
                    else
                    {
                        component = g_wmap_layout->cells[cell].spirits[i];
                        fallback = 0;

                        if (component >= 0)
                        {
                            fallback = g_wmap_spirit_sprites[component * WMAP_SPIRIT_SPRITE_DIAGONAL];
                        }

                        output[i] = fallback;
                    }
                }
                else
                {
                    component = g_saved_game.layout.lands[current_land].levels[i];
                    level = g_wmap_proposed_spirits[current_land][i];

                    if (component >= 0 && level >= 0)
                    {
                        value = g_wmap_spirit_sprites[component * WMAP_SPIRIT_SPRITE_COLUMNS + level];
                    }

                    output[i] = value;
                }
            }
        }
        else
        {
            for (i = 0; i < FIELD_LAND_SPIRIT_COUNT; i++)
            {
                output[i] = g_wmap_spirit_sprites[0];
            }
        }
    }
}

/** @brief Load the map layout and reconcile saved placement data.
 * @return Current layout status.
 */
s32 wmap_load_land_layout(void)
{
    s32 index;
    u8* cell;
    s32 empty;
    u8 saved_second;
    u8 saved_first;
    u32 status;

    g_wmap_event_cursor = -1;
    cdrom_wait_queue_empty();
    cdrom_queue_read(((u16)g_saved_game.layout.world_map_cell + WMAP_LAYOUT_RESOURCE_BASE) & 0xFFFF, &g_wmap_layout_buffer);
    cdrom_wait_queue_empty();
    g_wmap_layout = &g_wmap_layout_buffer;
    g_wmap_land_scenes = g_wmap_layout_buffer.scenes;
    g_wmap_land_music_ids = g_wmap_layout_buffer.music_ids;

    if ((g_saved_game.layout.options.bits.game_cleared != 1) && (g_saved_game.layout.control.fields.placed_land_count == 1))
    {
        g_saved_game.layout.flag_bits[0] = (g_saved_game.layout.flag_bits[0] | 2);
    }

    if ((g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA].word & FIELD_LAND_CLEARED) &&
        !(g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA_2].word & FIELD_LAND_CLEARED))
    {
        status = g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA_2].word | 7;
        status &= ~WMAP_STATUS_X_MASK;
        status |= g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA].word & WMAP_STATUS_X_MASK;
        status &= ~WMAP_STATUS_Y_MASK;
        status |= g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA].word & WMAP_STATUS_Y_MASK;
        g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA_2].word = status;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[0] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[1] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[2] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[3] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[4] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[5] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[6] = 3;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].levels[7] = 3;
        saved_first = g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].placement_order;
        saved_second = g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].artifact_order;
        g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA].word &= ~2;
        g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA].word &= ~1;
        g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA].word |= 0xFF00;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].placement_order = 0U;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].artifact_order = 0U;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[0] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[1] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[2] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[3] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[4] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[5] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[6] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA].levels[7] = 0;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].placement_order = saved_first;
        g_saved_game.layout.lands[FIELD_LAND_LUCEMIA_2].artifact_order = saved_second;
    }

    empty = WMAP_NO_LAND;
    index = 0x23;
    cell = g_wmap_land_lookup + index;

    for (; index >= 0; index--)
    {
        *cell = empty;
        cell--;
    }

    wmap_rebuild_land_lookup();
    wmap_build_artifact_list(0);
    return g_wmap_layout_selection;
}

/**
 * @brief Find the next pending world-map flag and consume nonpersistent entries.
 * @return The flag index, or -1 when no pending entry remains.
 */
s32 wmap_next_land_event(void)
{
    s32* save_words;
    s32 bank;
    s32 mask;
    s32* words;

    g_wmap_event_cursor++;

    while (g_wmap_event_cursor < FIELD_LAND_COUNT)
    {
        save_words = g_saved_game.words;
        bank = g_wmap_event_cursor / 32;
        words = save_words + bank;
        mask = 1 << (g_wmap_event_cursor - (bank << 5));

        if (words[SAVED_GAME_FLAG_WORD_INDEX] & mask)
        {
            if (g_wmap_event_cursor == 0)
            {
                g_field_secondary_music_id = 31;
            }

            if ((g_wmap_event_cursor - 2) >= 2U && g_wmap_event_cursor != 22 && g_wmap_event_cursor != 23 && g_wmap_event_cursor != 9 &&
                g_wmap_event_cursor != 10 && g_wmap_event_cursor != 11 && g_wmap_event_cursor != 12 && g_wmap_event_cursor != 13 && g_wmap_event_cursor != 14 &&
                g_wmap_event_cursor != 24 && g_wmap_event_cursor != 0 && g_wmap_event_cursor != 27 && g_wmap_event_cursor != 28)
            {
                words[SAVED_GAME_FLAG_WORD_INDEX] &= ~(1 << (g_wmap_event_cursor % 32));
            }

            return g_wmap_event_cursor;
        }

        g_wmap_event_cursor++;
    }

    return -1;
}

/** @brief Rebuild the world-map lookup from placed save records. */
void wmap_rebuild_land_lookup(void)
{
    s32 index;
    s32 x;
    u32 y;
    u8 position;
    u32 placed;

    for (index = 0; index < FIELD_LAND_COUNT; index++)
    {
        placed = (g_saved_game.land_words.lands[index].bytes.flags >> 1) & 1;

        if (placed == 1)
        {
            if (index == FIELD_LAND_LUCEMIA)
            {
                if (!(g_saved_game.land_words.lands[FIELD_LAND_LUCEMIA_2].word & FIELD_LAND_CLEARED))
                {
                    position = ((SavedGame*)(g_saved_game.bytes + FIELD_LAND_LUCEMIA * sizeof(FieldLandWords)))->land_words.lands[0].bytes.cell;
                    x = position & WMAP_COORD_MASK;
                    y = position >> WMAP_COORD_Y_SHIFT;
                }
            }
            else
            {
                position = g_saved_game.land_words.lands[index].bytes.cell;
                x = position & WMAP_COORD_MASK;
                y = position >> WMAP_COORD_Y_SHIFT;
            }

            g_wmap_land_lookup[x + y * WMAP_GRID_SIZE] = index;
        }
    }
}

/**
 * @brief Scroll the artifact carousel and refresh its twelve display slots.
 * @param forward Nonzero to advance, zero to move backward.
 * @param visible_entries Circular array of twelve artifact indices.
 */
void wmap_scroll_artifact_list(s32 forward, s32* visible_entries)
{
    s32 row;
    s32 entry;
    s32 i;

    if (forward != 0)
    {
        if (g_wmap_artifact_list[g_wmap_artifact_list_end] == -1)
        {
            if (g_wmap_artifact_list_end >= WMAP_ARTIFACT_SLOTS)
            {
                if (g_wmap_artifact_list[g_wmap_artifact_list_head] == -1)
                {
                    g_wmap_artifact_list_end--;
                    g_wmap_artifact_list_head--;
                }
            }
        }

        g_wmap_artifact_display_head++;
        g_wmap_artifact_list_head++;
    }
    else
    {
        if (g_wmap_artifact_list[g_wmap_artifact_list_end] == -1 && g_wmap_artifact_list_end >= WMAP_ARTIFACT_SLOTS &&
            g_wmap_artifact_list[g_wmap_artifact_list_tail] == -1)
        {
            g_wmap_artifact_list_end--;
        }

        g_wmap_artifact_display_head--;
        g_wmap_artifact_list_head--;
    }

    g_wmap_artifact_display_head = wmap_wrap_index(g_wmap_artifact_display_head, 0, WMAP_ARTIFACT_SLOTS - 1);
    g_wmap_artifact_list_head = wmap_wrap_index(g_wmap_artifact_list_head, 0, g_wmap_artifact_list_end);
    g_wmap_artifact_display_tail = wmap_wrap_index(g_wmap_artifact_display_head + WMAP_ARTIFACT_SLOTS - 1, 0, WMAP_ARTIFACT_SLOTS - 1);
    g_wmap_artifact_list_tail = wmap_wrap_index(g_wmap_artifact_list_head + WMAP_ARTIFACT_SLOTS - 1, 0, g_wmap_artifact_list_end);

    for (i = 0; i < WMAP_ARTIFACT_SLOTS; i++)
    {
        row = wmap_wrap_index(g_wmap_artifact_display_head + i, 0, WMAP_ARTIFACT_SLOTS - 1);
        entry = wmap_wrap_index(g_wmap_artifact_list_head + i, 0, g_wmap_artifact_list_end);
        visible_entries[row] = g_wmap_artifact_list[entry];
    }
}

/** @brief Sort unplaced artifacts by acquisition order and initialize the selection range.
 * @param selection Requested selection position.
 * @return Number of visible map entries.
 */
s32 wmap_build_artifact_list(s32 selection)
{
    s32 i;
    s32 j;
    s32 current;
    s32 previous;
    s32 previous_index;
    s32* current_slot;
    s32* cursor;
    s32 empty;
    u32 placed;

    g_wmap_artifact_list_end = 0;
    empty = -1;

    for (i = 63; i >= 0; i--)
    {
        g_wmap_artifact_list[i] = empty;
    }

    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        previous_index = g_saved_game.land_words.lands[i].bytes.artifact_order != 0;

        if (previous_index)
        {
            placed = (g_saved_game.land_words.lands[i].bytes.flags >> 1) & 1;

            if (placed != 1)
            {
                j = wmap_append_artifact(i);

                while (j > 0)
                {
                    cursor = &g_wmap_artifact_list[j];
                    current_slot = cursor;
                    previous_index = j - 1;
                    cursor = &g_wmap_artifact_list[previous_index];
                    current = *current_slot;
                    previous = *cursor;
                    j = previous;

                    if (g_saved_game.land_words.lands[current].bytes.artifact_order > g_saved_game.land_words.lands[j].bytes.artifact_order)
                    {
                        *current_slot = previous;
                        *cursor = current;
                    }

                    j = previous_index;
                }
            }
        }
    }

    i = g_wmap_artifact_list_end;

    for (; i < g_wmap_artifact_list_end + WMAP_ARTIFACT_SLOTS; i++)
    {
        g_wmap_artifact_list[i] = -1;
    }

    g_wmap_artifact_list_end += WMAP_ARTIFACT_SLOTS - 1;
    g_wmap_artifact_display_head = wmap_wrap_index(selection + 3, 0, WMAP_ARTIFACT_SLOTS - 1);
    g_wmap_artifact_list_head = wmap_wrap_index(g_wmap_artifact_list_end - (WMAP_ARTIFACT_SLOTS - 1), 0, g_wmap_artifact_list_end);
    g_wmap_artifact_display_tail = wmap_wrap_index(g_wmap_artifact_display_head + WMAP_ARTIFACT_SLOTS - 1, 0, WMAP_ARTIFACT_SLOTS - 1);
    g_wmap_artifact_list_tail = wmap_wrap_index(g_wmap_artifact_list_head + WMAP_ARTIFACT_SLOTS - 1, 0, g_wmap_artifact_list_end);
    return g_wmap_artifact_list_end - (WMAP_ARTIFACT_SLOTS - 1);
}

/** @brief Append a map cell's saved values and optional change indicators.
 * @param mode Nonzero to suppress change indicators.
 * @param x Map column.
 * @param y Map row.
 * @param mask Flag to set when the cell is occupied.
 * @param count Current output length, updated for each appended value.
 * @param flags Output flags.
 * @param output Destination values.
 * @param unused Unused argument slot.
 * @param comparison Proposed spirit levels indexed by land.
 */
void wmap_append_land_spirit_labels(s32 mode, u32 x, s32 y, s32 mask, s32* count, s32* flags, s32* output, s32 unused,
                                    s32 (*comparison)[FIELD_LAND_SPIRIT_COUNT])
{

    u32 level;
    s32 difference;
    s32 spirit;
    s32 label_base = WMAP_LABEL_LEVEL_BASE;
    s32 cell = WMAP_NO_LAND;

    if (x < (u32)WMAP_GRID_SIZE && y >= 0 && y < WMAP_GRID_SIZE)
    {
        cell = g_wmap_land_lookup[x + y * WMAP_GRID_SIZE];
    }
    else
    {
        cell = WMAP_NO_LAND;
    }

    if (cell != WMAP_NO_LAND)
    {
        *flags |= mask;
        for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT; spirit++)
        {
            level = g_saved_game.layout.lands[cell].levels[spirit];
            output[*count] = level + (s8)label_base;
            (*count)++;
        }

        if (mode == 0)
        {
            for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT; spirit++)
            {
                level = g_saved_game.layout.lands[cell].levels[spirit];
                difference = comparison[cell][spirit] - level;

                if (difference == 0)
                {
                    output[*count] = 0;
                    (*count)++;
                    output[*count] = 0;
                }
                else
                {
                    if (difference < 0)
                    {
                        output[*count] = WMAP_LABEL_DECREASE;
                    }
                    else
                    {
                        output[*count] = WMAP_LABEL_INCREASE;
                    }

                    (*count)++;
                    output[*count] = difference + WMAP_LABEL_CHANGE_BASE;
                }

                (*count)++;
            }
        }
        else
        {
            for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT * 2; spirit++)
            {
                output[*count] = 0;
                (*count)++;
            }
        }
    }
}

/**
 * @brief Build spirit levels and change labels for the three-by-three placement grid.
 * @param selected_cell Selected cell within the displayed three-by-three grid.
 * @param x Selected map column.
 * @param y Selected map row.
 * @param groups Receives the mask of nonempty label groups.
 * @param output Destination for the packed label groups.
 * @param land Artifact being previewed.
 */
void wmap_build_placement_labels(s32 selected_cell, u32 x, s32 y, s32* groups, s32* output, s32 land)
{
    u32 left_x;
    s32 count;
    s32 difference;
    s32 group_mask;
    s32 cell_y;
    s32 row_offset;
    s32 top_y;
    s32 terrain_offset;
    s32 next_count;
    s32 cell_index;
    s32 blank_count;
    s32 terrain_total;
    s32 component;
    WmapLayout* terrain;
    s32 spirit;
    s32 record;
    s32 current_group_row;
    s32 component_offset;
    u32 cell_x;
    s32 level;
    u8 current_land;
    u8 cell_land;

    record = 0;

    count = 0;

    *groups = 0;
    for (record = 0; record < FIELD_LAND_COUNT; record++)
    {
        for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT; spirit++)
        {
            g_wmap_proposed_spirits[record][spirit] = g_saved_game.layout.lands[record].levels[spirit];
        }
    }

    for (spirit = 0; spirit < FIELD_LAND_SPIRIT_COUNT; spirit++)
    {
        g_wmap_proposed_spirits[land][spirit] = g_wmap_land_attributes[land].data.spirits[spirit];
    }

    if ((x < WMAP_GRID_SIZE) && (y >= 0) && (y < WMAP_GRID_SIZE))
    {
        current_land = g_wmap_land_lookup[x + y * WMAP_GRID_SIZE];
    }
    else
    {
        current_land = WMAP_NO_LAND;
    }

    if (current_land == WMAP_NO_LAND)
    {

        wmap_apply_land_influence(land, x, y, g_wmap_proposed_spirits);
    }

    record = 0;

    left_x = x - (selected_cell % 3);
    top_y = y - (selected_cell / 3);

    while (record < 3)
    {
        spirit = 0;
        /* Preserve the row copy through loop strength reduction. */
        current_group_row = (record * 3) + record;
        current_group_row -= record;
        cell_y = top_y + record;
        row_offset = cell_y * WMAP_GRID_SIZE;
        cell_x = left_x;

        while (spirit < 3)
        {
            group_mask = 1 << (current_group_row + spirit);

            if ((cell_x < WMAP_GRID_SIZE) && (cell_y >= 0) && (cell_y < WMAP_GRID_SIZE))
            {
                cell_land = *(cell_x + row_offset + (u8*)&g_wmap_land_lookup);
            }
            else
            {
                cell_land = WMAP_NO_LAND;
            }

            if (cell_land != WMAP_NO_LAND)
            {

                wmap_append_land_spirit_labels(0, cell_x, top_y + record, group_mask, &count, groups, output, land, g_wmap_proposed_spirits);

                cell_x++;
            }
            else
            {
                terrain_total = 0;
                component = 0;
                terrain = g_wmap_layout;
                cell_index = cell_x + row_offset;
                component_offset = cell_index * sizeof(WmapTerrainCell);

                while (component < FIELD_LAND_SPIRIT_COUNT)
                {
                    component_offset = component + (cell_index * sizeof(WmapTerrainCell));
                    terrain_total += ((WmapLayout*)((u8*)terrain + component_offset))->cells[0].spirits[0];
                    component++;
                }

                if (terrain_total > 0)
                {
                    component = 0;

                    *groups |= group_mask;
                    while (component < FIELD_LAND_SPIRIT_COUNT)
                    {
                        terrain_offset = component + ((cell_x + row_offset) * sizeof(WmapTerrainCell));
                        component++;
                        level = ((WmapLayout*)(((u8*)g_wmap_layout + terrain_offset)))->cells[0].spirits[0];
                        output[count] = level + WMAP_LABEL_TERRAIN_BASE;
                        count++;
                    }

                    component = 0;

                    while (component < 0x10)
                    {
                        component++;
                        output[count] = 0;
                        count++;
                    }
                }

                cell_x++;
            }

            spirit++;
        }

        record++;
    }

    record = 0;

    *groups |= WMAP_LABEL_PROPOSED_GROUP;
    while (record < FIELD_LAND_SPIRIT_COUNT)
    {
        level = g_wmap_land_attributes[land].data.spirits[record];
        difference = g_wmap_proposed_spirits[land][record] - level;

        if (difference == 0)
        {
            blank_count = count + 1;
            output[count] = 0;
            count = blank_count;
            output[blank_count] = 0;
        }
        else
        {
            if (difference < 0)
            {
                output[count] = WMAP_LABEL_PROPOSED_DECREASE;
            }
            else
            {
                output[count] = WMAP_LABEL_PROPOSED_INCREASE;
            }

            next_count = count + 1;
            count = next_count;
            output[next_count] = difference + WMAP_LABEL_PROPOSED_CHANGE_BASE;
        }

        record++;
        count++;
    }
}

/**
 * @brief Empty world-map handler (no operation).
 */
void wmap_land_layout_stub_1(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void wmap_land_layout_stub_2(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void wmap_land_layout_stub_3(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void wmap_land_layout_stub_4(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void wmap_land_layout_stub_5(void)
{
}

/**
 * @brief Read the current day of the world-map cycle.
 * @return Value masked to seven bits.
 */
s32 wmap_get_day(void)
{
    return g_saved_game.layout.control.fields.weekday & WMAP_DAY_MASK;
}

/**
 * @brief Select the map graphics tier from the number of placed lands.
 * @return Clamped table value.
 */
s32 wmap_get_land_count_tier(void)
{
    WmapValueTable table = g_wmap_land_count_tiers;
    s32 value;
    s32 result;

    value = table.values[g_saved_game.layout.control.fields.placed_land_count];

    if (value >= 0)
    {
        result = 9;

        if (value < 10)
        {
            result = value;
        }
    }
    else
    {
        result = 0;
    }

    return result;
}

/**
 * @brief Test FIELD_LAND_CLEARED of a saved land.
 * @param record_index Saved land index.
 * @return One when the land is cleared, otherwise zero.
 */
s32 wmap_is_land_cleared(s32 record_index)
{
    return (g_saved_game.layout.lands[record_index].flags >> 2) & 1;
}

/**
 * @brief Test whether a different grid location has a valid table entry.
 * @param x Grid column.
 * @param y Grid row.
 * @return Nonzero if the location differs from the current location and is valid.
 */
s32 wmap_is_other_land_cell(u32 x, s32 y)
{
    s32 value;

    if (((g_wmap_replacement_land_status >> 8) & WMAP_COORD_MASK) == x && ((g_wmap_replacement_land_status >> 12) & WMAP_COORD_MASK) == y)
    {
        return 0;
    }

    if (x < 6 && y >= 0 && y < WMAP_GRID_SIZE)
    {
        value = g_wmap_land_lookup[x + y * WMAP_GRID_SIZE];
    }
    else
    {
        value = WMAP_NO_LAND;
    }

    return value != WMAP_NO_LAND;
}

/**
 * @brief Read the remaining land count, accounting for the special saved land.
 * @return The layout's land count minus the placed land count, plus one while the SS Buccaneer is placed on a cell with WMAP_TERRAIN_SEA.
 */
s32 wmap_get_remaining_land_count(void)
{
    u32 flags = g_saved_game.land_words.lands[FIELD_LAND_SS_BUCCANEER].word;
    WmapLayout* data;
    s32 column;
    s32 row;
    s32 cell;
    WmapLayout* entry;

    if (flags & FIELD_LAND_PLACED)
    {
        data = g_wmap_layout;
        column = (flags >> 8) & WMAP_COORD_MASK;
        row = (flags >> 12) & WMAP_COORD_MASK;
        cell = column + row * WMAP_GRID_SIZE;
        entry = (WmapLayout*)((u8*)data + cell * sizeof(WmapTerrainCell));

        if (entry->cells[0].flags & WMAP_TERRAIN_SEA)
        {
            return data->land_count - g_saved_game.layout.control.fields.placed_land_count + 1;
        }
    }

    return g_wmap_layout->land_count - g_saved_game.layout.control.fields.placed_land_count;
}

/**
 * @brief Find the land occupying a map cell.
 * @param column Column index.
 * @param row Row index.
 * @return Table entry, or 0xFF when an index is outside the table.
 */
s32 wmap_get_land_at_cell(u32 column, s32 row)
{
    if (column < WMAP_GRID_SIZE && row >= 0 && row < WMAP_GRID_SIZE)
    {
        return g_wmap_land_lookup[column + row * WMAP_GRID_SIZE];
    }

    return WMAP_NO_LAND;
}

/**
 * @brief Resolve the eight spirit icons for an occupied or empty map cell.
 * @return Unspecified; callers use only the output buffer.
 * @param column Map column.
 * @param row Map row.
 * @param output Destination for eight words.
 */
s32 wmap_get_cell_spirit_sprites(u32 column, s32 row, s32* output)
{
    s32 record;
    s32 i;
    s32 index;
    s32 value;
    s32 cell;

    if (column < WMAP_GRID_SIZE && row >= 0 && row < WMAP_GRID_SIZE)
    {
        cell = column + row * WMAP_GRID_SIZE;
        record = wmap_get_land_at_cell(column, row);

        for (i = 0; i < FIELD_LAND_SPIRIT_COUNT; i++)
        {
            if (record == WMAP_NO_LAND)
            {
                index = g_wmap_layout->cells[cell].spirits[i] + 3;
            }
            else
            {
                index = g_saved_game.layout.lands[record].levels[i];
            }

            value = 0;

            if (index >= 0)
            {
                value = g_wmap_spirit_sprites[index * WMAP_SPIRIT_SPRITE_DIAGONAL];
            }

            output[i] = value;
        }
    }
}

/**
 * @brief Resolve the eight base spirit icons for an artifact.
 * @param table_index Table row, or -1 to use entry three for every output.
 * @param output Destination for eight words.
 */
void wmap_get_artifact_spirit_sprites(s32 table_index, s32* output)
{
    s32* entry;
    s32 i;
    s32 value;
    s32 index;

    entry = output;

    for (i = 0; i < FIELD_LAND_SPIRIT_COUNT; i++)
    {
        index = FIELD_LAND_SPIRIT_NEUTRAL;

        if (table_index != -1)
        {
            index = g_wmap_land_attributes[table_index].data.spirits[i];
        }

        value = 0;

        if (index >= 0)
        {
            value = g_wmap_spirit_sprites[index * WMAP_SPIRIT_SPRITE_DIAGONAL];
        }

        *entry = value;
        entry++;
    }
}

/**
 * @brief Test whether a land's artifact is obtained but the land is not placed yet.
 * @param record_index Saved land index.
 * @return One when both flag conditions hold, otherwise zero.
 */
s32 wmap_is_artifact_unplaced(s32 record_index)
{
    u32 flags = g_saved_game.layout.lands[record_index].flags;

    if (flags & FIELD_LAND_ARTIFACT_OBTAINED)
    {
        return ((flags >> 1) & 1) ^ 1;
    }

    return 0;
}

/**
 * @brief Read the starting land coordinates, falling back to the layout defaults.
 * @param x Receives the starting column.
 * @param y Receives the starting row.
 * @return The hero's character type (FIELD_CHARACTER_TYPE_MASK of info byte 0); nonzero selects the alternate traveler sprites.
 * @note The starting land is g_current_land.
 */
s32 wmap_get_starting_cell(s32* x, s32* y)
{
    if (!(g_saved_game.land_words.lands[0].word & FIELD_LAND_PLACED))
    {
        *x = g_wmap_layout->starting_x;
        *y = g_wmap_layout->starting_y;
    }
    else
    {
        *x = g_saved_game.land_words.lands[g_current_land].bytes.cell & WMAP_COORD_MASK;
        *y = g_saved_game.land_words.lands[g_current_land].bytes.cell >> WMAP_COORD_Y_SHIFT;
    }

    return g_wmap_hero_character_info & FIELD_CHARACTER_TYPE_MASK;
}

/**
 * @brief Return the artifact at the carousel selection position.
 * @return Value at the wrapped index.
 */
s32 wmap_get_selected_artifact(void)
{
    return g_wmap_artifact_list[wmap_wrap_index(g_wmap_artifact_list_head + 4, 0, g_wmap_artifact_list_end)];
}

/**
 * @brief Wrap a value into an inclusive range.
 * @param value Value to wrap.
 * @param minimum Lower endpoint.
 * @param maximum Upper endpoint; must not be less than minimum.
 * @return Wrapped value.
 */
s32 wmap_wrap_index(s32 value, s32 minimum, s32 maximum)
{
    s32 adjusted;

    if (value < minimum)
    {
        adjusted = value - minimum + 1;
        return wmap_wrap_index(adjusted + maximum, minimum, maximum);
    }

    if (maximum < value)
    {
        adjusted = value - 1;
        adjusted -= maximum;
        return wmap_wrap_index(adjusted + minimum, minimum, maximum);
    }

    return value;
}

/**
 * @brief Resolve a spirit icon from its base and proposed levels.
 * @param row Base spirit level.
 * @param column Proposed spirit level.
 * @return Table value, or zero if either index is negative.
 */
s32 wmap_get_spirit_sprite(s32 row, s32 column)
{
    s32 value = 0;

    if (row >= 0 && column >= 0)
    {
        value = g_wmap_spirit_sprites[row * WMAP_SPIRIT_SPRITE_COLUMNS + column];
    }

    return value;
}

/**
 * @brief Test whether saved flags 0, 9 through 14, 24, 27, or 28 are set.
 * @return One if any of the selected flags is set, otherwise zero.
 */
s32 wmap_has_growth_blocking_event(void)
{
    s32 index;
    s32 found;
    s32 word_index;
    s32* words;
    u32 bits;
    u32 flags;

    found = 0;

    for (index = 0; index < FIELD_LAND_COUNT; index++)
    {
        word_index = index / 32;
        words = &g_saved_game.words[word_index];
        bits = 1U << (index - word_index * 32);
        flags = words[SAVED_GAME_FLAG_WORD_INDEX];

        if ((flags & bits) && ((u32)(index - 9) < 6 || index == 24 || index == 0 || index == 27 || index == 28))
        {
            found = 1;
        }
    }

    return found;
}

/**
 * @brief Test whether saved flags 2, 3, 22, or 23 are set.
 * @return One if any of the four flags is set, otherwise zero.
 */
s32 wmap_has_persistent_land_event(void)
{
    s32 index;
    s32 found;
    s32 word_index;
    s32* words;
    u32 bits;
    u32 flags;

    found = 0;

    for (index = 0; index < FIELD_LAND_COUNT; index++)
    {
        word_index = index / 32;
        words = &g_saved_game.words[word_index];
        bits = 1U << (index - word_index * 32);
        flags = words[SAVED_GAME_FLAG_WORD_INDEX];

        if ((flags & bits) && ((u32)(index - 2) < 2 || index == 22 || index == 23))
        {
            found = 1;
        }
    }

    return found;
}

/**
 * @brief Add an artifact's spirit influence and clamp the destination levels.
 * @param table_row Adjustment row, or WMAP_NO_LAND to skip.
 * @param output_row Destination row, or WMAP_NO_LAND to skip.
 * @param output_address Byte address of the destination spirit-level table.
 */
void wmap_add_artifact_influence(s32 table_row, s32 output_row, u8* output_address)
{
    wmap_adjust_neighbor_spirits(table_row, output_row, (s32(*)[8])output_address);
}

/**
 * @brief Add a saved land's spirit influence relative to the neutral level.
 * @param record_index Record index; 0xFF disables the update.
 * @param other_index A second index; 0xFF disables the update.
 * @param unused Unused argument retained by the calling convention.
 * @param values Eight values updated in place.
 */
void wmap_add_saved_land_influence(s32 record_index, s32 other_index, s32 unused, s32* values)
{
    wmap_accumulate_land_spirits(record_index, other_index, unused, values);
}
