/** @file field_scene_control.c
 * @brief Object and part transforms, animation controls, scene state, and scene queries.
 */

#include "field_scene_internal.h"

u8* field_find_count_table_span(u8*, s32, volatile s8*);

/** @brief Movie/streaming control block at 0x801ED500. */
#define FIELD_MOVIE_STATE ((volatile FieldMovieState*)0x801ED500)
/** @brief Field CD/movie flag word at 0x801ED800. */
#define FIELD_CD_FLAGS (*(volatile s32*)0x801ED800)

/**
 * @brief Full-word bitfield view of the field CD flag word at 0x801ED800.
 * @note Clearing bit 0x40 through this view reproduces the target's codegen for
 *       the movie tear-down path (see field_update_scene_animations).
 */
typedef struct { u32 word : 32; } CdWordBits;

extern u16 g_field_movie_frame_width;
extern u16 g_field_movie_frame_height;

void func_800157B0(s32);
s32 cdrom_process_state(void);
void cdrom_reset(void);
void cdrom_stream(s32, void*);
void cdrom_queue_seek(s32);
void cdrom_queue_read(s32, void*);
s32 cdrom_can_queue_resource(s32);
void field_update_part_sweep(FieldPart*);
void field_blit_animation_frame(FieldAnimDef*, FieldAnim*, s32);
void field_apply_animation_tween(FieldAnimDef*, FieldAnim*, s32);
void field_update_animation_sfx(FieldAnimDef*, FieldAnim*);
void field_retarget_cel_cluts(FieldAnimDef*, FieldAnimCel*, s32);
u_long* field_blend_animation_frames(FieldAnimDef*, FieldAnim*);
void field_tint_animation_cel(FieldAnimDef*, FieldAnimCel*, FieldTintSrc*, s32);
void field_tint_animation_cel_list(FieldAnimDef*, FieldTintSrc*, s32);
void field_advance_animation_keyframe(FieldAnimDef*, FieldAnim*);
void field_retarget_cel_list_cluts(FieldAnimDef*, FieldTintSrc*, s32);
void field_queue_vram_upload(FieldImageReq*);
void func_80059F18(void);
void func_8005A744(FieldSeq*, u8);
s32 func_8005A84C(s32, s32);
void func_80084240(void);
void func_80140358(s32, s32, s32, s32);
void func_801406E4(void);
void func_80140D48(void);

/**
 * @brief World position of a field object or part, in whole pixels.
 *
 * The stored offsets on FieldObj / FieldPart are the same three values shifted
 * left by 8, so a coordinate here is worth 256 of theirs.
 */
typedef struct
{
    /** 0x00 horizontal position. */
    s16 x;
    /** 0x02 vertical position. */
    s16 y;
    /** 0x04 depth. */
    s16 z;
} FieldPos;

/**
 * @brief Rotation and scale record handed to func_8005B034.
 *
 * Holds the same five halfwords FieldPart carries at 0x3A..0x43, but in its own
 * order: the two scales first, then the three angles.
 */
typedef struct
{
    /** 0x00 horizontal scale, 8.8 fixed point. */
    u16 scale_x;
    /** 0x02 vertical scale, 8.8 fixed point. */
    u16 scale_y;
    /** 0x04 rotation applied to the vertical (row) step. */
    u16 row_angle;
    /** 0x06 rotation applied to the horizontal (column) step. */
    u16 column_angle;
    /** 0x08 rotation of the grid as a whole. */
    u16 rotation_angle;
} FieldPartTransform;

FieldObj* func_8005AB4C(s32);
FieldPart* func_8005AB80(s32, s32);

/**
 * @brief Move a scene object - or a single one of its parts - to a new
 *        position, and rebias the affected parts' CLUT ids by the depth change.
 *
 * The target is resolved by walking the scene's object list: @p part_index of
 * -1 selects the whole object (func_8005AB4C), any other value selects that
 * part of it (func_8005AB80). The object/part offsets are stored 8.8 fixed
 * point, so each component of @p pos is shifted left by 8 on the way in.
 *
 * When @p rebias_cluts is set the routine first records the depth the target is
 * moving by, @c old_z/256 - @c pos->z, then - after the position has been
 * written - adds it to every corner CLUT id of the affected parts, clamped to
 * 0..0x7FF. For the whole-object case that walk covers the resolved object and
 * every object after it in the list; for the single-part case it touches only
 * that part.
 *
 * Objects whose FieldObjFlags::node_count (or FieldPart::node_count) is
 * non-zero also get the per-axis movement pushed through func_8005AA68 /
 * func_8005A984 before the new position lands, one call per axis.
 *
 * @param obj_index     Index of the object in the scene's object list.
 * @param part_index    Index of the part within that object, or -1 for the
 *                      object itself.
 * @param pos           New position, in whole pixels.
 * @param rebias_cluts  Zero to only reposition; non-zero to also push the
 *                      per-axis movement through the notifier and rebias the
 *                      parts' CLUT ids by the depth change.
 */
