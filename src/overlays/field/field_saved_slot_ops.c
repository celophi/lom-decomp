/**
 * @file field_saved_slot_ops.c
 * @brief Create, release, query and rename the stored companion records.
 */

#include "game_audio.h"
#include "common.h"
#include "main.h"
#include "sdk/rand.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Resource id of the companion template table. */
#define FIELD_RESOURCE_COMPANION_TEMPLATES 7

/** @brief Diagnostic codes of this file (DIAG_BAD_COMPANION is shared). */
#define DIAG_BAD_COMPANION_STATUS 0x76   /**< Status query for a companion index out of range. */
#define DIAG_BAD_COMPANION_RENAME 0x77   /**< Rename of a companion index out of range. */
#define DIAG_NO_COMPANION_TEMPLATES 0x78 /**< The companion template table (resource 7) is not loaded. */

/** @brief Returned when the template table is missing or no stored companion can be released. */
#define FIELD_COMPANION_NONE 0xFF

/** @brief Random-name source of the name-entry screen for a stored companion. */
#define FIELD_NAME_SOURCE_COMPANION 3

/** @brief Unique-id bits taken from the random value. */
#define FIELD_RANDOM_VALUE_MASK 0xFF00FF00

/** @brief Unique-id bits taken from the game-state words at unkD4/unkD6. */
#define FIELD_FIXED_VALUE_MASK 0x00FF00FF

/** @brief Companion status bit 30: the companion gains experience. */
#define FIELD_COMPANION_GAINS_EXPERIENCE_BIT 30

/** @brief Results of field_get_stored_companion_status. */
enum
{
    FIELD_COMPANION_STATUS_NEW_HELD = 0,         /**< Newly joined, not acknowledged while unk42 is set. */
    FIELD_COMPANION_STATUS_NEW = 1,              /**< Newly joined; the flag is cleared by the query. */
    FIELD_COMPANION_STATUS_GAINS_EXPERIENCE = 2, /**< Status bit 30 is set. */
    FIELD_COMPANION_STATUS_IDLE = 3,
    FIELD_COMPANION_STATUS_EMPTY = 0xFF /**< The record is not in use. */
};

/** @brief Companion template table (resource 7). */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldRegionRecord templates[1];
} FieldCompanionTemplateTable;

extern FieldGameState* g_field_game_state;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern s32 D_801227F0;

extern void* func_800C1E40(s32 resource_id);
extern void* func_800C1EC8(void* source, void* destination, s32 size);

/**
 * @brief Copy a companion template into the first free stored record and give it a unique id.
 * @param template_index Index of the template in resource 7.
 * @return Record index, FIELD_REGION_COUNT when every record is in use, or FIELD_COMPANION_NONE when resource 7 is not loaded.
 */
s32 field_add_stored_companion(s32 template_index)
{
    FieldCompanionTemplateTable* table;
    FieldRegionRecord* companion_template;
    s32 slot;
    s32 scan;
    s32 retry;
    s32 unique_id;
    s32 random_high;

    table = func_800C1E40(FIELD_RESOURCE_COMPANION_TEMPLATES);
    if (table == NULL)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_NO_COMPANION_TEMPLATES, template_index, g_scene_mode);
        return FIELD_COMPANION_NONE;
    }

    companion_template = &table->templates[template_index];
    field_set_text_macro(1, companion_template->name, FIELD_COMPANION_NAME_LENGTH);

    for (slot = 0; slot < FIELD_REGION_COUNT; slot++)
    {
        if (g_field_game_state->regions[slot].name[0] == 0)
        {
            func_800C1EC8(companion_template, &g_field_game_state->regions[slot], sizeof(FieldRegionRecord));
            retry = -1;
            do
            {
                random_high = rand();
                unique_id = (((random_high << 16) + rand()) & FIELD_RANDOM_VALUE_MASK) |
                            ((g_field_game_state->unkD4 + (g_field_game_state->unkD6 << 16)) & FIELD_FIXED_VALUE_MASK);
                if (unique_id != 0)
                {
                    retry = 0;
                }
                for (scan = 0; scan < FIELD_REGION_COUNT; scan++)
                {
                    if (g_field_game_state->regions[scan].name[0] != 0 && g_field_game_state->regions[scan].unique_id == unique_id)
                    {
                        retry = -1;
                    }
                }
            } while (retry != 0);
            g_field_game_state->regions[slot].unique_id = unique_id;
            return slot;
        }
    }
    return slot;
}

/**
 * @brief Release the stored companion named by the first gosub result.
 * @return The released index, FIELD_REGION_COUNT when it is the active companion, or FIELD_COMPANION_NONE.
 */
s32 field_release_stored_companion(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < FIELD_REGION_COUNT)
        {
            field_set_text_macro(0, g_field_game_state->regions[index].name, FIELD_COMPANION_NAME_LENGTH);
            index = g_gosub_result_values[0];
            if (index != g_field_game_state->region_index)
            {
                g_field_game_state->regions[index].name[0] = 0;
                return g_gosub_result_values[0];
            }
            return FIELD_REGION_COUNT;
        }
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMPANION, index, 0);
    }
    return FIELD_COMPANION_NONE;
}

/**
 * @brief Report a stored companion's state, acknowledging a newly joined one.
 * @param index Stored companion index.
 * @return A FIELD_COMPANION_STATUS_ value; unspecified for an out-of-range index.
 */
s32 field_get_stored_companion_status(s32 index)
{
    s32 status;

    if (index >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMPANION_STATUS, index, 0);
    }
    else
    {
        if (g_field_game_state->regions[index].name[0] != 0)
        {
            field_set_text_macro(0, g_field_game_state->regions[index].name, FIELD_COMPANION_NAME_LENGTH);
            status = g_field_game_state->regions[index].status.word;
            if (status < 0)
            {
                if (g_field_game_state->regions[index].unk42 != 0)
                {
                    return FIELD_COMPANION_STATUS_NEW_HELD;
                }
                g_field_game_state->regions[index].status.word = status & ~FIELD_COMPANION_NEW;
                return FIELD_COMPANION_STATUS_NEW;
            }
            if (((u32)status >> FIELD_COMPANION_GAINS_EXPERIENCE_BIT) & 1)
            {
                return FIELD_COMPANION_STATUS_GAINS_EXPERIENCE;
            }
            return FIELD_COMPANION_STATUS_IDLE;
        }
        return FIELD_COMPANION_STATUS_EMPTY;
    }
}

/**
 * @brief Open the name-entry screen for a stored companion.
 * @param index Stored companion index; an out-of-range index records a diagnostic.
 */
void field_rename_stored_companion(s32 index)
{
    FieldRegionRecord* companion;

    if (index >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMPANION_RENAME, index, 0);
        return;
    }
    field_set_text_macro(0, g_field_game_state->regions[index].name, FIELD_COMPANION_NAME_LENGTH);
    companion = &g_field_game_state->regions[index];
    field_run_name_entry(companion->name, companion->name, FIELD_NAME_SOURCE_COMPANION, g_field_game_state->regions[index].unk15, 0);
}
