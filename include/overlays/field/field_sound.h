#include "main/audio/akao.h"
#ifndef FIELD_SOUND_H
#define FIELD_SOUND_H

#include "common.h"

#include "main/audio/game_audio.h"

/**
 * @file field_sound.h
 * @brief Menu sound effects played through FIELD's resident field_play_sound.
 */

/** @brief Played by the memory-card screens when a save has been written. */
#define FIELD_SOUND_SAVE_DONE 0x7A

/** @brief Played by the memory-card screens when a save has been read. */
#define FIELD_SOUND_LOAD_DONE 0x7B

/** @brief Sound of a window opening. */
#define FIELD_SOUND_WINDOW_OPEN 0xB9

/**
 * @brief Play a sound effect at full volume.
 * @param sound_id Sound effect id, such as SFX_CURSOR.
 * @param pan Pan position; AKAO_PAN_CENTER for a centred sound.
 */
void field_play_sound(s32 sound_id, s32 pan);

#endif