void field_set_object_position(s32 obj_index, s32 part_index, FieldPos* pos, s32 rebias_cluts)
{
    FieldObj* obj;
    FieldPart* part;
    s32 delta;
    s32 clut_sum;
    s32 clamped_clut;
    s32 zpos;

    part = NULL;
    delta = 0;
    if (part_index == -1)
    {
        obj = func_8005AB4C(obj_index);
        if (rebias_cluts != 0)
        {
            zpos = pos->z;
            delta = (s16)(obj->z / 256) - zpos;
        }
        if (obj->flags.b.node_count != 0)
        {
            func_8005AA68(obj, (pos->x << 8) - obj->x, 0);
            func_8005AA68(obj, (pos->y << 8) - obj->y, 1);
            func_8005AA68(obj, (pos->z << 8) - obj->z, 2);
        }
        obj->x = pos->x << 8;
        obj->y = pos->y << 8;
        obj->z = pos->z << 8;
    }
    else
    {
        part = func_8005AB80(obj_index, part_index);
        if (rebias_cluts != 0)
        {
            zpos = pos->z;
            delta = (s16)(part->z / 256) - zpos;
        }
        if (part->node_count != 0)
        {
            func_8005A984(part, (pos->x << 8) - part->x, 0);
            func_8005A984(part, (pos->y << 8) - part->y, 1);
            func_8005A984(part, (pos->z << 8) - part->z, 2);
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
                    if (clamped_clut >= 0x800)
                    {
                        clamped_clut = 0x7FF;
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
                    if (clamped_clut >= 0x800)
                    {
                        clamped_clut = 0x7FF;
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
                    if (clamped_clut >= 0x800)
                    {
                        clamped_clut = 0x7FF;
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
                    if (clamped_clut >= 0x800)
                    {
                        clamped_clut = 0x7FF;
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
 * @brief Arm the animation node a sequence command refers to.
 *
 * The command record is the sequence's own FieldAnimDef, read here with its
 * sequence-command meanings: FieldAnimDef::unk0 picks which of the scene's
 * three animation lists to walk, FieldAnimDef::unk2 is the index within it,
 * FieldAnimDef::unk3 is the repeat count and FieldAnimDef::flags doubles as the
 * stop keyframe (0xFF meaning "no stop").
 *
 * Once the node is located its own definition drives a reset: the frame and
 * keyframe indices are seeded from FieldAnimDef::unk1, the timer from the
 * keyframe's span duration (or 1 when the definition does not use spans), and
 * the tween handlers get an initial pass. The last write sets bit 0x40, which
 * is what field_update_scene_animations tests before ticking the node, so the
 * animation only starts running here.
 *
 * @param seq Sequence node whose definition carries the command.
 *
 * @note The handler kind is the word at FieldAnimDef::flags masked with
 *       0xFF000007 - the low three bits of byte 0x04 plus the sub-kind byte at
 *       0x07 - so it is read as @c *(s32 *) &def->flags, the same spelling
 *       field_apply_animation_tween uses. Kind 4 skips the whole reset; kinds 5
 *       and 6 additionally get a tween pass.
 * @note FieldAnimDef::unk1 is read into @p frame before either store, because
 *       the stores are through FieldAnim and gcc cannot rule out an alias.
 *
 * @see decomp.me (100%) TODO
 */
void field_start_animation(FieldSeq* seq)
{
    FieldAnimDef* cmd;
    FieldAnimDef* def;
    FieldAnim* anim;
    FieldScene* scene;
    FieldTweenSpan* span;
    s32 i;
    u8 frame;
    volatile s8 base;

    cmd = seq->def;
    scene = g_field_scene.scene;
    switch (cmd->unk0)
    {
    case 0:
        anim = scene->anims;
        break;
    case 1:
        anim = scene->strips;
        break;
    default:
        anim = scene->sprites;
        break;
    }
    i = cmd->unk2;
    i--;
    while (i != -1)
    {
        anim = anim->next;
        i--;
    }
    def = anim->def;
    if ((*(s32*)&def->flags & 0xFF000007) != 4)
    {
        anim->flags.word &= ~4;
        if (*(s32*)&def->flags & 0x40)
        {
            frame = def->unk1;
            anim->flags.b.keyframe = 0;
            anim->flags.b.state = frame;
        }
        else
        {
            frame = def->unk1;
            anim->flags.b.state = frame;
            anim->flags.b.keyframe = frame;
        }
        span = (FieldTweenSpan*)field_find_count_table_span((u8*)def, anim->flags.b.keyframe, &base);
        if (*(s32*)&def->flags & 0x20)
        {
            anim->timer = span->duration;
        }
        else
        {
            anim->timer = 1;
        }
        if (((*(s32*)&def->flags & 0xFF000007) == 3) ||
            ((def->handler_group == 1) && ((def->flags & 7) >= 2)))
        {
            anim->flags.word |= 0x20;
        }
        if ((u32)((*(s32*)&def->flags & 0xFF000007) - 5) < 2)
        {
            field_apply_animation_tween(def, anim, 0);
        }
    }
    anim->repeat_count = cmd->unk3;
    if (cmd->flags == 0xFF)
    {
        anim->flags.word &= ~2;
    }
    else
    {
        anim->flags.b.stop_keyframe = cmd->flags;
        anim->flags.word |= 2;
    }
    anim->flags.word = (anim->flags.word & ~1) | ((*(u32*)&def->flags >> 3) & 1) | 0x40;
}

/**
 * @brief Apply a control operation to one animation node, or restart a whole
 *        sequence list.
 *
 * @p list_kind picks the scene list to index into: 0 the animation list, 1 the
 * strip list, 3 the sequence list, anything else the sprite list. For the three
 * animation lists the node at @p index is located and @p op applied to it:
 *
 * - op 0 clears the stop request and marks the node running (bit 0x40). When
 *   the caller asked for the animation list and the node handler kind is 4, the
 *   node is only started if it was not already running, and its frame index is
 *   reset.
 * - op 1 stops the node. With @p keyframe of -1 the run bit is cleared, and a
 *   handler-kind-7 node on the animation list first has its sound silenced
 *   through akao_cmd_21. Any other @p keyframe instead records a stop keyframe.
 * - op 2 re-seeds the node the way field_start_animation does - frame and
 *   keyframe from the definition, timer from the keyframe span, an initial
 *   tween pass - and then falls through into op 0 to start it.
 * - op 4 takes the stop keyframe from the definition rather than the caller.
 * - op 3 and anything else seek the node to @p keyframe, setting bit 4 to
 *   record whether the seek runs backwards.
 *
 * @p list_kind 3 is the odd one out: it does not index an animation at all.
 * With @p op of 1 it walks the whole sequence list, clears the active bits of
 * every sequence whose byte at flags+1 matches @p index, and recursively
 * re-applies itself to each sequence definition; with any other @p op it hands
 * the sequence at @p index to func_8005A744.
 *
 * @param list_kind Which scene list to work on (0 anims, 1 strips, 3 seqs,
 *                  otherwise sprites).
 * @param index     Index of the node within that list.
 * @param keyframe  Stop/seek keyframe, or -1 to mean "no keyframe" for op 1.
 * @param op        Operation selector; see above.
 *
 * @see decomp.me (100%)
 */
void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op)
{
    FieldAnim* anim;
    FieldAnimDef* def;
    FieldAnimDef* cmd;
    FieldAnimDef* sfx_def;
    FieldScene* scene;
    FieldSeq* seq;
    FieldSfxKey* key;
    FieldTweenSpan* span;
    s32 chan_mask;
    s32 sfx_id;
    s32 i;
    s32 flags;
    const s32 repeat_mask = ~1;
    s32 start_sequence;
    u8 frame;
    volatile s8 base;

    scene = g_field_scene.scene;
    switch (list_kind)
    {
    case 0:
        anim = scene->anims;
        break;
    case 1:
        anim = scene->strips;
        break;
        /*hack*/
        do
        {
        } while (0);
    case 3:
        seq = g_field_scene.scene->seqs;
        start_sequence = op != 1;
        if (start_sequence)
        {
            i = index - 1;
            if (index != 0)
            {
                do
                {
                    seq = seq->next;
                    i--;
                } while (i != -1);
            }
            func_8005A744(seq, index & 0xFF);
            return;
        }
        while (seq != NULL)
        {
            if ((seq->flags & 3) != 0)
            {
                if (((u8*)&seq->flags)[1] == index)
                {
                    seq->flags &= ~3;
                }
                cmd = seq->def;
                field_control_animation(cmd->unk0, cmd->unk2, -1, 1);
            }
            seq = seq->next;
        }
        return;
    default:
        anim = scene->sprites;
        break;
    }
    index--;
    while (index != -1)
    {
        anim = anim->next;
        index--;
    }
    switch (op)
    {
    case 2:
        def = anim->def;
        if ((*(s32*)&def->flags & 0xFF000007) != 4)
        {
            anim->flags.word &= ~4;
            if (*(s32*)&def->flags & 0x40)
            {
                frame = def->unk1;
                anim->flags.b.keyframe = 0;
                anim->flags.b.state = frame;
            }
            else
            {
                frame = def->unk1;
                anim->flags.b.state = frame;
                anim->flags.b.keyframe = frame;
            }
            span = (FieldTweenSpan*)field_find_count_table_span((u8*)def, anim->flags.b.keyframe, &base);
            if (*(s32*)&def->flags & 0x20)
            {
                anim->timer = span->duration;
            }
            else
            {
                anim->timer = 1;
            }
            if (((*(s32*)&def->flags & 0xFF000007) == 3) ||
                ((def->handler_group == 1) && ((def->flags & 7) >= 2)))
            {
                anim->flags.word |= 0x20;
            }
            if ((u32)((*(s32*)&def->flags & 0xFF000007) - 5) < 2)
            {
                field_apply_animation_tween(def, anim, 0);
            }
        }
        /* fallthrough */
    case 0:
        def = anim->def;
        anim->flags.word &= ~2;
        anim->repeat_count = 0;
        flags = anim->flags.word & repeat_mask;
        flags |= (*(u32*)&def->flags >> 3) & 1;
        anim->flags.word = flags;
        if ((list_kind == 0) && ((*(s32*)&def->flags & 7) == 4))
        {
            if ((anim->flags.word & 0x40) == 0)
            {
                anim->flags.word |= 0x40;
                anim->flags.b.state = 0;
            }
        }
        else
        {
            anim->flags.word |= 0x40;
        }
        break;
    case 1:
        if (keyframe == -1)
        {
            if ((list_kind == 0) && (anim->flags.word & 0x40))
            {
                sfx_def = anim->def;
                if ((*(s32*)&sfx_def->flags & 7) == 7)
                {
                    key = (FieldSfxKey*)sfx_def->data;
                    if (key->sound.word & 0x8000)
                    {
                        if (key->control.word & 0x1F00)
                        {
                            chan_mask = 1 << (((key->control.word >> 8) & 0x1F) - 1);
                            sfx_id = 0;
                        }
                        else
                        {
                            chan_mask = 0;
                            sfx_id = key->sfx_id & 0x3FF;
                        }
                        akao_cmd_21(sfx_id, chan_mask);
                    }
                }
            }
            anim->flags.word &= ~0x40;
        }
        else
        {
            anim->flags.word |= 2;
            anim->flags.b.stop_keyframe = keyframe;
        }
        anim->repeat_count = 0;
        break;
    case 4:
        def = anim->def;
        if (*(s32*)&def->flags & 0x10)
        {
            if (*(s32*)&def->flags & 8)
            {
                anim->flags.b.stop_keyframe = 0;
            }
            else
            {
                anim->flags.b.stop_keyframe = def->unk5;
            }
            anim->repeat_count = 0;
            anim->flags.word |= 2;
        }
        break;
    case 3:
    default:
        if (anim->flags.b.keyframe != keyframe)
        {
            anim->flags.word |= 1;
            if (anim->flags.b.keyframe < keyframe)
            {
                anim->flags.word &= ~4;
            }
            else
            {
                anim->flags.word |= 4;
            }
            anim->repeat_count = 0;
            anim->timer = 1;
            anim->flags.word |= 0x42;
            anim->flags.b.stop_keyframe = keyframe;
        }
        break;
    }
}

extern s32 D_801ED02C;

/*
 * func_8005A0D0 is deliberately left undeclared here. It is defined at the end
 * of this file taking a s16 and three u16, and a prototype in scope would make
 * the call sites below narrow their arguments, which costs 2 rows in
 * field_update_scene_fade. The implicit declaration passes them as ints, which
 * is what the callee expects.
 */

/**
 * @brief Advance the scene-transition fade by one frame.
 *
 * Does nothing unless FieldMemState::fade_mode is 1 (fading out) or 3 (fading
 * in). Either way the level moves 8 towards its endpoint and is pushed to the
 * global colour scale through func_8005A0D0, which takes the level for all
 * three channels.
 *
 * Fading out finishes at level 0, and that is where the scene is torn down:
 * the first object is marked done and its first two parts hidden, then every
 * remaining object, all four animation lists and the sequence list have their
 * active bits SAVED one position up and then cleared - bit 0 to bit 1 for
 * objects, bit 6 to bit 7 for animations, bits 0-1 to bits 2-3 for sequences -
 * so the state can be restored when the next scene fades in. All animations are
 * then stopped through field_control_animation, the fade moves to mode 2, and
 * the colour scale is restored to full.
 *
 * Fading in finishes at level 0x100 and simply clears the mode.
 *
 * @note @c fade_mode is written two different ways on purpose and neither is
 *       interchangeable: the mode-2 store at the end of the fade-out uses the
 *       standalone symbol @c D_801ED02C (costs 2 rows written through
 *       @c state), while the mode-0 store at the end of the fade-in goes
 *       through @c state (costs 1 row written as @c D_801ED02C). Same address,
 *       different addressing mode - the same split FieldCamera has.
 * @note @c state and @c scene are both locals, and @c scene has to be read at
 *       the very top, before the switch: reading it where it is first used
 *       instead costs 36 rows.
 * @note The object loop reads the flag bit as a BYTE
 *       (@c obj->flags.b.unk0 @c & @c 1) while the animation loops shift the
 *       whole WORD (@c anim->flags.word @c << @c 1). Swapping either spelling
 *       for the other costs 7 rows.
 * @note Measured non-factors, both still 100%: spelling the level read as a
 *       @c u16 union member instead of @c (u16) on the word, and using an early
 *       @c return for the already-done object instead of the nested @c if.
 *
 * @see decomp.me (100%) TODO
 */
void field_update_scene_fade(void)
{
    FieldMemState* state;
    FieldScene* scene;
    FieldObj* obj;
    FieldPart* part;
    FieldAnim* anim;
    FieldSeq* seq;
    s32 level;

    state = (FieldMemState*)0x801ED000;
    scene = g_field_scene.scene;
    switch (state->fade_mode)
    {
    case 1:
        state->fade_level -= 8;
        level = (u16)state->fade_level;
        func_8005A0D0(-1, level, level, level);
        if (state->fade_level == 0)
        {
            obj = scene->objects;
            if ((obj->flags.word & 1) == 0)
            {
                obj->flags.word |= 1;
                part = obj->parts;
                part->visible = 0;
                part = part->next;
                part->visible = 0;
                obj = obj->next;
                while (obj != NULL)
                {
                    obj->flags.word = ((obj->flags.word & ~2) | ((obj->flags.b.unk0 & 1) << 1)) & ~1;
                    obj = obj->next;
                }
                anim = scene->anims;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~0x80) | ((anim->flags.word << 1) & 0x80)) & ~0x40;
                    anim = anim->next;
                }
                anim = scene->strips;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~0x80) | ((anim->flags.word << 1) & 0x80)) & ~0x40;
                    anim = anim->next;
                }
                anim = scene->sprites;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~0x80) | ((anim->flags.word << 1) & 0x80)) & ~0x40;
                    anim = anim->next;
                }
                anim = scene->effects;
                while (anim != NULL)
                {
                    anim->flags.word = ((anim->flags.word & ~0x80) | ((anim->flags.word << 1) & 0x80)) & ~0x40;
                    anim = anim->next;
                }
                seq = scene->seqs;
                while (seq != NULL)
                {
                    seq->flags = ((seq->flags & ~0xC) | ((((u8*)&seq->flags)[0] & 3) << 2)) & ~3;
                    seq = seq->next;
                }
                field_control_animation(0, 0, 0, 0);
                D_801ED02C = 2;
                func_8005A0D0(0, 0x100, 0x100, 0x100);
            }
        }
        break;
    case 3:
        state->fade_level += 8;
        level = (u16)state->fade_level;
        func_8005A0D0(-1, level, level, level);
        if (state->fade_level == 0x100)
        {
            state->fade_mode = 0;
        }
        break;
    }
}

