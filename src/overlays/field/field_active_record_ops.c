#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_actor_runtime.h"
#include "field_records.h"

/** @brief Number of guest characters with a template in resource 3. */
#define FIELD_GUEST_COUNT 12

/** @brief Resource id of the guest template table. */
#define FIELD_RESOURCE_GUEST_TEMPLATES 3

/** @brief First game-state word of the per-guest experience bonuses. */
#define FIELD_GUEST_EXPERIENCE_WORD 0x68

/** @brief Script variable that holds the active companion index. */
#define FIELD_VARIABLE_COMPANION 0x1F10

/** @brief Script variable of guest 0; guest n uses FIELD_VARIABLE_GUEST_BASE + n * 8. */
#define FIELD_VARIABLE_GUEST_BASE 0xF87

/** @brief Resource ids of the companion equipment templates. */
#define FIELD_RESOURCE_WEAPON_TEMPLATES 0xD
#define FIELD_RESOURCE_ARMOR_TEMPLATES 0xE

/** @brief Bytes of a stored companion name copied into the party record. */
#define FIELD_COMPANION_NAME_LENGTH 0x15

/** @brief Character info bits 0-6: character type. */
#define FIELD_CHARACTER_TYPE_MASK 0x7F

/** @brief Character type of a guest in party slot 1. */
#define FIELD_CHARACTER_GUEST 2

/** @brief Character type of a stored companion in party slot 2. */
#define FIELD_CHARACTER_COMPANION 3

/** @brief Character info bit 7: the character is AI-controlled. */
#define FIELD_CHARACTER_AI 0x80

/** @brief Stat bits 0-8: the stat times four. */
#define FIELD_STAT_SCALED_MASK 0x1FF

/** @brief Largest experience value a character can hold. */
#define FIELD_EXPERIENCE_MAX 9999999

/** @brief One guest template: an id and a character record per hero-level band. */
typedef struct
{
    s32 id;
    FieldCharacterRecord banks[4];
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

/** @brief Context level, preserved flag byte, and packed value updated by the record load. */
typedef struct
{
    u8 pad0[0x2E5];
    u8 level;
    u8 pad2E6[0x858 - 0x2E6];
    union
    {
        u32 word;
        u8 byte[4];
    } flags;
    u8 pad85C[4];
    union
    {
        u32 word;
        u8 byte[4];
    } packed;
} Context;
extern FieldGameState* D_80122B74;
extern s32 D_800F190C[];
extern void* func_800C1E40(s32 resource_id);
extern void func_800C1EC8(void* source, void* destination, s32 size);
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern s32 D_801227F0;
extern void func_800BD520(s32, s32, s32);

/**
 * @brief Load a matching level-dependent record or report a lookup failure.
 * @param record_id Record identifier to find.
 * @return -1 after applying a matching record, or zero after the fallback.
 */
s32 func_800C2B14(s32 record_id)
{
    s32 level_index;
    s32 record_index;
    s32 record_offset;
    u32 packed_value;
    u32 saved_flag;
    u8 level;
    u8* table;
    u8* bank;

    if (record_id < 0xC)
    {
        table = func_800C1E40(3);
        do
        {
            do
            {
                record_index = 0;
            } while (0);
        } while (0);
        if (*(u16*)(table + 2) != 0)
        {
            do
            {
                if ((*(s32*)(table + record_index * 0x944 + 4)) == record_id)
                {
                    record_offset = record_index * 0x944 + 4;
                    level = ((Context*)D_80122B74)->level;
                    saved_flag = ((Context*)D_80122B74)->flags.byte[0] >> 7;
                    if (level < 6U)
                    {
                        bank = table + record_offset + 4;
                    }
                    else if (level < 0xCU)
                    {
                        bank = table + record_offset + 0x254;
                    }
                    else if (level < 0x12U)
                    {
                        bank = table + record_offset + 0x4A4;
                    }
                    else
                    {
                        bank = table + record_offset + 0x6F4;
                    }
                    func_800C1EC8(bank, (u8*)D_80122B74 + 0x840, 0x250);
                    ((Context*)D_80122B74)->flags.word = (((Context*)D_80122B74)->flags.word & ~0x80) | (saved_flag << 7);
                    if (((Context*)D_80122B74)->level < 0x20U)
                    {
                        level_index = ((Context*)D_80122B74)->level - 1;
                    }
                    else
                    {
                        level_index = 0x1F;
                    }
                    packed_value = ((Context*)D_80122B74)->packed.byte[0];
                    packed_value =
                        packed_value |
                        ((*(s32*)((u8*)((s32)D_80122B74 - -((record_id + 0x68 + level_index - level_index) * 4)) + 0xE4) + D_800F190C[level_index]) << 8);
                    ((Context*)D_80122B74)->packed.word = packed_value;
                    if ((s32)(packed_value >> 8) > 0x98967F)
                    {
                        u32 clamped_value;

                        clamped_value = packed_value & 0xFF;
                        clamped_value |= 0x98967F00;
                        ((Context*)D_80122B74)->packed.word = clamped_value;
                    }
                    func_800C11F0(1, 0);
                    func_800B7C58(1);
                    func_800BD520(0, record_id * 8 + 0xF87, 1);
                    return -1;
                }
                record_index++;
            } while (record_index < (s32) * (u16*)(table + 2));
        }
        record_game_diagnostic(0x8001, 0x6D, record_id, 0);
    }
    else
    {
        record_game_diagnostic(0x8001, 0x6D, record_id, 1);
    }
    return 0;
}
void func_800C32C8(void);
void func_800BD520(s32 arg0, s32 variable, s32 value);
s32 func_800BD414(s32 arg0, s32 variable);
void func_800C2E30(s32 companion_index);

/**
 * @brief Make the stored companion named by the first gosub result the active companion.
 * @return The companion's info byte 1 on success, else 0xFF.
 */
s32 func_800C2D08(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < FIELD_REGION_COUNT)
        {
            if (D_80122B74->regions[index].name[0] != 0)
            {
                D_80122B74->region_index = index;
                func_800BD520(0, FIELD_VARIABLE_COMPANION, g_gosub_result_values[0]);
                func_800C2E30(g_gosub_result_values[0]);
                return D_80122B74->characters[2].info.bytes[1];
            }
        }
        record_game_diagnostic(0x8001, 0x6E, index, 0);
    }
    return 0xFF;
}

