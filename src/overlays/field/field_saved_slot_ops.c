/**
 * @file field_saved_slot_ops.c
 * @brief Create, release, query and rename the stored companion records.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Resource id of the companion template table. */
#define FIELD_RESOURCE_COMPANION_TEMPLATES 7

/** @brief Unique-id bits taken from the random value. */
#define FIELD_RANDOM_VALUE_MASK 0xFF00FF00

/** @brief Unique-id bits taken from the game-state words at unkD4/unkD6. */
#define FIELD_FIXED_VALUE_MASK 0x00FF00FF

/** @brief Companion status bit 31: the companion has just joined. */
#define FIELD_COMPANION_NEW 0x80000000

/** @brief Companion template table (resource 7). */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldRegionRecord templates[1];
} FieldCompanionTemplateTable;

void func_800B2844(s32 arg0, void* script, s32 arg2);
void field_run_name_entry(FieldRegionRecord* initial_name, FieldRegionRecord* active_name, s32 source_mode, s32 history_index, s32 custom_name);

extern FieldGameState* D_80122B74;
extern u16 g_scene_mode;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern s32 D_801227F0;

extern void* func_800C1E40(s32 resource_id);
extern void* func_800C1EC8(void* source, void* destination, s32 size);
extern s32 rand(void);

/**
 * @brief Copy a companion template into the first free stored record and give it a unique id.
 * @param template_index Index of the template in resource 7.
 * @return Allocated record index, FIELD_REGION_COUNT if all records are in use, or 0xFF if resource 7 is unavailable.
 */
s32 func_800C2264(s32 template_index)
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
        record_game_diagnostic(0x8001, 0x78, template_index, g_scene_mode);
        return 0xFF;
    }

    companion_template = &table->templates[template_index];
    func_800B2844(1, companion_template, 0x15);

    for (slot = 0; slot < FIELD_REGION_COUNT; slot++)
    {
        if (D_80122B74->regions[slot].name[0] == 0)
        {
            func_800C1EC8(companion_template, &D_80122B74->regions[slot], sizeof(FieldRegionRecord));
            retry = -1;
            do
            {
                random_high = rand();
                unique_id =
                    (((random_high << 16) + rand()) & FIELD_RANDOM_VALUE_MASK) | ((D_80122B74->unkD4 + (D_80122B74->unkD6 << 16)) & FIELD_FIXED_VALUE_MASK);
                if (unique_id != 0)
                {
                    retry = 0;
                }
                for (scan = 0; scan < FIELD_REGION_COUNT; scan++)
                {
                    if (D_80122B74->regions[scan].name[0] != 0 && D_80122B74->regions[scan].unique_id == unique_id)
                    {
                        retry = -1;
                    }
                }
            } while (retry != 0);
            D_80122B74->regions[slot].unique_id = unique_id;
            return slot;
        }
    }
    return slot;
}

/**
 * @brief Release the stored companion named by the first gosub result.
 * @return The released index, FIELD_REGION_COUNT when it is the active companion, or 0xFF on failure.
 */
s32 func_800C23F4(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < FIELD_REGION_COUNT)
        {
            func_800B2844(0, &D_80122B74->regions[index], 0x15);
            index = g_gosub_result_values[0];
            if (index != D_80122B74->region_index)
            {
                D_80122B74->regions[index].name[0] = 0;
                return g_gosub_result_values[0];
            }
            return FIELD_REGION_COUNT;
        }
        record_game_diagnostic(0x8001, 0x6E, index, 0);
    }
    return 0xFF;
}

/**
 * @brief Report a stored companion's state, acknowledging a newly joined one.
 * @param index Stored companion index.
 * @return 0 new but unk42 set, 1 newly joined (flag cleared), 2 gaining experience,
 *         3 idle, 0xFF empty; unspecified for an out-of-range index.
 */
s32 func_800C24BC(s32 index)
{
    s32 status;

    if (index >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(0x8001, 0x76, index, 0);
    }
    else
    {
        if (D_80122B74->regions[index].name[0] != 0)
        {
            func_800B2844(0, &D_80122B74->regions[index], 0x15);
            status = D_80122B74->regions[index].status.word;
            if (status < 0)
            {
                if (D_80122B74->regions[index].unk42 != 0)
                {
                    return 0;
                }
                D_80122B74->regions[index].status.word = status & ~FIELD_COMPANION_NEW;
                return 1;
            }
            if (((u32)status >> 30) & 1)
            {
                return 2;
            }
            return 3;
        }
        return 0xFF;
    }
}

/**
 * @brief Open name entry for a stored companion, or report an invalid index.
 * @param index Stored companion index; >= FIELD_REGION_COUNT records a diagnostic.
 */
void func_800C25A0(s32 index)
{
    FieldRegionRecord* companion;

    if (index >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(0x8001, 0x77, index, 0);
        return;
    }
    func_800B2844(0, &D_80122B74->regions[index], 0x15);
    companion = &D_80122B74->regions[index];
    field_run_name_entry(companion, companion, 3, D_80122B74->regions[index].unk15, 0);
}
