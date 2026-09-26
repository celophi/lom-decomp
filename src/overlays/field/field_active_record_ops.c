#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_script.h"
#include "field_actor_runtime.h"
#include "field_records.h"

/** @brief Number of guest characters with a template in resource 3. */
#define FIELD_GUEST_COUNT 12

/** @brief Hero levels covered by one guest template bank. */
#define FIELD_GUEST_BANK_LEVELS 6

/** @brief Number of level banks in one guest template. */
#define FIELD_GUEST_BANK_COUNT 4

/** @brief Resource id of the guest template table. */
#define FIELD_RESOURCE_GUEST_TEMPLATES 3

/** @brief Resource ids of the companion equipment templates. */
#define FIELD_RESOURCE_WEAPON_TEMPLATES 0xD
#define FIELD_RESOURCE_ARMOR_TEMPLATES 0xE

/** @brief First game-state word of the per-guest experience bonuses. */
#define FIELD_GUEST_EXPERIENCE_WORD 0x68

/** @brief Number of entries in g_field_level_experience. */
#define FIELD_LEVEL_EXPERIENCE_COUNT 32

/** @brief Script variable that holds the active companion index. */
#define FIELD_VARIABLE_COMPANION 0x1F10

/** @brief Script variable of guest 0; guest n uses FIELD_VARIABLE_GUEST_BASE + n * 8. */
#define FIELD_VARIABLE_GUEST_BASE 0xF87

/**
 * @brief Offset added to a golem's info byte 1 to form its resource variant.
 * @note Guess: golem variants follow the 65 character variants.
 */
#define FIELD_GOLEM_VARIANT_BASE 65

/** @brief Bytes of a stored companion name copied into the party record. */
#define FIELD_COMPANION_NAME_LENGTH 21

/** @brief Bit position of FIELD_CHARACTER_AI. */
#define FIELD_CHARACTER_AI_SHIFT 7

/** @brief Number of equipment_totals values in a character or stored companion record. */
#define FIELD_EQUIPMENT_TOTAL_COUNT 4

/** @brief Number of entries in FieldCharacterRecord.button_actions. */
#define FIELD_CHARACTER_ORDER_COUNT 8

/** @brief First armor slot; the armor slots follow the weapon slot. */
#define FIELD_ARMOR_SLOT (FIELD_WEAPON_SLOT + 1)

/** @brief Number of armor slots. */
#define FIELD_ARMOR_SLOT_COUNT (FIELD_EQUIPMENT_SLOT_COUNT - 1)

/** @brief Stat bits 0-8: the stat times four. */
#define FIELD_STAT_SCALED_MASK 0x1FF

/** @brief Largest experience value a character can hold. */
#define FIELD_EXPERIENCE_MAX 9999999

/** @brief One guest template: an id and a character record per hero-level band. */
typedef struct
{
    s32 id;
    FieldCharacterRecord banks[FIELD_GUEST_BANK_COUNT];
} FieldGuestTemplate;

/** @brief Guest template table (resource 3). */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldGuestTemplate guests[1];
} FieldGuestTemplateTable;

/** @brief Item template table (resources 0xD and 0xE). */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldItemRecord templates[1];
} FieldItemTemplateTable;

extern FieldGameState* g_field_game_state;
/** @brief Experience needed to reach each level; entry n is for level n + 1. */
extern s32 g_field_level_experience[FIELD_LEVEL_EXPERIENCE_COUNT];
extern void* func_800C1E40(s32 resource_id);
extern void func_800C1EC8(void* source, void* destination, s32 size);
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern s32 D_801227F0;

static void field_load_companion(s32 companion_index);
static void field_store_companion(void);

/**
 * @brief Put a guest into party slot 1, using the template bank for the hero's level.
 * @param guest_id Guest index, below FIELD_GUEST_COUNT.
 * @return -1 when the guest joined, 0 when it has no template.
 */