/**
 * @brief Make the stored companion named by the companion variable the active companion.
 * @return The companion's info byte 1 on success, else 0xFF.
 */
s32 func_800C2DC0(void)
{
    s32 index = func_800BD414(0, FIELD_VARIABLE_COMPANION);
    s32 result;

    if ((u32)index < FIELD_REGION_COUNT)
    {
        D_80122B74->region_index = index;
        func_800C2E30(index);
        result = D_80122B74->characters[2].info.bytes[1];
    }
    else
    {
        record_game_diagnostic(0x8001, 0x6E, index, 1);
        result = 0xFF;
    }
    return result;
}

/**
 * @brief Copy a stored companion into party slot 2 and rebuild its equipment records.
 * @param companion_index Stored companion index.
 */
void func_800C2E30(s32 companion_index)
{
    FieldItemTemplateTable* table;
    FieldGameState* state;
    FieldItemRecord* item;
    u32 scaled;
    s32 i;

    for (i = 0; i < FIELD_COMPANION_NAME_LENGTH; i++)
    {
        D_80122B74->characters[2].name[i] = D_80122B74->regions[companion_index].name[i];
    }
    D_80122B74->characters[2].info.word =
        ((D_80122B74->characters[2].info.word & ~FIELD_CHARACTER_TYPE_MASK) | FIELD_CHARACTER_COMPANION) & ~FIELD_CHARACTER_AI;
    D_80122B74->characters[2].info.bytes[1] = D_80122B74->regions[companion_index].unk15;
    D_80122B74->characters[2].progress.bits.level = D_80122B74->regions[companion_index].progress.bits.level;
    D_80122B74->characters[2].progress.bits.experience = D_80122B74->regions[companion_index].progress.bits.experience;
    D_80122B74->characters[2].hp = D_80122B74->regions[companion_index].hp;
    D_80122B74->characters[2].unk26 = D_80122B74->regions[companion_index].unk1E;
    for (i = 0; i < 4; i++)
    {
        D_80122B74->characters[2].equipment_totals[i] = D_80122B74->regions[companion_index].equipment_totals[i];
    }
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        scaled = D_80122B74->regions[companion_index].stats[i] & FIELD_STAT_SCALED_MASK;
        D_80122B74->characters[2].stats[i] = (D_80122B74->characters[2].stats[i] & ~FIELD_STAT_SCALED_MASK) | scaled;
        D_80122B74->characters[2].stats[i] =
            (D_80122B74->characters[2].stats[i] & FIELD_STAT_SCALED_MASK) | (D_80122B74->regions[companion_index].stats[i] & ~FIELD_STAT_SCALED_MASK);
    }
    D_80122B74->characters[2].unk40 = D_80122B74->regions[companion_index].unk38[0];
    D_80122B74->characters[2].unk41 = D_80122B74->regions[companion_index].unk38[1];
    D_80122B74->characters[2].unk42 = D_80122B74->regions[companion_index].unk38[2];
    D_80122B74->characters[2].unk43 = D_80122B74->regions[companion_index].unk38[3];
    for (i = 0; i < 8; i++)
    {
        D_80122B74->characters[2].unk48[i] = i;
    }
    table = func_800C1E40(FIELD_RESOURCE_WEAPON_TEMPLATES);
    if (table != NULL)
    {
        func_800C1EC8(&table->templates[D_80122B74->regions[companion_index].weapon_id], &D_80122B74->characters[2].equipment[0], sizeof(FieldItemRecord));
    }
    D_80122B74->characters[2].equipment[0].derived.values[0] = D_80122B74->regions[companion_index].unk1E;
    table = func_800C1E40(FIELD_RESOURCE_ARMOR_TEMPLATES);
    if (table != NULL)
    {
        for (i = 0; i < 3; i++)
        {
            func_800C1EC8(&table->templates[D_80122B74->regions[companion_index].armor_ids[i]], &D_80122B74->characters[2].equipment[1 + i],
                          sizeof(FieldItemRecord));
        }
    }
    state = D_80122B74;
    item = &state->characters[2].equipment[1];
    for (i = 0; i < 4; i++)
    {
        item->derived.values[i] = state->regions[companion_index].equipment_totals[i];
    }
    D_80122B74->characters[2].equipment[1].flags2C = D_80122B74->regions[companion_index].unk38[0];
    D_80122B74->characters[2].equipment[1].flags2D = D_80122B74->regions[companion_index].unk38[2];
    func_800B7C58(2);
}

