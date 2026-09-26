/** @file field_animation.c
 * @brief Per-frame update of the scene animation lists: tile and palette
 *        animation, tweens, keyframe sounds, in-scene movies and sequences.
 */

#include "movie.h"
#include "movie_state.h"
#include "field_animation.h"
#include "field_scene_internal.h"
#include "akao_cmd.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "controller.h"
#include "scene_state.h"
#include "field_calls.h"

/** VRAM halfwords per tile column of an animation rectangle (16 pixels at 4 bpp). */
#define FIELD_ANIM_TILE_VRAM_WIDTH 4
/** Bytes of image data per tile of an animation rectangle. */
#define FIELD_ANIM_TILE_BYTES 128

/** 15-bit colour: semi-transparency bit and the mask of one 5-bit component. */
#define FIELD_COLOR_STP 0x8000
#define FIELD_COLOR_COMPONENT 0x1F

/** Movies 0 and 1 play on a pair of scene cels; the others fill the lower tile bank. */
#define FIELD_SCENE_MOVIE_COUNT 2
/** CD resource of movie 0's stream; each movie has a stream and a still image resource. */
#define FIELD_MOVIE_RESOURCE_FIRST 0x16A6
#define FIELD_MOVIE_STREAM_RESOURCE(def) ((def)->head.b.unk1 * 2 + FIELD_MOVIE_RESOURCE_FIRST)
#define FIELD_MOVIE_STILL_RESOURCE(def) ((def)->head.b.unk1 * 2 + (FIELD_MOVIE_RESOURCE_FIRST + 1))
/** MOVIE.BIN is streamed here and started in place; the still image reuses the buffer. */
#define FIELD_MOVIE_BUFFER ((void*)0x80140000)
/** Pixels of a full-screen still image, after its size word. */
#define FIELD_MOVIE_STILL_PIXELS ((u_long*)0x80140004)
/** movie_init frame count of a full-screen movie. */
#define FIELD_MOVIE_FULL_SCREEN_FRAMES 302

/** Movie handler steps in FieldAnim::flags.b.state. */
enum
{
    FIELD_MOVIE_STEP_LOAD = 0,   /**< load MOVIE.BIN and seek to the stream */
    FIELD_MOVIE_STEP_START = 1,  /**< start playback once the seek is done */
    FIELD_MOVIE_STEP_STREAM = 2  /**< playing; follow MovieState::end_state */
};

/** MovieState::end_state values seen by the movie handler. */
enum
{
    FIELD_MOVIE_END_DONE = 2,        /**< the stream has ended (END_STATE_DONE) */
    FIELD_MOVIE_END_STILL_QUEUED = 3, /**< the still image is being read */
    FIELD_MOVIE_END_STILL_SHOWN = 4   /**< the still image has been uploaded */
};

/** Base of the CD system block of the main executable (CdSystem). */
#define FIELD_CD_SYSTEM_ADDRESS 0x801ED800U
/** CdSystem status flag that blocks new queue commands (CD_STATUS_QUEUE_LOCK). */
#define FIELD_CD_STATUS_QUEUE_LOCK 0x40

/** @brief Status word at the start of the CD system block. */
typedef struct
{
    u32 word;
} FieldCdStatus;

#define FIELD_CD_STATUS ((FieldCdStatus*)FIELD_CD_SYSTEM_ADDRESS)

/** FieldTweenKey::visibility bit copied to the target's visibility. */
#define FIELD_TWEEN_VISIBLE_SHIFT 15
static void field_update_part_sweep(FieldPart* part);
static void field_update_animation_sfx(FieldAnimDef* def, FieldAnim* anim);
static void field_retarget_cel_cluts(FieldAnimDef* anim_def, FieldPart* cel, s32 frame);
static u_long* field_blend_animation_frames(FieldAnimDef* def, FieldAnim* anim);
static void field_tint_animation_cel_list(FieldAnimDef* def, FieldTintSrc* src, s32 shade);
static void field_advance_animation_keyframe(FieldAnimDef* def, FieldAnim* anim);
static void field_retarget_cel_list_cluts(FieldAnimDef* def, FieldTintSrc* src, s32 frame);

/**
 * @brief Advance every animation list of the current field scene by one frame.
 *
 * Runs the part sweeps, then the tile list (frame blits, cel cycling, tile
 * uploads, movies, tweens and keyframe sounds), the palette list (CLUT
 * retargeting, colour cycling and CLUT uploads), the tint list, the effect
 * list and finally the sequence list, which starts linked sequences when
 * their delays run out.
 */