/**
 * @brief Reactivate the scene and start the fade back in.
 *
 * The exact counterpart to the teardown half of field_update_scene_fade: that
 * one SAVED every list's active bits one position up and cleared them, this one
 * shifts them back down. Objects restore bit 1 into bit 0, animations bit 7 into
 * bit 6, and sequences bits 2-3 into bits 0-1.
 *
 * The first object is handled separately, as it is there: its done bit is
 * cleared outright and its first two parts are hidden. The fade mode then goes
 * to 3, which is what makes field_update_scene_fade step the level back up to
 * 0x100 on the following frames.
 *
 * @note @c list exists to make @c scene->objects address-taken. Without it gcc
 *       can prove the @c D_801ED02C store does not alias the load and hoists
 *       the load above it, which the target does not do (2 rows). The inline
 *       spelling @c *(&scene->objects) does NOT work - gcc folds the @c *&
 *       pair before aliasing is computed, so the pointer has to be a real named
 *       local. See idiom [SCHED-10].
 * @note All three restore shifts need the @c (u32) cast, otherwise the shift
 *       comes out as @c sra rather than @c srl (1 row each). Unlike the
 *       teardown, the object loop here reads the WHOLE WORD - taking the bit
 *       from the byte view instead costs 3 rows.
 * @note @c part must be re-assigned as its own statement rather than chained as
 *       @c part->next->visible - the chained form costs 8 rows.
 * @note Measured non-factor, still 100%: writing @c D_801ED02C before rather
 *       than after the @c scene read.
 *
 * @see decomp.me (100%) TODO
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
    D_801ED02C = 3;
    list = &scene->objects;
    obj = *list;
    obj->flags.word &= ~1;
    part = obj->parts;
    part->visible = 0;
    part = part->next;
    part->visible = 0;
    obj = obj->next;
    while (obj != NULL)
    {
        obj->flags.word = (obj->flags.word & ~1) | (((u32)obj->flags.word >> 1) & 1);
        obj = obj->next;
    }
    anim = scene->anims;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~0x40) | (((u32)anim->flags.word >> 1) & 0x40);
        anim = anim->next;
    }
    anim = scene->strips;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~0x40) | (((u32)anim->flags.word >> 1) & 0x40);
        anim = anim->next;
    }
    anim = scene->sprites;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~0x40) | (((u32)anim->flags.word >> 1) & 0x40);
        anim = anim->next;
    }
    anim = scene->effects;
    while (anim != NULL)
    {
        anim->flags.word = (anim->flags.word & ~0x40) | (((u32)anim->flags.word >> 1) & 0x40);
        anim = anim->next;
    }
    seq = scene->seqs;
    while (seq != NULL)
    {
        seq->flags = (seq->flags & ~3) | (((u32)seq->flags >> 2) & 3);
        seq = seq->next;
    }
}

void func_8005A428(FieldPart*);
FieldAnimCel* func_8005ABD8(FieldTileGrid*, FieldTintSrc**);
void func_8005ADA8(FieldAnimCel*, FieldAnim*);

/**
 * @brief Push a new colour scale onto the scene's tint sources and rebuild the
 *        affected tile records.
 *
 * The scene's object list is walked as a list of FieldTintSrc records. An
 * object is retinted only when @p index is -1 (meaning "every object") or when
 * it matches the object's position in the list, and only when the scale it
 * already carries differs from the one being pushed. Retinting multiplies the
 * record's own colour triple by the new scale into a three-word colour, stores
 * the new scale, and expands the colour into the scratchpad table through
 * func_8005AC50; every part of the object that holds instances is then rebuilt
 * by func_8005A428, except for parts that carry neither a shared rgb/code word
 * nor an empty @c unk8.
 *
 * When @p index is -1 the scene's animation and effect lists are rescaled as
 * well. Each node's runtime record is resolved with func_8005ABD8, which also
 * hands back the tint source behind it; that source is rescaled the same way
 * and, unless the resolved record carries a shared rgb/code word, handed to
 * func_8005ADA8 together with its node. Animations are skipped unless their
 * definition selects handler kind 0 or 1; effects are always rescaled.
 *
 * @param index Object index to retint, or -1 for every object plus the
 *              animation and effect lists.
 * @param red_scale   Red scale, 0x100 is unattenuated.
 * @param green_scale Green scale.
 * @param blue_scale  Blue scale.
 *
 * @note @p index must be a @c s16 and the three scales @c u16: widening them to
 *       @c s32 costs 52 and 84 rows respectively. The scales arrive
 *       sign-extended, so every use masks them, while the three writebacks
 *       store the raw parameter.
 * @note @c tint must be a separate local assigned BEFORE the three @c rgb
 *       products. Assigning it after them, or reading @c owner->palette->data
 *       in one go where @c pal is set, leaves the palette load stuck below the
 *       @c rgb stores and costs 8 rows.
 * @note The handler-kind test reads the whole word at FieldAnimDef::flags as
 *       @c u32; as @c s32 the range check compares signed and costs a row.
 * @note Measured non-factors, all still 100%: @c i as @c u16 or as @c s32 with
 *       a @c (u16) cast on the compare, nesting the index and colour tests
 *       instead of joining them with @c &&, @c &pal[2] instead of @c pal @c +
 *       @c 2, and prototyping the four callees instead of leaving them
 *       implicit.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005A0D0(s16 index, u16 red_scale, u16 green_scale, u16 blue_scale)
{
    FieldScene* scene;
    FieldTintSrc* owner;
    FieldTintPal* tint;
    FieldPart* part;
    FieldAnim* anim;
    FieldAnimCel* cel;
    u16* pal;
    u16 i;
    s32 rgb[3];

    i = 0;
    scene = g_field_scene.scene;
    owner = (FieldTintSrc*)scene->objects;
    while (owner != NULL)
    {
        if ((index == -1 || index == i) &&
            (owner->red_scale != red_scale || owner->green_scale != green_scale ||
             owner->blue_scale != blue_scale))
        {
            tint = owner->palette;
            rgb[0] = owner->red * red_scale;
            rgb[1] = owner->green * green_scale;
            rgb[2] = owner->blue * blue_scale;
            owner->red_scale = red_scale;
            owner->green_scale = green_scale;
            owner->blue_scale = blue_scale;
            pal = tint->data;
            func_8005AC50(pal + 2, pal[0], rgb);
            part = (FieldPart*)owner->cels;
            while (part != NULL)
            {
                if (part->instance_count != 0 && (part->code_word != 0 || part->unk8 == 0))
                {
                    func_8005A428(part);
                }
                part = part->next;
            }
        }
        owner = owner->next;
        i++;
    }
    if (index == -1)
    {
        anim = scene->anims;
        while (anim != NULL)
        {
            if ((*(u32*)&anim->def->flags & 7) < 2)
            {
                cel = func_8005ABD8(((FieldTileAnimDef*)anim->def)->grid, &owner);
                tint = owner->palette;
                rgb[0] = owner->red * red_scale;
                rgb[1] = owner->green * green_scale;
                rgb[2] = owner->blue * blue_scale;
                pal = tint->data;
                func_8005AC50(pal + 2, pal[0], rgb);
                if (cel->code_word == 0)
                {
                    func_8005ADA8(cel, anim);
                }
            }
            anim = anim->next;
        }
        anim = scene->effects;
        while (anim != NULL)
        {
            cel = func_8005ABD8(((FieldTileAnimDef*)anim->def)->grid, &owner);
            tint = owner->palette;
            rgb[0] = owner->red * red_scale;
            rgb[1] = owner->green * green_scale;
            rgb[2] = owner->blue * blue_scale;
            pal = tint->data;
            func_8005AC50(pal + 2, pal[0], rgb);
            if (cel->code_word == 0)
            {
                func_8005ADA8(cel, anim);
            }
            anim = anim->next;
        }
    }
}

/**
 * @brief Re-tint one part's cell records from the scratchpad colour table.
 *
 * Walks @p part 's bit plane, consuming one packed tile descriptor per grid
 * cell and one record per SET bit. Each resolved cell copies the rgb/code entry
 * its descriptor selects out of the scratchpad table at 0x1F800000 into the
 * record's colour halves. Absent cells still advance the descriptor cursor but
 * not the record cursor, and the record stride shrinks by 4 for each of the two
 * words the part shares (rgb/code and texture page).
 *
 * When the part carries a SHARED rgb/code word the colour belongs to the whole
 * part rather than to individual records, so the first present cell whose
 * descriptor resolves writes it into @c code_word and the byte after it, then
 * the function returns immediately.
 *
 * Only part kinds 0 and 2 through 5 are tinted; kind 1 and anything from 6 up
 * return untouched.
 *
 * @param part Runtime part to re-tint.
 *
 * @note @c part->def is addressed as a FieldTileGrid: its identity key at 0x00
 *       doubles as the tile-descriptor array, which is exactly why two parts
 *       sharing that word are interchangeable.
 * @note The two case arms are the SAME block written out twice, which is what
 *       the original did: giving @c case @c 0 and @c case @c 2..5 one shared
 *       body compiles to a single loop, 63 insns short of the target (57.31%).
 *       gcc cross-jumps only the shared-word tail the two copies end in.
 * @note The counter must be the multiply result decremented IN PLACE
 *       (@c count @c = @c rows @c * @c cols, then @c while @c (--count @c !=
 *       @c -1)). Spelled as @c rows @c * @c cols @c - @c 1 with the decrement
 *       at the loop bottom, combine folds the entry guard's @c (n-1) @c ==
 *       @c -1 into @c n @c == @c 0 and the preheader loses 2 insns (89.50%).
 * @note @c dst must be read at the TOP of the arm, before the stride
 *       computation; after it, the load will not schedule up next to the
 *       @c code_word read and it costs 6 rows.
 * @note @c word must be cleared once before the switch. Clearing it at the head
 *       of each arm instead costs 22 rows.
 * @note The empty @c case @c 1 is required: without it gcc builds a different
 *       decision tree and the dispatch costs 13 rows.
 * @note Measured non-factors, all still 100%: switching on a @c s32 @c kind
 *       local instead of the @c u8 field directly, @c stride @c = @c 8 instead
 *       of @c stride @c -= @c 4 for the shared rgb/code word, nesting the
 *       present test instead of joining it with @c &&, and @c bit as @c s32.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005A428(FieldPart* part)
{
    FieldTileGrid* grid;
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
    grid = (FieldTileGrid*)part->def;
    tile = grid->tiles;
    pal = (FieldTintColor*)0x1F800000;
    switch (part->kind)
    {
    case 0:
        dst = part->records;
        stride = 12;
        if (part->code_word != 0)
        {
            stride -= 4;
        }
        if (part->tpage_word != 0)
        {
            stride -= 4;
        }
        mask = (u32*)part->bits;
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
                if ((word & bit) && (tile->clut_slot & 0x80))
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
        stride = 12;
        if (part->code_word != 0)
        {
            stride -= 4;
        }
        if (part->tpage_word != 0)
        {
            stride -= 4;
        }
        mask = (u32*)part->bits;
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
                if ((word & bit) && (tile->clut_slot & 0x80))
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
 * @brief Restart one sequence by index, or clear every armed sequence's state.
 *
 * With @p op zero the scene's sequence list is walked to the entry at
 * @p index and that one entry is handed to func_8005A744, which restarts it.
 * Otherwise every sequence whose low two state bits are set has those bits
 * cleared (but only when its own stored index matches @p index) and is then
 * pushed through field_control_animation as a stop.
 *
 * @param index Sequence index; also the value each entry's stored index at
 *              byte 1 of FieldSeq::flags is matched against.
 * @param op Zero selects the single-sequence restart, anything else the
 *           clear-all walk.
 *
 * @note This is the same body field_control_animation runs inline for
 *       @c list_kind @c 3; the two are kept in sync deliberately.
 * @note The list head must be read ONCE before the @p op test. Reading it
 *       separately in each arm costs 8 rows and 6 insns.
 * @note The walk to @p index must count DOWN. Written as
 *       @c for @c (i @c = @c 0; @c i @c < @c index; @c i++) it costs 5 rows.
 * @note Measured non-factors, all still 100%: hoisting @c i @c = @c index @c -
 *       @c 1 above the @c index @c != @c 0 guard or leaving it inside,
 *       spelling the countdown as @c while @c (--i @c != @c -1), @c (u8)index
 *       instead of @c index @c & @c 0xFF, an @c if/goto instead of the early
 *       @c return, a union byte member instead of the @c ((u8*)&flags)[1]
 *       cast, dropping the @c cmd temp, and a guarded @c do/while for the
 *       outer walk.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005A67C(s32 index, s32 op)
{
    FieldAnimDef* cmd;
    FieldSeq* seq;
    s32 i;

    seq = g_field_scene.scene->seqs;
    if (op == 0)
    {
        i = index - 1;
        if (index != 0)
        {
            do
            {
                seq = seq->next;
                i--;
            } while (i != -1);
        }
        func_8005A744(seq, index & 0xFF);
        return;
    }
    while (seq != NULL)
    {
        if ((seq->flags & 3) != 0)
        {
            if (((u8*)&seq->flags)[1] == index)
            {
                seq->flags &= ~3;
            }
            cmd = seq->def;
            field_control_animation(cmd->unk0, cmd->unk2, -1, 1);
        }
        seq = seq->next;
    }
}

/**
 * @brief Arm a sequence and start its animation, then chain to the sequence
 *        its definition points at.
 *
 * Sets the sequence's countdown to 1, replaces its low two state bits with 1,
 * records @p index in byte 1 of FieldSeq::flags, and hands the node to
 * field_start_animation. If the definition names a follow-on sequence
 * (FieldAnimDef::unk5 is not 0xFF) and carries no delay (FieldAnimDef::unk8 is
 * zero), that sequence is located by walking the scene list to its index and
 * armed the same way, recursively.
 *
 * @param seq Sequence node to arm.
 * @param index Sequence index, stored into byte 1 of FieldSeq::flags and
 *              carried through the whole chain unchanged.
 *
 * @note @p index must be a @c u8. As a @c s32 with an explicit
 *       @c index @c & @c 0xFF on the recursive call the instructions are all
 *       right but @p index and @c scene swap saved registers (89.48%): the
 *       @c u8 spelling puts the mask in a zero-extend of its own, which raises
 *       the parameter's allocation priority above the scene pointer's. An
 *       explicit @c (u8) cast on the argument is NOT equivalent.
 * @note The follow-on index must count DOWN in place -
 *       @c i @c = @c def->unk5 then @c while @c (--i @c != @c -1). Reading
 *       @c unk5 twice and initialising @c i @c = @c def->unk5 @c - @c 1 leaves
 *       the pre-decrement value live, so combine folds the entry guard into
 *       @c unk5 @c != @c 0 and 3 rows go (see [EXPAND-22] in idioms.md).
 * @note @c scene must be read at the top, before the field_start_animation
 *       call, even though it is not used until after it. Reading it later
 *       costs 9 rows, and it cannot be sunk into the @c if because the call
 *       sits in between.
 * @note Measured non-factors, all still 100%: nesting the two guard tests
 *       instead of joining them with @c &&, ordering @c def @c = @c seq->def
 *       before the @c scene read, and moving the byte store above it.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005A744(FieldSeq* seq, u8 index)
{
    FieldAnimDef* def;
    FieldScene* scene;
    FieldSeq* walk;
    s32 i;

    scene = g_field_scene.scene;
    seq->unkC = 1;
    seq->flags = (seq->flags & ~3) | 1;
    def = seq->def;
    ((u8*)&seq->flags)[1] = index;
    field_start_animation(seq);
    i = def->unk5;
    if (i != 0xFF && def->unk8 == 0)
    {
        walk = scene->seqs;
        while (--i != -1)
        {
            walk = walk->next;
        }
        func_8005A744(walk, index);
    }
}

/**
 * @brief Report whether no armed sequence currently carries a given index.
 *
 * Walks the scene's sequence list looking for an entry whose low two state bits
 * are set and whose stored index at byte 1 of FieldSeq::flags equals @p index.
 * That is the same pair of tests func_8005A67C uses to decide which sequences
 * to stop, so this is the query form of it.
 *
 * @param index Sequence index to look for.
 * @return 0 as soon as a match is found, 1 when the whole list is walked
 *         without one.
 *
 * @note @p index must be a @c s32. As a @c u8 the compare needs its own mask
 *       and it costs a row.
 * @note The early @c return @c 0 is required. Setting a found flag, breaking
 *       out and returning at the bottom costs 11 rows.
 * @note Measured non-factors, all still 100%: joining the two tests with
 *       @c && instead of nesting them, a guarded @c do/while for the walk,
 *       and a union byte member instead of the @c ((u8*)&flags)[1] cast.
 *
 * @see decomp.me (100%) TODO
 */
