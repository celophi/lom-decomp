#ifndef FIELD_SCENE_H
#define FIELD_SCENE_H

#include "common.h"

/** @brief Runtime scene records and storage shared with in-scene movies. */
typedef struct
{
    struct FieldSceneHeader* header;
    struct FieldObj* objects; /* head of the object list */
    struct FieldNode* nodes;  /* head of the attached-node list */
    /** last node owned by a swinging part (FIELD_PART_SWEEP_MASK); its movement carries
        movers in field_collision_resolve_move. */
    struct FieldNode* sweep_node;
    struct FieldMarker* markers; /* head of the marker list */
    struct FieldSeq* seqs;       /* head of the sequence list */
    struct FieldAnim* anims;     /* head of the animation list */
    struct FieldAnim* strips;    /* head of the strip list */
    struct FieldAnim* sprites;   /* head of the sprite list */
    struct FieldAnim* effects;   /* head of the effect list */
    /** base of the per-group tile bitmask rows, or 0 when no groups
        are active (then group_count holds a FIELD_COLLISION_GROUP_ERROR_*
        code); field_set_node_enabled gates its field_collision_rasterize_groups call on this. */
    u8* group_work;
    /** base of the per-group byte tile maps. */
    u8* group_tiles;
    /** end of the per-group work area. */
    u8* group_work_end;
    struct FieldImageReq* uploads; /* head of the pending upload list */
    /** set by a movie animation while the scene builds, then the MDEC VLC table it gets. */
    u8* vlc_table;
    /** cleared by the scene build. */
    s32 unk3C;
    /** tile edge in pixels, 4 or 8. */
    u8 tile_size;
    /** number of active groups; 0 or 1 when the scan found nothing. */
    u8 group_count;
    /** bitmask words per group in the work area. */
    s16 group_stride;
    /** tiles per group (tile_cols * tile_rows). */
    u16 group_tile_count;
    /** tile columns. */
    u16 tile_cols;
    /** tile rows. */
    u16 tile_rows;
    /** group ids (floor heights), sorted ascending by field_collision_collect_groups. */
    s16 group_ids[10];
    /** per-group counters, zeroed alongside group_ids. */
    s16 group_counters[10];
} FieldScene;

/** @brief Active scene pointer in the loaded FIELD resource. */
typedef struct
{
    FieldScene* scene;
} FieldSceneGlobals;

/** @brief Active FIELD scene. */
extern FieldSceneGlobals g_field_scene;

#endif