void field_update_scene_animations(void)
{
    FieldScene* scene;
    FieldSceneHeader* header;
    FieldObj* object;
    FieldPart* part;
    FieldAnim* anim;
    FieldAnimDef* definition;
    FieldAnimDef* frame_definition;
    FieldAnimDef* strip_definition;
    FieldPart* cel;
    FieldImageReq* upload;
    FieldSeq* sequence;
    FieldSeq* target_sequence;
    FieldSeqDef* command;
    u16* source_pixels;
    u16* destination_pixels;
    s32 previous_frame;
    s32 control_flags;
    s32 sweep_mode;
    s32 copy_count;
    s32 wrap_count;
    s32 index;
    s32 upload_extent;
    s32 upload_width;
    s32 previous_frame_index;

    scene = g_field_scene.scene;

    for (object = scene->objects; object != NULL; object = object->next)
    {
        for (part = object->parts; part != NULL; part = part->next)
        {
            if (part->def->u.word & FIELD_PART_SWEEP_MASK)
            {
                sweep_mode = FIELD_PART_SWEEP_MODE(part->def);
                if ((sweep_mode != 0) && (sweep_mode < FIELD_PART_SWEEP_MODE_COUNT))
                {
                    field_update_part_sweep(part);
                }
            }
        }
    }

    header = scene->header;
    for (anim = scene->anims; anim != NULL; anim = anim->next)
    {
        definition = anim->def;
        frame_definition = anim->def;
        if (anim->flags.word & FIELD_ANIM_FLAG_UPLOAD_PENDING)
        {
            upload = &anim->upload;
            if (FIELD_ANIM_KIND(definition) == FIELD_TILE_ANIM_UPLOAD)
            {
                upload->rect.x = definition->u.tile.rect_x * FIELD_ANIM_TILE_VRAM_WIDTH + FIELD_TILE_LOWER_BANK_VRAM_X;
                upload->rect.y = definition->u.tile.rect_y * FIELD_TILE_SIZE + FIELD_TILE_LOWER_BANK_VRAM_Y;
                upload->rect.w = definition->u.tile.rect_width * FIELD_ANIM_TILE_VRAM_WIDTH;
                upload->rect.h = definition->u.tile.rect_height * FIELD_TILE_SIZE;
                upload->data = (u_long*)(definition->data + ((anim->flags.b.state * definition->u.tile.rect_width * definition->u.tile.rect_height) << 7));
                field_queue_vram_upload(upload);
            }
            anim->flags.word &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
        }
        if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
        {
            anim->timer--;
            switch (definition->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
            {
            case FIELD_TILE_ANIM_MOVIE:
                switch (anim->flags.b.state)
                {
                case FIELD_MOVIE_STEP_LOAD:
                    if (cdrom_process_state() == 0)
                    {
                        cdrom_stream(CD_RES_MOVIE_BIN, FIELD_MOVIE_BUFFER);
                        cdrom_queue_seek(FIELD_MOVIE_STREAM_RESOURCE(definition));
                        anim->flags.b.state = FIELD_MOVIE_STEP_START;
                        FIELD_CD_STATUS->word |= FIELD_CD_STATUS_QUEUE_LOCK;
                    }
                    /* fallthrough */
                case FIELD_MOVIE_STEP_START:
                    if (cdrom_can_queue_resource(FIELD_MOVIE_STREAM_RESOURCE(definition)) != 0)
                    {
                        MOVIE_STATE->rects[0].x = frame_definition->u.tile.rect_x * FIELD_ANIM_TILE_VRAM_WIDTH + FIELD_TILE_LOWER_BANK_VRAM_X;
                        MOVIE_STATE->rects[0].y = frame_definition->u.tile.rect_y * FIELD_TILE_SIZE + FIELD_TILE_LOWER_BANK_VRAM_Y;
                        MOVIE_STATE->rects[0].w = frame_definition->u.tile.rect_width * FIELD_ANIM_TILE_VRAM_WIDTH;
                        MOVIE_STATE->rects[0].h = frame_definition->u.tile.rect_height * FIELD_TILE_SIZE;
                        cel = anim->cels;
                        FIELD_CD_STATUS->word &= ~FIELD_CD_STATUS_QUEUE_LOCK;
                        if (definition->head.b.unk1 < FIELD_SCENE_MOVIE_COUNT)
                        {
                            movie_init(FIELD_MOVIE_STREAM_RESOURCE(definition), 1, definition->flags.b.last_frame - 2, cel->visible);
                        }
                        else
                        {
                            movie_init(FIELD_MOVIE_STREAM_RESOURCE(definition), 1, FIELD_MOVIE_FULL_SCREEN_FRAMES, 0);
                        }
                        anim->flags.b.state = FIELD_MOVIE_STEP_STREAM;
                    }
                    anim->timer = 1;
                    break;
                default:
                    if (MOVIE_STATE->end_state >= FIELD_MOVIE_END_STILL_QUEUED)
                    {
                        if (MOVIE_STATE->end_state == FIELD_MOVIE_END_STILL_QUEUED)
                        {
                            if (cdrom_can_queue_resource(FIELD_MOVIE_STILL_RESOURCE(definition)) != 0)
                            {
                                upload = &anim->upload;
                                if (definition->head.b.unk1 < FIELD_SCENE_MOVIE_COUNT)
                                {
                                    cel = anim->cels;
                                    upload->rect.x = MOVIE_STATE->rects[cel->visible].x;
                                    upload->rect.y = MOVIE_STATE->rects[cel->visible].y;
                                    upload->rect.w = MOVIE_STATE->rects[cel->visible].w;
                                    upload->rect.h = MOVIE_STATE->rects[cel->visible].h;
                                    upload->data = (u_long*)FIELD_MOVIE_BUFFER;
                                    field_queue_vram_upload(upload);
                                    if (cel->visible == 1)
                                    {
                                        cel->visible = 0;
                                        cel = cel->next;
                                        cel->visible = 1;
                                    }
                                    else
                                    {
                                        cel->visible = 1;
                                        cel = cel->next;
                                        cel->visible = 0;
                                    }
                                }
                                else
                                {
                                    cel = anim->cels;
                                    upload->rect.x = FIELD_TILE_LOWER_BANK_VRAM_X;
                                    upload->rect.y = FIELD_TILE_LOWER_BANK_VRAM_Y;
                                    upload->data = FIELD_MOVIE_STILL_PIXELS;
                                    upload->rect.w = g_field_movie_frame_width;
                                    upload->rect.h = g_field_movie_frame_height;
                                    field_queue_vram_upload(upload);
                                    cel->visible = 0;
                                    cel = cel->next;
                                    cel->visible = 0;
                                }
                                MOVIE_STATE->end_state = FIELD_MOVIE_END_STILL_SHOWN;
                            }
                        }
                        else
                        {
                            if (definition->head.b.unk1 >= FIELD_SCENE_MOVIE_COUNT)
                            {
                                field_begin_scene_fade_in();
                            }
                            anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                            field_reset_actor_resources();
                        }
                        anim->timer = 1;
                        break;
                    }
                    if (definition->head.b.unk1 < FIELD_SCENE_MOVIE_COUNT)
                    {
                        set_controller_vsync_interval(2);
                    }
                    movie_update();
                    movie_service_video_ops();
                    if (MOVIE_STATE->frame_ready == 1)
                    {
                        cel = anim->cels;
                        if (MOVIE_STATE->chunk_idx == 1)
                        {
                            cel->visible = 1;
                            cel = cel->next;
                            cel->visible = 0;
                        }
                        else
                        {
                            cel->visible = 0;
                            cel = cel->next;
                            cel->visible = 1;
                        }
                        MOVIE_STATE->frame_ready = 0;
                    }
                    if (MOVIE_STATE->end_state == FIELD_MOVIE_END_DONE)
                    {
                        cdrom_reset();
                        cdrom_queue_read(FIELD_MOVIE_STILL_RESOURCE(definition), FIELD_MOVIE_BUFFER);
                        MOVIE_STATE->end_state = FIELD_MOVIE_END_STILL_QUEUED;
                    }
                    anim->timer = 1;
                    break;
                }
                break;
            case FIELD_TILE_ANIM_TWEEN_PART:
            case FIELD_TILE_ANIM_TWEEN_OBJECT:
                field_apply_animation_tween(definition, anim, 1);
                break;
            }
            if (anim->timer == 0)
            {
                previous_frame = anim->flags.b.state;
                field_advance_animation_keyframe(definition, anim);
                switch (definition->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
                {
                case FIELD_TILE_ANIM_BLIT:
                    if (previous_frame != anim->flags.b.state)
                    {
                        field_blit_animation_frame(definition, anim, anim->flags.b.state);
                    }
                    break;
                case FIELD_TILE_ANIM_CEL_CYCLE:
                    cel = anim->cels;
                    index = previous_frame;
                    while (--index != -1)
                    {
                        cel = cel->next;
                    }
                    cel->visible = 0;
                    cel = anim->cels;
                    index = anim->flags.b.state;
                    while (--index != -1)
                    {
                        cel = cel->next;
                    }
                    cel->visible = 1;
                    break;
                case FIELD_TILE_ANIM_UPLOAD:
                    upload = &anim->upload;
                    upload->rect.x = frame_definition->u.tile.rect_x * FIELD_ANIM_TILE_VRAM_WIDTH + FIELD_TILE_LOWER_BANK_VRAM_X;
                    upload->rect.y = frame_definition->u.tile.rect_y * FIELD_TILE_SIZE + FIELD_TILE_LOWER_BANK_VRAM_Y;
                    upload->rect.w = frame_definition->u.tile.rect_width * FIELD_ANIM_TILE_VRAM_WIDTH;
                    upload->rect.h = frame_definition->u.tile.rect_height * FIELD_TILE_SIZE;
                    upload->data = (u_long*)(frame_definition->data + ((anim->flags.b.state * frame_definition->u.tile.rect_width * frame_definition->u.tile.rect_height) << 7));
                    field_queue_vram_upload(upload);
                    break;
                case FIELD_TILE_ANIM_TWEEN_PART:
                case FIELD_TILE_ANIM_TWEEN_OBJECT:
                    while (anim->timer == 0)
                    {
                        field_apply_animation_tween(definition, anim, 1);
                        field_advance_animation_keyframe(definition, anim);
                    }
                    break;
                case FIELD_TILE_ANIM_SOUND:
                    field_update_animation_sfx(definition, anim);
                    break;
                }
            }
        }
    }

    for (anim = scene->strips; anim != NULL; anim = anim->next)
    {
        definition = anim->def;
        control_flags = anim->flags.word;
        strip_definition = anim->def;
        if (control_flags & FIELD_ANIM_FLAG_UPLOAD_PENDING)
        {
            upload = &anim->upload;
            switch (definition->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
            {
            case FIELD_PALETTE_ANIM_CYCLE:
                if (control_flags & FIELD_ANIM_FLAG_SECOND_BUFFER)
                {
                    if (definition->u.clut.clut_mode == 0)
                    {
                        destination_pixels = anim->scratch_pixels + 16;
                    }
                    else
                    {
                        destination_pixels = anim->scratch_pixels + 256;
                    }
                    anim->flags.word &= ~FIELD_ANIM_FLAG_SECOND_BUFFER;
                }
                else
                {
                    destination_pixels = anim->scratch_pixels;
                    anim->flags.word = control_flags | FIELD_ANIM_FLAG_SECOND_BUFFER;
                }
                upload->data = (u_long*)destination_pixels;
                wrap_count = 0;
                if (anim->flags.b.state != 0)
                {
                    if (strip_definition->u.clut.reverse != 0)
                    {
                        previous_frame_index = anim->flags.b.state - 1;
                        wrap_count = (definition->flags.b.last_frame - previous_frame_index) * strip_definition->u.clut.length;
                        copy_count = anim->flags.b.state * strip_definition->u.clut.length;
                    }
                    else
                    {
                        wrap_count = anim->flags.b.state * strip_definition->u.clut.length;
                        copy_count = (definition->flags.b.last_frame + 1 - anim->flags.b.state) * strip_definition->u.clut.length;
                    }
                }
                else
                {
                    copy_count = (definition->flags.b.last_frame + 1) * strip_definition->u.clut.length;
                }
                if (strip_definition->u.clut.clut_mode == 0)
                {
                    source_pixels = header->pixel_data + strip_definition->u.clut.clut_slot * FIELD_CLUT_4BIT_COLORS + strip_definition->u.clut.clut_offset + wrap_count;
                }
                else
                {
                    source_pixels = header->pixel_data + strip_definition->u.clut.clut_slot * FIELD_CLUT_8BIT_COLORS + strip_definition->u.clut.clut_offset + wrap_count;
                }
                copy_count--;
                while (copy_count != -1)
                {
                    *destination_pixels++ = *source_pixels++;
                    copy_count--;
                }
                if (wrap_count != 0)
                {
                    if (strip_definition->u.clut.clut_mode == 0)
                    {
                        source_pixels = header->pixel_data + strip_definition->u.clut.clut_slot * FIELD_CLUT_4BIT_COLORS + strip_definition->u.clut.clut_offset;
                    }
                    else
                    {
                        source_pixels = header->pixel_data + strip_definition->u.clut.clut_slot * FIELD_CLUT_8BIT_COLORS + strip_definition->u.clut.clut_offset;
                    }
                    copy_count = wrap_count - 1;
                    while (copy_count != -1)
                    {
                        *destination_pixels++ = *source_pixels++;
                        copy_count--;
                    }
                }
                if (strip_definition->u.clut.clut_mode == 0)
                {
                    upload->rect.x = strip_definition->u.clut.clut_offset + ((strip_definition->u.clut.clut_slot & 0xF) * 0x10);
                    upload->rect.y = (strip_definition->u.clut.clut_slot >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = definition->flags.b.last_frame + 1;
                    upload->rect.h = 1;
                }
                else
                {
                    upload->rect.x = strip_definition->u.clut.clut_offset;
                    upload->rect.y = strip_definition->u.clut.clut_slot + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = definition->flags.b.last_frame + 1;
                    upload->rect.h = 1;
                }
                field_queue_vram_upload(upload);
                break;
            case FIELD_PALETTE_ANIM_CLUT_ROW:
                if (definition->u.clut.clut_mode == 0)
                {
                    upload->rect.x = definition->u.clut.clut_offset + ((definition->u.clut.clut_slot & 0xF) * 0x10);
                    upload->rect.y = (definition->u.clut.clut_slot >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = definition->u.clut.length;
                    upload->rect.h = 1;
                    if (anim->flags.b.state == 0)
                    {
                        upload->data = (u_long*)(header->pixel_data + definition->u.clut.clut_slot * FIELD_CLUT_4BIT_COLORS + (definition->u.clut.clut_offset & 0xE));
                    }
                    else
                    {
                        upload->data = (u_long*)(header->pixel_data + header->pixel_stride + definition->u.clut.pixel_offset + (anim->flags.b.state - 1) * definition->u.clut.length);
                    }
                }
                else
                {
                    upload->rect.x = definition->u.clut.clut_offset;
                    upload->rect.y = definition->u.clut.clut_slot + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = definition->u.clut.length;
                    upload->rect.h = 1;
                    if (anim->flags.b.state == 0)
                    {
                        upload->data = (u_long*)(header->pixel_data + definition->u.clut.clut_slot * FIELD_CLUT_8BIT_COLORS + definition->u.clut.clut_offset);
                    }
                    else
                    {
                        upload->data = (u_long*)(header->pixel_data + header->pixel_stride + definition->u.clut.pixel_offset + (anim->flags.b.state - 1) * definition->u.clut.length);
                    }
                }
                field_queue_vram_upload(upload);
                break;
            case FIELD_PALETTE_ANIM_CLUT_BLOCK:
                if (definition->u.clut.clut_mode == 0)
                {
                    upload->rect.x = (definition->u.clut.clut_slot & 0xF) * 0x10;
                    upload->rect.y = (definition->u.clut.clut_slot >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                    upload_extent = definition->u.clut.length * 0x10;
                    upload_width = 0x100;
                    if (upload_extent < 0x101)
                    {
                        upload_width = upload_extent;
                    }
                    upload->rect.w = upload_width;
                    upload->rect.h = (definition->u.clut.length + 0xF) / 0x10;
                    if (anim->flags.b.state == 0)
                    {
                        upload->data = (u_long*)(header->pixel_data + definition->u.clut.clut_slot * FIELD_CLUT_4BIT_COLORS);
                    }
                    else
                    {
                        upload->data =
                            (u_long*)(header->pixel_data + header->pixel_stride + definition->u.clut.pixel_offset + (anim->flags.b.state - 1) * definition->u.clut.length * FIELD_CLUT_4BIT_COLORS);
                    }
                }
                else
                {
                    upload->rect.x = 0;
                    upload->rect.y = definition->u.clut.clut_slot + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = 0x100;
                    upload->rect.h = definition->u.clut.length;
                    if (anim->flags.b.state == 0)
                    {
                        upload->data = (u_long*)(header->pixel_data + definition->u.clut.clut_slot * FIELD_CLUT_8BIT_COLORS);
                    }
                    else
                    {
                        upload->data =
                            (u_long*)(header->pixel_data + header->pixel_stride + definition->u.clut.pixel_offset + (anim->flags.b.state - 1) * definition->u.clut.length * FIELD_CLUT_8BIT_COLORS);
                    }
                }
                field_queue_vram_upload(upload);
                break;
            case FIELD_PALETTE_ANIM_BLEND:
                upload->data = field_blend_animation_frames(definition, anim);
                if (definition->u.clut.clut_mode == 0)
                {
                    upload->rect.x = (definition->u.clut.clut_slot & 0xF) * 0x10;
                    upload->rect.y = (definition->u.clut.clut_slot >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                    upload_extent = definition->u.clut.length * 0x10;
                    upload_width = 0x100;
                    if (upload_extent < 0x101)
                    {
                        upload_width = upload_extent;
                    }
                    upload->rect.w = upload_width;
                    upload->rect.h = (definition->u.clut.length + 0xF) / 0x10;
                }
                else
                {
                    upload_extent = 0x100;
                    upload->rect.x = 0;
                    upload->rect.y = definition->u.clut.clut_slot + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = upload_extent;
                    upload->rect.h = definition->u.clut.length;
                }
                field_queue_vram_upload(upload);
                break;
            }
            anim->flags.word &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
        }
        if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
        {
            anim->timer--;
            if (FIELD_ANIM_KIND(definition) == FIELD_PALETTE_ANIM_BLEND)
            {
                anim->flags.word |= FIELD_ANIM_FLAG_UPLOAD_PENDING;
            }
            if (anim->timer == 0)
            {
                field_advance_animation_keyframe(definition, anim);
                switch (definition->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
                {
                case FIELD_PALETTE_ANIM_CEL_CLUT:
                    field_retarget_cel_cluts(definition, anim->cels, anim->flags.b.state);
                    break;
                case FIELD_PALETTE_ANIM_CEL_LIST_CLUT:
                    field_retarget_cel_list_cluts(definition, (FieldTintSrc*)anim->cels, anim->flags.b.state);
                    break;
                default:
                    anim->flags.word |= FIELD_ANIM_FLAG_UPLOAD_PENDING;
                    break;
                }
            }
        }
    }

    for (anim = scene->sprites; anim != NULL; anim = anim->next)
    {
        definition = anim->def;
        if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
        {
            if (--anim->timer == 0)
            {
                field_advance_animation_keyframe(definition, anim);
                switch (definition->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
                {
                case FIELD_TINT_ANIM_CEL:
                    field_tint_animation_cel(definition, anim->cels, anim->owner.tint_src, anim->flags.b.state);
                    break;
                case FIELD_TINT_ANIM_CEL_LIST:
                    field_tint_animation_cel_list(definition, (FieldTintSrc*)anim->cels, anim->flags.b.state);
                    break;
                case 2:
                    break;
                }
            }
        }
    }

    for (anim = scene->effects; anim != NULL; anim = anim->next)
    {
        definition = anim->def;
        if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
        {
            if (--anim->timer == 0)
            {
                field_advance_animation_keyframe(definition, anim);
                field_blit_animation_frame(definition, anim, anim->flags.b.state);
            }
        }
    }

    for (sequence = scene->seqs; sequence != NULL; sequence = sequence->next)
    {
        if ((sequence->flags.word & FIELD_SEQ_PHASE_MASK) != 0)
        {
            command = sequence->def;
            if ((sequence->flags.word & FIELD_SEQ_PHASE_MASK) == FIELD_SEQ_PHASE_RUNNING)
            {
                if ((command->start_link != FIELD_SEQ_NO_LINK) && (command->start_delay == sequence->phase_frames))
                {
                    target_sequence = scene->seqs;
                    index = command->start_link;
                    while (--index != -1)
                    {
                        target_sequence = target_sequence->next;
                    }
                    field_start_sequence(target_sequence, sequence->flags.b.index);
                }
                if (field_get_animation_state(command->list_kind, command->anim_index) == FIELD_ANIM_STATE_FINISHED)
                {
                    if (command->end_link != FIELD_SEQ_NO_LINK)
                    {
                        sequence->phase_frames = 0;
                        sequence->flags.word = (sequence->flags.word & ~FIELD_SEQ_PHASE_MASK) | FIELD_SEQ_PHASE_FINISHED;
                    }
                    else
                    {
                        sequence->flags.word = sequence->flags.word & ~FIELD_SEQ_PHASE_MASK;
                    }
                }
            }
            if (((sequence->flags.word & FIELD_SEQ_PHASE_MASK) == FIELD_SEQ_PHASE_FINISHED) && (command->end_delay == sequence->phase_frames))
            {
                target_sequence = scene->seqs;
                index = command->end_link;
                while (--index != -1)
                {
                    target_sequence = target_sequence->next;
                }
                field_start_sequence(target_sequence, sequence->flags.b.index);
                if (target_sequence != sequence)
                {
                    sequence->flags.word &= ~FIELD_SEQ_PHASE_MASK;
                }
            }
            sequence->phase_frames = sequence->phase_frames + 1;
        }
    }
}

/**
 * @brief Swing a part to and fro and move its attached nodes with it.
 *
 * The sweep phase counts down over the sweep period and drives a sine swing
 * of the part's rotation angle; the amplitude depends on the sweep mode. The
 * nodes attached to the part are displaced around the pivot the mode selects
 * (the centre, the top or the bottom of the scene extent).
 *
 * @param part Part whose sweep is advanced.
 */
static void field_update_part_sweep(FieldPart* part)
{
    FieldScene* scene;
    FieldNode* node;
    FieldNodeDef* def;
    s32 mode;
    u32 divisor;
    s32 sine;
    s32 negative_sine;
    s32 sweep_origin;
    s32 remaining_nodes;
    s32 product;
    s32 offset;
    s32 base_offset;
    s32 previous;
    s16* angle_table;
    s16* angle_entry;

    scene = g_field_scene.scene;
    part->sweep_phase = part->sweep_phase - 1;
    mode = FIELD_PART_SWEEP_MODE(part->def);
    switch (mode)
    {
    case 1:
        divisor = 289;
        break;
    case 2:
        divisor = 161;
        break;
    case 3:
    default:
        divisor = 257;
        break;
    }
    sine = rsin((part->sweep_phase << 12) / part->sweep_period);
    if (sine >= 0)
    {
        part->rotation_angle = sine / divisor;
    }
    else
    {
        part->rotation_angle = ONE - ((u32)-sine / divisor);
    }
    negative_sine = -rsin(part->rotation_angle);
    if (part->node_count != 0)
    {
        angle_table = g_field_node_angle_table;
        mode = FIELD_PART_SWEEP_MODE(part->def);
        switch (mode)
        {
        case 1:
        case 2:
            sweep_origin = scene->header->unk30 / 2;
            break;
        case 3:
            sweep_origin = 0;
            break;
        case 4:
        default:
            sweep_origin = scene->header->unk30;
            break;
        }
        remaining_nodes = part->node_count;
        for (node = scene->nodes; node != NULL; node = node->next)
        {
            if (node->part == part)
            {
                offset = sweep_origin;
                def = node->def;
                angle_entry = &angle_table[def->x_angle_index * 2];
                product = (*angle_entry - offset) * negative_sine;
                offset = product >> 4;
                if (product < 0)
                {
                    offset = (product + 0xF) >> 4;
                }
                base_offset = def->base_x << 8;
                if ((offset + base_offset) < 0)
                {
                    offset = -base_offset;
                }
                previous = node->x;
                node->x = offset;
                node->delta_x = offset - previous;
                angle_entry = &angle_table[def->y_angle_index * 2];
                product = (*angle_entry - sweep_origin) * negative_sine;
                offset = product >> 4;
                if (product < 0)
                {
                    offset = (product + 0xF) >> 4;
                }
                base_offset = def->base_y << 8;
                if ((offset + base_offset) < 0)
                {
                    offset = -base_offset;
                }
                previous = node->y;
                remaining_nodes -= 1;
                node->y = offset;
                node->delta_y = offset - previous;
                if (remaining_nodes == 0)
                {
                    break;
                }
            }
        }
    }
    if (part->sweep_phase == 0)
    {
        part->sweep_phase = part->sweep_period;
    }
}

/**
 * @brief Copy one animation frame into a cel's packed tile records.
 * @param def Animation definition describing the source rectangle.
 * @param anim Animation node containing frame data and the target cel.
 * @param frame Frame index to copy.
 */
void field_blit_animation_frame(FieldAnimDef* def, FieldAnim* anim, s32 frame)
{
    FieldPart* cel;
    FieldPartDef* grid;
    u8* record_cursor;
    u32* frame_words;
    u32* mask_cursor;
    u32 mask_word;
    u32 mask_bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 word_count;

    cel = anim->cels;
    grid = def->u.tile.grid;
    record_cursor = cel->records;
    stride = 0;
    switch (cel->kind)
    {
    case 0:
    case 2:
    case 3:
    case 4:
    case 5:
        stride = FIELD_CEL_RECORD_SIZE;
        break;
    case 1:
    case 6:
        break;
    }
    if (cel->code_word != 0)
    {
        stride -= FIELD_CEL_SHARED_WORD_SIZE;
    }
    if (cel->tpage_word != 0)
    {
        stride -= FIELD_CEL_SHARED_WORD_SIZE;
    }
    frame_words = (u32*)(anim->frame_data + anim->frame_tile_count * stride * frame);
    mask_bit = 1;
    mask_cursor = cel->bits;
    mask_word = *mask_cursor++;
    for (row = 0; row != grid->u.b.rows; row++)
    {
        if (row < def->u.tile.rect_y)
        {
            /* Above the sub-rectangle: step the cursor over the whole row. */
            col = grid->u.b.cols;
            while (--col != -1)
            {
                if (mask_word & mask_bit)
                {
                    record_cursor += stride;
                }
                mask_bit <<= 1;
                if (mask_bit == 0)
                {
                    mask_word = *mask_cursor++;
                    mask_bit = 1;
                }
            }
        }
        else
        {
            if (row >= def->u.tile.rect_y + def->u.tile.rect_height)
            {
                return;
            }
            for (col = 0; col != grid->u.b.cols; col++)
            {
                if (mask_word & mask_bit)
                {
                    if ((col >= def->u.tile.rect_x) && (col < def->u.tile.rect_x + def->u.tile.rect_width))
                    {
                        word_count = stride >> 2;
                        while (--word_count != -1)
                        {
                            *(u32*)record_cursor = *frame_words++;
                            record_cursor += 4;
                        }
                    }
                    else
                    {
                        record_cursor += stride;
                    }
                }
                mask_bit <<= 1;
                if (mask_bit == 0)
                {
                    mask_word = *mask_cursor++;
                    mask_bit = 1;
                }
            }
        }
    }
}

/**
 * @brief Interpolate the current tween keyframe and optionally move its target.
 *
 * The keyframe's end offsets are scaled by the elapsed part of the keyframe
 * (8.8 fixed point). Kind FIELD_TILE_ANIM_TWEEN_PART moves the animation's
 * part, the other kind its object; the attached nodes follow.
 *
 * @param def Animation definition containing the tween keyframes.
 * @param anim Animation node to evaluate.
 * @param apply_to_target Nonzero to move the target by the change since the last call.
 */
void field_apply_animation_tween(FieldAnimDef* def, FieldAnim* anim, s32 apply_to_target)
{
    FieldAnimDef* definition_copy;
    FieldObj* object;
    FieldPart* part;
    FieldTweenKey* keyframe;
    s32 duration;
    s32 elapsed;
    s32 value;
    s32 delta;
    u8 range_start;

    definition_copy = def;
    object = NULL;
    part = NULL;
    if (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_TWEEN_PART)
    {
        part = anim->cels;
    }
    else
    {
        object = (FieldObj*)anim->cels;
    }
    keyframe = &((FieldTweenKey*)definition_copy->data)[anim->flags.b.state];
    duration = field_find_count_table_span(def, anim->flags.b.keyframe, &range_start)->duration;
    if (duration == 0)
    {
        duration = 1;
    }
    elapsed = duration - anim->timer;
    if (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_TWEEN_PART)
    {
        part->visible = keyframe->visibility >> FIELD_TWEEN_VISIBLE_SHIFT;
    }
    else
    {
        object->flags.word = (object->flags.word & ~FIELD_OBJ_VISIBLE) | (keyframe->visibility >> FIELD_TWEEN_VISIBLE_SHIFT);
    }

    value = ((elapsed * keyframe->x) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_x;
        if (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_TWEEN_PART)
        {
            part->x += delta;
            field_move_part_nodes(part, delta, FIELD_AXIS_X);
        }
        else
        {
            object->x += delta;
            field_move_object_nodes(object, delta, FIELD_AXIS_X);
        }
        if (anim->timer == 0)
        {
            anim->tween_x = 0;
        }
        else
        {
            anim->tween_x = value;
        }
    }
    else
    {
        anim->tween_x = value;
    }

    value = ((elapsed * keyframe->y) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_y;
        if (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_TWEEN_PART)
        {
            part->y += delta;
            field_move_part_nodes(part, delta, FIELD_AXIS_Y);
        }
        else
        {
            object->y += delta;
            field_move_object_nodes(object, delta, FIELD_AXIS_Y);
        }
        if (anim->timer == 0)
        {
            anim->tween_y = 0;
        }
        else
        {
            anim->tween_y = value;
        }
    }
    else
    {
        anim->tween_y = value;
    }

    value = ((elapsed * keyframe->z) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_z;
        if (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_TWEEN_PART)
        {
            part->z += delta;
            field_move_part_nodes(part, delta, FIELD_AXIS_Z);
        }
        else
        {
            object->z += delta;
            field_move_object_nodes(object, delta, FIELD_AXIS_Z);
        }
        if (anim->timer == 0)
        {
            anim->tween_z = 0;
        }
        else
        {
            anim->tween_z = value;
        }
    }
    else
    {
        anim->tween_z = value;
    }
}

/**
 * @brief Play, update or stop the sound of the animation's current keyframe.
 *
 * A positional sound gets its pan from the screen x and its volume from the
 * screen y of the part's centre; outside the screen both fade out. One-shot
 * sounds start once and afterwards only get their volume and pan retimed.
 *
 * @param def Animation definition containing the sound keyframes.
 * @param anim Animation node whose sound is updated.
 */
static void field_update_animation_sfx(FieldAnimDef* def, FieldAnim* anim)
{
    FieldPart* part;
    FieldObj* object;
    FieldSfxKey* keyframe;
    s32 sfx_id;
    s32 channel_mask;
    s32 key_type;
    s32 camera_screen_y;
    s32 grid_y_offset;
    s32 x;
    s32 y;
    u32 pan;
    u32 volume;
    u32 adjusted_value;
    s32 grid_x_offset;
    s32 camera_component;
    s32 object_y;
    s32 keyframe_offset;
    FieldPartDef* part_definition;

    part = anim->cels;
    object = anim->owner.object;
    keyframe_offset = anim->flags.b.state * 8;
    key_type = def->data[keyframe_offset] & FIELD_SFX_KEY_KIND_MASK;
    if (key_type == FIELD_SFX_KEY_SOUND)
    {
        keyframe = (FieldSfxKey*)(def->data + anim->flags.b.state * 8);
        if (keyframe->control.word & FIELD_SFX_CHANNEL_MASK)
        {
            channel_mask = key_type << (((keyframe->control.word >> 8) & 0x1F) - 1);
            sfx_id = 0;
        }
        else
        {
            channel_mask = 0;
            sfx_id = keyframe->sfx_id & FIELD_SFX_ID_MASK;
        }
        if (keyframe->control.word & FIELD_SFX_PLAY)
        {
            if (keyframe->control.word & FIELD_SFX_FIXED_PAN)
            {
                if (keyframe->sound.word & FIELD_SFX_ONE_SHOT)
                {
                    if (anim->flags.word & FIELD_ANIM_FLAG_START_PENDING)
                    {
                        akao_play_sfx(sfx_id, channel_mask, keyframe->sound.b.lo, FIELD_SFX_VOLUME(keyframe));
                        anim->flags.word &= ~FIELD_ANIM_FLAG_START_PENDING;
                    }
                }
                else
                {
                    akao_play_sfx(sfx_id, channel_mask, keyframe->sound.b.lo, FIELD_SFX_VOLUME(keyframe));
                }
            }
            else
            {
                if (object->def->flags.word & FIELD_OBJ_DEF_SCREEN_FIXED)
                {
                    x = 0;
                    camera_screen_y = 0;
                    y = 0;
                }
                else
                {
                    x = SCENE_STATE->camera_x;
                    camera_screen_y = SCENE_STATE->camera_y;
                    y = SCENE_STATE->camera_z;
                }
                if (x >= 0)
                {
                    camera_component = x >> 8;
                }
                else
                {
                    camera_component = (x + 0xFF) >> 8;
                }
                x = camera_component;
                if (camera_screen_y >= 0)
                {
                    camera_screen_y = camera_screen_y >> 8;
                }
                else
                {
                    camera_screen_y = (camera_screen_y + 0xFF) >> 8;
                }
                if (y >= 0)
                {
                    camera_component = y >> 9;
                    camera_screen_y = camera_screen_y - camera_component;
                }
                else
                {
                    camera_component = (y + 0x1FF) >> 9;
                    camera_screen_y = camera_screen_y - camera_component;
                }
                y = camera_screen_y;
                {
                    s32 position;
                    s32 mid;
                    s32 columns;
                    FieldPartDef* geometry_definition;

                    geometry_definition = part->def;
                    columns = geometry_definition->u.b.cols;
                    position = object->x + part->x;
                    grid_x_offset = columns * 8;
                    /* Without both wrappers and the volatile reload the reload, load order and temporaries change. */
                    do
                    {
                        mid = x + position / 256;
                    } while (0);
                    x = mid + grid_x_offset;
                    do
                    {
                        part_definition = *(FieldPartDef* volatile*)&part->def;
                    } while (0);
                }
                object_y = object->y;
                {
                    s32 coordinate;
                    s32 part_value;
                    s32 y_sum;
                    s32 mid;

                    part_value = part_definition->u.b.rows;
                    coordinate = part->y;
                    volume = part_value * 8;
                    y_sum = object_y + coordinate;
                    coordinate = object->z;
                    part_value = part->z;
                    mid = y + (y_sum * 2 - (coordinate + part_value)) / 512;
                    grid_y_offset = volume - 0xE0;
                    y = mid - grid_y_offset;
                }
                if (x < -0x20)
                {
                    pan = (-0x20 - x) >> 2;
                    if (pan < 0x3F)
                    {
                        pan = 0x3F - pan;
                    }
                    else
                    {
                        pan = 0;
                    }
                }
                else if (x > 0x160)
                {
                    pan = (x - 0x160) >> 2;
                    adjusted_value = pan + 0xC0;
                    if (adjusted_value < 0x100)
                    {
                        pan = adjusted_value;
                    }
                    else
                    {
                        pan = 0xFF;
                    }
                }
                else
                {
                    pan = ((x + 0x20) * 0x7F) / 384 + 0x40;
                }
                if (y < -0x20)
                {
                    volume = (-0x20 - y) >> 2;
                    adjusted_value = FIELD_SFX_VOLUME(keyframe);
                    if (volume < adjusted_value)
                    {
                        volume = adjusted_value - volume;
                    }
                    else
                    {
                        volume = 0;
                    }
                }
                else if (y > 0x100)
                {
                    volume = (y - 0x100) >> 2;
                    adjusted_value = FIELD_SFX_VOLUME(keyframe);
                    if (volume < adjusted_value)
                    {
                        volume = adjusted_value - volume;
                    }
                    else
                    {
                        volume = 0;
                    }
                }
                else
                {
                    volume = FIELD_SFX_VOLUME(keyframe);
                }
                if (keyframe->sound.word & FIELD_SFX_ONE_SHOT)
                {
                    if (anim->flags.word & FIELD_ANIM_FLAG_START_PENDING)
                    {
                        akao_play_sfx(sfx_id, channel_mask, pan, volume);
                        anim->flags.word &= ~FIELD_ANIM_FLAG_START_PENDING;
                    }
                    else
                    {
                        akao_cmd_a1(sfx_id, channel_mask, anim->timer * 2, volume);
                        akao_cmd_a3(sfx_id, channel_mask, anim->timer * 2, pan);
                    }
                }
                else
                {
                    akao_play_sfx(sfx_id, channel_mask, pan, volume);
                }
            }
        }
        else
        {
            akao_cmd_21(sfx_id, channel_mask);
        }
    }
}

/**
 * @brief Retarget a cel's CLUT references to an animation frame.
 * @param anim_def Animation definition describing the CLUT band.
 * @param cel Cel whose tile records are updated.
 * @param frame Frame index selecting the CLUT band.
 */
static void field_retarget_cel_cluts(FieldAnimDef* anim_def, FieldPart* cel, s32 frame)
{
    FieldAnimDef* definition;
    FieldPartDef* grid;
    FieldTileDesc* tile;
    u8* record_cursor;
    s16* clut_cursor;
    u32* mask_cursor;
    u32 mask_word;
    u32 mask_bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 clut_slot;
    s32 y;
    s32 first_slot;
    s32 last_slot;
    s32 packing_mode;
    s16 clut;

    definition = anim_def;
    grid = cel->def;
    stride = 0;
    if (definition->u.clut.clut_mode == ((grid->u.word >> 4) & 3))
    {
        tile = grid->tiles;
        record_cursor = cel->records;
        switch (cel->kind)
        {
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
            stride = FIELD_CEL_RECORD_SIZE;
            break;
        case 1:
        case 6:
            break;
        }
        if (cel->code_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        mask_bit = 1;
        if (cel->tpage_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        mask_cursor = cel->bits;
        first_slot = definition->u.clut.clut_slot;
        last_slot = first_slot + definition->u.clut.length;
        packing_mode = definition->u.clut.clut_mode;
        mask_word = *mask_cursor++;
        for (row = 0; row != grid->u.b.rows; row++)
        {
            col = 0;
            if (grid->u.b.cols != 0)
            {
                clut_cursor = (s16*)(record_cursor + 2);
                do
                {
                    if (mask_word & mask_bit)
                    {
                        u8 packed = tile->clut_slot;

                        if (packed & FIELD_TILE_ANIMATED)
                        {
                            clut_slot = packed & FIELD_TILE_CLUT_MASK;
                            if ((clut_slot >= first_slot) && (clut_slot < last_slot))
                            {
                                y = clut_slot + (frame * definition->u.clut.length);
                                if (packing_mode == 0)
                                {
                                    clut = getClut(FIELD_TILE_4BIT_CLUT_X(y), (y >> 4) + FIELD_TILE_CLUT_VRAM_Y);
                                }
                                else
                                {
                                    clut = getClut(0, y + FIELD_TILE_CLUT_VRAM_Y);
                                }
                                *clut_cursor = clut;
                            }
                        }
                        clut_cursor = (s16*)((u8*)clut_cursor + stride);
                        record_cursor += stride;
                    }
                    mask_bit <<= 1;
                    if (mask_bit == 0)
                    {
                        mask_word = *mask_cursor++;
                        mask_bit = 1;
                    }
                    tile++;
                    col++;
                } while (col != grid->u.b.cols);
            }
        }
    }
}

/**
 * @brief Blend the current animation frame with its neighboring frame.
 * @param def Animation definition describing the source frames.
 * @param anim Animation node supplying timing and scratch storage.
 * @return Pointer to the blended scratch pixel buffer.
 */
static u_long* field_blend_animation_frames(FieldAnimDef* def, FieldAnim* anim)
{
    FieldAnimDef* definition_copy;
    FieldSceneHeader* header;
    u16* current_pixels;
    u16* other_pixels;
    u16* destination;
    u16* output;
    s32 duration;
    s32 remaining;
    s32 elapsed;
    s32 keyframe_index;
    s32 other_frame;
    s32 pixel_count;
    s32 control_flags;
    u16 current_pixel;
    u16 other_pixel;
    u8 range_start;

    definition_copy = def;
    header = g_field_scene.scene->header;
    duration = field_find_count_table_span(definition_copy, anim->flags.b.keyframe, &range_start)->duration;
    keyframe_index = anim->flags.b.keyframe;
    remaining = anim->timer;
    control_flags = anim->flags.word;
    elapsed = duration - remaining;
    if (control_flags & 1)
    {
        if (control_flags & 4)
        {
            if (keyframe_index == 0)
            {
                keyframe_index = 1;
            }
            else
            {
                keyframe_index = keyframe_index - 1;
            }
        }
        else if (keyframe_index == definition_copy->flags.b.last_frame)
        {
            keyframe_index = keyframe_index - 1;
        }
        else
        {
            keyframe_index = keyframe_index + 1;
        }
    }
    else
    {
        if (keyframe_index == definition_copy->flags.b.last_frame)
        {
            keyframe_index = 0;
        }
        else
        {
            keyframe_index = keyframe_index + 1;
        }
    }
    if (def->flags.word & FIELD_ANIM_DEF_SPAN_INDEXED)
    {
        other_frame = (field_find_count_table_span(def, keyframe_index, &range_start)->range_start + keyframe_index) - range_start;
    }
    else
    {
        other_frame = keyframe_index;
    }
    if (definition_copy->u.clut.clut_mode == 0)
    {
        pixel_count = definition_copy->u.clut.length * FIELD_CLUT_4BIT_COLORS;
        if (anim->flags.b.state == 0)
        {
            current_pixels = header->pixel_data + definition_copy->u.clut.clut_slot * FIELD_CLUT_4BIT_COLORS;
        }
        else
        {
            current_pixels = header->pixel_data + header->pixel_stride + definition_copy->u.clut.pixel_offset + (anim->flags.b.state - 1) * pixel_count;
        }
        if (other_frame == 0)
        {
            other_pixels = header->pixel_data + definition_copy->u.clut.clut_slot * FIELD_CLUT_4BIT_COLORS;
        }
        else
        {
            other_pixels = header->pixel_data + header->pixel_stride + definition_copy->u.clut.pixel_offset + (other_frame - 1) * pixel_count;
        }
    }
    else
    {
        pixel_count = definition_copy->u.clut.length * FIELD_CLUT_8BIT_COLORS;
        if (anim->flags.b.state == 0)
        {
            current_pixels = header->pixel_data + definition_copy->u.clut.clut_slot * FIELD_CLUT_8BIT_COLORS;
        }
        else
        {
            current_pixels = header->pixel_data + header->pixel_stride + definition_copy->u.clut.pixel_offset + (anim->flags.b.state - 1) * pixel_count;
        }
        if (other_frame == 0)
        {
            other_pixels = header->pixel_data + definition_copy->u.clut.clut_slot * FIELD_CLUT_8BIT_COLORS;
        }
        else
        {
            other_pixels = header->pixel_data + header->pixel_stride + definition_copy->u.clut.pixel_offset + (other_frame - 1) * pixel_count;
        }
    }
    control_flags = anim->flags.word;
    if (control_flags & FIELD_ANIM_FLAG_SECOND_BUFFER)
    {
        destination = &anim->scratch_pixels[pixel_count];
        anim->flags.word = control_flags & ~FIELD_ANIM_FLAG_SECOND_BUFFER;
    }
    else
    {
        destination = anim->scratch_pixels;
        anim->flags.word = control_flags | FIELD_ANIM_FLAG_SECOND_BUFFER;
    }
    pixel_count--;
    output = destination;
    while (pixel_count != -1)
    {
        current_pixel = *current_pixels++;
        other_pixel = *other_pixels++;
        if ((elapsed == 0) || (current_pixel == other_pixel))
        {
            *destination++ = current_pixel;
        }
        else
        {
            *destination++ = ((current_pixel | other_pixel) & FIELD_COLOR_STP) | ((((current_pixel & FIELD_COLOR_COMPONENT) * remaining) + ((other_pixel & FIELD_COLOR_COMPONENT) * elapsed)) / duration) |
                   (((((current_pixel >> 5) & FIELD_COLOR_COMPONENT) * remaining) + (((other_pixel >> 5) & FIELD_COLOR_COMPONENT) * elapsed)) / duration) << 5 |
                   (((((current_pixel >> 10) & FIELD_COLOR_COMPONENT) * remaining) + (((other_pixel >> 10) & FIELD_COLOR_COMPONENT) * elapsed)) / duration) << 10;
        }
        pixel_count--;
    }
    return (u_long*)output;
}

/**
 * @brief Apply a tint-table shade to one cel.
 * @param def Animation definition selecting the tintable color range.
 * @param cel Cel whose visible tile colors are updated.
 * @param src Tint source and palette.
 * @param shade Tint-table shade offset.
 */
void field_tint_animation_cel(FieldAnimDef* def, FieldPart* cel, FieldTintSrc* src, s32 shade)
{
    FieldPartDef* grid;
    FieldTileDesc* tile;
    FieldTintColor* palette;
    FieldTintColor* color_entry;
    u8* record_cursor;
    u16* palette_data;
    u32* mask_cursor;
    u32 mask_word;
    u32 mask_bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 first_slot;
    s32 last_slot;
    s32 color_slot;
    s32 shared_code;
    s32 rgb[3];

    rgb[0] = src->red * src->red_scale;
    rgb[1] = src->green * src->green_scale;
    rgb[2] = src->blue * src->blue_scale;
    stride = 0;
    palette = FIELD_TINT_COLORS;
    palette_data = src->palette->data;
    field_build_tint_colors((u8*)(palette_data + 2), palette_data[0], rgb);
    grid = cel->def;
    record_cursor = cel->records;
    tile = grid->tiles;
    switch (cel->kind)
    {
    case 0:
    case 2:
    case 3:
    case 4:
    case 5:
        stride = FIELD_CEL_RECORD_SIZE;
        break;
    case 1:
    case 6:
        break;
    }
    shared_code = cel->code_word;
    if (shared_code != 0)
    {
        stride -= FIELD_CEL_SHARED_WORD_SIZE;
    }
    if (cel->tpage_word != 0)
    {
        stride -= FIELD_CEL_SHARED_WORD_SIZE;
    }
    mask_bit = 1;
    mask_cursor = cel->bits;
    first_slot = def->u.tint.first_slot;
    last_slot = first_slot + def->u.tint.slot_count;
    mask_word = *mask_cursor++;
    for (row = 0; row != grid->u.b.rows; row++)
    {
        for (col = 0; col != grid->u.b.cols; col++)
        {
            if (mask_word & mask_bit)
            {
                if (tile->clut_slot & FIELD_TILE_ANIMATED)
                {
                    color_slot = tile->color_index;
                    if ((color_slot >= first_slot) && (last_slot >= color_slot))
                    {
                        color_entry = &palette[color_slot] + shade;
                        if (shared_code != 0)
                        {
                            ((FieldTintColor*)&cel->code_word)->rg = color_entry->rg;
                            ((FieldTintColor*)&cel->code_word)->b = color_entry->b;
                            return;
                        }
                        ((FieldCellTint*)record_cursor)->rg = color_entry->rg;
                        ((FieldCellTint*)record_cursor)->b = color_entry->b;
                    }
                    else
                    {
                        if (shared_code != 0)
                        {
                            return;
                        }
                    }
                }
                record_cursor += stride;
            }
            mask_bit <<= 1;
            if (mask_bit == 0)
            {
                mask_word = *mask_cursor++;
                mask_bit = 1;
            }
            tile++;
        }
    }
}

/**
 * @brief Apply a tint-table shade to every cel in a tint source.
 * @param def Animation definition selecting the tintable color range.
 * @param src Tint source whose cel list and palette are used.
 * @param shade Tint-table shade offset.
 */
static void field_tint_animation_cel_list(FieldAnimDef* def, FieldTintSrc* src, s32 shade)
{
    FieldPart* cel;
    FieldPartDef* grid;
    FieldTileDesc* tile;
    FieldTintColor* palette;
    FieldTintColor* color_entry;
    u8* record_cursor;
    u16* palette_data;
    u32* mask_cursor;
    u32 mask_word;
    u32 mask_bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 first_slot;
    s32 last_slot;
    s32 color_slot;
    s32 shared_code;
    s32 rgb[3];

    rgb[0] = src->red * src->red_scale;
    rgb[1] = src->green * src->green_scale;
    rgb[2] = src->blue * src->blue_scale;
    stride = 0;
    palette = FIELD_TINT_COLORS;
    palette_data = src->palette->data;
    field_build_tint_colors((u8*)(palette_data + 2), palette_data[0], rgb);
    first_slot = def->u.tint.first_slot;
    last_slot = first_slot + def->u.tint.slot_count;
    for (cel = src->cels; cel != NULL; cel = cel->next)
    {
        record_cursor = cel->records;
        grid = cel->def;
        tile = grid->tiles;
        switch (cel->kind)
        {
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
            stride = FIELD_CEL_RECORD_SIZE;
            break;
        case 1:
        case 6:
            break;
        }
        shared_code = cel->code_word;
        if (shared_code != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        if (cel->tpage_word != 0)
        {
            stride -= FIELD_CEL_SHARED_WORD_SIZE;
        }
        mask_bit = 1;
        mask_cursor = cel->bits;
        mask_word = *mask_cursor++;
        for (row = 0; row != grid->u.b.rows; row++)
        {
            for (col = 0; col != grid->u.b.cols; col++)
            {
                if (mask_word & mask_bit)
                {
                    if (tile->clut_slot & FIELD_TILE_ANIMATED)
                    {
                        color_slot = tile->color_index;
                        if ((color_slot >= first_slot) && (last_slot >= color_slot))
                        {
                            color_entry = &palette[color_slot] + shade;
                            if (shared_code != 0)
                            {
                                /* A cel with a shared colour word takes the first animated tile's colour. */
                                ((FieldTintColor*)&cel->code_word)->rg = color_entry->rg;
                                ((FieldTintColor*)&cel->code_word)->b = color_entry->b;
                                goto next_cel;
                            }
                            ((FieldCellTint*)record_cursor)->rg = color_entry->rg;
                            ((FieldCellTint*)record_cursor)->b = color_entry->b;
                        }
                        else
                        {
                            if (shared_code != 0)
                            {
                                goto next_cel;
                            }
                        }
                    }
                    record_cursor += stride;
                }
                mask_bit <<= 1;
                if (mask_bit == 0)
                {
                    mask_word = *mask_cursor++;
                    mask_bit = 1;
                }
                tile++;
            }
        }
    next_cel:;
    }
}

/**
 * @brief Advance an animation's keyframe cursor and reload its timer.
 * @param def Animation definition controlling playback and repeat behavior.
 * @param anim Animation node to advance.
 */
static void field_advance_animation_keyframe(FieldAnimDef* def, FieldAnim* anim)
{
    FieldTweenSpan* keyframe_span;
    s32 current_keyframe;
    s32 next_keyframe;
    u8 keyframe;
    u8 range_start;

    if (!(anim->flags.word & FIELD_ANIM_FLAG_START_PENDING))
    {
        if (anim->flags.word & FIELD_ANIM_FLAG_PING_PONG)
        {
            if (anim->flags.word & FIELD_ANIM_FLAG_REVERSE)
            {
                if (anim->flags.b.keyframe == 0)
                {
                    if (def->flags.word & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT)
                    {
                        anim->flags.b.keyframe = anim->flags.b.keyframe + 1;
                    }
                    else if (((def->flags.b.handler_group == FIELD_ANIM_GROUP_TILE) && FIELD_ANIM_IS_TWEEN(def)) || ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) == FIELD_ANIM_PALETTE_BLEND_WORD))
                    {
                        if (anim->repeat_count == 0)
                        {
                            anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                        }
                        else
                        {
                            anim->repeat_count--;
                            anim->flags.b.keyframe++;
                        }
                    }
                    anim->flags.word &= ~FIELD_ANIM_FLAG_REVERSE;
                }
                else
                {
                    anim->flags.b.keyframe = anim->flags.b.keyframe - 1;
                    if (!(def->flags.word & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) && (anim->flags.b.keyframe == 0) &&
                        (((def->flags.b.handler_group == FIELD_ANIM_GROUP_TILE) && !FIELD_ANIM_IS_TWEEN(def)) ||
                         ((def->flags.b.handler_group == FIELD_ANIM_GROUP_PALETTE) && (FIELD_ANIM_KIND(def) != FIELD_PALETTE_ANIM_BLEND)) || (def->flags.b.handler_group > FIELD_ANIM_GROUP_PALETTE)))
                    {
                        if (anim->repeat_count == 0)
                        {
                            anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                        }
                        else
                        {
                            anim->repeat_count--;
                        }
                    }
                }
            }
            else
            {
                keyframe = anim->flags.b.keyframe;
                if (keyframe == def->flags.b.last_frame)
                {
                    if (!(def->flags.word & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) && (keyframe == 0))
                    {
                        anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                    }
                    else
                    {
                        anim->flags.b.keyframe--;
                        anim->flags.word |= FIELD_ANIM_FLAG_REVERSE;
                    }
                }
                else
                {
                    anim->flags.b.keyframe = keyframe + 1;
                }
            }
        }
        else
        {
            current_keyframe = anim->flags.b.keyframe;
            if (current_keyframe == def->flags.b.last_frame)
            {
                if (!(def->flags.word & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) &&
                    (((def->flags.b.handler_group == FIELD_ANIM_GROUP_TILE) && FIELD_ANIM_IS_TWEEN(def)) || ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) == FIELD_ANIM_PALETTE_BLEND_WORD)))
                {
                    if (anim->repeat_count == 0)
                    {
                        anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                    }
                    else
                    {
                        anim->repeat_count--;
                    }
                }
                anim->flags.b.keyframe = 0;
            }
            else
            {
                next_keyframe = current_keyframe + 1;
                anim->flags.b.keyframe = next_keyframe;
                if (!(def->flags.word & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) && ((u8)next_keyframe == def->flags.b.last_frame) &&
                    (((def->flags.b.handler_group == FIELD_ANIM_GROUP_TILE) && !FIELD_ANIM_IS_TWEEN(def)) ||
                     ((def->flags.b.handler_group == FIELD_ANIM_GROUP_PALETTE) && (FIELD_ANIM_KIND(def) != FIELD_PALETTE_ANIM_BLEND)) || (def->flags.b.handler_group > FIELD_ANIM_GROUP_PALETTE)))
                {
                    if (anim->repeat_count == 0)
                    {
                        anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                    }
                    else
                    {
                        anim->repeat_count--;
                    }
                }
            }
        }
        if ((anim->flags.word & FIELD_ANIM_FLAG_ACTIVE) && (anim->repeat_count == 0) && (anim->flags.word & FIELD_ANIM_FLAG_STOP_AT_KEYFRAME) && (anim->flags.b.stop_keyframe == anim->flags.b.keyframe))
        {
            anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
        }
    }
    else
    {
        anim->flags.word &= ~FIELD_ANIM_FLAG_START_PENDING;
    }
    keyframe_span = field_find_count_table_span(def, anim->flags.b.keyframe, &range_start);
    anim->timer = keyframe_span->duration;
    if (def->flags.word & FIELD_ANIM_DEF_SPAN_INDEXED)
    {
        anim->flags.b.state = (keyframe_span->range_start + anim->flags.b.keyframe) - range_start;
    }
    else
    {
        anim->flags.b.state = anim->flags.b.keyframe;
    }
}

/**
 * @brief Retarget every cel in a tint source to an animation frame's CLUTs.
 * @param def Animation definition describing the CLUT band.
 * @param src Tint source whose cel list is updated.
 * @param frame Frame index selecting the CLUT band.
 */
static void field_retarget_cel_list_cluts(FieldAnimDef* def, FieldTintSrc* src, s32 frame)
{
    FieldPart* cel;

    for (cel = src->cels; cel != NULL; cel = cel->next)
    {
        field_retarget_cel_cluts(def, cel, frame);
    }
}

/**
 * @brief Find the keyframe span that covers a keyframe index.
 * @param def Animation definition; its head is the first span of the table.
 * @param keyframe Keyframe index to look up.
 * @param range_start_out Receives the number of keyframes before the returned span.
 * @return Span covering @p keyframe.
 * @note The later spans follow the definition, whose size is chosen per
 *       handler group (all four groups use the same 0x18-byte record).
 */
FieldTweenSpan* field_find_count_table_span(FieldAnimDef* def, s32 keyframe, u8* range_start_out)
{
    /* The table starts with the definition's own head span; the rest follow the record. */
    union
    {
        FieldAnimDef* def;
        FieldTweenSpan* span;
    } table;
    u8 header_count;
    u8 count;
    u8 range_start;
    s32 range_end;

    table.def = def;
    *range_start_out = 0;
    header_count = table.span->count & FIELD_SPAN_COUNT_MASK;
    if (keyframe >= header_count)
    {
        *range_start_out = header_count;
        switch (table.def->flags.b.handler_group)
        {
        case 0:
            table.def++;
            break;
        case 1:
            table.def++;
            break;
        case 2:
            table.def++;
            break;
        default:
            table.def++;
            break;
        }
        count = table.span->count;
        range_start = *range_start_out;
        range_end = (u8)(range_start + (count & FIELD_SPAN_COUNT_MASK));
        while (keyframe >= range_end)
        {
            table.span++;
            range_start += count & FIELD_SPAN_COUNT_MASK;
            *range_start_out = range_start;
            count = table.span->count;
            range_end = (u8)(range_start + (count & FIELD_SPAN_COUNT_MASK));
        }
    }
    return table.span;
}

/**
 * @brief Push a VRAM upload request onto the scene's pending-upload list.
 * @param req Request to link at the head of the pending list.
 */
void field_queue_vram_upload(FieldImageReq* req)
{
    FieldScene* scene;

    scene = g_field_scene.scene;
    req->next = (FieldImageReq*)scene->uploads;
    scene->uploads = req;
}
