/** @file field_scene_control.c
 * @brief Object and part transforms, animation and sequence control, the
 *        scene fade, the tint colour table and small scene queries.
 */

#include "akao_cmd.h"
#include "field_scene_internal.h"
#include "field_animation.h"
#include "field_calls.h"

/** Scene lists selected by field_control_animation and field_get_animation_state (FieldSeqDef::list_kind). */
enum
{
    FIELD_LIST_TILE_ANIMS = 0,    /**< FieldScene::anims */
    FIELD_LIST_PALETTE_ANIMS = 1, /**< FieldScene::strips */
    FIELD_LIST_TINT_ANIMS = 2,    /**< FieldScene::sprites (any other value selects it too) */
    FIELD_LIST_SEQUENCES = 3      /**< FieldScene::seqs */
};

/** Operations of field_control_animation. */
enum
{
    FIELD_ANIM_OP_START = 0,      /**< run the node */
    FIELD_ANIM_OP_STOP = 1,       /**< stop now, or at a keyframe */
    FIELD_ANIM_OP_RESTART = 2,    /**< rewind to the first frame, then run */
    FIELD_ANIM_OP_SEEK = 3,       /**< run towards a keyframe and stop there */
    FIELD_ANIM_OP_FINISH_LOOP = 4 /**< let a looping node stop at the end of its loop */
};

/** part_index value that selects the object itself instead of one of its parts. */
#define FIELD_WHOLE_OBJECT (-1)
/** Object index of field_set_color_scale that selects every object and the tile and effect lists. */
#define FIELD_ALL_OBJECTS (-1)
/** Keyframe argument of FIELD_ANIM_OP_STOP that stops the node at once. */
#define FIELD_KEYFRAME_NONE (-1)
/** FieldSeqDef::stop_keyframe value for "run on". */
#define FIELD_SEQ_NO_STOP 0xFF

/** Largest CLUT id a part corner may be rebiased to. */
#define FIELD_CLUT_ID_MAX 0x7FF

/** FieldMemState::fade_mode values. */
#define FIELD_FADE_IDLE 0
#define FIELD_FADE_OUT 1
#define FIELD_FADE_HELD 2
#define FIELD_FADE_IN 3
/** Fade level of a fully lit scene; also the neutral colour scale. */
#define FIELD_FADE_LEVEL_FULL 0x100
/** Fade level change per frame. */
#define FIELD_FADE_STEP 8

/** Bits the fade-out saves the active state into, one position above the live bit. */
#define FIELD_OBJ_SAVED_VISIBLE 2
#define FIELD_ANIM_FLAG_SAVED_ACTIVE 0x80
#define FIELD_SEQ_SAVED_PHASE_MASK 0xC

/** Tint colour saturation: products above this give a component of 0xFF. */
#define FIELD_TINT_PRODUCT_MAX 0xFEFFFF

/** GPU primitive codes stamped into the tint colour table. */
#define FIELD_PRIM_CODE_SPRT_16 0x7C
#define FIELD_PRIM_CODE_SPRT 0x64
#define FIELD_PRIM_CODE_POLY_FT4 0x2C
#define FIELD_PRIM_CODE_POLY_GT4 0x3C

/** 15-bit pixel: one 5-bit colour component and the semi-transparency bit. */
#define FIELD_PIXEL_COMPONENT_MASK 0x1F
#define FIELD_PIXEL_STP 0x8000

/**
 * @brief Rotation and scale of a part, in the order scripts pass them.
 *
 * Holds the same five halfwords as FieldPart::row_angle..scale_y, with the two
 * scales first.
 */
typedef struct
{
    /** Horizontal scale, 8.8 fixed point. */
    u16 scale_x;
    /** Vertical scale, 8.8 fixed point. */
    u16 scale_y;
    /** Rotation applied to the vertical (row) step. */
    u16 row_angle;
    /** Rotation applied to the horizontal (column) step. */
    u16 column_angle;
    /** Rotation of the grid as a whole. */
    u16 rotation_angle;
} FieldPartTransform;

/** Colour lookup tables of field_apply_pixel_lookup: one entry per 5-bit component level. */
extern u16 g_field_pixel_lookup_tables[][32];

static void field_start_animation(FieldSeq* seq);
/* Not in field_calls.h: field_actor_runtime.c calls it undeclared, and its code changes with the prototype. */
void field_set_color_scale(s16 index, u16 red_scale, u16 green_scale, u16 blue_scale);
static void field_tint_part_cells(FieldPart* part);
static void field_tint_animation_frames(FieldPart* cel, FieldAnim* anim);

/**
 * @brief Move a scene object, or one of its parts, and optionally rebias its
 *        corner CLUT ids by the depth change.
 *
 * Objects or parts with attached nodes first push the per-axis movement to
 * them. With @p rebias_cluts set, the depth moved (old z minus new z) is added
 * to the four corner CLUT ids, clamped to 0..FIELD_CLUT_ID_MAX: for a part
 * only that part, for an object every part of it and of every object after it
 * in the scene list.
 *
 * @param obj_index Index of the object in the scene's object list.
 * @param part_index Index of the part within that object, or FIELD_WHOLE_OBJECT.
 * @param pos New position in whole pixels.
 * @param rebias_cluts Non-zero to also rebias the CLUT ids.
 */
