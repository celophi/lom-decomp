#include "wmap_land_layout.h"
#include "wmap_land_transition.h"
#include "saved_game.h"
#include "main.h"
#include "cdrom.h"

/* WMAP interprets its portion of the resident save buffer through this view. */
#define WMAP_SAVED_GAME (*(WmapSave*)&g_saved_game)
#define WMAP_LAND_COUNT 64
#define WMAP_GRID_SIZE 6
#define WMAP_SPIRIT_COUNT 8
#define WMAP_NO_LAND 0xFF
#define WMAP_SPIRIT_NEUTRAL 3
#define WMAP_SPIRIT_MAX 6
#define WMAP_LAND_AVAILABLE 0x01
#define WMAP_LAND_PLACED 0x02
#define WMAP_LAND_ACTIVE 0x04
#define WMAP_DAY_MASK 0x7F
#define WMAP_LAYOUT_RESOURCE_BASE 0x12EA
#define WMAP_TERRAIN_FLAG_0 0x01
#define WMAP_TERRAIN_FLAG_1 0x02
#define WMAP_TERRAIN_FLAG_2 0x04
#define WMAP_ARTIFACT_TERRAIN_REQUIRED_SHIFT 10
#define WMAP_ARTIFACT_FLAG_8_SHIFT 8
#define WMAP_REPLACED_LAND 24
#define WMAP_REPLACEMENT_LAND 33
#define WMAP_SPECIAL_LAND 31
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

#define WMAP_SAVED_SPIRITS_OFFSET ((s32) & ((WmapSave*)0)->lands[0].spirits)

/** @brief Spirit strengths and placement restrictions for an artifact. */
typedef union
{
    u8 bytes[12];
    struct
    {
        u8 spirits[WMAP_SPIRIT_COUNT];
        u32 flags;
    } data;
} WmapLandAttributes;

/** @brief Terrain spirit bonuses and placement flags for one map cell. */
typedef struct
{
    u8 spirits[WMAP_SPIRIT_COUNT];
    u16 flags;
    u16 unknown_0x0a;
} WmapTerrainCell;

/** @brief Layout file header followed by the six-by-six terrain grid. */
typedef struct
{
    u16 starting_x;
    u16 starting_y;
    s32 land_count;
    WmapTerrainCell cells[WMAP_GRID_SIZE * WMAP_GRID_SIZE];
    u16 scenes[256];
    u8 entry_flags[256];
} WmapLayout;

/** @brief Saved land state; coordinates share a byte and flags share a word. */
typedef union
{
    u32 word;
    struct
    {
        u8 flags;
        u8 position;
        u8 placement_order;
        u8 artifact_order;
    } data;
} WmapLandStatus;

/** @brief Saved placement status and eight spirit levels. */
typedef struct
{
    WmapLandStatus status;
    u8 spirits[WMAP_SPIRIT_COUNT];
} WmapSavedLand;

/** @brief Save fields used by the land-layout routines. */
typedef struct
{
    u8 unknown_0x000[0x2E4];
    union
    {
        u32 word;
        struct
        {
            u8 unknown_0x2e4;
            u8 placed_land_count;
            u16 day;
        } data;
    } calendar;
    u32 events[2];
    WmapSavedLand lands[WMAP_LAND_COUNT];
} WmapSave;

/** @brief Timed save record updated when a travel day elapses. */
typedef struct
{
    u8 unknown_0x000[0x2EF4];
    u8 active;
    u8 unknown_0x2ef5[0x2F36 - 0x2EF5];
    u16 age;
    s32 direction;
} WmapTimedSaveView;

/** @brief Save view for the daily growth counters. */
typedef struct
{
    u8 unknown_0x000[0x26F8];
    u8 growth[8];
} WmapGrowthSaveView;

/** @brief Eight growth counters in one saved slot. */
typedef struct
{
    u8 spirits[8];
    u8 unknown_0x08[8];
} WmapGrowthSlot;

/** @brief Eight growth slots followed by twelve bytes of other saved data. */
typedef struct
{
    WmapGrowthSlot slots[8];
    u8 unknown_0x80[12];
} WmapGrowthGroup;

/** @brief Saved growth groups advanced when the party travels. */
typedef struct
{
    u8 unknown_0x000[0x26F8];
    WmapGrowthGroup groups[4];
} WmapGrowthSave;

