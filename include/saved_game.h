#ifndef SAVED_GAME_H
#define SAVED_GAME_H

#include "common.h"

#define SAVED_GAME_DATA_SIZE 0x3268
#define SAVED_GAME_BUFFER_SIZE 0x4000
#define SAVED_CHARACTER_SIZE 0x250
#define SAVED_CHARACTER_NAME_LENGTH 24

#define SAVED_OPTION_FLAG_2 0x04
#define SAVED_OPTION_FLAG_3 0x08

/** @brief Persistent character data initialized when a new game begins. */
typedef struct
{
    u8 name[SAVED_CHARACTER_NAME_LENGTH];
    u8 name_source_flags;
    u8 unknown_0x19[SAVED_CHARACTER_SIZE - 0x19];
} SavedCharacter;

/** @brief Mapped fields in the persistent game state. */
typedef struct
{
    u8 unknown_0x000[0x18];
    s32 field_entry_config;
    s16 option_id;
    s8 sub_mode;
    u8 unknown_0x01f;
    u32 music_track;
    u16 scene_mode;
    u8 field_flags;
    u8 layout_flags;
    u32 option_flags;
    u8 unknown_0x02c[0x34 - 0x2C];
    u32 weapon_category_masks[11];
    u8 unknown_0x060[0xD4 - 0x60];
    s16 rng_seed;
    u8 unknown_0x0d6[0x2E0 - 0xD6];
    s32 mode_flags;
    u8 unknown_0x2e4[0x5F0 - 0x2E4];
    SavedCharacter player;
} SavedGameLayout;

/** @brief Game-state workspace; the leading SAVED_GAME_DATA_SIZE bytes are saved to the memory card. */
typedef union
{
    SavedGameLayout layout;
    u8 bytes[SAVED_GAME_BUFFER_SIZE];
    s32 words[SAVED_GAME_BUFFER_SIZE / sizeof(s32)];
} SavedGame;

extern SavedGame g_saved_game;

#endif