void field_set_object_position(s32 obj_index, s32 part_index, FieldPos* pos, s32 rebias_cluts)
{
    FieldObj* obj;
    FieldPart* part;
    s32 delta;
    s32 clut_sum;
    s32 clamped_clut;
    s32 new_z;

    part = NULL;
    delta = 0;
    if (part_index == FIELD_WHOLE_OBJECT)
    {
        obj = field_get_object(obj_index);
        if (rebias_cluts != 0)
        {
            new_z = pos->z;
            delta = (s16)(obj->z / 256) - new_z;
        }
        if (obj->flags.b.node_count != 0)
        {
            field_move_object_nodes(obj, (pos->x << 8) - obj->x, FIELD_AXIS_X);
            field_move_object_nodes(obj, (pos->y << 8) - obj->y, FIELD_AXIS_Y);
            field_move_object_nodes(obj, (pos->z << 8) - obj->z, FIELD_AXIS_Z);
        }
        obj->x = pos->x << 8;
        obj->y = pos->y << 8;
        obj->z = pos->z << 8;
    }
    else
    {
        part = field_get_object_part(obj_index, part_index);
        if (rebias_cluts != 0)
        {
            new_z = pos->z;
            delta = (s16)(part->z / 256) - new_z;
        }
        if (part->node_count != 0)
        {
            field_move_part_nodes(part, (pos->x << 8) - part->x, FIELD_AXIS_X);
            field_move_part_nodes(part, (pos->y << 8) - part->y, FIELD_AXIS_Y);
            field_move_part_nodes(part, (pos->z << 8) - part->z, FIELD_AXIS_Z);
        }
        part->x = pos->x << 8;
        part->y = pos->y << 8;
        part->z = pos->z << 8;
        obj = NULL;
    }
    if (rebias_cluts != 0)
    {
        do
        {
            if (obj != NULL)
            {
                part = obj->parts;
            }
            while (part != NULL)
            {
                clut_sum = part->clut_bl + (s16)delta;
                if (clut_sum > 0)
                {
                    clamped_clut = clut_sum;
                    if (clamped_clut > FIELD_CLUT_ID_MAX)
                    {
                        clamped_clut = FIELD_CLUT_ID_MAX;
                    }
                }
                else
                {
                    clamped_clut = 0;
                }
                part->clut_bl = clamped_clut;

                clut_sum = part->clut_tl + (s16)delta;
                if (clut_sum > 0)
                {
                    clamped_clut = clut_sum;
                    if (clamped_clut > FIELD_CLUT_ID_MAX)
                    {
                        clamped_clut = FIELD_CLUT_ID_MAX;
                    }
                }
                else
                {
                    clamped_clut = 0;
                }
                part->clut_tl = clamped_clut;

                clut_sum = part->clut_br + (s16)delta;
                if (clut_sum > 0)
                {
                    clamped_clut = clut_sum;
                    if (clamped_clut > FIELD_CLUT_ID_MAX)
                    {
                        clamped_clut = FIELD_CLUT_ID_MAX;
                    }
                }
                else
                {
                    clamped_clut = 0;
                }
                part->clut_br = clamped_clut;

                clut_sum = part->clut_tr + (s16)delta;
                if (clut_sum > 0)
                {
                    clamped_clut = clut_sum;
                    if (clamped_clut > FIELD_CLUT_ID_MAX)
                    {
                        clamped_clut = FIELD_CLUT_ID_MAX;
                    }
                    clut_sum = clamped_clut;
                }
                else
                {
                    clut_sum = 0;
                }
                part->clut_tr = clut_sum;

                if (obj == NULL)
                {
                    return;
                }
                part = part->next;
            }
            obj = obj->next;
        } while (obj != NULL);
    }
}

/**
 * @brief Rewind an animation node to its definition's first frame.
 *
 * Sets the frame and keyframe from the definition, reloads the timer and
 * requests the first upload or tween pass. Movie nodes are left alone.
 *
 * @param def Definition of the node.
 * @param anim Node to rewind.
 */
static inline void field_rewind_animation(FieldAnimDef* def, FieldAnim* anim)
{
    FieldTweenSpan* span;
    u8 frame;
    u8 range_start;

    if ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) != FIELD_ANIM_GROUP_KIND(FIELD_ANIM_GROUP_TILE, FIELD_TILE_ANIM_MOVIE))
    {
        anim->flags.word &= ~FIELD_ANIM_FLAG_REVERSE;
        if (def->flags.word & FIELD_ANIM_DEF_SPAN_INDEXED)
        {
            frame = def->head.b.unk1;
            anim->flags.b.keyframe = 0;
            anim->flags.b.state = frame;
        }
        else
        {
            frame = def->head.b.unk1;
            anim->flags.b.state = frame;
            anim->flags.b.keyframe = frame;
        }
        span = field_find_count_table_span(def, anim->flags.b.keyframe, &range_start);
        if (def->flags.word & FIELD_ANIM_DEF_TIMED)
        {
            anim->timer = span->duration;
        }
        else
        {
            anim->timer = 1;
        }
        if (((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) == FIELD_ANIM_GROUP_KIND(FIELD_ANIM_GROUP_TILE, FIELD_TILE_ANIM_UPLOAD)) ||
            ((def->flags.b.handler_group == FIELD_ANIM_GROUP_PALETTE) && ((def->flags.b.kind_flags & FIELD_ANIM_KIND_MASK) >= FIELD_PALETTE_ANIM_CYCLE)))
        {
            anim->flags.word |= FIELD_ANIM_FLAG_UPLOAD_PENDING;
        }
        if (((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) >= FIELD_TILE_ANIM_TWEEN_PART) &&
            ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) <= FIELD_TILE_ANIM_TWEEN_OBJECT))
        {
            field_apply_animation_tween(def, anim, 0);
        }
    }
}

/**
 * @brief Start the animation node a sequence names.
 *
 * Rewinds the node, gives it the command's repeat count and stop keyframe and
 * sets it running.
 *
 * @param seq Sequence whose command names the node.
 */
static void field_start_animation(FieldSeq* seq)
{
    FieldSeqDef* cmd;
    FieldAnimDef* def;
    FieldAnim* anim;
    FieldScene* scene;
    s32 i;

    cmd = seq->def;
    scene = g_field_scene.scene;
    switch (cmd->list_kind)
    {
    case FIELD_LIST_TILE_ANIMS:
        anim = scene->anims;
        break;
    case FIELD_LIST_PALETTE_ANIMS:
        anim = scene->strips;
        break;
    default:
        anim = scene->sprites;
        break;
    }
    i = cmd->anim_index;
    while (--i != -1)
    {
        anim = anim->next;
    }
    def = anim->def;
    field_rewind_animation(def, anim);
    anim->repeat_count = cmd->repeat_count;
    if (cmd->stop_keyframe == FIELD_SEQ_NO_STOP)
    {
        anim->flags.word &= ~FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
    }
    else
    {
        anim->flags.b.stop_keyframe = cmd->stop_keyframe;
        anim->flags.word |= FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
    }
    anim->flags.word = (anim->flags.word & ~FIELD_ANIM_FLAG_PING_PONG) | ((def->flags.word >> 3) & FIELD_ANIM_FLAG_PING_PONG) | FIELD_ANIM_FLAG_ACTIVE;
}

/**
 * @brief Start the sequence at @p index, or stop the sequences of a chain.
 *
 * Stopping clears the phase of every running or finished sequence that carries
 * chain @p index, and stops the animation node of every running or finished
 * sequence.
 *
 * @param index Sequence index to start, or chain index to stop.
 * @param start Non-zero to start, zero to stop.
 */