s32 func_8005A7EC(s32 index)
{
    FieldSeq* seq;

    seq = g_field_scene.scene->seqs;
    while (seq != NULL)
    {
        if ((seq->flags & 3) != 0)
        {
            if (((u8*)&seq->flags)[1] == index)
            {
                return 0;
            }
        }
        seq = seq->next;
    }
    return 1;
}

/**
 * @brief Report the play state of one animation or sequence.
 *
 * @p list_kind picks the list the same way field_control_animation does - 0
 * anims, 1 strips, 3 sequences, anything else sprites - and @p index selects
 * the entry within it.
 *
 * For the sequence list the answer is just whether an armed sequence carries
 * @p index: 0 when one does, 2 when none does. For the three animation lists
 * the node at @p index is inspected: 2 when it has never been started, 1 when
 * it is held, 3 when it is an anim-list node whose definition selects handler
 * kind 4 and which is still on one of its first two frames, and 0 otherwise.
 *
 * @param list_kind Which list to walk; see above.
 * @param index Entry index within that list.
 * @return 0, 1, 2 or 3 as described above.
 *
 * @note The unreachable @c do/while(0) between the @c case @c 1 and @c case
 *       @c 3 arms is required to match, and is not a placeholder for deleted
 *       code - it is there for the NOTE_INSN_LOOP_END it leaves in front of the
 *       @c case @c 3 label. Without it cse follows the dispatch branch into
 *       that arm with its value table intact and folds the second
 *       @c g_field_scene read into the one above the switch, costing 4 rows.
 *       See [CSE-11] in idioms.md; field_control_animation needs the same
 *       thing for the same reason.
 * @note The tail must be written as @c if @c (flags @c & @c 0x40) @c { @c ...
 *       @c return @c 0; @c } @c return @c 2; - putting the @c return @c 2
 *       first as an early exit emits it inline instead of at the end and costs
 *       8 rows.
 * @note The sequence arm must set @c status @c = @c 1 AFTER its loop and reach
 *       the shared @c return through a @c goto. Seeding @c status @c = @c 1
 *       before the loop and breaking out costs 5 rows, because the constant
 *       then lives in a register across the loop instead of being
 *       rematerialised in the two exit branches' delay slots.
 * @note Measured non-factors, all still 100%: @c status @c << @c 1 instead of
 *       @c * @c 2, @c u32 instead of @c s32 on the definition-flags cast,
 *       dropping the @c def temp, an explicit @c (s32) on the state compare,
 *       and joining the last two tests with @c && instead of nesting them.
 *
 * @see decomp.me (100%) TODO
 */
