#ifndef _FIELD_ENTRY_H
#define _FIELD_ENTRY_H

#include "common.h"

/*
 * Field entry settings handed to field_set_scene_parameters on each field entry
 * and saved with the game (SavedGameLayout).
 */

/**
 * @brief Scene of the next field entry.
 * @note The main executable, WMAP and TITLE write the scene as a halfword (id);
 *       FIELD and GOVER read and write the whole word.
 */
typedef union FieldSceneId
{
    s32 word;
    u16 id;
} FieldSceneId;

/** @brief Spawn record of the next field entry; the top seven bits are flags. */
extern u32 g_field_spawn_id;
extern FieldSceneId g_field_scene_id;
/** @brief Sound-bank resource of the current field; -1 clears the loaded bank header. */
extern s32 g_field_sound_bank_id;
/** @brief Primary music resource of the current field. */
extern s32 g_field_music_id;
/** @brief Field object selected for the render context on field entry. */
extern s32 g_field_object_id;
/** @brief Secondary music resource of the current field; -1 retains the current one. */
extern s32 g_field_secondary_music_id;
/** @brief Set when FIELD requests a scene change; run_field_scene clears it before each entry. */
extern s32 g_field_scene_request_pending;

#endif
