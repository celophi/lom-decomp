/**
 * @file field_group_stat_transfer.c
 * @brief Build the golem party companion from its saved group record.
 */

#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "field_golem_layout.h"
#include "field_records.h"

/** @brief Level of every golem companion. */
#define GOLEM_COMPANION_LEVEL 99
/** @brief Equipment slot that receives the golem's armor values and flags. */
#define GOLEM_ARMOR_SLOT 1
/** @brief Number of battle command slots in FieldCharacterRecord.info. */
#define FIELD_COMMAND_SLOT_COUNT 2
/** @brief Number of battle skill slots in FieldCharacterRecord.info. */
#define FIELD_SKILL_SLOT_COUNT 4

/**
 * @brief Fill a party character record from one saved golem group.
 * @param group Golem group whose record is copied.
 * @param record Party record to fill (party slot 2).
 */
void field_golem_build_companion(s32 group, FieldCharacterRecord* record)
{
    s32 i;

    record->equipment[FIELD_WEAPON_SLOT].kind = 1;
    record->equipment[GOLEM_ARMOR_SLOT].kind = 1;
    record->equipment[2].kind = 0;
    record->equipment[3].kind = 0;
    for (i = 0; i < GOLEM_NAME_LENGTH; i++)
    {
        record->name[i] = GOLEM.group_records[group].name[i];
    }
    record->info.word = ((record->info.word & ~FIELD_CHARACTER_TYPE_MASK) | FIELD_CHARACTER_GOLEM) & ~FIELD_CHARACTER_AI;
    record->info.actions.unk19 = GOLEM.group_records[group].logic_class;
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
    record->hp = GOLEM.group_records[group].hp;
    record->equipment[FIELD_WEAPON_SLOT].derived.weapon.power = record->unk26 = GOLEM.group_records[group].power;
    for (i = 0; i < HISTORY_RECORD_STAT_COUNT; i++)
    {
        record->equipment_totals[i] = GOLEM.group_records[group].equipment_totals[i];
        /* Through the slot address: an indexed equipment[] store shares the totals address and changes the loop. */
        (&record->equipment[GOLEM_ARMOR_SLOT])->derived.values[i] = record->equipment_totals[i];
    }
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        /* FieldCharacterRecord.stats is a plain u16 array; the copy stores the two FieldStat bit-fields. */
        ((FieldStat*)record->stats)[i].bits.base = GOLEM.group_records[group].stats[i].bits.base;
        ((FieldStat*)record->stats)[i].bits.effective = GOLEM.group_records[group].stats[i].bits.effective;
    }
    record->equipment[GOLEM_ARMOR_SLOT].flags2C = record->unk40 = GOLEM.group_records[group].armor_flags;
    record->equipment[FIELD_WEAPON_SLOT].flags2C = record->unk41 = GOLEM.group_records[group].weapon_flags;
    record->equipment[GOLEM_ARMOR_SLOT].flags2D = record->unk42 = GOLEM.group_records[group].armor_flags2;
    record->unk43 = GOLEM.group_records[group].unknown_0x3F;
    record->button_actions[0] = 0;
    record->button_actions[1] = 0;
    record->button_actions[2] = 0;
    record->button_actions[3] = 0;
    record->button_actions[4] = 0;
    record->button_actions[5] = 0;
    record->button_actions[6] = 0;
    record->button_actions[7] = 0;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n0 = GOLEM.group_records[group].weapon_bonus.n0;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n1 = GOLEM.group_records[group].weapon_bonus.n1;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n2 = GOLEM.group_records[group].weapon_bonus.n2;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n3 = GOLEM.group_records[group].weapon_bonus.n3;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n4 = GOLEM.group_records[group].weapon_bonus.n4;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n5 = GOLEM.group_records[group].weapon_bonus.n5;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n6 = GOLEM.group_records[group].weapon_bonus.n6;
    record->equipment[FIELD_WEAPON_SLOT].bonus_nibbles.bits.n7 = GOLEM.group_records[group].weapon_bonus.n7;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n0 = GOLEM.group_records[group].armor_bonus.n0;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n1 = GOLEM.group_records[group].armor_bonus.n1;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n2 = GOLEM.group_records[group].armor_bonus.n2;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n3 = GOLEM.group_records[group].armor_bonus.n3;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n4 = GOLEM.group_records[group].armor_bonus.n4;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n5 = GOLEM.group_records[group].armor_bonus.n5;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n6 = GOLEM.group_records[group].armor_bonus.n6;
    record->equipment[GOLEM_ARMOR_SLOT].bonus_nibbles.bits.n7 = GOLEM.group_records[group].armor_bonus.n7;
    record->unk150[0].derived.golem.logic_class = GOLEM.group_records[group].logic_class;
    record->unk150[0].derived.golem.grid_bound = GOLEM.group_records[group].grid_bound;
    record->unk150[0].derived.bytes[1] = GOLEM.group_records[group].unknown_0x45;
    record->unk150[0].derived.bytes[2] = GOLEM.group_records[group].unknown_0x46;
    record->unk150[0].derived.bytes[3] = GOLEM.group_records[group].unknown_0x47;
    record->unk150[0].derived.golem.unknown_0x48 = GOLEM.group_records[group].unknown_0x48;
}