s32 field_join_guest(s32 guest_id)
{
    FieldGuestTemplateTable* table;
    FieldCharacterRecord* bank;
    u32 progress;
    u32 ai_flag;
    s32 level_index;
    s32 i;
    u8 hero_level;

    if (guest_id < FIELD_GUEST_COUNT)
    {
        table = func_800C1E40(FIELD_RESOURCE_GUEST_TEMPLATES);
        for (i = 0; i < table->count; i++)
        {
            if (table->guests[i].id == guest_id)
            {
                hero_level = g_field_game_state->control.fields.hero_level;
                ai_flag = g_field_game_state->characters[FIELD_PARTY_GUEST].info.bytes[0] >> FIELD_CHARACTER_AI_SHIFT;
                if (hero_level < FIELD_GUEST_BANK_LEVELS)
                {
                    bank = &table->guests[i].banks[0];
                }
                else if (hero_level < 2 * FIELD_GUEST_BANK_LEVELS)
                {
                    bank = &table->guests[i].banks[1];
                }
                else if (hero_level < 3 * FIELD_GUEST_BANK_LEVELS)
                {
                    bank = &table->guests[i].banks[2];
                }
                else
                {
                    bank = &table->guests[i].banks[3];
                }
                func_800C1EC8(bank, &g_field_game_state->characters[FIELD_PARTY_GUEST], sizeof(FieldCharacterRecord));
                g_field_game_state->characters[FIELD_PARTY_GUEST].info.word =
                    (g_field_game_state->characters[FIELD_PARTY_GUEST].info.word & ~FIELD_CHARACTER_AI) | (ai_flag << FIELD_CHARACTER_AI_SHIFT);
                if (g_field_game_state->control.fields.hero_level < FIELD_LEVEL_EXPERIENCE_COUNT)
                {
                    level_index = g_field_game_state->control.fields.hero_level - 1;
                }
                else
                {
                    level_index = FIELD_LEVEL_EXPERIENCE_COUNT - 1;
                }
                progress = g_field_game_state->characters[FIELD_PARTY_GUEST].progress.level;
                /* Without the net-zero level_index terms, loop.c hoists the word index out of the search loop. */
                progress |= (g_field_game_state->words[FIELD_GUEST_EXPERIENCE_WORD + guest_id + level_index - level_index] + g_field_level_experience[level_index])
                            << 8;
                g_field_game_state->characters[FIELD_PARTY_GUEST].progress.word = progress;
                if ((s32)(progress >> 8) > FIELD_EXPERIENCE_MAX)
                {
                    g_field_game_state->characters[FIELD_PARTY_GUEST].progress.bits.experience = FIELD_EXPERIENCE_MAX;
                }
                field_apply_character_level_ups(FIELD_PARTY_GUEST, 0);
                field_refresh_party_member(FIELD_PARTY_GUEST);
                field_set_script_var(0, guest_id * 8 + FIELD_VARIABLE_GUEST_BASE, 1);
                return -1;
            }
        }
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_GUEST, guest_id, 0);
    }
    else
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_GUEST, guest_id, 1);
    }
    return 0;
}

/**
 * @brief Make the stored companion named by the first gosub result the active companion.
 * @return The companion's resource variant (info byte 1), or FIELD_NO_VARIANT on failure.
 */
s32 field_join_companion(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < FIELD_REGION_COUNT)
        {
            if (g_field_game_state->regions[index].name[0] != 0)
            {
                g_field_game_state->region_index = index;
                field_set_script_var(0, FIELD_VARIABLE_COMPANION, g_gosub_result_values[0]);
                field_load_companion(g_gosub_result_values[0]);
                return g_field_game_state->characters[FIELD_PARTY_COMPANION].info.bytes[1];
            }
        }
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMPANION, index, 0);
    }
    return FIELD_NO_VARIANT;
}

/**
 * @brief Make the stored companion named by the companion variable the active companion.
 * @return The companion's resource variant (info byte 1), or FIELD_NO_VARIANT on failure.
 */
s32 field_rejoin_companion(void)
{
    s32 index = field_get_script_var(0, FIELD_VARIABLE_COMPANION);
    s32 result;

    if ((u32)index < FIELD_REGION_COUNT)
    {
        g_field_game_state->region_index = index;
        field_load_companion(index);
        result = g_field_game_state->characters[FIELD_PARTY_COMPANION].info.bytes[1];
    }
    else
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMPANION, index, 1);
        result = FIELD_NO_VARIANT;
    }
    return result;
}

/**
 * @brief Copy a stored companion into party slot 2 and rebuild its equipment records.
 * @param companion_index Stored companion index.
 */