static inline void field_start_or_stop_sequence(s32 index, s32 start)
{
    FieldSeqDef* cmd;
    FieldSeq* seq;
    s32 i;

    seq = g_field_scene.scene->seqs;
    if (start)
    {
        i = index;
        while (--i != -1)
        {
            seq = seq->next;
        }
        field_start_sequence(seq, index & 0xFF);
        return;
    }
    while (seq != NULL)
    {
        if ((seq->flags.word & FIELD_SEQ_PHASE_MASK) != 0)
        {
            if (seq->flags.b.index == index)
            {
                seq->flags.word &= ~FIELD_SEQ_PHASE_MASK;
            }
            cmd = seq->def;
            field_control_animation(cmd->list_kind, cmd->anim_index, FIELD_KEYFRAME_NONE, FIELD_ANIM_OP_STOP);
        }
        seq = seq->next;
    }
}

/**
 * @brief Report whether no running or finished sequence carries a chain index.
 * @param index Chain index to look for.
 * @return 0 when a sequence carries it, otherwise 1.
 * @see field_is_sequence_chain_idle, the out-of-line function with the same body.
 */
static inline s32 field_scan_sequence_chain(s32 index)
{
    FieldSeq* seq;

    seq = g_field_scene.scene->seqs;
    while (seq != NULL)
    {
        if ((seq->flags.word & FIELD_SEQ_PHASE_MASK) != 0)
        {
            if (seq->flags.b.index == index)
            {
                return 0;
            }
        }
        seq = seq->next;
    }
    return 1;
}

/**
 * @brief Apply a control operation to one animation node, or start or stop
 *        sequences.
 *
 * For the three animation lists the node at @p index is located and @p op
 * applied. FIELD_ANIM_OP_STOP with FIELD_KEYFRAME_NONE stops the node at once
 * (silencing the channel of a sound node on the tile list first); with a
 * keyframe it stops the node there. FIELD_ANIM_OP_SEEK (and any unknown op)
 * runs the node in whichever direction reaches @p keyframe. For
 * FIELD_LIST_SEQUENCES, FIELD_ANIM_OP_STOP stops chain @p index and every
 * other op starts sequence @p index.
 *
 * @param list_kind Scene list (FIELD_LIST_*).
 * @param index Index of the node within that list.
 * @param keyframe Stop or seek keyframe, or FIELD_KEYFRAME_NONE.
 * @param op Operation (FIELD_ANIM_OP_*).
 */
void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op)
{
    FieldAnim* anim;
    FieldAnimDef* def;
    FieldAnimDef* sfx_def;
    FieldScene* scene;
    FieldSfxKey* key;
    s32 channel_mask;
    s32 sfx_id;
    s32 flags;

    scene = g_field_scene.scene;
    switch (list_kind)
    {
    case FIELD_LIST_TILE_ANIMS:
        anim = scene->anims;
        break;
    case FIELD_LIST_PALETTE_ANIMS:
        anim = scene->strips;
        break;
        /* Unreachable: its loop note keeps cse from carrying the scene load into the next arm (CSE-11). */
        do
        {
        } while (0);
    case FIELD_LIST_SEQUENCES:
        field_start_or_stop_sequence(index, op != FIELD_ANIM_OP_STOP);
        return;
    default:
        anim = scene->sprites;
        break;
    }
    while (--index != -1)
    {
        anim = anim->next;
    }
    switch (op)
    {
    case FIELD_ANIM_OP_RESTART:
        def = anim->def;
        field_rewind_animation(def, anim);
        /* fallthrough */
    case FIELD_ANIM_OP_START:
        def = anim->def;
        anim->flags.word &= ~FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
        anim->repeat_count = 0;
        flags = anim->flags.word & ~FIELD_ANIM_FLAG_PING_PONG;
        flags |= (def->flags.word >> 3) & FIELD_ANIM_FLAG_PING_PONG;
        anim->flags.word = flags;
        if ((list_kind == FIELD_LIST_TILE_ANIMS) && ((def->flags.word & FIELD_ANIM_KIND_MASK) == FIELD_TILE_ANIM_MOVIE))
        {
            if ((anim->flags.word & FIELD_ANIM_FLAG_ACTIVE) == 0)
            {
                anim->flags.word |= FIELD_ANIM_FLAG_ACTIVE;
                anim->flags.b.state = 0;
            }
        }
        else
        {
            anim->flags.word |= FIELD_ANIM_FLAG_ACTIVE;
        }
        break;
    case FIELD_ANIM_OP_STOP:
        if (keyframe == FIELD_KEYFRAME_NONE)
        {
            if ((list_kind == FIELD_LIST_TILE_ANIMS) && (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE))
            {
                sfx_def = anim->def;
                if ((sfx_def->flags.word & FIELD_ANIM_KIND_MASK) == FIELD_TILE_ANIM_SOUND)
                {
                    key = (FieldSfxKey*)sfx_def->data;
                    if (key->sound.word & FIELD_SFX_ONE_SHOT)
                    {
                        if (key->control.word & FIELD_SFX_CHANNEL_MASK)
                        {
                            channel_mask = 1 << (((key->control.word >> 8) & 0x1F) - 1);
                            sfx_id = 0;
                        }
                        else
                        {
                            channel_mask = 0;
                            sfx_id = key->sfx_id & FIELD_SFX_ID_MASK;
                        }
                        akao_cmd_21(sfx_id, channel_mask);
                    }
                }
            }
            anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
        }
        else
        {
            anim->flags.word |= FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
            anim->flags.b.stop_keyframe = keyframe;
        }
        anim->repeat_count = 0;
        break;
    case FIELD_ANIM_OP_FINISH_LOOP:
        def = anim->def;
        if (def->flags.word & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT)
        {
            if (def->flags.word & FIELD_ANIM_DEF_PING_PONG)
            {
                anim->flags.b.stop_keyframe = 0;
            }
            else
            {
                anim->flags.b.stop_keyframe = def->flags.b.last_frame;
            }
            anim->repeat_count = 0;
            anim->flags.word |= FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
        }
        break;
    case FIELD_ANIM_OP_SEEK:
    default:
        if (anim->flags.b.keyframe != keyframe)
        {
            anim->flags.word |= FIELD_ANIM_FLAG_PING_PONG;
            if (anim->flags.b.keyframe < keyframe)
            {
                anim->flags.word &= ~FIELD_ANIM_FLAG_REVERSE;
            }
            else
            {
                anim->flags.word |= FIELD_ANIM_FLAG_REVERSE;
            }
            anim->repeat_count = 0;
            anim->timer = 1;
            anim->flags.word |= FIELD_ANIM_FLAG_ACTIVE | FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
            anim->flags.b.stop_keyframe = keyframe;
        }
        break;
    }
}

