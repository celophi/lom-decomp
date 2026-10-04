#ifndef FIELD_PLAYER_RECORDS_H
#define FIELD_PLAYER_RECORDS_H

#include "common.h"

/**
 * @file field_player_records.h
 * @brief Party member records shared by actor commands, resources and the HUD.
 */

/** @brief FieldPlayerRecord::flags bit: the party member is present. */
#define FIELD_PLAYER_ACTIVE 0x1

/** @brief FieldPlayerRecord::character_kind values. */
#define FIELD_PLAYER_KIND_HERO 0
#define FIELD_PLAYER_KIND_PARTNER 1
#define FIELD_PLAYER_KIND_COMPANION 2

/** @brief Flag byte and weapon type of a party member, also updated as one halfword. */
typedef union FieldPlayerHead
{
    u16 word;
    struct
    {
        /** @brief FIELD_PLAYER_ACTIVE and the other bits of @c bits. */
        u8 flags;
        /** @brief Weapon type (item type) of the equipped weapon, 0xFF before the first party update. */
        u8 weapon_type;
    } bytes;
    struct
    {
        /** @brief Same bit as FIELD_PLAYER_ACTIVE. */
        u16 active : 1;
        /** @brief Selects the hero's alternate sprite and portrait set. */
        u16 alt_appearance : 1;
        u16 unk2 : 14;
    } bits;
} FieldPlayerHead;

/** @brief Per-party-member record (0x268 bytes). */
typedef struct FieldPlayerRecord
{
    FieldPlayerHead head;
    /** @brief Character within character_kind (partner or companion id). */
    u8 character_id;
    /** @brief FIELD_PLAYER_KIND_* value selecting the resource set. */
    u8 character_kind;
    u8 unk4[0x254 - 4];
    /** @brief CD resource id of the loaded sprite package. */
    u16 resource_id;
    /** @brief Portrait currently cached for this member, 0xFF for none. */
    u8 portrait_index;
    /** @brief Frames left to chain a combo action. */
    u8 combo_timer;
    u8 unk258;
    u8 hit_state;
    u8 unk25A;
    u8 unk25B;
    u8 unk25C;
    u8 unk25D;
    /** @brief Frames spent knocked down; at revive_delay the member revives (the HUD shows it as the recovery gauge). */
    s16 revive_time;
    s16 revive_delay;
    /** @brief Animation, effect resource and sound (-1 for none) used when the revive timer runs out. */
    s16 revive_animation;
    s16 revive_effect;
    s16 revive_sound;
} FieldPlayerRecord;

extern FieldPlayerRecord g_field_player_records[];

#endif /* FIELD_PLAYER_RECORDS_H */