static void field_load_companion(s32 companion_index)
{
    FieldItemTemplateTable* table;
    FieldGameState* state;
    FieldItemRecord* item;
    u32 scaled;
    s32 i;

    for (i = 0; i < FIELD_COMPANION_NAME_LENGTH; i++)
    {
        g_field_game_state->characters[FIELD_PARTY_COMPANION].name[i] = g_field_game_state->regions[companion_index].name[i];
    }
    g_field_game_state->characters[FIELD_PARTY_COMPANION].info.word =
        ((g_field_game_state->characters[FIELD_PARTY_COMPANION].info.word & ~FIELD_CHARACTER_TYPE_MASK) | FIELD_CHARACTER_COMPANION) & ~FIELD_CHARACTER_AI;
    g_field_game_state->characters[FIELD_PARTY_COMPANION].info.bytes[1] = g_field_game_state->regions[companion_index].unk15;
    g_field_game_state->characters[FIELD_PARTY_COMPANION].progress.bits.level = g_field_game_state->regions[companion_index].progress.bits.level;
    g_field_game_state->characters[FIELD_PARTY_COMPANION].progress.bits.experience = g_field_game_state->regions[companion_index].progress.bits.experience;
    g_field_game_state->characters[FIELD_PARTY_COMPANION].hp = g_field_game_state->regions[companion_index].hp;
    g_field_game_state->characters[FIELD_PARTY_COMPANION].unk26 = g_field_game_state->regions[companion_index].unk1E;
    for (i = 0; i < FIELD_EQUIPMENT_TOTAL_COUNT; i++)
    {
        g_field_game_state->characters[FIELD_PARTY_COMPANION].equipment_totals[i] = g_field_game_state->regions[companion_index].equipment_totals[i];
    }
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        scaled = g_field_game_state->regions[companion_index].stats[i] & FIELD_STAT_SCALED_MASK;
        g_field_game_state->characters[FIELD_PARTY_COMPANION].stats[i] = (g_field_game_state->characters[FIELD_PARTY_COMPANION].stats[i] & ~FIELD_STAT_SCALED_MASK) | scaled;
        g_field_game_state->characters[FIELD_PARTY_COMPANION].stats[i] = (g_field_game_state->characters[FIELD_PARTY_COMPANION].stats[i] & FIELD_STAT_SCALED_MASK) |
                                                                 (g_field_game_state->regions[companion_index].stats[i] & ~FIELD_STAT_SCALED_MASK);
    }
    g_field_game_state->characters[FIELD_PARTY_COMPANION].unk40 = g_field_game_state->regions[companion_index].unk38[0];
    g_field_game_state->characters[FIELD_PARTY_COMPANION].unk41 = g_field_game_state->regions[companion_index].unk38[1];
    g_field_game_state->characters[FIELD_PARTY_COMPANION].unk42 = g_field_game_state->regions[companion_index].unk38[2];
    g_field_game_state->characters[FIELD_PARTY_COMPANION].unk43 = g_field_game_state->regions[companion_index].unk38[3];
    for (i = 0; i < FIELD_CHARACTER_ORDER_COUNT; i++)
    {
        g_field_game_state->characters[FIELD_PARTY_COMPANION].button_actions[i] = i;
    }
    table = func_800C1E40(FIELD_RESOURCE_WEAPON_TEMPLATES);
    if (table != NULL)
    {
        func_800C1EC8(&table->templates[g_field_game_state->regions[companion_index].weapon_id],
                      &g_field_game_state->characters[FIELD_PARTY_COMPANION].equipment[FIELD_WEAPON_SLOT], sizeof(FieldItemRecord));
    }
    g_field_game_state->characters[FIELD_PARTY_COMPANION].equipment[FIELD_WEAPON_SLOT].derived.values[0] = g_field_game_state->regions[companion_index].unk1E;
    table = func_800C1E40(FIELD_RESOURCE_ARMOR_TEMPLATES);
    if (table != NULL)
    {
        for (i = 0; i < FIELD_ARMOR_SLOT_COUNT; i++)
        {
            func_800C1EC8(&table->templates[g_field_game_state->regions[companion_index].armor_ids[i]],
                          &g_field_game_state->characters[FIELD_PARTY_COMPANION].equipment[FIELD_ARMOR_SLOT + i], sizeof(FieldItemRecord));
        }
    }
    state = g_field_game_state;
    item = &state->characters[FIELD_PARTY_COMPANION].equipment[FIELD_ARMOR_SLOT];
    for (i = 0; i < FIELD_EQUIPMENT_TOTAL_COUNT; i++)
    {
        item->derived.values[i] = state->regions[companion_index].equipment_totals[i];
    }
    g_field_game_state->characters[FIELD_PARTY_COMPANION].equipment[FIELD_ARMOR_SLOT].flags2C = g_field_game_state->regions[companion_index].unk38[0];
    g_field_game_state->characters[FIELD_PARTY_COMPANION].equipment[FIELD_ARMOR_SLOT].flags2D = g_field_game_state->regions[companion_index].unk38[2];
    field_refresh_party_member(FIELD_PARTY_COMPANION);
}