/**
 * @brief Advance the scene fade by one frame.
 *
 * Fading out steps the level down to 0 and pushes it to every object's colour
 * scale. At 0 the scene is suspended: the first object (the cover) is shown
 * with its first two parts hidden, every other object, animation and sequence
 * has its active state saved one bit up and cleared, the first tile
 * animation is started, and the fade is held with the cover at full colour. Fading in steps the level back up to FIELD_FADE_LEVEL_FULL.
 */
void field_update_scene_fade(void)
{
    FieldMemState* state;
    FieldScene* scene;
    FieldObj* obj;
    FieldPart* part;
    FieldAnim* anim;
    FieldSeq* seq;

    state = FIELD_MEM_STATE;
    scene = g_field_scene.scene;
    switch (state->fade_mode)
    {
    case FIELD_FADE_OUT:
        state->fade_level -= FIELD_FADE_STEP;
        field_set_color_scale(FIELD_ALL_OBJECTS, state->fade_level, state->fade_level, state->fade_level);
        if (state->fade_level == 0)
        {
            obj = scene->objects;
            if ((obj->flags.word & FIELD_OBJ_VISIBLE) == 0)
            {
                obj->flags.word |= FIELD_OBJ_VISIBLE;
                part = obj->parts;
                part->visible = 0;
                part = part->next;
                part->visible = 0;
                obj = obj->next;
                while (obj != NULL)
                {
                    obj->flags.word = ((obj->flags.word & ~FIELD_OBJ_SAVED_VISIBLE) | ((obj->flags.b.unk0 & FIELD_OBJ_VISIBLE) << 1)) & ~FIELD_OBJ_VISIBLE;
                    obj = obj->next;
                }
                anim = scene->anims;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~FIELD_ANIM_FLAG_SAVED_ACTIVE) | ((anim->flags.word << 1) & FIELD_ANIM_FLAG_SAVED_ACTIVE)) &
                                       ~FIELD_ANIM_FLAG_ACTIVE;
                    anim = anim->next;
                }
                anim = scene->strips;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~FIELD_ANIM_FLAG_SAVED_ACTIVE) | ((anim->flags.word << 1) & FIELD_ANIM_FLAG_SAVED_ACTIVE)) &
                                       ~FIELD_ANIM_FLAG_ACTIVE;
                    anim = anim->next;
                }
                anim = scene->sprites;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~FIELD_ANIM_FLAG_SAVED_ACTIVE) | ((anim->flags.word << 1) & FIELD_ANIM_FLAG_SAVED_ACTIVE)) &
                                       ~FIELD_ANIM_FLAG_ACTIVE;
                    anim = anim->next;
                }
                anim = scene->effects;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~FIELD_ANIM_FLAG_SAVED_ACTIVE) | ((anim->flags.word << 1) & FIELD_ANIM_FLAG_SAVED_ACTIVE)) &
                                       ~FIELD_ANIM_FLAG_ACTIVE;
                    anim = anim->next;
                }
                seq = scene->seqs;
                while (seq != NULL)
                {
                    seq->flags.word =
                        ((seq->flags.word & ~FIELD_SEQ_SAVED_PHASE_MASK) | ((seq->flags.b.state & FIELD_SEQ_PHASE_MASK) << 2)) & ~FIELD_SEQ_PHASE_MASK;
                    seq = seq->next;
                }
                field_control_animation(FIELD_LIST_TILE_ANIMS, 0, 0, FIELD_ANIM_OP_START);
                g_field_scene_fade_mode = FIELD_FADE_HELD;
                field_set_color_scale(0, FIELD_FADE_LEVEL_FULL, FIELD_FADE_LEVEL_FULL, FIELD_FADE_LEVEL_FULL);
            }
        }
        break;
    case FIELD_FADE_IN:
        state->fade_level += FIELD_FADE_STEP;
        field_set_color_scale(FIELD_ALL_OBJECTS, state->fade_level, state->fade_level, state->fade_level);
        if (state->fade_level == FIELD_FADE_LEVEL_FULL)
        {
            state->fade_mode = FIELD_FADE_IDLE;
        }
        break;
    }
}

/**
 * @brief Resume the suspended scene and start the fade in.
 *
 * Undoes the suspension of field_update_scene_fade: the cover object is hidden
 * again and every other object, animation and sequence gets its saved active
 * state back.
 */
void field_begin_scene_fade_in(void)
{
    FieldScene* scene;
    FieldObj* obj;
    FieldObj** list;
    FieldPart* part;
    FieldAnim* anim;
    FieldSeq* seq;

    scene = g_field_scene.scene;
    g_field_scene_fade_mode = FIELD_FADE_IN;
    /* Read through a pointer: as scene->objects gcc moves the load above the store before it. */
    list = &scene->objects;
    obj = *list;
    obj->flags.word &= ~FIELD_OBJ_VISIBLE;
    part = obj->parts;
    part->visible = 0;
    part = part->next;
    part->visible = 0;
    obj = obj->next;
    while (obj != NULL)
    {
        obj->flags.word = (obj->flags.word & ~FIELD_OBJ_VISIBLE) | (((u32)obj->flags.word >> 1) & FIELD_OBJ_VISIBLE);
        obj = obj->next;
    }
    anim = scene->anims;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~FIELD_ANIM_FLAG_ACTIVE) | (((u32)anim->flags.word >> 1) & FIELD_ANIM_FLAG_ACTIVE);
        anim = anim->next;
    }
    anim = scene->strips;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~FIELD_ANIM_FLAG_ACTIVE) | (((u32)anim->flags.word >> 1) & FIELD_ANIM_FLAG_ACTIVE);
        anim = anim->next;
    }
    anim = scene->sprites;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~FIELD_ANIM_FLAG_ACTIVE) | (((u32)anim->flags.word >> 1) & FIELD_ANIM_FLAG_ACTIVE);
        anim = anim->next;
    }
    anim = scene->effects;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~FIELD_ANIM_FLAG_ACTIVE) | (((u32)anim->flags.word >> 1) & FIELD_ANIM_FLAG_ACTIVE);
        anim = anim->next;
    }
    seq = scene->seqs;
    while (seq != NULL)
    {
        seq->flags.word = (seq->flags.word & ~FIELD_SEQ_PHASE_MASK) | (((u32)seq->flags.word >> 2) & FIELD_SEQ_PHASE_MASK);
        seq = seq->next;
    }
}