/** @brief Save header fields used to select the map layout. */
typedef struct
{
    u8 unknown_0x00[0x28];
    unsigned int unused_flags : 2;
    unsigned int option : 1;
    unsigned int other_flags : 29;
    u8 unknown_0x2c[0xE0 - 0x2C];
    u16 layout_id;
} WmapSaveHeader;

/** @brief Saved map entry status at its original byte offsets. */
typedef struct
{
    u8 pad_000[0x2F0];
    union
    {
        struct
        {
            unsigned int available : 1;
            unsigned int placed : 1;
            unsigned int other_flags : 6;
        } flags;
        struct
        {
            u8 flags;
            u8 pad[2];
            u8 artifact_order;
        } data;
    } entry;
} WmapSavedEntry;

/** @brief Table of 64 world-map values copied to a local buffer. */
typedef struct
{
    s32 values[64];
} WmapValueTable;

extern u16 g_wmap_saved_day;
extern u8 g_wmap_placed_land_count;
extern u8 g_wmap_saved_map_status;
extern s32 g_wmap_layout_selection;
extern WmapLandAttributes g_wmap_land_attributes[WMAP_LAND_COUNT];
extern s32 g_wmap_spirit_sprites[];
extern s32 g_wmap_proposed_spirits[WMAP_LAND_COUNT][WMAP_SPIRIT_COUNT];
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

void wmap_apply_land_influence(s32 land, u32 x, s32 y, s32 (*spirits)[8]);
void wmap_rebuild_land_lookup(void);
void wmap_append_land_spirit_labels(s32 mode, u32 x, s32 y, s32 mask, s32* count, s32* flags, s32* output, s32 unused, s32 (*comparison)[8]);
s32 wmap_wrap_index(s32 value, s32 minimum, s32 maximum);

/**
 * @brief Test the active flag of the land occupying a known occupied cell.
 * @param x Cell column.
 * @param y Cell row.
 * @return One if the land is active, otherwise zero.
 */