/**
 * @brief Rebuild the golem in party slot 2 and return its resource variant.
 * @return The golem's info byte 1 plus FIELD_GOLEM_VARIANT_BASE.
 * @note field_golem_select_logic_type rebuilds party slot 2 from the joined golem group.
 */
s32 field_join_golem(void)
{
    /* field_golem_select_logic_type takes a type; the original call leaves $a0 as it is. */
    ((void (*)(void))field_golem_select_logic_type)();
    return g_field_game_state->characters[FIELD_PARTY_COMPANION].info.bytes[1] + FIELD_GOLEM_VARIANT_BASE;
}

/**
 * @brief Remove the guest (slot 1) or the companion (slot 2) from the party.
 * @param companion Zero removes the guest in slot 1, nonzero the companion or golem in slot 2.
 */
void field_leave_party(s32 companion)
{
    if (companion == 0)
    {
        if ((g_field_game_state->characters[FIELD_PARTY_GUEST].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GUEST)
        {
            field_set_script_var(0, (g_field_game_state->characters[FIELD_PARTY_GUEST].info.bytes[1] << 3) + FIELD_VARIABLE_GUEST_BASE, 0);
        }
        field_set_script_var(0, FIELD_VARIABLE_GUEST_VARIANT, FIELD_NO_VARIANT);
        g_field_game_state->characters[FIELD_PARTY_GUEST].name[0] = 0;
        g_field_game_state->characters[FIELD_PARTY_GUEST].info.word |= FIELD_CHARACTER_TYPE_MASK;
    }
    else
    {
        if ((g_field_game_state->characters[FIELD_PARTY_COMPANION].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_COMPANION)
        {
            field_store_companion();
            g_field_game_state->region_index = FIELD_REGION_COUNT;
        }
        else
        {
            field_golem_commit_group_edit(0);
        }
        g_field_game_state->characters[FIELD_PARTY_COMPANION].name[0] = 0;
        g_field_game_state->characters[FIELD_PARTY_COMPANION].info.word |= FIELD_CHARACTER_TYPE_MASK;
        field_set_script_var(0, FIELD_VARIABLE_COMPANION_VARIANT, FIELD_NO_VARIANT);
    }
    field_release_actor_resource_slot(companion);
}

/**
 * @brief Write the companion in party slot 2 back to its stored record.
 */
static void field_store_companion(void)
{
    s32 i;

    if ((u32)g_field_game_state->region_index < FIELD_REGION_COUNT)
    {
        for (i = 0; i < FIELD_COMPANION_NAME_LENGTH; i++)
        {
            g_field_game_state->regions[g_field_game_state->region_index].name[i] = g_field_game_state->characters[FIELD_PARTY_COMPANION].name[i];
        }
        g_field_game_state->regions[g_field_game_state->region_index].progress.bits.level = g_field_game_state->characters[FIELD_PARTY_COMPANION].progress.bits.level;
        g_field_game_state->regions[g_field_game_state->region_index].progress.bits.experience = g_field_game_state->characters[FIELD_PARTY_COMPANION].progress.bits.experience;
        g_field_game_state->regions[g_field_game_state->region_index].hp = g_field_game_state->characters[FIELD_PARTY_COMPANION].hp;
        func_800C1EC8(g_field_game_state->characters[FIELD_PARTY_COMPANION].stats, g_field_game_state->regions[g_field_game_state->region_index].stats,
                      sizeof(g_field_game_state->characters[FIELD_PARTY_COMPANION].stats));
    }
}