/**
 * @brief Refresh the active companion and return its letter code.
 * @return The companion's info byte 1 plus 'A'.
 */
s32 func_800C318C(void)
{
    /* func_800C3B50 takes a type; the original call leaves $a0 as it is. */
    ((void (*)(void))func_800C3B50)();
    return D_80122B74->characters[2].info.bytes[1] + 'A';
}

/**
 * @brief Remove the guest (slot 1) or the companion (slot 2) from the party.
 * @param companion Zero removes the guest in slot 1, nonzero the companion in slot 2.
 */
void func_800C31BC(s32 companion)
{
    if (companion == 0)
    {
        if ((D_80122B74->characters[1].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GUEST)
        {
            func_800BD520(0, (D_80122B74->characters[1].info.bytes[1] << 3) + FIELD_VARIABLE_GUEST_BASE, 0);
        }
        func_800BD520(0, 0x2F08, 0xFF);
        D_80122B74->characters[1].name[0] = 0;
        D_80122B74->characters[1].info.word |= FIELD_CHARACTER_TYPE_MASK;
    }
    else
    {
        if ((D_80122B74->characters[2].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_COMPANION)
        {
            func_800C32C8();
            D_80122B74->region_index = FIELD_REGION_COUNT;
        }
        else
        {
            func_800C3A00(0);
        }
        D_80122B74->characters[2].name[0] = 0;
        D_80122B74->characters[2].info.word |= FIELD_CHARACTER_TYPE_MASK;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    field_release_actor_resource_slot(companion);
}

/**
 * @brief Write the companion in party slot 2 back to its stored record.
 */
void func_800C32C8(void)
{
    s32 i;

    if ((u32)D_80122B74->region_index < FIELD_REGION_COUNT)
    {
        for (i = 0; i < FIELD_COMPANION_NAME_LENGTH; i++)
        {
            D_80122B74->regions[D_80122B74->region_index].name[i] = D_80122B74->characters[2].name[i];
        }
        D_80122B74->regions[D_80122B74->region_index].progress.bits.level = D_80122B74->characters[2].progress.bits.level;
        D_80122B74->regions[D_80122B74->region_index].progress.bits.experience = D_80122B74->characters[2].progress.bits.experience;
        D_80122B74->regions[D_80122B74->region_index].hp = D_80122B74->characters[2].hp;
        func_800C1EC8(D_80122B74->characters[2].stats, D_80122B74->regions[D_80122B74->region_index].stats, sizeof(D_80122B74->characters[2].stats));
    }
}