static inline s32 wmap_cell_has_active_land(u32 x, s32 y)
{
    WmapSave* save = (WmapSave*)g_saved_game.bytes;
    s32 land = wmap_get_land_at_cell(x, y);
    return (save->lands[land].status.data.flags >> 2) & 1;
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
static inline void wmap_adjust_neighbor_spirits(s32 table_row, s32 output_row, s32 (*output)[8])
{
    s32* entry;
    u8* table;
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
            table = (u8*)g_wmap_land_attributes;
            row_offset = table_row * (s32)sizeof(WmapLandAttributes);
            entry = (s32*)((output_row * (s32)sizeof(g_wmap_proposed_spirits[0])) + (s32)output);
            for (; index < WMAP_SPIRIT_COUNT; index++, entry++)
            {
                adjustment = *(u8*)(index + row_offset + (s32)table) - WMAP_SPIRIT_NEUTRAL;
                value = *entry + adjustment;
                *entry = value;
                if (value >= 0)
                {
                    clamped = WMAP_SPIRIT_MAX;
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
            index = 0;
            do
            {
                value = values[index] - WMAP_SPIRIT_NEUTRAL;
                values[index] = value + WMAP_SAVED_GAME.lands[record_index].spirits[index];
                index++;
            } while (index < WMAP_SPIRIT_COUNT);
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

    if (x < (u32)WMAP_GRID_SIZE && y >= 0 && y < WMAP_GRID_SIZE)
    {
        index = 0;
        layout = g_wmap_layout;
        for (; index < WMAP_SPIRIT_COUNT; index++)
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
    s32 growth[4][WMAP_SPIRIT_COUNT];
    WmapTimedSaveView* timed_record;
    s32 flag_word;
    s32 event;
    s32 blocked;
    s32 spirit;
    s32 day_bonus;
    s32 growth_slot;
    s32 growth_group;
    s32 event_dividend;
    s32 total;
    s32 clamped;
    u16 age;
    u16 remaining_age;
    u32 day_flags;
    u32 day;
    s32* day_growth;

    blocked = 0;
    event = 0;
    do
    {
        event_dividend = event;
        if (event < 0)
        {
            event_dividend = event + 0x1F;
        }
        flag_word = event_dividend >> 5;
        if ((((WmapSave*)((flag_word * 4) + (u8*)&g_saved_game))->events[0] & (1 << (event - (flag_word << 5)))) &&
            (((u32)(event - 9) < (u32)WMAP_GRID_SIZE) || (event == 0x18) || (event == 0) || (event == 0x1B) || (event == 0x1C)))
        {
            blocked = 1;
        }
        event += 1;
    } while (event < WMAP_LAND_COUNT);
    if (blocked == 0)
    {
        day_flags =
            (WMAP_SAVED_GAME.calendar.word & 0xFF80FFFF) | ((((((u32)WMAP_SAVED_GAME.calendar.word >> 0x10) & WMAP_DAY_MASK) + 1) & WMAP_DAY_MASK) << 0x10);
        WMAP_SAVED_GAME.calendar.word = day_flags;
        if ((u32)((day_flags >> 0x10) & WMAP_DAY_MASK) >= 6U)
        {
            WMAP_SAVED_GAME.calendar.word = (u32)(day_flags & 0xFF80FFFF);
        }
        growth_group = 0;
        do
        {
            timed_record = (WmapTimedSaveView*)(g_saved_game.bytes + growth_group * 96);
            if (timed_record->active != 0)
            {
                if (timed_record->direction < 0)
                {
                    remaining_age = timed_record->age;
                    if (remaining_age != 0)
                    {
                        timed_record->age = (u16)(remaining_age - 1);
                    }
                }
                else
                {
                    age = timed_record->age;
                    if (age <= 0xFFFEU)
                    {
                        timed_record->age = (u16)(age + 1);
                    }
                }
            }
            growth_group += 1;
        } while (growth_group < 5);
        for (spirit = 0; spirit < WMAP_SPIRIT_COUNT; spirit++)
        {
            growth[0][spirit] = WMAP_SAVED_GAME.lands[0].spirits[spirit] - WMAP_SPIRIT_NEUTRAL;
            growth[1][spirit] = WMAP_SAVED_GAME.lands[32].spirits[spirit] - WMAP_SPIRIT_NEUTRAL;
            growth[2][spirit] = 0;
            growth[3][spirit] = 0;
        }
        day = g_wmap_saved_day & WMAP_DAY_MASK;
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
        growth_group = 0;
        day_growth = &growth[0][day_bonus];
        day_growth[0] = (s32)(day_growth[0] + 1);
        day_growth[8] = (s32)(day_growth[8] + 1);
        day_growth[16] = (s32)(day_growth[16] + 1);
        day_growth[24] = (s32)(day_growth[24] + 1);
        growth[0][0] += 1;
        growth[0][1] += 1;
        growth[1][0] += 1;
        growth[1][1] += 1;
        growth[2][0] += 1;
        growth[2][1] += 1;
        growth[3][0] += 1;
        growth[3][1] += 1;
        do
        {
            growth_slot = 0;
            do
            {
                spirit = 0;
                do
                {
                    total = ((WmapGrowthSave*)&g_saved_game)->groups[growth_group].slots[growth_slot].spirits[spirit];
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
                    ((WmapGrowthSave*)&g_saved_game)->groups[growth_group].slots[growth_slot].spirits[spirit] = clamped;
                    spirit += 1;
                } while (spirit < WMAP_SPIRIT_COUNT);
                growth_slot += 1;
            } while (growth_slot < WMAP_SPIRIT_COUNT);
            growth_group += 1;
        } while (growth_group < 4);
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
        if (wmap_get_artifact_flag(land, WMAP_ARTIFACT_TERRAIN_REQUIRED_SHIFT) == 1)
        {
            if (!(g_wmap_layout->cells[cell].flags & WMAP_TERRAIN_FLAG_2))
            {
                return 0;
            }
        }
        else if ((u32)(land - WMAP_REPLACED_LAND) >= 2U && land != WMAP_SPECIAL_LAND && (g_wmap_layout->cells[cell].flags & WMAP_TERRAIN_FLAG_0))
        {
            return 0;
        }
        if (g_wmap_placed_land_count == 0)
        {
            return 1;
        }
        if (!(g_wmap_layout->cells[cell].flags & WMAP_TERRAIN_FLAG_1))
        {
            if (wmap_get_artifact_flag(land, WMAP_ARTIFACT_FLAG_8_SHIFT) == 1)
            {
                return 0;
            }
        }
        if (y > 0 && wmap_get_land_at_cell(x, y - 1) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_active_land(x, y - 1) == 1)
            {
                return 1;
            }
        }
        if (y < WMAP_GRID_SIZE - 1 && wmap_get_land_at_cell(x, y + 1) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_active_land(x, y + 1) == 1)
            {
                return 1;
            }
        }
        if ((s32)x > 0 && wmap_get_land_at_cell(x - 1, y) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_active_land(x - 1, y) == 1)
            {
                return 1;
            }
        }
        if ((s32)x < WMAP_GRID_SIZE - 1 && wmap_get_land_at_cell(x + 1, y) != WMAP_NO_LAND)
        {
            if (wmap_cell_has_active_land(x + 1, y) == 1)
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
    /* The original routine reserves an unused eight-byte local. */
    u8 unused_spirits[WMAP_SPIRIT_COUNT];
    s32 record;
    s32 field;
    s32 row_bits;
    WmapSave* entry;
    u8* save_data;

    save_data = g_saved_game.bytes;
    entry = (WmapSave*)(save_data + index * (s32)sizeof(WmapSavedLand));
    row_bits = y * 16;
    entry->lands[0].status.data.position = (x & WMAP_COORD_MASK) | row_bits;
    entry->lands[0].status.data.flags |= WMAP_LAND_PLACED;
    WMAP_SAVED_GAME.calendar.data.placed_land_count++;
    entry->lands[0].status.data.placement_order = WMAP_SAVED_GAME.calendar.data.placed_land_count;
    if (WMAP_SAVED_GAME.calendar.data.placed_land_count == 1)
    {
        g_music_track_index = index;
    }
    /* Apply placement in a word workspace before committing the byte-sized save levels. */
    for (record = 0; record < WMAP_LAND_COUNT; record++)
    {
        for (field = 0; field < WMAP_SPIRIT_COUNT; field++)
        {
            g_wmap_proposed_spirits[record][field] = WMAP_SAVED_GAME.lands[record].spirits[field];
        }
    }
    for (field = 0; field < WMAP_SPIRIT_COUNT; field++)
    {
        g_wmap_proposed_spirits[index][field] = g_wmap_land_attributes[index].data.spirits[field];
    }
    wmap_apply_land_influence(index, x, y, g_wmap_proposed_spirits);
    for (record = 0; record < WMAP_LAND_COUNT; record++)
    {
        for (field = 0; field < WMAP_SPIRIT_COUNT; field++)
        {
            WMAP_SAVED_GAME.lands[record].spirits[field] = g_wmap_proposed_spirits[record][field];
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
void wmap_apply_land_influence(s32 land, u32 x, s32 y, s32 (*spirits)[8])
{
    s32 neighbor_influence[WMAP_SPIRIT_COUNT];
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

    if ((x < (u32)WMAP_GRID_SIZE) && (y >= 0) && (y < WMAP_GRID_SIZE) && (wmap_can_place_land(x, y, land) != 0))
    {
        /* Neighbors receive the artifact contribution, clamped independently. */
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x - 1, y), spirits);
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x + 1, y), spirits);
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x, y - 1), spirits);
        wmap_adjust_neighbor_spirits(land, wmap_get_land_at_cell(x, y + 1), spirits);
        spirit = 0;
        terrain = g_wmap_layout;
        cell_offset = (x + y * WMAP_GRID_SIZE) * (s32)sizeof(WmapTerrainCell);
        terrain_spirit = (s32*)(land * (s32)sizeof(spirits[0]) + (s32)spirits);
        do
        {
            terrain_bonus = ((WmapLayout*)((u8*)terrain + (spirit + cell_offset)))->cells[0].spirits[0];
            spirit++;
            *terrain_spirit += terrain_bonus;
            terrain_spirit++;
        } while (spirit < WMAP_SPIRIT_COUNT);
        /* The new land receives half the combined influence of its four neighbors. */
        spirit = WMAP_SPIRIT_COUNT - 1;
        clear_entry = &neighbor_influence[WMAP_SPIRIT_COUNT - 1];
        do
        {
            *clear_entry = 0;
            spirit -= 1;
            clear_entry--;
        } while (spirit >= 0);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x - 1, y), land, 0, neighbor_influence);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x + 1, y), land, 0, neighbor_influence);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x, y - 1), land, 0, neighbor_influence);
        wmap_accumulate_land_spirits(wmap_get_land_at_cell(x, y + 1), land, 0, neighbor_influence);
        wmap_accumulate_terrain_spirits(x - 1, y, neighbor_influence);
        wmap_accumulate_terrain_spirits(x + 1, y, neighbor_influence);
        wmap_accumulate_terrain_spirits(x, y - 1, neighbor_influence);
        wmap_accumulate_terrain_spirits(x, y + 1, neighbor_influence);
        spirit = 0;
        land_spirit = (s32*)(land * (s32)sizeof(spirits[0]) + (s32)spirits);
        influence = neighbor_influence;
        do
        {
            neighbor_total = *influence;
            total = *land_spirit + (neighbor_total / 2);
            *land_spirit = total;
            if (total >= 0)
            {
                clamped = WMAP_SPIRIT_MAX;
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
            spirit += 1;
            influence++;
        } while (spirit < WMAP_SPIRIT_COUNT);
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

    if (x < (u32)WMAP_GRID_SIZE && y >= 0 && y < 6 && land < (u32)WMAP_LAND_COUNT)
    {
        if (proposed_x < (u32)WMAP_GRID_SIZE && proposed_y >= 0 && proposed_y < WMAP_GRID_SIZE)
        {
            cell = x + y * WMAP_GRID_SIZE;
            current_land = wmap_get_land_at_cell(x, y);
            for (i = 0; i < WMAP_LAND_COUNT; i++)
            {
                for (j = 0; j < WMAP_SPIRIT_COUNT; j++)
                {
                    g_wmap_proposed_spirits[i][j] = WMAP_SAVED_GAME.lands[i].spirits[j];
                }
            }
            for (j = 0; j < WMAP_SPIRIT_COUNT; j++)
            {
                g_wmap_proposed_spirits[land][j] = g_wmap_land_attributes[land].data.spirits[j];
            }
            wmap_apply_land_influence(land, proposed_x, proposed_y, g_wmap_proposed_spirits);
            for (i = 0; i < WMAP_SPIRIT_COUNT; i++)
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
                    component = WMAP_SAVED_GAME.lands[current_land].spirits[i];
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
            for (i = 0; i < WMAP_SPIRIT_COUNT; i++)
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
    WmapSaveHeader* header;
    s32 index;
    u8* cell;
    s32 empty;
    u8 saved_second;
    u8 saved_first;
    u32 status;

    g_wmap_event_cursor = -1;
    cdrom_wait_queue_empty();
    header = (WmapSaveHeader*)&g_saved_game;
    cdrom_queue_read((header->layout_id + WMAP_LAYOUT_RESOURCE_BASE) & 0xFFFF, &g_wmap_layout_buffer);
    cdrom_wait_queue_empty();
    g_wmap_layout = &g_wmap_layout_buffer;
    g_wmap_land_scenes = g_wmap_layout_buffer.scenes;
    g_wmap_land_entry_flags = g_wmap_layout_buffer.entry_flags;
    if ((header->option != 1) && (WMAP_SAVED_GAME.calendar.data.placed_land_count == 1))
    {
        WMAP_SAVED_GAME.events[0] = (s32)(WMAP_SAVED_GAME.events[0] | 2);
    }
    if ((WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.word & WMAP_LAND_ACTIVE) &&
        !(WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].status.word & WMAP_LAND_ACTIVE))
    {
        status = WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].status.word | 7;
        status &= ~WMAP_STATUS_X_MASK;
        status |= WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.word & WMAP_STATUS_X_MASK;
        status &= ~WMAP_STATUS_Y_MASK;
        status |= WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.word & WMAP_STATUS_Y_MASK;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].status.word = status;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[0] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[1] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[2] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[3] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[4] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[5] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[6] = 3;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].spirits[7] = 3;
        saved_first = WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.data.placement_order;
        saved_second = WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.data.artifact_order;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.word &= ~2;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.word &= ~1;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.word |= 0xFF00;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.data.placement_order = 0U;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].status.data.artifact_order = 0U;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[0] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[1] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[2] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[3] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[4] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[5] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[6] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACED_LAND].spirits[7] = 0;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].status.data.placement_order = saved_first;
        WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].status.data.artifact_order = saved_second;
    }
    empty = WMAP_NO_LAND;
    index = 0x23;
    cell = g_wmap_land_lookup + index;
    do
    {
        *cell = empty;
        index -= 1;
        cell -= 1;
    } while (index >= 0);
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
    WmapSave* flags;

    g_wmap_event_cursor++;
    while (g_wmap_event_cursor < WMAP_LAND_COUNT)
    {
        save_words = g_saved_game.words;
        bank = g_wmap_event_cursor / 32;
        flags = (WmapSave*)(save_words + bank);
        mask = 1 << (g_wmap_event_cursor - (bank << 5));
        if (flags->events[0] & mask)
        {
            if (g_wmap_event_cursor == 0)
            {
                g_layout_sub_mode = 31;
            }
            if ((u32)(g_wmap_event_cursor - 2) >= 2U && g_wmap_event_cursor != 22 && g_wmap_event_cursor != 23 && g_wmap_event_cursor != 9 &&
                g_wmap_event_cursor != 10 && g_wmap_event_cursor != 11 && g_wmap_event_cursor != 12 && g_wmap_event_cursor != 13 && g_wmap_event_cursor != 14 &&
                g_wmap_event_cursor != 24 && g_wmap_event_cursor != 0 && g_wmap_event_cursor != 27 && g_wmap_event_cursor != 28)
            {
                flags->events[0] &= ~(1 << (g_wmap_event_cursor % 32));
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
    WmapSave* record;
    s32 x;
    u32 y;
    u8 position;
    u32 placed;

    index = 0;
    do
    {
        record = (WmapSave*)(g_saved_game.bytes + index * (s32)sizeof(WmapSavedLand));
        placed = ((u32)record->lands[0].status.data.flags >> 1) & 1;
        if (placed == 1)
        {
            if (index == WMAP_REPLACED_LAND)
            {
                if (!(WMAP_SAVED_GAME.lands[WMAP_REPLACEMENT_LAND].status.word & WMAP_LAND_ACTIVE))
                {
                    position = ((WmapSave*)(g_saved_game.bytes + WMAP_REPLACED_LAND * (s32)sizeof(WmapSavedLand)))->lands[0].status.data.position;
                    x = position & WMAP_COORD_MASK;
                    y = position >> WMAP_COORD_Y_SHIFT;
                }
            }
            else
            {
                position = record->lands[0].status.data.position;
                x = position & WMAP_COORD_MASK;
                y = position >> WMAP_COORD_Y_SHIFT;
            }
            g_wmap_land_lookup[x + y * WMAP_GRID_SIZE] = index;
        }
        index++;
    } while (index < WMAP_LAND_COUNT);
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
            if (g_wmap_artifact_list_end >= (u32)WMAP_ARTIFACT_SLOTS)
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
        if (g_wmap_artifact_list[g_wmap_artifact_list_end] == -1 && g_wmap_artifact_list_end >= (u32)WMAP_ARTIFACT_SLOTS &&
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
    WmapSavedEntry* entry;
    WmapSavedEntry* current_record;

    g_wmap_artifact_list_end = 0;
    empty = -1;
    for (i = 63; i >= 0; i--)
    {
        g_wmap_artifact_list[i] = empty;
    }
    for (i = 0; i < WMAP_LAND_COUNT; i++)
    {
        entry = (WmapSavedEntry*)((u8*)&g_saved_game + i * (s32)sizeof(WmapSavedLand));
        previous_index = entry->entry.data.artifact_order != 0;
        if (previous_index)
        {
            placed = ((u32)entry->entry.data.flags >> 1) & 1;
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
                    current_record = (WmapSavedEntry*)((u8*)&g_saved_game + current * (s32)sizeof(WmapSavedLand));
                    j = previous;
                    if (current_record->entry.data.artifact_order >
                        ((WmapSavedEntry*)((u8*)&g_saved_game + j * (s32)sizeof(WmapSavedLand)))->entry.data.artifact_order)
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
    for (; (u32)i < g_wmap_artifact_list_end + WMAP_ARTIFACT_SLOTS; i++)
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
void wmap_append_land_spirit_labels(s32 mode, u32 x, s32 y, s32 mask, s32* count, s32* flags, s32* output, s32 unused, s32 (*comparison)[8])
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
        for (spirit = 0; spirit < WMAP_SPIRIT_COUNT; spirit++)
        {
            level = WMAP_SAVED_GAME.lands[cell].spirits[spirit];
            output[*count] = level + (s8)label_base;
            (*count)++;
        }
        if (mode == 0)
        {
            for (spirit = 0; spirit < WMAP_SPIRIT_COUNT; spirit++)
            {
                level = WMAP_SAVED_GAME.lands[cell].spirits[spirit];
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
            for (spirit = 0; spirit < WMAP_SPIRIT_COUNT * 2; spirit++)
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
 * @note TODO: This reconstruction still differs from the original assembly.
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
    s32 group_row;
    s32 current_group_row;
    s32 component_offset;
    u32 cell_x;
    s32 level;
    u8 current_land;
    u8 cell_land;

    record = 0;

    count = 0;
    *groups = 0;
    for (record = 0; record < WMAP_LAND_COUNT; record++)
    {
        for (spirit = 0; spirit < WMAP_SPIRIT_COUNT; spirit++)
        {
            g_wmap_proposed_spirits[record][spirit] = WMAP_SAVED_GAME.lands[record].spirits[spirit];
        }
    }
    for (spirit = 0; spirit < WMAP_SPIRIT_COUNT; spirit++)
    {
        g_wmap_proposed_spirits[land][spirit] = g_wmap_land_attributes[land].data.spirits[spirit];
    }
    if ((x < (u32)WMAP_GRID_SIZE) && (y >= 0) && (y < WMAP_GRID_SIZE))
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

    group_row = 0;
    left_x = x - (selected_cell % 3);
    top_y = y - (selected_cell / 3);
    do
    {
        spirit = 0;
        current_group_row = group_row;
        cell_y = top_y + record;
        row_offset = cell_y * WMAP_GRID_SIZE;
        cell_x = left_x;

        do
        {
            group_mask = 1 << (current_group_row + spirit);
            if ((cell_x < (u32)WMAP_GRID_SIZE) && (cell_y >= 0) && (cell_y < WMAP_GRID_SIZE))
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

                cell_x += 1;
            }
            else
            {
                terrain_total = 0;
                component = 0;
                terrain = g_wmap_layout;
                cell_index = cell_x + row_offset;
                component_offset = cell_index * (s32)sizeof(WmapSavedLand);
                do
                {
                    component_offset = component + (cell_index * (s32)sizeof(WmapSavedLand));
                    terrain_total += ((WmapLayout*)((u8*)terrain + component_offset))->cells[0].spirits[0];
                    component++;
                } while (component < WMAP_SPIRIT_COUNT);
                if (terrain_total > 0)
                {
                    component = 0;
                    *groups |= group_mask;
                    do
                    {
                        terrain_offset = component + ((cell_x + row_offset) * (s32)sizeof(WmapSavedLand));
                        component += 1;
                        level = ((WmapLayout*)(((u8*)g_wmap_layout + terrain_offset)))->cells[0].spirits[0];
                        output[count] = level + WMAP_LABEL_TERRAIN_BASE;
                        count += 1;
                    } while (component < WMAP_SPIRIT_COUNT);
                    component = 0;
                    do
                    {
                        component += 1;
                        output[count] = 0;
                        count += 1;
                    } while (component < 0x10);
                }
                cell_x += 1;
            }
            spirit += 1;

        } while (spirit < 3);
        record += 1;
        group_row += 3;
    } while (record < 3);
    record = 0;
    *groups |= WMAP_LABEL_PROPOSED_GROUP;
    do
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
        record += 1;
        count += 1;
    } while (record < WMAP_SPIRIT_COUNT);
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D46C(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D474(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D47C(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D484(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D48C(void)
{
}

/**
 * @brief Read the current day of the world-map cycle.
 * @return Value masked to seven bits.
 */
s32 wmap_get_day(void)
{
    return g_wmap_saved_day & WMAP_DAY_MASK;
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

    value = table.values[g_wmap_placed_land_count];
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
 * @brief Test the active flag of a saved land.
 * @param record_index Saved land index.
 * @return One when bit two is set, otherwise zero.
 */
s32 wmap_is_land_active(s32 record_index)
{
    return (WMAP_SAVED_GAME.lands[record_index].status.data.flags >> 2) & 1;
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
 * @return Difference from the saved progress byte, or from the current progress byte.
 */
s32 wmap_get_remaining_land_count(void)
{
    u32 flags = WMAP_SAVED_GAME.lands[27].status.word;
    WmapLayout* data;
    s32 column;
    s32 row;
    s32 cell;
    WmapLayout* entry;

    if (flags & WMAP_LAND_PLACED)
    {
        data = g_wmap_layout;
        column = (flags >> 8) & WMAP_COORD_MASK;
        row = (flags >> 12) & WMAP_COORD_MASK;
        cell = column + row * WMAP_GRID_SIZE;
        entry = (WmapLayout*)((u8*)data + cell * (s32)sizeof(WmapTerrainCell));
        if (entry->cells[0].flags & 4)
        {
            return data->land_count - WMAP_SAVED_GAME.calendar.data.placed_land_count + 1;
        }
    }
    return g_wmap_layout->land_count - g_wmap_placed_land_count;
}

/**
 * @brief Find the land occupying a map cell.
 * @param column Column index.
 * @param row Row index.
 * @return Table entry, or 0xFF when an index is outside the table.
 */
s32 wmap_get_land_at_cell(u32 column, s32 row)
{
    if (column < (u32)WMAP_GRID_SIZE && row >= 0 && row < WMAP_GRID_SIZE)
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

    if (column < (u32)WMAP_GRID_SIZE && row >= 0 && row < WMAP_GRID_SIZE)
    {
        cell = column + row * WMAP_GRID_SIZE;
        record = wmap_get_land_at_cell(column, row);
        i = 0;
        do
        {
            if (record == WMAP_NO_LAND)
            {
                index = g_wmap_layout->cells[cell].spirits[i] + 3;
            }
            else
            {
                index = WMAP_SAVED_GAME.lands[record].spirits[i];
            }
            value = 0;
            if (index >= 0)
            {
                value = g_wmap_spirit_sprites[index * WMAP_SPIRIT_SPRITE_DIAGONAL];
            }
            output[i] = value;
            i++;
        } while (i < WMAP_SPIRIT_COUNT);
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
    i = 0;
    do
    {
        index = WMAP_SPIRIT_NEUTRAL;
        if (table_index != -1)
        {
            index = g_wmap_land_attributes[table_index].data.spirits[i];
        }
        value = 0;
        if ((s32)index >= 0)
        {
            value = g_wmap_spirit_sprites[index * WMAP_SPIRIT_SPRITE_DIAGONAL];
        }
        *entry = value;
        i += 1;
        entry += 1;
    } while (i < WMAP_SPIRIT_COUNT);
}

/**
 * @brief Test whether an artifact is available and has not been placed.
 * @param record_index Saved land index.
 * @return One when both flag conditions hold, otherwise zero.
 */
s32 wmap_is_artifact_unplaced(s32 record_index)
{
    u32 flags = WMAP_SAVED_GAME.lands[record_index].status.data.flags;
    if (flags & WMAP_LAND_AVAILABLE)
    {
        return ((flags >> 1) & 1) ^ 1;
    }
    return 0;
}

/**
 * @brief Read the starting land coordinates, falling back to the layout defaults.
 * @param x Receives the starting column.
 * @param y Receives the starting row.
 * @return Low seven bits of the saved status byte; its meaning is unresolved.
 * @note WMAP uses the resident music-index slot to hold its starting land index.
 */
s32 wmap_get_starting_cell(s32* x, s32* y)
{
    if (!(WMAP_SAVED_GAME.lands[0].status.word & WMAP_LAND_PLACED))
    {
        *x = g_wmap_layout->starting_x;
        *y = g_wmap_layout->starting_y;
    }
    else
    {
        *x = WMAP_SAVED_GAME.lands[g_music_track_index].status.data.position & WMAP_COORD_MASK;
        *y = WMAP_SAVED_GAME.lands[g_music_track_index].status.data.position >> WMAP_COORD_Y_SHIFT;
    }
    return g_wmap_saved_map_status & WMAP_DAY_MASK;
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
    u32 bits;
    u32 flags;
    WmapSave* word_base;

    found = 0;
    index = 0;
    do
    {
        word_index = index / 32;
        word_base = (WmapSave*)(word_index * 4 + g_saved_game.bytes);
        bits = 1U << (index - word_index * 32);
        flags = word_base->events[0];
        if ((flags & bits) && ((u32)(index - 9) < 6 || index == 24 || index == 0 || index == 27 || index == 28))
        {
            found = 1;
        }
        index++;
    } while (index < WMAP_LAND_COUNT);
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
    u32 bits;
    u32 flags;
    WmapSave* word_base;

    found = 0;
    index = 0;
    do
    {
        word_index = index / 32;
        word_base = (WmapSave*)(word_index * 4 + g_saved_game.bytes);
        bits = 1U << (index - word_index * 32);
        flags = word_base->events[0];
        if ((flags & bits) && ((u32)(index - 2) < 2 || index == 22 || index == 23))
        {
            found = 1;
        }
        index++;
    } while (index < WMAP_LAND_COUNT);
    return found;
}

/**
 * @brief Add an artifact's spirit influence and clamp the destination levels.
 * @param table_row Adjustment row, or WMAP_NO_LAND to skip.
 * @param output_row Destination row, or WMAP_NO_LAND to skip.
 * @param output_address Byte address of the destination spirit-level table.
 */
void wmap_add_artifact_influence(s32 table_row, s32 output_row, s32 output_address)
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