/**
 * @brief Set the colour scale of one scene object, or of all of them, and
 *        rebuild the tints that depend on it.
 *
 * An object whose scale changes gets its colour times the scale expanded into
 * the tint colour table and the cells of its owning parts re-tinted. With
 * FIELD_ALL_OBJECTS the tile animations of handler kinds 0 and 1 and every
 * effect are re-tinted from their tint sources as well.
 *
 * @param index Object index, or FIELD_ALL_OBJECTS.
 * @param red_scale Red scale; FIELD_FADE_LEVEL_FULL is neutral.
 * @param green_scale Green scale.
 * @param blue_scale Blue scale.
 */
void field_set_color_scale(s16 index, u16 red_scale, u16 green_scale, u16 blue_scale)
{
    FieldScene* scene;
    FieldTintSrc* owner;
    FieldTintPal* tint;
    FieldPart* part;
    FieldAnim* anim;
    FieldPart* cel;
    u16* pal;
    u16 i;
    s32 rgb[3];

    i = 0;
    scene = g_field_scene.scene;
    owner = (FieldTintSrc*)scene->objects;
    while (owner != NULL)
    {
        if ((index == FIELD_ALL_OBJECTS || index == i) &&
            (owner->red_scale != red_scale || owner->green_scale != green_scale || owner->blue_scale != blue_scale))
        {
            tint = owner->palette;
            rgb[0] = owner->red * red_scale;
            rgb[1] = owner->green * green_scale;
            rgb[2] = owner->blue * blue_scale;
            owner->red_scale = red_scale;
            owner->green_scale = green_scale;
            owner->blue_scale = blue_scale;
            pal = tint->data;
            field_build_tint_colors((u8*)(pal + 2), pal[0], rgb);
            part = owner->cels;
            while (part != NULL)
            {
                if (part->instance_count != 0 && (part->code_word != 0 || part->shared == NULL))
                {
                    field_tint_part_cells(part);
                }
                part = part->next;
            }
        }
        owner = owner->next;
        i++;
    }
    if (index == FIELD_ALL_OBJECTS)
    {
        anim = scene->anims;
        while (anim != NULL)
        {
            if ((anim->def->flags.word & FIELD_ANIM_KIND_MASK) < FIELD_TILE_ANIM_CEL_CYCLE)
            {
                cel = field_find_grid_part(anim->def->u.tile.grid, &owner);
                tint = owner->palette;
                rgb[0] = owner->red * red_scale;
                rgb[1] = owner->green * green_scale;
                rgb[2] = owner->blue * blue_scale;
                pal = tint->data;
                field_build_tint_colors((u8*)(pal + 2), pal[0], rgb);
                if (cel->code_word == 0)
                {
                    field_tint_animation_frames(cel, anim);
                }
            }
            anim = anim->next;
        }
        anim = scene->effects;
        while (anim != NULL)
        {
            cel = field_find_grid_part(anim->def->u.tile.grid, &owner);
            tint = owner->palette;
            rgb[0] = owner->red * red_scale;
            rgb[1] = owner->green * green_scale;
            rgb[2] = owner->blue * blue_scale;
            pal = tint->data;
            field_build_tint_colors((u8*)(pal + 2), pal[0], rgb);
            if (cel->code_word == 0)
            {
                field_tint_animation_frames(cel, anim);
            }
            anim = anim->next;
        }
    }
}

/**
 * @brief Re-tint a part's cell records from the tint colour table.
 *
 * Each present cell copies the colour its tile descriptor selects into its
 * record. A part with a shared rgb/code word takes the colour of the first
 * present animated tile into that word instead. Part kinds 1 and 6 up are not
 * tinted.
 *
 * @param part Part to re-tint.
 * @note The case 0 and case 2..5 bodies are the same code written twice.
 */
