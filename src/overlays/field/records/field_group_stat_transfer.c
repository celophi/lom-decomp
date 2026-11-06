/**
 * @file field_group_stat_transfer.c
 * @brief Build the golem party companion from its saved group record.
 */

#include "common.h"

#include "../internal/field_calls.h"
#include "../internal/field_records.h"
#include "common/golem_shape.h"
#include "common/saved_game.h"
#include "main/main.h"

/** @brief Level of every golem companion. */
#define GOLEM_COMPANION_LEVEL 99

/**
 * @brief Fill a party character record from one saved golem group.
 * @param group Golem group whose record is copied.
 * @param record Party record to fill (party slot 2).
 */
void field_golem_build_companion(s32 group, FieldCharacterRecord* record)
{
    s32 i;

    record->equipment[FIELD_WEAPON_SLOT].name[0] = 1;
    record->equipment[FIELD_ARMOR_SLOT].name[0] = 1;
    record->equipment[2].name[0] = 0;
    record->equipment[3].name[0] = 0;

    for (i = 0; i < GOLEM_NAME_LENGTH; i++)
    {
        record->name[i] = g_saved_game.layout.golem_records[group].name[i];
    }

    record->info.word = ((record->info.word & ~FIELD_CHARACTER_TYPE_MASK) | FIELD_CHARACTER_GOLEM) & ~FIELD_CHARACTER_PAD_CONTROLLED;
    record->info.actions.variant = g_saved_game.layout.golem_records[group].logic_layout.bits.logic_class;

    for (i = 0; i < FIELD_COMMAND_SLOT_COUNT; i++)
    {
        record->info.actions.commands[i] = 0;
    }

    for (i = 0; i < FIELD_SKILL_SLOT_COUNT; i++)
    {
        record->info.actions.skills[i] = 0;
    }

    record->progress.bits.level = GOLEM_COMPANION_LEVEL;
    record->progress.bits.experience = 0;
    record->hp = g_saved_game.layout.golem_records[group].hp;
    record->equipment[FIELD_WEAPON_SLOT].derived.weapon.power = record->power = g_saved_game.layout.golem_records[group].power;

    for (i = 0; i < COMPANION_STAT_COUNT; i++)
    {
        record->equipment_totals[i] = g_saved_game.layout.golem_records[group].equipment_totals[i];
        (&record->equipment[FIELD_ARMOR_SLOT])->derived.values[i] = record->equipment_totals[i];
    }

    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        /* FieldCharacterRecord.stats is a plain u16 array; the copy stores the two FieldStat bit-fields. */
        ((FieldStat*)record->stats)[i].bits.base = g_saved_game.layout.golem_records[group].stats[i].bits.base;
        ((FieldStat*)record->stats)[i].bits.effective = g_saved_game.layout.golem_records[group].stats[i].bits.effective;
    }

    record->equipment[FIELD_ARMOR_SLOT].status_flags = record->status_immunities = g_saved_game.layout.golem_records[group].armor_flags;
    record->equipment[FIELD_WEAPON_SLOT].status_flags = record->weapon_flags = g_saved_game.layout.golem_records[group].weapon_flags;
    record->equipment[FIELD_ARMOR_SLOT].element_flags = record->element_resistances = g_saved_game.layout.golem_records[group].armor_flags2;
    record->companion_ability = g_saved_game.layout.golem_records[group].companion_ability;
    record->button_actions[0] = 0;
    record->button_actions[1] = 0;
    record->button_actions[2] = 0;
    record->button_actions[3] = 0;
    record->button_actions[4] = 0;
    record->button_actions[5] = 0;
    record->button_actions[6] = 0;
    record->button_actions[7] = 0;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n0 = g_saved_game.layout.golem_records[group].weapon_bonus.n0;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n1 = g_saved_game.layout.golem_records[group].weapon_bonus.n1;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n2 = g_saved_game.layout.golem_records[group].weapon_bonus.n2;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n3 = g_saved_game.layout.golem_records[group].weapon_bonus.n3;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n4 = g_saved_game.layout.golem_records[group].weapon_bonus.n4;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n5 = g_saved_game.layout.golem_records[group].weapon_bonus.n5;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n6 = g_saved_game.layout.golem_records[group].weapon_bonus.n6;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n7 = g_saved_game.layout.golem_records[group].weapon_bonus.n7;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n0 = g_saved_game.layout.golem_records[group].armor_bonus.n0;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n1 = g_saved_game.layout.golem_records[group].armor_bonus.n1;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n2 = g_saved_game.layout.golem_records[group].armor_bonus.n2;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n3 = g_saved_game.layout.golem_records[group].armor_bonus.n3;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n4 = g_saved_game.layout.golem_records[group].armor_bonus.n4;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n5 = g_saved_game.layout.golem_records[group].armor_bonus.n5;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n6 = g_saved_game.layout.golem_records[group].armor_bonus.n6;
    record->equipment[FIELD_ARMOR_SLOT].bonus_nibbles.bits.n7 = g_saved_game.layout.golem_records[group].armor_bonus.n7;
    record->instruments[0].derived.golem.logic_class = g_saved_game.layout.golem_records[group].logic_layout.bits.logic_class;
    record->instruments[0].derived.golem.grid_bound = g_saved_game.layout.golem_records[group].logic_layout.bits.grid_bound;
    record->instruments[0].derived.bytes[1] = g_saved_game.layout.golem_records[group].unknown_0x45;
    record->instruments[0].derived.bytes[2] = g_saved_game.layout.golem_records[group].random_chance;
    record->instruments[0].derived.bytes[3] = g_saved_game.layout.golem_records[group].unknown_0x47;
    record->instruments[0].derived.golem.palette = g_saved_game.layout.golem_records[group].palette;
}