s32 func_8005A84C(s32 list_kind, s32 index)
{
    FieldAnim* anim;
    FieldAnimDef* def;
    FieldScene* scene;
    FieldSeq* seq;
    s32 status;

    scene = g_field_scene.scene;
    switch (list_kind)
    {
    case 0:
        anim = scene->anims;
        break;
    case 1:
        anim = scene->strips;
        break;
        /*
         * Unreachable, and required to match: the NOTE_INSN_LOOP_END this
         * leaves between the case 1 arm's barrier and the case 3 label is what
         * stops cse_end_of_basic_block from following the dispatch branch into
         * the case 3 block, so g_field_scene is re-read there instead of being
         * folded into the load above the switch. See [CSE-11] in idioms.md.
         */
        do
        {
        } while (0);
    case 3:
        seq = g_field_scene.scene->seqs;
        while (seq != NULL)
        {
            if ((seq->flags & 3) != 0)
            {
                if (((u8*)&seq->flags)[1] == index)
                {
                    status = 0;
                    goto done;
                }
            }
            seq = seq->next;
        }
        status = 1;
    done:
        return status * 2;
    default:
        anim = scene->sprites;
        break;
    }
    index--;
    while (index != -1)
    {
        anim = anim->next;
        index--;
    }
    if (anim->flags.word & 0x40)
    {
        if ((anim->flags.word & 2) != 0)
        {
            return 1;
        }
        if (list_kind == 0)
        {
            def = anim->def;
            if ((*(s32*)&def->flags & 7) == 4)
            {
                if (anim->flags.b.state < 2)
                {
                    return 3;
                }
            }
        }
        return 0;
    }
    return 2;
}

