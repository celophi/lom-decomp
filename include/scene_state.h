#ifndef _SCENE_STATE_H
#define _SCENE_STATE_H

#include "common.h"

/**
 * @brief Persistent scene-selection / mode state at fixed address 0x801ED480.
 *
 * Lives in the global RAM region (outside every overlay's address range), so it
 * survives overlay swaps. Reset on mode entry by both the TITLE overlay
 * (RunTitle) and the FIELD overlay (field_scene_reset). FIELD reads @c unk0 and
 * @c unk2 back as the map id and object index when initializing a scene.
 *
 * @note The first two fields are also reached field-by-field through the
 *       standalone symbols @c D_801ED480 (== @c unk0) and @c D_801ED482
 *       (== @c unk2) elsewhere in the FIELD overlay.
 */
typedef struct
{
    u16 unk0;  /**< 0x00 map id       (standalone symbol D_801ED480) */
    u16 unk2;  /**< 0x02 object index (standalone symbol D_801ED482) */
    u32 unk4;  /**< 0x04 TODO: unknown; zeroed on TITLE entry */
    u32 unk8;  /**< 0x08 TODO: unknown; zeroed on TITLE entry */
    u32 unkC;  /**< 0x0C TODO: unknown; zeroed on TITLE entry */
    u32 unk10; /**< 0x10 TODO: unknown; zeroed on FIELD entry */
} S_801ED480;

#endif
