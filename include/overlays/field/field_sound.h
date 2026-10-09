#include "main/audio/akao.h"
#ifndef FIELD_SOUND_H
#define FIELD_SOUND_H

#include "common.h"

/**
 * @file field_sound.h
 * @brief Menu sound effects played through FIELD's resident field_play_sound.
 */

/** @brief Refusal buzz; menus also play it when backing out with Circle. */
#define FIELD_SOUND_ACTION_REFUSED 0x78

/** @brief Played by the memory-card screens when a save has been written. */
#define FIELD_SOUND_SAVE_DONE 0x7A

/** @brief Played by the memory-card screens when a save has been read. */
#define FIELD_SOUND_LOAD_DONE 0x7B

/** @brief Cursor movement. */
#define FIELD_SOUND_CURSOR 0x7D

/** @brief Confirming a selection. */
#define FIELD_SOUND_SELECT 0x7E

/** @brief Cancelling a selection. */
#define FIELD_SOUND_CANCEL 0x7F

/**
 * @brief Play a sound effect at full volume.
 * @param sound_id Sound effect id, such as FIELD_SOUND_CURSOR.
 * @param pan Pan position; AKAO_PAN_CENTER for a centred sound.
 */
void field_play_sound(s32 sound_id, s32 pan);

#endif