/**
 * @brief Shift every FieldNode attached to a part along one axis.
 *
 * Walks the scene's node list for the ones owned by @p part and adds @p delta
 * to the pair of accumulators @p axis selects: axis 0 moves @c unk24 and
 * @c unk34, axis 1 moves both delta and position pairs, and anything else
 * moves @c unk30 and @c unk40. The walk stops as soon as the part's
 * @c node_count nodes have been found.
 *
 * @param part Part whose attached nodes to move.
 * @param delta Amount to add; zero returns immediately.
 * @param axis Which accumulator pair to move; see above.
 *
 * @note func_8005AA68 is the same routine keyed on the owning OBJECT instead,
 *       and it SUBTRACTS on axis 1 where this one adds. Keep the two in sync.

 * @note The @c case @c 2 label is required even though it shares the
 *       @c default arm and 2 already reached it. stmt.c's
 *       @c balance_case_nodes only bisects the case list when it holds more
 *       than two nodes, so two cases plus a default emit a flat ascending
 *       compare chain while three emit the balanced tree the target has -
 *       equality against the middle value first, then a bound test. Dropping
 *       it costs 6 rows; giving @c case @c 2 its own body instead costs more.
 *       See [JUMP-17] in idioms.md.
 * @note The scene pointer must be read at the top, before both guards, even
 *       though it is not used until after them. Reading it where the node list
 *       is taken instead costs 3 rows.
 * @note @c count must be a @c s32. As a @c u8 the decrement needs a mask and
 *       it costs a row.
 * @note Measured non-factors, all still 100%: joining the two guards with
 *       @c &&, a plain @c while instead of the guarded @c do/while, and
 *       @c if @c (--count @c == @c 0) instead of a separate decrement.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005A984(FieldPart* part, s32 delta, s32 axis)
{
    FieldScene* scene;
    FieldNode* node;
    s32 count;

    count = part->node_count;
    scene = g_field_scene.scene;
    if (count != 0)
    {
        if (delta != 0)
        {
            node = scene->nodes;
            if (node != NULL)
            {
                do
                {
                    if (node->part == part)
                    {
                        switch (axis)
                        {
                        case 0:
                            node->unk24 += delta;
                            node->unk34 += delta;
                            break;
                        case 1:
                            node->delta_x += delta;
                            node->delta_y += delta;
                            node->x += delta;
                            node->y += delta;
                            break;
                        case 2:
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
                } while (node != NULL);
            }
        }
    }
}

/**
 * @brief Shift every FieldNode attached to an object along one axis.
 *
 * The object-level counterpart of func_8005A984: same walk and the same
 * accumulator pairs, but it matches nodes on FieldNode::obj and takes its
 * budget from FieldObjFlags::node_count.
 *
 * @param obj Object whose attached nodes to move.
 * @param delta Amount to add; zero returns immediately.
 * @param axis Which accumulator pair to move.
 *
 * @note Axis 1 SUBTRACTS @p delta here where func_8005A984 adds it. That is
 *       what the target does and it is the only behavioural difference
 *       between the two.

 * @note The @c case @c 2 label is required even though it shares the
 *       @c default arm and 2 already reached it. stmt.c's
 *       @c balance_case_nodes only bisects the case list when it holds more
 *       than two nodes, so two cases plus a default emit a flat ascending
 *       compare chain while three emit the balanced tree the target has -
 *       equality against the middle value first, then a bound test. Dropping
 *       it costs 6 rows; giving @c case @c 2 its own body instead costs more.
 *       See [JUMP-17] in idioms.md.
 * @note The scene pointer must be read at the top, before both guards, even
 *       though it is not used until after them. Reading it where the node list
 *       is taken instead costs 3 rows.
 * @note @c count must be a @c s32. As a @c u8 the decrement needs a mask and
 *       it costs a row.
 * @note Measured non-factors, all still 100%: joining the two guards with
 *       @c &&, a plain @c while instead of the guarded @c do/while, and
 *       @c if @c (--count @c == @c 0) instead of a separate decrement.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005AA68(FieldObj* obj, s32 delta, s32 axis)
{
    FieldScene* scene;
    FieldNode* node;
    s32 count;

    count = obj->flags.b.node_count;
    scene = g_field_scene.scene;
    if (count != 0)
    {
        if (delta != 0)
        {
            node = scene->nodes;
            if (node != NULL)
            {
                do
                {
                    if (node->obj == obj)
                    {
                        switch (axis)
                        {
                        case 0:
                            node->unk24 += delta;
                            node->unk34 += delta;
                            break;
                        case 1:
                            node->delta_x -= delta;
                            node->delta_y -= delta;
                            node->x -= delta;
                            node->y -= delta;
                            break;
                        case 2:
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
                } while (node != NULL);
            }
        }
    }
}

/**
 * @brief Walk the scene's object list @p index steps from the head.
 *
 * The counter is pre-decremented and compared against -1 rather than counting
 * down to 0; writing it as a plain @c index-step loop changes the compare and
 * costs the tail rows.
 *
 * @param index Number of @c next hops to take. 0 returns the list head.
 * @return The object @p index steps into the list.
 * @see decomp.me (100%) TODO
 */
FieldObj* func_8005AB4C(s32 index)
{
    FieldObj* obj;
    s32 remaining;

    obj = g_field_scene.scene->objects;
    remaining = index - 1;
    if (index != 0)
    {
        do
        {
            obj = obj->next;
            remaining -= 1;
        } while (remaining != -1);
    }
    return obj;
}

/**
 * @brief Resolve a (object, part) index pair to a part in the current scene.
 *
 * Walks the scene's object list @p obj_index steps, then walks that object's
 * part list @p part_index steps. Neither walk is bounds-checked; both indices
 * are assumed to be in range for the scene.
 *
 * @param obj_index Number of @c next hops along the object list. 0 selects the
 *                  list head.
 * @param part_index Number of @c next hops along the chosen object's part list.
 *                   0 selects that object's first part.
 * @return The selected part.
 * @see decomp.me (100%) TODO
 */
FieldPart* func_8005AB80(s32 obj_index, s32 part_index)
{
    FieldObj* obj;
    FieldPart* part;
    s32 remaining;

    obj = g_field_scene.scene->objects;
    remaining = obj_index - 1;
    if (obj_index != 0)
    {
        do
        {
            obj = obj->next;
            remaining -= 1;
        } while (remaining != -1);
    }
    part = obj->parts;
    part_index -= 1;
    if (part_index != -1)
    {
        do
        {
            part = part->next;
            part_index -= 1;
        } while (part_index != -1);
    }
    return part;
}

/**
 * @brief Find the cel laid out on a given grid anywhere in the current scene.
 *
 * Walks the scene's object list as a list of FieldTintSrc records - the same
 * view func_8005A0D0 takes of it - and within each record its cel list, for the
 * first cel whose @c grid is @p grid. The tint source behind the match is
 * reported through @p out_src when the caller wants it.
 *
 * @param grid Grid to search for; compared by pointer identity.
 * @param out_src Optional out-parameter receiving the tint source owning the
 *                match. Pass NULL when only the cel is needed.
 * @return The matching cel, or NULL when no record in the scene holds one.
 * @see decomp.me (100%) TODO
 */