static void field_tint_part_cells(FieldPart* part)
{
    FieldPartDef* grid;
    FieldTileDesc* tile;
    FieldTintColor* pal;
    FieldTintColor* entry;
    u8* dst;
    u32* mask;
    u32 word;
    u32 bit;
    s32 stride;
    s32 count;

    word = 0;
    grid = part->def;
    tile = grid->tiles;
    pal = FIELD_TINT_COLORS;
    switch (part->kind)
    {
    case 0:
        dst = part->records;
        stride = FIELD_CEL_RECORD_SIZE;
        if (part->code_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        if (part->tpage_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        mask = part->bits;
        count = grid->u.b.rows * grid->u.b.cols;
        bit = 0;
        while (--count != -1)
        {
            if (bit == 0)
            {
                word = *mask++;
                bit = 1;
            }
            if (part->code_word != 0)
            {
                if ((word & bit) && (tile->clut_slot & FIELD_TILE_ANIMATED))
                {
                    entry = &pal[tile->color_index];
                    ((FieldTintColor*)&part->code_word)->rg = entry->rg;
                    ((FieldTintColor*)&part->code_word)->b = entry->b;
                    return;
                }
            }
            else if (word & bit)
            {
                entry = &pal[tile->color_index];
                ((FieldCellTint*)dst)->rg = entry->rg;
                ((FieldCellTint*)dst)->b = entry->b;
                dst += stride;
            }
            bit <<= 1;
            tile++;
        }
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        dst = part->records;
        stride = FIELD_CEL_RECORD_SIZE;
        if (part->code_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        if (part->tpage_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        mask = part->bits;
        count = grid->u.b.rows * grid->u.b.cols;
        bit = 0;
        while (--count != -1)
        {
            if (bit == 0)
            {
                word = *mask++;
                bit = 1;
            }
            if (part->code_word != 0)
            {
                if ((word & bit) && (tile->clut_slot & FIELD_TILE_ANIMATED))
                {
                    entry = &pal[tile->color_index];
                    ((FieldTintColor*)&part->code_word)->rg = entry->rg;
                    ((FieldTintColor*)&part->code_word)->b = entry->b;
                    return;
                }
            }
            else if (word & bit)
            {
                entry = &pal[tile->color_index];
                ((FieldCellTint*)dst)->rg = entry->rg;
                ((FieldCellTint*)dst)->b = entry->b;
                dst += stride;
            }
            bit <<= 1;
            tile++;
        }
        break;
    }
}

/**
 * @brief Start a sequence, or stop a sequence chain (script command).
 * @param index Sequence index to start, or chain index to stop.
 * @param op Zero to start, non-zero to stop.
 */
void field_control_sequence(s32 index, s32 op)
{
    field_start_or_stop_sequence(index, op == 0);
}

/**
 * @brief Start a sequence and, recursively, the sequence it starts at once.
 *
 * The sequence is set running with chain index @p index and its animation node
 * is started. When its start link names a sequence and there is no start
 * delay, that sequence is started the same way.
 *
 * @param seq Sequence to start.
 * @param index Chain index stored in every sequence of the chain.
 */
void field_start_sequence(FieldSeq* seq, u8 index)
{
    FieldSeqDef* def;
    FieldScene* scene;
    FieldSeq* link;
    s32 i;

    scene = g_field_scene.scene;
    seq->phase_frames = 1;
    seq->flags.word = (seq->flags.word & ~FIELD_SEQ_PHASE_MASK) | FIELD_SEQ_PHASE_RUNNING;
    def = seq->def;
    seq->flags.b.index = index;
    field_start_animation(seq);
    i = def->start_link;
    if (i != FIELD_SEQ_NO_LINK && def->start_delay == 0)
    {
        link = scene->seqs;
        while (--i != -1)
        {
            link = link->next;
        }
        field_start_sequence(link, index);
    }
}

/**
 * @brief Report whether no running or finished sequence carries a chain index.
 * @param index Chain index to look for.
 * @return 0 when a sequence carries it, otherwise 1.
 */
s32 field_is_sequence_chain_idle(s32 index)
{
    FieldSeq* seq;

    seq = g_field_scene.scene->seqs;
    while (seq != NULL)
    {
        if ((seq->flags.word & FIELD_SEQ_PHASE_MASK) != 0)
        {
            if (seq->flags.b.index == index)
            {
                return 0;
            }
        }
        seq = seq->next;
    }
    return 1;
}

/**
 * @brief Report the play state of an animation node, or of a sequence chain.
 * @param list_kind Scene list (FIELD_LIST_*).
 * @param index Index of the node within that list, or the chain index.
 * @return A FIELD_ANIM_STATE_* value; a chain is RUNNING or FINISHED.
 */
s32 field_get_animation_state(s32 list_kind, s32 index)
{
    FieldAnim* anim;
    FieldAnimDef* def;
    FieldScene* scene;

    scene = g_field_scene.scene;
    switch (list_kind)
    {
    case FIELD_LIST_TILE_ANIMS:
        anim = scene->anims;
        break;
    case FIELD_LIST_PALETTE_ANIMS:
        anim = scene->strips;
        break;
        /* Unreachable: its loop note keeps cse from carrying the scene load into the next arm (CSE-11). */
        do
        {
        } while (0);
    case FIELD_LIST_SEQUENCES:
        return field_scan_sequence_chain(index) * FIELD_ANIM_STATE_FINISHED;
    default:
        anim = scene->sprites;
        break;
    }
    while (--index != -1)
    {
        anim = anim->next;
    }
    if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
    {
        if ((anim->flags.word & FIELD_ANIM_FLAG_STOP_AT_KEYFRAME) != 0)
        {
            return FIELD_ANIM_STATE_STOPPING;
        }
        if (list_kind == FIELD_LIST_TILE_ANIMS)
        {
            def = anim->def;
            if ((def->flags.word & FIELD_ANIM_KIND_MASK) == FIELD_TILE_ANIM_MOVIE)
            {
                if (anim->flags.b.state < 2)
                {
                    return FIELD_ANIM_STATE_MOVIE_STARTING;
                }
            }
        }
        return FIELD_ANIM_STATE_RUNNING;
    }
    return FIELD_ANIM_STATE_FINISHED;
}

/**
 * @brief Move the nodes attached to a part along one axis.
 * @param part Part whose nodes to move.
 * @param delta Distance to add.
 * @param axis FIELD_AXIS_X, FIELD_AXIS_Y or FIELD_AXIS_Z.
 * @note field_move_object_nodes is the object version; it subtracts on FIELD_AXIS_Y.
 */
void field_move_part_nodes(FieldPart* part, s32 delta, s32 axis)
{
    FieldScene* scene;
    FieldNode* node;
    s32 count;

    count = part->node_count;
    scene = g_field_scene.scene;
    if (count != 0 && delta != 0)
    {
        node = scene->nodes;
        while (node != NULL)
        {
            if (node->part == part)
            {
                switch (axis)
                {
                case FIELD_AXIS_X:
                    node->unk24 += delta;
                    node->unk34 += delta;
                    break;
                case FIELD_AXIS_Y:
                    node->delta_x += delta;
                    node->delta_y += delta;
                    node->x += delta;
                    node->y += delta;
                    break;
                case FIELD_AXIS_Z:
                default:
                    node->unk30 += delta;
                    node->unk40 += delta;
                    break;
                }
                count--;
                if (count == 0)
                {
                    break;
                }
            }
            node = node->next;
        }
    }
}

/**
 * @brief Move the nodes attached to an object along one axis.
 * @param obj Object whose nodes to move.
 * @param delta Distance to add (subtracted on FIELD_AXIS_Y).
 * @param axis FIELD_AXIS_X, FIELD_AXIS_Y or FIELD_AXIS_Z.
 */
void field_move_object_nodes(FieldObj* obj, s32 delta, s32 axis)
{
    FieldScene* scene;
    FieldNode* node;
    s32 count;

    count = obj->flags.b.node_count;
    scene = g_field_scene.scene;
    if (count != 0 && delta != 0)
    {
        node = scene->nodes;
        while (node != NULL)
        {
            if (node->obj == obj)
            {
                switch (axis)
                {
                case FIELD_AXIS_X:
                    node->unk24 += delta;
                    node->unk34 += delta;
                    break;
                case FIELD_AXIS_Y:
                    node->delta_x -= delta;
                    node->delta_y -= delta;
                    node->x -= delta;
                    node->y -= delta;
                    break;
                case FIELD_AXIS_Z:
                default:
                    node->unk30 += delta;
                    node->unk40 += delta;
                    break;
                }
                count--;
                if (count == 0)
                {
                    break;
                }
            }
            node = node->next;
        }
    }
}

/**
 * @brief Return a scene object by its index in the object list.
 * @param index Object index.
 * @return The object.
 */
FieldObj* field_get_object(s32 index)
{
    FieldObj* obj;

    obj = g_field_scene.scene->objects;
    while (--index != -1)
    {
        obj = obj->next;
    }
    return obj;
}

/**
 * @brief Return a part of a scene object by object and part index.
 * @param obj_index Object index.
 * @param part_index Part index within the object.
 * @return The part.
 */
FieldPart* field_get_object_part(s32 obj_index, s32 part_index)
{
    FieldObj* obj;
    FieldPart* part;

    obj = g_field_scene.scene->objects;
    while (--obj_index != -1)
    {
        obj = obj->next;
    }
    part = obj->parts;
    while (--part_index != -1)
    {
        part = part->next;
    }
    return part;
}

/**
 * @brief Find the part laid out on a tile grid, searching every scene object.
 * @param grid Grid definition to look for.
 * @param out_src Receives the object (as its tint source) owning the part; may be NULL.
 * @return The part, or NULL when no object has one on @p grid.
 */
FieldPart* field_find_grid_part(FieldPartDef* grid, FieldTintSrc** out_src)
{
    FieldTintSrc* src;
    FieldPart* cel;

    src = (FieldTintSrc*)g_field_scene.scene->objects;
    while (src != NULL)
    {
        cel = src->cels;
        while (cel != NULL)
        {
            if (grid == cel->def)
            {
                if (out_src != NULL)
                {
                    *out_src = src;
                }
                return cel;
            }
            cel = cel->next;
        }
        src = src->next;
    }
    return NULL;
}

/**
 * @brief Expand a palette into the tint colour table, scaling each component.
 *
 * Each component is multiplied by its scale and the product's bits 16-23 are
 * stored, saturating at 0xFF. The primitive code byte of each entry is left
 * alone.
 *
 * @param colors Palette entries, 4 bytes each (red, green, blue, unused).
 * @param count Number of entries.
 * @param rgb_scale Red, green and blue factors.
 */
void field_build_tint_colors(u8* colors, s32 count, s32* rgb_scale)
{
    u8* dst;
    s32 red;
    s32 green;
    s32 blue;
    u32 v;

    dst = (u8*)FIELD_TINT_COLORS;
    red = rgb_scale[0];
    green = rgb_scale[1];
    blue = rgb_scale[2];
    while (--count != -1)
    {
        v = colors[0] * red;
        if (v > FIELD_TINT_PRODUCT_MAX)
        {
            dst[0] = 0xFF;
        }
        else
        {
            dst[0] = v >> 16;
        }
        v = colors[1] * green;
        if (v > FIELD_TINT_PRODUCT_MAX)
        {
            dst[1] = 0xFF;
        }
        else
        {
            dst[1] = v >> 16;
        }
        v = colors[2] * blue;
        if (v > FIELD_TINT_PRODUCT_MAX)
        {
            dst[2] = 0xFF;
        }
        else
        {
            dst[2] = v >> 16;
        }
        colors += 4;
        dst += 4;
    }
}

/**
 * @brief Stamp the primitive code of a part kind into the tint colour table.
 * @param format Part kind (FieldPart::kind).
 * @param count Number of table entries.
 * @param primitive_code Code the table holds now; updated when it changes.
 */
void field_set_tint_primitive_code(u8 format, s32 count, u8* primitive_code)
{
    s32 code;
    u8* entry_code;

    switch (format)
    {
    case 0:
        code = FIELD_PRIM_CODE_SPRT_16;
        break;
    case 1:
        code = FIELD_PRIM_CODE_SPRT;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        code = FIELD_PRIM_CODE_POLY_FT4;
        break;
    default:
        code = FIELD_PRIM_CODE_POLY_GT4;
        break;
    }
    if (code != *primitive_code)
    {
        entry_code = &FIELD_TINT_COLORS[0].code;
        while (--count != -1)
        {
            *entry_code = code;
            entry_code += sizeof(FieldTintColor);
        }
        *primitive_code = code;
    }
}

/**
 * @brief Re-tint an animation's frame records from the tint colour table.
 *
 * Every record takes the colour of the table entry its source tile names. Cel
 * kinds 1 and 6 up are not tinted.
 *
 * @param cel Cel whose kind and shared words set the record stride.
 * @param anim Animation whose frame records to re-tint.
 * @note The case 0 and case 2..5 bodies are the same code written twice.
 */
static void field_tint_animation_frames(FieldPart* cel, FieldAnim* anim)
{
    FieldAnimDef* def;
    FieldTintColor* pal;
    FieldTintColor* entry;
    FieldTileDesc* tile;
    u8* dst;
    s32 stride;
    s32 count;

    pal = FIELD_TINT_COLORS;
    def = anim->def;
    switch (cel->kind)
    {
    case 0:
        stride = FIELD_CEL_RECORD_SIZE;
        dst = anim->frame_data;
        if (cel->code_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        if (cel->tpage_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        tile = (FieldTileDesc*)def->data;
        count = anim->frame_tile_count * def->flags.b.frame_count;
        while (--count != -1)
        {
            entry = &pal[tile->color_index];
            ((FieldCellTint*)dst)->rg = entry->rg;
            tile++;
            ((FieldCellTint*)dst)->b = entry->b;
            dst += stride;
        }
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        stride = FIELD_CEL_RECORD_SIZE;
        dst = anim->frame_data;
        if (cel->code_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        if (cel->tpage_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        tile = (FieldTileDesc*)def->data;
        count = anim->frame_tile_count * def->flags.b.frame_count;
        while (--count != -1)
        {
            entry = &pal[tile->color_index];
            ((FieldCellTint*)dst)->rg = entry->rg;
            tile++;
            ((FieldCellTint*)dst)->b = entry->b;
            dst += stride;
        }
        break;
    }
}

/**
 * @brief Show or hide a scene object, or one of its parts.
 * @param obj_index Object index.
 * @param part_index Part index within the object, or FIELD_WHOLE_OBJECT.
 * @param visible Non-zero to show.
 */
void field_set_object_visible(s32 obj_index, s32 part_index, s32 visible)
{
    FieldObj* obj;
    FieldPart* part;

    if (part_index == FIELD_WHOLE_OBJECT)
    {
        obj = field_get_object(obj_index);
        obj->flags.word = (obj->flags.word & ~FIELD_OBJ_VISIBLE) | (visible & FIELD_OBJ_VISIBLE);
    }
    else
    {
        part = field_get_object_part(obj_index, part_index);
        part->visible = visible;
    }
}

/**
 * @brief Read the position of a scene object, or of one of its parts.
 * @param obj_index Object index.
 * @param part_index Part index within the object, or FIELD_WHOLE_OBJECT.
 * @param out Receives the position in whole pixels.
 */
void field_get_object_position(s32 obj_index, s32 part_index, FieldPos* out)
{
    FieldObj* obj;
    FieldPart* part;

    if (part_index == FIELD_WHOLE_OBJECT)
    {
        obj = field_get_object(obj_index);
        out->x = SHIFT_TOWARD_ZERO(obj->x, 8);
        out->y = SHIFT_TOWARD_ZERO(obj->y, 8);
        out->z = SHIFT_TOWARD_ZERO(obj->z, 8);
    }
    else
    {
        part = field_get_object_part(obj_index, part_index);
        out->x = SHIFT_TOWARD_ZERO(part->x, 8);
        out->y = SHIFT_TOWARD_ZERO(part->y, 8);
        out->z = SHIFT_TOWARD_ZERO(part->z, 8);
    }
}

/**
 * @brief Set the rotation and scale of a part of a scene object.
 * @param obj_index Object index.
 * @param part_index Part index within the object.
 * @param xf New rotation and scale.
 */
void field_set_part_transform(s32 obj_index, s32 part_index, FieldPartTransform* xf)
{
    FieldPart* part;

    part = field_get_object_part(obj_index, part_index);
    part->scale_x = xf->scale_x;
    part->scale_y = xf->scale_y;
    part->row_angle = xf->row_angle;
    part->column_angle = xf->column_angle;
    part->rotation_angle = xf->rotation_angle;
}

/**
 * @brief Read the rotation and scale of a part of a scene object.
 * @param obj_index Object index.
 * @param part_index Part index within the object.
 * @param xf Receives the rotation and scale.
 */
void field_get_part_transform(s32 obj_index, s32 part_index, FieldPartTransform* xf)
{
    FieldPart* part;

    part = field_get_object_part(obj_index, part_index);
    xf->scale_x = part->scale_x;
    xf->scale_y = part->scale_y;
    xf->row_angle = part->row_angle;
    xf->column_angle = part->column_angle;
    xf->rotation_angle = part->rotation_angle;
}

/**
 * @brief Run an effect animation back to its first keyframe, or on to its last.
 *
 * A timed effect that is not on keyframe 0 runs backwards to it, unless
 * @p forward is set. Otherwise an effect that is not on its last keyframe runs
 * forwards to it, with a pending start when it is untimed and on keyframe 0.
 * The call does nothing when @p index is past the end of the effect list.
 *
 * @param index Index in the scene's effect list.
 * @param forward Non-zero to always run forwards.
 */
void field_play_effect_animation(s32 index, s32 forward)
{
    FieldAnim* anim;
    FieldAnimDef* def;

    anim = g_field_scene.scene->effects;
    if (anim != NULL)
    {
        while (--index != -1)
        {
            anim = anim->next;
            if (anim == NULL)
            {
                return;
            }
        }
        def = anim->def;
        if (((def->flags.word & FIELD_ANIM_DEF_TIMED) != 0) && (forward == 0))
        {
            if (anim->flags.b.keyframe != 0)
            {
                anim->timer = 1;
                anim->flags.word |= FIELD_ANIM_FLAG_ACTIVE | FIELD_ANIM_FLAG_REVERSE | FIELD_ANIM_FLAG_PING_PONG;
            }
        }
        else if (anim->flags.b.keyframe != def->flags.b.last_frame)
        {
            anim->flags.word |= FIELD_ANIM_FLAG_ACTIVE;
            anim->flags.word &= ~FIELD_ANIM_FLAG_REVERSE;
            anim->flags.word &= ~FIELD_ANIM_FLAG_PING_PONG;
            anim->timer = 1;
            if (((def->flags.word & FIELD_ANIM_DEF_TIMED) == 0) && (anim->flags.b.keyframe == 0))
            {
                anim->flags.word |= FIELD_ANIM_FLAG_START_PENDING;
            }
        }
    }
}

/**
 * @brief Start the scene fade out unless a fade is already running.
 */
void field_begin_scene_fade_out(void)
{
    if (FIELD_MEM_STATE->fade_mode == FIELD_FADE_IDLE)
    {
        FIELD_MEM_STATE->fade_mode = FIELD_FADE_OUT;
        FIELD_MEM_STATE->fade_level = FIELD_FADE_LEVEL_FULL;
    }
}

/**
 * @brief Report whether the scene fade is running or held.
 * @return 1 when the fade is not idle, otherwise 0.
 */
s32 field_is_scene_fading(void)
{
    return g_field_scene_fade_mode != FIELD_FADE_IDLE;
}

/**
 * @brief Enable or disable a scene node for the collision group scan.
 * @param index Index in the scene's node list.
 * @param enabled Non-zero to enable.
 * @see decomp.me (100%) https://decomp.me/scratch/lN7ye
 */
void field_set_node_enabled(s32 index, s32 enabled)
{
    FieldNode* node;
    FieldScene* scene = g_field_scene.scene;

    node = scene->nodes;
    while (--index != -1)
    {
        node = node->next;
    }
    node->unk18 = enabled;
    if (scene->group_work != 0)
    {
        field_collision_rasterize_groups(0, node);
    }
}

/**
 * @brief Select the pixel lookup table field_load_map applies to the next map.
 * @param selector Table number plus one, or 0 for none.
 */
void field_set_pixel_lookup(s32 selector)
{
    g_field_pixel_lookup_selector = selector;
}

/**
 * @brief Recolour 15-bit pixels through a lookup table.
 *
 * Each pixel is replaced by the table entry for its brightest component, with
 * its semi-transparency bit kept.
 *
 * @param pixels Pixels to recolour.
 * @param pixel_count Number of pixels.
 * @param table_index Lookup table to use.
 * @param unused Not used.
 */
void field_apply_pixel_lookup(u16* pixels, s32 pixel_count, s32 table_index, void* unused)
{
    u16* table;
    s32 count;
    u16 pixel;
    u32 green;
    u32 red;
    u32 level;

    count = pixel_count;
    table = g_field_pixel_lookup_tables[table_index];
    while (--count != -1)
    {
        pixel = *pixels;
        level = pixel & FIELD_PIXEL_COMPONENT_MASK;
        green = (pixel >> 5) & FIELD_PIXEL_COMPONENT_MASK;
        if (level < green)
        {
            level = green;
        }
        red = (pixel >> 10) & FIELD_PIXEL_COMPONENT_MASK;
        if (level < red)
        {
            level = red;
        }
        *pixels = table[level] + (pixel & FIELD_PIXEL_STP);
        pixels++;
    }
}

/**
 * @brief Find the scene object built from an object definition.
 * @param definition Object definition to look for.
 * @return The object, or NULL when none uses @p definition.
 * @see decomp.me (100%) https://decomp.me/scratch/FThyS
 */
FieldObj* field_find_object_by_definition(void* definition)
{
    FieldObj* obj;

    obj = g_field_scene.scene->objects;
    while (obj != NULL)
    {
        if (definition == obj->def)
        {
            return obj;
        }
        obj = obj->next;
    }
    return NULL;
}