FieldAnimCel* func_8005ABD8(FieldTileGrid* grid, FieldTintSrc** out_src)
{
    FieldTintSrc* src;
    FieldAnimCel* cel;

    src = (FieldTintSrc*)g_field_scene.scene->objects;
    while (src != NULL)
    {
        cel = src->cels;
        while (cel != NULL)
        {
            if (grid == cel->grid)
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
 * @brief Expand a palette into the scratchpad colour table, scaling each
 *        component and clamping it to 8 bits.
 *
 * Each 4-byte source entry contributes three components, which are multiplied
 * by the matching entry of @p rgb_scale and taken from the high half of the
 * 24.8-ish product (@c >>16). Any product above 0xFEFFFF - the largest value
 * whose high byte is still 0xFE - saturates to 0xFF instead of wrapping. The
 * fourth byte of each entry (the primitive code) is left untouched, so the
 * table can be copied into a tile record whole.
 *
 * @param colors Source palette, 4 bytes per entry.
 * @param count Number of entries to expand.
 * @param rgb_scale Three scale factors, one per component; 0x100 is
 *                  unattenuated.
 * @note @p colors and @p src walk the same array. Both are needed: gcc gives
 *       the parameter to the cursor read at +1/+2 and a copy to the one read at
 *       +0, which is the entry @c addu @c t1, @c a0, @c zero. Folding them into
 *       one cursor costs 12 rows.
 * @note @c count is an @c s32 even though the two scene-builder views declare
 *       this function with a @c u16 second parameter. As a @c u16 the entry
 *       needs an @c andi mask and the function grows two instructions.
 * @note @c v must be unsigned: the compare is @c sltu and the shift @c srl,
 *       and a signed @c v turns both into their signed forms (6 rows).
 * @see decomp.me (100%) TODO
 */
void func_8005AC50(u8* colors, s32 count, s32* rgb_scale)
{
    u8* src;
    u8* dst;
    s32 red;
    s32 green;
    s32 blue;
    s32 remaining;
    u32 v;

    src = colors;
    dst = (u8*)0x1F800000;
    red = rgb_scale[0];
    green = rgb_scale[1];
    blue = rgb_scale[2];
    remaining = count;
    remaining -= 1;
    if (count != 0)
    {
        do
        {
            v = src[0] * red;
            if (v > 0xFEFFFF)
            {
                dst[0] = 0xFF;
            }
            else
            {
                dst[0] = v >> 16;
            }
            v = colors[1] * green;
            if (v > 0xFEFFFF)
            {
                dst[1] = 0xFF;
            }
            else
            {
                dst[1] = v >> 16;
            }
            v = colors[2] * blue;
            if (v > 0xFEFFFF)
            {
                dst[2] = 0xFF;
            }
            else
            {
                dst[2] = v >> 16;
            }
            colors += 4;
            src += 4;
            dst += 4;
            remaining -= 1;
        } while (remaining != -1);
    }
}

/**
 * @brief Stamp the GPU primitive code for a texture format across the whole
 *        scratchpad colour table.
 *
 * Maps @p format to a primitive code and writes it into the @c code byte of
 * every entry of the table at 0x1F800000 (offset 3 of each FieldTintColor, so
 * the walk strides 4). @p primitive_code caches the code the table currently
 * carries; when it already matches, the whole pass is skipped.
 *
 * The codes are the standard GPU primitive tags: 0x7C SPRT_16, 0x64 SPRT,
 * 0x2C POLY_FT4 and 0x3C POLY_GT4 for anything else.
 *
 * @param format Texture format selector taken from FieldAnimCel.
 * @param count Number of table entries to stamp.
 * @param primitive_code In/out cache of the code already in the table; updated
 *                       once the table has been rewritten.
 * @note @c format is a @c u8 (it needs the entry @c andi) but @c count is an
 *       @c s32, even though the two scene-builder views declare the second parameter
 *       @c u16; as a @c u16 the in-place decrement needs masking and the
 *       function loses five instructions (16 rows).
 * @note @p primitive_code is @c u8*, not the @c s8* those two files declare -
 *       the target reads it with @c lbu (1 row).
 * @note Cases 2-5 must share ONE arm. Giving each its own arm with a duplicate
 *       body takes gcc's case list from three nodes to six, which rebuilds the
 *       whole comparison tree (15 rows); see idiom [EXPAND-13]. An equivalent
 *       if/else-if chain costs 21 rows.
 * @see decomp.me (100%) TODO
 */
void func_8005AD20(u8 format, s32 count, u8* primitive_code)
{
    s32 code;
    u8* p;

    switch (format)
    {
    case 0:
        code = 0x7C;
        break;
    case 1:
        code = 0x64;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        code = 0x2C;
        break;
    default:
        code = 0x3C;
        break;
    }
    if (code != *primitive_code)
    {
        p = (u8*)0x1F800003;
        while (--count != -1)
        {
            *p = code;
            p += 4;
        }
        *primitive_code = code;
    }
}

/**
 * @brief Re-tint an animation's built per-frame tile records from the
 *        scratchpad colour table.
 *
 * Every record in @p anim 's frame data carries a palette index in its byte 3.
 * That index selects an entry of the table at 0x1F800000, whose colour halves
 * are written back over the record's own rgb/code word, so a tint pushed into
 * the table by func_8005AC50 reaches primitives that were already emitted.
 *
 * The record stride follows the cel's format the same way field_tint_animation_cel
 * derives it: twelve bytes, less four when the cel carries a shared rgb/code
 * word and four more when it carries a shared texture-page word. Formats 1 and
 * 6-and-up are not record formats and are skipped.
 *
 * @param cel Cel whose format and shared-word flags set the record stride.
 * @param anim Animation holding the frame data to rewrite.
 * @note The two arms are deliberately identical. The original emits the body
 *       twice, once for format 0 and once for formats 2-5; collapsing them onto
 *       a shared arm emits it once and does not match.
 * @note @c case @c 1 must be present and empty, as in field_tint_animation_cel -
 *       it is what shapes gcc's comparison tree. See idiom [EXPAND-13].
 * @note @c pal has to be materialised above the switch, not inside each arm.
 * @note @c dst must be read before the two stride tests so it lands in the
 *       first block, and the FieldCellTint cursor must be initialised from
 *       @c dst itself rather than @c dst @c + @c 4 - the same pairing
 *       field_tint_animation_cel documents.
 * @see decomp.me (100%) TODO
 */
void func_8005ADA8(FieldAnimCel* cel, FieldAnim* anim)
{
    FieldAnimDef* def;
    FieldTintColor* pal;
    FieldTintColor* entry;
    u8* src;
    u8* dst;
    s32 stride;
    s32 n;

    pal = (FieldTintColor*)0x1F800000;
    def = anim->def;
    switch (cel->format)
    {
    case 0:
        stride = 12;
        dst = anim->frame_data;
        if (cel->code_word != 0)
        {
            stride -= 4;
        }
        if (cel->tpage_word != 0)
        {
            stride -= 4;
        }
        src = def->data;
        n = anim->frame_tile_count * def->unk6;
        while (--n != -1)
        {
            entry = &pal[src[3]];
            ((FieldCellTint*)dst)->rg = entry->rg;
            src += 4;
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
        stride = 12;
        dst = anim->frame_data;
        if (cel->code_word != 0)
        {
            stride -= 4;
        }
        if (cel->tpage_word != 0)
        {
            stride -= 4;
        }
        src = def->data;
        n = anim->frame_tile_count * def->unk6;
        while (--n != -1)
        {
            entry = &pal[src[3]];
            ((FieldCellTint*)dst)->rg = entry->rg;
            src += 4;
            ((FieldCellTint*)dst)->b = entry->b;
            dst += stride;
        }
        break;
    }
}

/**
 * @brief Show or hide a scene object, or one of its parts.
 *
 * Resolves the target the same way the move helper does: @p part_index of -1
 * selects the whole object, and its active bit (bit 0 of the flags word at
 * 0x0C) takes the new state; any other value selects that part of the object
 * and sets its @c visible byte instead.
 *
 * @param obj_index Index of the object in the scene's object list.
 * @param part_index Index of the part within that object, or -1 for the object
 *                   as a whole.
 * @param visible Non-zero to show, zero to hide. Only bit 0 reaches the object
 *                flags word.
 * @note The @c & @c 1 on @p visible is not redundant - without it the flags
 *       word is or-ed with the whole value.
 * @note The @c -1 case has to be the @c if and the part case the @c else;
 *       swapping them inverts the branch and reorders both blocks.
 * @see decomp.me (100%) TODO
 */
void func_8005AF04(s32 obj_index, s32 part_index, s32 visible)
{
    FieldObj* obj;
    FieldPart* part;

    if (part_index == -1)
    {
        obj = func_8005AB4C(obj_index);
        obj->flags.word = (obj->flags.word & ~1) | (visible & 1);
    }
    else
    {
        part = func_8005AB80(obj_index, part_index);
        part->visible = visible;
    }
}

/**
 * @brief Read back the whole-unit position of a scene object, or of one of its
 *        parts.
 *
 * Resolves the target the same way the show/hide helper does - @p part_index of
 * -1 selects the object itself, anything else selects that part - and converts
 * its three 24.8 fixed-point coordinates to whole units.
 *
 * @param obj_index Index of the object in the scene's object list.
 * @param part_index Index of the part within that object, or -1 for the object
 *                   as a whole.
 * @param out Receives the position in whole units.
 * @note The @c -1 case has to be the @c if and the part case the @c else, as in
 *       func_8005AF04; swapping them reorders both blocks.
 * @note gcc emits the depth conversion and store ONCE and jumps the object arm
 *       into the part arm's tail. Both arms are still written out in full here;
 *       the sharing is the compiler's, not the source's.
 * @note The shift must go through SHIFT_TOWARD_ZERO rather than `/ 256`: gcc
 *       does emit the branchy expansion for this divisor, but finishes it with
 *       `sra` where the target has `srl`. See idiom [EXPAND-23].
 * @see decomp.me (100%) TODO
 */
void func_8005AF5C(s32 obj_index, s32 part_index, FieldPos* out)
{
    FieldObj* obj;
    FieldPart* part;

    if (part_index == -1)
    {
        obj = func_8005AB4C(obj_index);
        out->x = SHIFT_TOWARD_ZERO(obj->x, 8);
        out->y = SHIFT_TOWARD_ZERO(obj->y, 8);
        out->z = SHIFT_TOWARD_ZERO(obj->z, 8);
    }
    else
    {
        part = func_8005AB80(obj_index, part_index);
        out->x = SHIFT_TOWARD_ZERO(part->x, 8);
        out->y = SHIFT_TOWARD_ZERO(part->y, 8);
        out->z = SHIFT_TOWARD_ZERO(part->z, 8);
    }
}

/**
 * @brief Apply a rotation and scale record to one part of a scene object.
 *
 * Copies the five halfwords of @p xf into the part's own rotation and scale
 * fields. Unlike the position and visibility helpers this one has no
 * object-level case; @p part_index always selects a part.
 *
 * @param obj_index Index of the object in the scene's object list.
 * @param part_index Index of the part within that object.
 * @param xf Source record; see FieldPartTransform.
 * @note The two scales must be stored before the three angles. Writing them in
 *       FieldPart's own field order instead costs ten rows - the source order
 *       is the record's, not the destination's.
 * @see decomp.me (100%) TODO
 */
void func_8005B034(s32 obj_index, s32 part_index, FieldPartTransform* xf)
{
    FieldPart* part;

    part = func_8005AB80(obj_index, part_index);
    part->scale_x = xf->scale_x;
    part->scale_y = xf->scale_y;
    part->row_angle = xf->row_angle;
    part->column_angle = xf->column_angle;
    part->rotation_angle = xf->rotation_angle;
}

/**
 * @brief Read back a part's rotation and scale into a transform record.
 *
 * The exact inverse of func_8005B034: same five fields, same record order.
 *
 * @param obj_index Index of the object in the scene's object list.
 * @param part_index Index of the part within that object.
 * @param xf Receives the part's current rotation and scale.
 * @note The two scales must be copied before the three angles, as in
 *       func_8005B034. A whole-struct copy does not match either - the fields
 *       move one at a time.
 * @see decomp.me (100%) TODO
 */
void func_8005B094(s32 obj_index, s32 part_index, FieldPartTransform* xf)
{
    FieldPart* part;

    part = func_8005AB80(obj_index, part_index);
    xf->scale_x = part->scale_x;
    xf->scale_y = part->scale_y;
    xf->row_angle = part->row_angle;
    xf->column_angle = part->column_angle;
    xf->rotation_angle = part->rotation_angle;
}

/**
 * @brief Retrigger one effect node in the scene's effect list.
 *
 * Walks @p index steps into the effect list and nudges that node's animation
 * state. Which nudge depends on the node's definition: a definition carrying
 * flag 0x20 (and only when @p from_keyframe is zero) restarts a node that
 * already holds a keyframe, setting control bits 0x45; otherwise the node is
 * rearmed whenever its keyframe differs from the definition's, taking bit 0x40
 * and dropping bits 4 and 1. In that second case a node that is not
 * flag-0x20 driven and holds no keyframe additionally takes bit 8.
 *
 * Both the walk and the body give up quietly if the list is shorter than
 * @p index.
 *
 * @param index Position in the scene's effect list.
 * @param from_keyframe Non-zero to suppress the keyframe-restart path.
 * @note @c anim->timer must be assigned AFTER the three flag operations. Moved
 *       ahead of them it splits the read-modify-write in two and keeps the
 *       flags word live, which also costs the reload before the @c 8 bit.
 * @note The two clears have to stay separate statements. As a single
 *       @c &= @c ~5 they fold into one @c and and the function loses two
 *       instructions - neither constant fits @c andi, so each needs its own
 *       register load.
 * @note The definition flags are read as a WORD through the byte field's
 *       address, the same spelling field_rescale_scene_tints uses; a plain
 *       @c def->flags byte read costs a row at each of the two sites.
 * @see decomp.me (100%) TODO
 */
void func_8005B0F4(s32 index, s32 from_keyframe)
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
        if (((*(u32*)&def->flags & 0x20) != 0) && (from_keyframe == 0))
        {
            if (anim->flags.b.keyframe != 0)
            {
                anim->timer = 1;
                anim->flags.word |= 0x45;
            }
        }
        else if (anim->flags.b.keyframe != def->unk5)
        {
            anim->flags.word |= 0x40;
            anim->flags.word &= ~4;
            anim->flags.word &= ~1;
            anim->timer = 1;
            if (((*(u32*)&def->flags & 0x20) == 0) && (anim->flags.b.keyframe == 0))
            {
                anim->flags.word |= 8;
            }
        }
    }
}


#include "common.h"

typedef struct {
    u8 _pad[0x2C];
    s32 unk2C;
    s32 unk30;
} ApiFieldState;

typedef struct ApiNode
{
    struct ApiNode *unk0;   /* 0x00 next pointer */
    s32 definition;      /* 0x04 compared by field_find_object_by_definition */
    u8 _pad[0x10];       /* 0x08-0x17 */
    s8 unk18;            /* 0x18 byte written by func_8005B228 */
} ApiNode;

struct CollNode;

typedef struct
{
    u8 _pad0[4];           /* 0x00-0x03 */
    ApiNode *objects;          /* 0x04 head of the scene object list */
    ApiNode *unk8;             /* 0x08 head of the node list */
    u8 _pad1[0x10 - 0xC];   /* 0x0C-0x0F */
    struct CollNode *coll_list; /* 0x10 collision-node list traversed by func_8005B368 */
    u8 _pad2[0x28 - 0x14];  /* 0x14-0x27 */
    s32 unk28;              /* 0x28 flag gating the func_8005F5BC call */
} ApiFieldScene;

typedef struct
{
    ApiFieldScene *scene;
} ApiFieldSceneGlobals;

extern s32 D_801ED02C;

extern u8 D_800CBF44[];
extern volatile s32 D_801ED490;

void func_8005F5BC(s32, ApiNode*, ApiFieldScene*, s32);

/**
 * @brief If D_801ED02C is zero, set it to 1 and write 0x100 to D_801ED030.
 * @see decomp.me (100%) TODO
 */
void func_8005B1EC(void) {
    volatile ApiFieldState *s = (volatile ApiFieldState *)0x801ED000;
    if (s->unk2C == 0) {
        s->unk2C = 1;
        s->unk30 = 0x100;
    }
}

/**
 * @brief Return non-zero if D_801ED02C is set.
 * @return 1 if D_801ED02C != 0, 0 otherwise.
 * @see decomp.me (100%) TODO
 */
s32 func_8005B218(void) {
    return D_801ED02C != 0;
}

/**
 * @brief Walk the field scene's node list @p arg0 steps and store @p arg1 at
 *        node->unk18, then poke func_8005F5BC if the scene flag is set.
 * @param arg0 Number of ->unk0 links to follow from the list head.
 * @param arg1 Byte value stored at the reached node's unk18.
 * @note WIP - NOT byte-perfect yet. Structure and types are exact (a single
 *       `register ApiNode *node asm("$5")` pin matches all but one schedule slot).
 *       The remaining gap is register coloring: the target colors `node` into
 *       $a1 (evacuating arg1 to $a3) with the loop sentinel -1 in $v1; natural C
 *       colors node into $v1 and the sentinel into $a0. Per GCC 2.8 global.c
 *       allocno_compare, node and the sentinel have near-equal priority and the
 *       tie breaks by allocno (creation) order, so node (born at scene->unk8,
 *       before the loop) wins $v1. No pin-free shape found yet that flips this
 *       without changing another instruction.
 * @see decomp.me (100%) https://decomp.me/scratch/lN7ye
 */
void func_8005B228(s32 arg0, s32 arg1) {
    ApiNode* var_a1;
    s32 var_v0;
    ApiFieldScene* scene = ((ApiFieldScene *)g_field_scene.scene);

    var_a1 = scene->unk8;
    var_v0 = arg0 - 1;
    while (var_v0 != -1) {
        var_a1 = var_a1->unk0;
        var_v0 -= 1;
    }
    var_a1->unk18 = arg1;
    if (scene->unk28 != 0) {
        func_8005F5BC(0, var_a1, scene, arg1);
    }
}

/**
 * @brief Set D_801ED490 to the given value.
 * @param arg0 Value to store.
 * @see decomp.me (100%) TODO
 */
void func_8005B288(s32 arg0) {
    D_801ED490 = arg0;
}

/**
 * @brief Apply a color-correction lookup to a range of 16-bit pixels.
 *
 * For each pixel in @p pixels[0..pixel_count-1], extracts the maximum of the
 * three 5-bit color components (B: bits 0-4, G: bits 5-9, R: bits 10-14),
 * uses that maximum as an index into a 64-entry lookup table selected by
 * @p table_index, adds the looked-up value to the STP bit (bit 15) of the
 * original pixel, and writes the result back.
 *
 * @param pixels      Pointer to an array of 16-bit pixel values.
 * @param pixel_count Number of pixels to process (0 = no-op).
 * @param table_index Lookup-table selector; table is D_800CBF44[table_index * 64 ..].
 * @param unused      Unused (FieldObject* in caller).
 *
 * @note Called by field_load_map with pixel_count = object->unk2A and
 *       table_index = D_801ED490 - 1.
 * @note Matches 100% with gcc280_g4 and gcc272_cdk.
 * @see decomp.me (100%) TODO
 */
void field_apply_pixel_lookup(u16* pixels, s32 pixel_count, s32 table_index, void* unused)
{
    u16* ptr;
    s32 count;
    u16 pixel;
    u32 b;
    u32 g;
    u32 r;
    u32 max_component;
    u32 table_base;

    ptr = pixels;
    count = pixel_count - 1;
    table_base = (u32)&D_800CBF44[table_index * 64];
    if (pixel_count != 0)
    {
        do
        {
            pixel = *ptr;
            max_component = pixel & 0x1F;
            g = (pixel >> 5) & 0x1F;
            if (max_component < g)
            {
                max_component = g;
            }
            r = (pixel >> 10) & 0x1F;
            if (max_component < r)
            {
                max_component = r;
            }
            count -= 1;
            *ptr = *(u16*)(table_base + max_component * 2) + (pixel & 0x8000);
            ptr += 1;
        }
        while (count != -1);
    }
}

/**
 * @brief Find a field object by its definition pointer.
 *
 * Walks the scene object list and returns the first object whose definition
 * pointer matches @p definition.
 *
 * @param definition Definition pointer to compare at object offset 0x04.
 * @return Pointer to the matching field object, or NULL if not found.
 *
 * @note Matches 100% with gcc280_g4 and gcc272_cdk.
 * @see decomp.me (100%) https://decomp.me/scratch/FThyS
 */
void* field_find_object_by_definition(s32 definition)
{
    ApiNode* node;

    node = ((ApiFieldScene *)g_field_scene.scene)->objects;
    if (node != 0)
    {
        do
        {
            if (definition == node->definition)
            {
                return node;
            }
            node = node->unk0;
        }
        while (node != 0);
    }
    return 0;
}
