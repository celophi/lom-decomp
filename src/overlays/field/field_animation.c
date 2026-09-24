#include "movie.h"
#include "field_animation.h"
#include "field_scene_internal.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "controller.h"
#include "scene_state.h"

#define FIELD_MOVIE_STATE ((volatile FieldMovieState*)0x801ED500)
#define FIELD_CD_FLAGS_ADDRESS 0x801ED800U
#define FIELD_CD_FLAG_MOVIE_STREAM 0x40

typedef struct
{
    u32 word;
} FieldCdFlags;

extern u16 g_field_movie_frame_width;
extern u16 g_field_movie_frame_height;

void func_8005A744(FieldSeq*, u8);
s32 func_8005A84C(s32, s32);
void func_80084240(void);
void func_80140358(s32, s32, s32, s32);
void func_801406E4(void);
void func_8005A984(FieldPart*, s32, s32);
void func_8005AA68(FieldObj*, s32, s32);
void akao_play_sfx(s32, s32, s32, s32);
void akao_cmd_21(s32, s32);
void akao_cmd_a1(s32, s32, s32, s32);
void akao_cmd_a3(s32, s32, s32, s32);

/**
 * @brief Advance the field scene's animation-related runtime lists by one frame.
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
    FieldAnimCel* cel;
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
    s32 rect_extent;
    s32 single_row_height;

    scene = g_field_scene.scene;

    object = scene->objects;
    if (object != NULL)
    {
        do
        {
            part = object->parts;
            if (part != NULL)
            {
                do
                {
                    if (part->def->u.word & 0xF000)
                    {
                        sweep_mode = (part->def->u.word >> 12) & 0xF;
                        if ((sweep_mode != 0) && (sweep_mode < 5))
                        {
                            field_update_part_sweep(part);
                        }
                    }
                    part = part->next;
                } while (part != NULL);
            }
            object = object->next;
        } while (object != NULL);
    }

    anim = scene->anims;
    header = scene->header;
    if (anim != NULL)
    {
        do
        {
            definition = anim->def;
            frame_definition = anim->def;
            if (anim->flags.word & FIELD_ANIM_FLAG_UPLOAD_PENDING)
            {
                upload = &anim->upload;
                if ((*(s32*)&definition->flags & FIELD_ANIM_KIND_MASK) == 3)
                {
                    upload->rect.x = definition->unkC * 4 + FIELD_TILE_LOWER_BANK_VRAM_X;
                    upload->rect.y = definition->unkD * 0x10 + FIELD_TILE_LOWER_BANK_VRAM_Y;
                    upload->rect.w = definition->unkE * 4;
                    upload->rect.h = definition->unkF * 0x10;
                    upload->data = (u_long*)(definition->data + ((anim->flags.b.state * definition->unkE * definition->unkF) << 7));
                    field_queue_vram_upload(upload);
                }
                anim->flags.word &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
            }
            if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
            {
                anim->timer--;
                switch (definition->flags & FIELD_ANIM_KIND_MASK)
                {
                case 4:
                    switch (anim->flags.b.state)
                    {
                    case 0:
                        if (cdrom_process_state() == 0)
                        {
                            cdrom_stream(CD_RES_MOVIE_BIN, (void*)0x80140000);
                            cdrom_queue_seek(definition->unk1 * 2 + 0x16A6);
                            anim->flags.b.state = 1;
                            /* The XOR hides the constant from loop.c; a plain address is hoisted into a saved register. */
                            (*(volatile s32*)((u32)anim ^ ((u32)anim ^ FIELD_CD_FLAGS_ADDRESS))) |= FIELD_CD_FLAG_MOVIE_STREAM;
                        }
                        /* fallthrough */
                    case 1:
                        if (cdrom_can_queue_resource(definition->unk1 * 2 + 0x16A6) != 0)
                        {
                            /* Plain store: FIELD_MOVIE_STATE here schedules differently. */
                            ((FieldMovieState*)0x801ED500)->rects[0].x = frame_definition->unkC * 4 + FIELD_TILE_LOWER_BANK_VRAM_X;
                            FIELD_MOVIE_STATE->rects[0].y = frame_definition->unkD * 0x10 + FIELD_TILE_LOWER_BANK_VRAM_Y;
                            FIELD_MOVIE_STATE->rects[0].w = frame_definition->unkE * 4;
                            FIELD_MOVIE_STATE->rects[0].h = frame_definition->unkF * 0x10;
                            cel = anim->cels;
                            ((volatile FieldCdFlags*)FIELD_CD_FLAGS_ADDRESS)->word &= ~FIELD_CD_FLAG_MOVIE_STREAM;
                            if (definition->unk1 < 2)
                            {
                                func_80140358(definition->unk1 * 2 + 0x16A6, 1, definition->unk5 - 2, cel->active);
                            }
                            else
                            {
                                func_80140358(definition->unk1 * 2 + 0x16A6, 1, 0x12E, 0);
                            }
                            anim->flags.b.state = 2;
                        }
                        anim->timer = 1;
                        break;
                    default:
                        if (FIELD_MOVIE_STATE->end_state >= 3)
                        {
                            if (FIELD_MOVIE_STATE->end_state == 3)
                            {
                                if (cdrom_can_queue_resource(definition->unk1 * 2 + 0x16A7) != 0)
                                {
                                    upload = &anim->upload;
                                    if (definition->unk1 < 2)
                                    {
                                        cel = anim->cels;
                                        upload->rect.x = FIELD_MOVIE_STATE->rects[cel->active].x;
                                        upload->rect.y = FIELD_MOVIE_STATE->rects[cel->active].y;
                                        upload->rect.w = FIELD_MOVIE_STATE->rects[cel->active].w;
                                        upload->rect.h = FIELD_MOVIE_STATE->rects[cel->active].h;
                                        upload->data = (u_long*)0x80140000;
                                        field_queue_vram_upload(upload);
                                        if (cel->active == 1)
                                        {
                                            cel->active = 0;
                                            cel = cel->next;
                                            cel->active = 1;
                                        }
                                        else
                                        {
                                            cel->active = 1;
                                            cel = cel->next;
                                            cel->active = 0;
                                        }
                                    }
                                    else
                                    {
                                        cel = anim->cels;
                                        upload->rect.x = 0x140;
                                        upload->rect.y = 0x100;
                                        upload->data = (u_long*)0x80140004;
                                        upload->rect.w = g_field_movie_frame_width;
                                        upload->rect.h = g_field_movie_frame_height;
                                        field_queue_vram_upload(upload);
                                        cel->active = 0;
                                        cel = cel->next;
                                        cel->active = 0;
                                    }
                                    FIELD_MOVIE_STATE->end_state = 4;
                                }
                            }
                            else
                            {
                                if (definition->unk1 >= 2)
                                {
                                    field_begin_scene_fade_in();
                                }
                                anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
                                func_80084240();
                            }
                        }
                        else
                        {
                            if (definition->unk1 < 2)
                            {
                                set_controller_vsync_interval(2);
                            }
                            func_801406E4();
                            movie_service_video_ops();
                            if (FIELD_MOVIE_STATE->frame_ready == 1)
                            {
                                cel = anim->cels;
                                if (FIELD_MOVIE_STATE->chunk_idx == 1)
                                {
                                    cel->active = 1;
                                    cel = cel->next;
                                    cel->active = 0;
                                }
                                else
                                {
                                    cel->active = 0;
                                    cel = cel->next;
                                    cel->active = 1;
                                }
                                FIELD_MOVIE_STATE->frame_ready = 0;
                            }
                            if (FIELD_MOVIE_STATE->end_state == 2)
                            {
                                cdrom_reset();
                                cdrom_queue_read(definition->unk1 * 2 + 0x16A7, (void*)0x80140000);
                                FIELD_MOVIE_STATE->end_state = 3;
                            }
                        }
                        anim->timer = 1;
                        break;
                    }
                    break;
                case 5:
                case 6:
                    field_apply_animation_tween(definition, anim, 1);
                    break;
                }
                if (anim->timer == 0)
                {
                    previous_frame = anim->flags.b.state;
                    field_advance_animation_keyframe(definition, anim);
                    switch (definition->flags & FIELD_ANIM_KIND_MASK)
                    {
                    case 0:
                        if (previous_frame != anim->flags.b.state)
                        {
                            field_blit_animation_frame(definition, anim, anim->flags.b.state);
                        }
                        break;
                    case 2:
                        cel = anim->cels;
                        index = previous_frame - 1;
                        while (index != -1)
                        {
                            cel = cel->next;
                            index--;
                        }
                        cel->active = 0;
                        cel = anim->cels;
                        index = anim->flags.b.state;
                        index--;
                        while (index != -1)
                        {
                            cel = cel->next;
                            index--;
                        }
                        cel->active = 1;
                        break;
                    case 3:
                        upload = &anim->upload;
                        upload->rect.x = frame_definition->unkC * 4 + FIELD_TILE_LOWER_BANK_VRAM_X;
                        upload->rect.y = frame_definition->unkD * 0x10 + FIELD_TILE_LOWER_BANK_VRAM_Y;
                        upload->rect.w = frame_definition->unkE * 4;
                        upload->rect.h = frame_definition->unkF * 0x10;
                        upload->data = (u_long*)(frame_definition->data + ((anim->flags.b.state * frame_definition->unkE * frame_definition->unkF) << 7));
                        field_queue_vram_upload(upload);
                        break;
                    case 5:
                    case 6:
                        while (anim->timer == 0)
                        {
                            field_apply_animation_tween(definition, anim, 1);
                            field_advance_animation_keyframe(definition, anim);
                        }
                        break;
                    case 7:
                        field_update_animation_sfx(definition, anim);
                        break;
                    }
                }
            }
            anim = anim->next;
        } while (anim != NULL);
    }

    anim = scene->strips;
    if (anim != NULL)
    {
        single_row_height = 1;
        do
        {
            definition = anim->def;
            control_flags = anim->flags.word;
            strip_definition = anim->def;
            if (control_flags & FIELD_ANIM_FLAG_UPLOAD_PENDING)
            {
                upload = &anim->upload;
                switch (definition->flags & FIELD_ANIM_KIND_MASK)
                {
                case 2:
                    if (control_flags & FIELD_ANIM_FLAG_SECOND_BUFFER)
                    {
                        if (definition->unkC == 0)
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
                        if (strip_definition->unkD != 0)
                        {
                            previous_frame_index = anim->flags.b.state - 1;
                            wrap_count = (definition->unk5 - previous_frame_index) * strip_definition->unk10;
                            copy_count = anim->flags.b.state * strip_definition->unk10;
                        }
                        else
                        {
                            wrap_count = anim->flags.b.state * strip_definition->unk10;
                            copy_count = (definition->unk5 + 1 - anim->flags.b.state) * strip_definition->unk10;
                        }
                    }
                    else
                    {
                        copy_count = (definition->unk5 + 1) * strip_definition->unk10;
                    }
                    if (strip_definition->unkC == 0)
                    {
                        source_pixels = (u16*)((u8*)header->pixel_data + (strip_definition->unkE << 5) + strip_definition->unkF * 2 + wrap_count * 2);
                    }
                    else
                    {
                        source_pixels = (u16*)((u8*)header->pixel_data + (strip_definition->unkE << 9) + strip_definition->unkF * 2 + wrap_count * 2);
                    }
                    copy_count--;
                    while (copy_count != -1)
                    {
                        *destination_pixels++ = *source_pixels++;
                        copy_count--;
                    }
                    if (wrap_count != 0)
                    {
                        if (strip_definition->unkC == 0)
                        {
                            source_pixels = (u16*)((u8*)header->pixel_data + (strip_definition->unkE << 5) + strip_definition->unkF * 2);
                        }
                        else
                        {
                            source_pixels = (u16*)((u8*)header->pixel_data + (strip_definition->unkE << 9) + strip_definition->unkF * 2);
                        }
                        copy_count = wrap_count - 1;
                        while (copy_count != -1)
                        {
                            *destination_pixels++ = *source_pixels++;
                            copy_count--;
                        }
                    }
                    if (strip_definition->unkC == 0)
                    {
                        upload->rect.x = strip_definition->unkF + ((strip_definition->unkE & 0xF) * 0x10);
                        rect_extent = strip_definition->unkE >> 4;
                    }
                    else
                    {
                        upload->rect.x = strip_definition->unkF;
                        rect_extent = strip_definition->unkE;
                    }
                    upload->rect.y = rect_extent + FIELD_TILE_CLUT_VRAM_Y;
                    upload->rect.w = definition->unk5 + 1;
                    upload->rect.h = single_row_height;
                    goto strip_upload;
                case 3:
                    if (definition->unkC == 0)
                    {
                        upload->rect.x = definition->unkF + ((definition->unkE & 0xF) * 0x10);
                        upload->rect.y = (definition->unkE >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                        upload->rect.w = definition->unk10;
                        upload->rect.h = single_row_height;
                        if (anim->flags.b.state == 0)
                        {
                            upload->data = (u_long*)((u8*)header->pixel_data + (definition->unkE << 5) + (definition->unkF & 0xE) * 2);
                        }
                        else
                        {
                            upload->data = (u_long*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition->unk12 * 2 + ((anim->flags.b.state - 1) * definition->unk10) * 2);
                        }
                    }
                    else
                    {
                        upload->rect.x = definition->unkF;
                        upload->rect.y = definition->unkE + FIELD_TILE_CLUT_VRAM_Y;
                        upload->rect.w = definition->unk10;
                        upload->rect.h = single_row_height;
                        if (anim->flags.b.state == 0)
                        {
                            upload->data = (u_long*)((u8*)header->pixel_data + (definition->unkE << 9) + definition->unkF * 2);
                        }
                        else
                        {
                            upload->data = (u_long*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition->unk12 * 2 + ((anim->flags.b.state - 1) * definition->unk10) * 2);
                        }
                    }
                    field_queue_vram_upload(upload);
                    break;
                case 4:
                    if (definition->unkC == 0)
                    {
                        upload->rect.x = (definition->unkE & 0xF) * 0x10;
                        upload->rect.y = (definition->unkE >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                        upload_extent = definition->unk10 * 0x10;
                        upload_width = 0x100;
                        if (upload_extent < 0x101)
                        {
                            upload_width = upload_extent;
                        }
                        upload->rect.w = upload_width;
                        upload->rect.h = (definition->unk10 + 0xF) / 0x10;
                        if (anim->flags.b.state == 0)
                        {
                            upload->data = (u_long*)((u8*)header->pixel_data + (definition->unkE << 5));
                        }
                        else
                        {
                            upload->data =
                                (u_long*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition->unk12 * 2 + (((anim->flags.b.state - 1) * definition->unk10) << 5));
                        }
                    }
                    else
                    {
                        upload->rect.x = 0;
                        upload->rect.y = definition->unkE + FIELD_TILE_CLUT_VRAM_Y;
                        upload->rect.w = 0x100;
                        upload->rect.h = definition->unk10;
                        if (anim->flags.b.state == 0)
                        {
                            upload->data = (u_long*)((u8*)header->pixel_data + (definition->unkE << 9));
                        }
                        else
                        {
                            upload->data =
                                (u_long*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition->unk12 * 2 + (((anim->flags.b.state - 1) * definition->unk10) << 9));
                        }
                    }
                    field_queue_vram_upload(upload);
                    break;
                case 5:
                    upload->data = field_blend_animation_frames(definition, anim);
                    /* The do/while(0) loop notes decide the anim/upload register priority. */
                    do
                    {
                        if (definition->unkC == 0)
                        {
                            upload->rect.x = (definition->unkE & 0xF) * 0x10;
                            upload->rect.y = (definition->unkE >> 4) + FIELD_TILE_CLUT_VRAM_Y;
                            upload_extent = definition->unk10 * 0x10;
                            upload_width = 0x100;
                            if (upload_extent < 0x101)
                            {
                                upload_width = upload_extent;
                            }
                            upload->rect.w = upload_width;
                            rect_extent = (definition->unk10 + 0xF) / 0x10;
                        }
                        else
                        {
                            upload_extent = 0x100;
                            upload->rect.x = 0;
                            upload->rect.y = definition->unkE + FIELD_TILE_CLUT_VRAM_Y;
                            upload->rect.w = upload_extent;
                            rect_extent = definition->unk10;
                        }
                        upload->rect.h = rect_extent;
                    strip_upload:
                        field_queue_vram_upload(upload);
                    } while (0);
                    break;
                }
                anim->flags.word &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
            }
            if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
            {
                anim->timer--;
                if ((*(s32*)&definition->flags & FIELD_ANIM_KIND_MASK) == 5)
                {
                    anim->flags.word |= FIELD_ANIM_FLAG_UPLOAD_PENDING;
                }
                if (anim->timer == 0)
                {
                    field_advance_animation_keyframe(definition, anim);
                    switch (definition->flags & FIELD_ANIM_KIND_MASK)
                    {
                    case 0:
                        field_retarget_cel_cluts(definition, anim->cels, anim->flags.b.state);
                        break;
                    case 1:
                        field_retarget_cel_list_cluts(definition, (FieldTintSrc*)anim->cels, anim->flags.b.state);
                        break;
                    default:
                        anim->flags.word |= FIELD_ANIM_FLAG_UPLOAD_PENDING;
                        break;
                    }
                }
            }
            anim = anim->next;
        } while (anim != NULL);
    }

    anim = scene->sprites;
    if (anim != NULL)
    {
        do
        {
            definition = anim->def;
            if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
            {
                if (--anim->timer == 0)
                {
                    field_advance_animation_keyframe(definition, anim);
                    switch (definition->flags & FIELD_ANIM_KIND_MASK)
                    {
                    case 0:
                        field_tint_animation_cel(definition, anim->cels, (FieldTintSrc*)anim->unk10, anim->flags.b.state);
                        break;
                    case 1:
                        field_tint_animation_cel_list(definition, (FieldTintSrc*)anim->cels, anim->flags.b.state);
                        break;
                    case 2:
                        break;
                    }
                }
            }
            anim = anim->next;
        } while (anim != NULL);
    }

    anim = scene->effects;
    if (anim != NULL)
    {
        do
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
            anim = anim->next;
        } while (anim != NULL);
    }

    sequence = scene->seqs;
    if (sequence != NULL)
    {
        do
        {
            if ((sequence->flags & 3) != 0)
            {
                command = sequence->def;
                if ((sequence->flags & 3) == 1)
                {
                    if ((command->start_link != 0xFF) && (command->start_delay == sequence->unkC))
                    {
                        target_sequence = scene->seqs;
                        index = command->start_link;
                        index--;
                        while (index != -1)
                        {
                            target_sequence = target_sequence->next;
                            index--;
                        }
                        func_8005A744(target_sequence, ((u8*)&sequence->flags)[1]);
                    }
                    if (func_8005A84C(command->list_kind, command->anim_index) == 2)
                    {
                        if (command->end_link != 0xFF)
                        {
                            sequence->unkC = 0;
                            sequence->flags = (sequence->flags & ~3) | 2;
                        }
                        else
                        {
                            sequence->flags = sequence->flags & ~3;
                        }
                    }
                }
                if (((sequence->flags & 3) == 2) && (command->end_delay == sequence->unkC))
                {
                    target_sequence = scene->seqs;
                    index = command->end_link;
                    index--;
                    while (index != -1)
                    {
                        target_sequence = target_sequence->next;
                        index--;
                    }
                    func_8005A744(target_sequence, ((u8*)&sequence->flags)[1]);
                    if (target_sequence != sequence)
                    {
                        sequence->flags &= ~3;
                    }
                }
                sequence->unkC = sequence->unkC + 1;
            }
            sequence = sequence->next;
        } while (sequence != NULL);
    }
}

/**
 * @brief Update the swept positions of a part's attached nodes.
 * @param part Part whose node sweep is advanced.
 */
void field_update_part_sweep(FieldPart* part)
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
    mode = (part->def->u.word >> 12) & 0xF;
    switch (mode)
    {
    case 1:
        divisor = 0x121;
        break;
    case 2:
        divisor = 0xA1;
        break;
    case 3:
    default:
        divisor = 0x101;
        break;
    }
    sine = rsin((part->sweep_phase << 12) / part->sweep_period);
    if (sine >= 0)
    {
        part->rotation_angle = sine / divisor;
    }
    else
    {
        part->rotation_angle = 0x1000 - ((u32)-sine / divisor);
    }
    negative_sine = -rsin(part->rotation_angle);
    if (part->node_count != 0)
    {
        angle_table = g_field_node_angle_table;
        mode = (part->def->u.word >> 12) & 0xF;
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
        node = scene->nodes;
        remaining_nodes = part->node_count;
        if (node != NULL)
        {
            do
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
                node = node->next;
            } while (node != NULL);
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
    FieldAnimCel* cel;
    FieldTileGrid* grid;
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
    grid = ((FieldTileAnimDef*)def)->grid;
    record_cursor = cel->tiles;
    stride = 0;
    switch (cel->format)
    {
    case 0:
    case 2:
    case 3:
    case 4:
    case 5:
        stride = 12;
        break;
    case 1:
    case 6:
        break;
    }
    if (cel->code_word != 0)
    {
        stride -= 4;
    }
    if (cel->tpage_word != 0)
    {
        stride -= 4;
    }
    frame_words = (u32*)(anim->frame_data + anim->frame_tile_count * stride * frame);
    mask_bit = 1;
    mask_cursor = cel->mask;
    mask_word = *mask_cursor++;
    for (row = 0; row != grid->u.b.rows; row++)
    {
        if (row < def->unkD)
        {
            /* Above the sub-rectangle: step the cursor over the whole row. */
            col = grid->u.b.cols;
            col--;
            while (col != -1)
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
                col--;
            }
        }
        else
        {
            if (row >= def->unkD + def->unkF)
            {
                return;
            }
            for (col = 0; col != grid->u.b.cols; col++)
            {
                if (mask_word & mask_bit)
                {
                    if ((col >= def->unkC) && (col < def->unkC + def->unkE))
                    {
                        word_count = stride >> 2;
                        word_count--;
                        while (word_count != -1)
                        {
                            *(u32*)record_cursor = *frame_words++;
                            record_cursor += 4;
                            word_count--;
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
 * @brief Interpolate the current tween keyframe and optionally apply its delta.
 * @param def Animation definition containing the tween table.
 * @param anim Animation node to evaluate.
 * @param apply_to_target Nonzero to apply the interpolated delta to its target.
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
    if ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) == 5)
    {
        part = (FieldPart*)anim->cels;
    }
    else
    {
        object = (FieldObj*)anim->cels;
    }
    keyframe = (FieldTweenKey*)(definition_copy->data + anim->flags.b.state * 8);
    duration = ((FieldTweenSpan*)field_find_count_table_span((u8*)def, anim->flags.b.keyframe, &range_start))->duration;
    if (duration == 0)
    {
        duration = 1;
    }
    elapsed = duration - anim->timer;
    if ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) == 5)
    {
        part->visible = keyframe->visibility >> 15;
    }
    else
    {
        object->flags.word = (object->flags.word & ~1) | (keyframe->visibility >> 15);
    }

    value = ((elapsed * keyframe->x) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_x;
        if ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) == 5)
        {
            part->x += delta;
            func_8005A984(part, delta, 0);
        }
        else
        {
            object->x += delta;
            func_8005AA68(object, delta, 0);
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
        if ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) == 5)
        {
            part->y += delta;
            func_8005A984(part, delta, 1);
        }
        else
        {
            object->y += delta;
            func_8005AA68(object, delta, 1);
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
        if ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) == 5)
        {
            part->z += delta;
            func_8005A984(part, delta, 2);
        }
        else
        {
            object->z += delta;
            func_8005AA68(object, delta, 2);
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
 * @brief Process the sound command for the animation's current keyframe.
 * @param def Animation definition containing sound keyframes.
 * @param anim Animation node whose sound state is updated.
 */
void field_update_animation_sfx(FieldAnimDef* def, FieldAnim* anim)
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

    part = (FieldPart*)anim->cels;
    object = (FieldObj*)anim->unk10;
    keyframe_offset = anim->flags.b.state * 8;
    key_type = def->data[keyframe_offset] & 7;
    if (key_type == 1)
    {
        keyframe = (FieldSfxKey*)(def->data + anim->flags.b.state * 8);
        if (keyframe->control.word & 0x1F00)
        {
            channel_mask = key_type << (((keyframe->control.word >> 8) & 0x1F) - 1);
            sfx_id = 0;
        }
        else
        {
            channel_mask = 0;
            sfx_id = keyframe->sfx_id & 0x3FF;
        }
        if (keyframe->control.word & 0x8000)
        {
            if (keyframe->control.word & 0x4000)
            {
                if (keyframe->sound.word & 0x8000)
                {
                    if (anim->flags.word & FIELD_ANIM_FLAG_START_PENDING)
                    {
                        akao_play_sfx(sfx_id, channel_mask, keyframe->sound.b.lo, (keyframe->sound.word >> 8) & 0x7F);
                        anim->flags.word &= ~FIELD_ANIM_FLAG_START_PENDING;
                    }
                }
                else
                {
                    akao_play_sfx(sfx_id, channel_mask, keyframe->sound.b.lo, (keyframe->sound.word >> 8) & 0x7F);
                }
            }
            else
            {
                if (object->def->flags & 2)
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
                    /* The loop notes and the forced reload keep the second part->def load. */
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
                    adjusted_value = (keyframe->sound.word >> 8) & 0x7F;
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
                    adjusted_value = (keyframe->sound.word >> 8) & 0x7F;
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
                    volume = (keyframe->sound.word >> 8) & 0x7F;
                }
                if (keyframe->sound.word & 0x8000)
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
void field_retarget_cel_cluts(FieldAnimDef* anim_def, FieldAnimCel* cel, s32 frame)
{
    FieldAnimDef* definition;
    FieldTileGrid* grid;
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
    grid = cel->grid;
    stride = 0;
    if (definition->unkC == ((grid->u.word >> 4) & 3))
    {
        tile = grid->tiles;
        record_cursor = cel->tiles;
        switch (cel->format)
        {
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
            stride = 12;
            break;
        case 1:
        case 6:
            break;
        }
        if (cel->code_word != 0)
        {
            stride -= 4;
        }
        mask_bit = 1;
        if (cel->tpage_word != 0)
        {
            stride -= 4;
        }
        row = 0;
        mask_cursor = cel->mask;
        first_slot = definition->unkE;
        last_slot = first_slot + definition->unk10;
        packing_mode = definition->unkC;
        mask_word = *mask_cursor++;
        if (grid->u.b.rows != 0)
        {
            do
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

                            if (packed & 0x80)
                            {
                                clut_slot = packed & 0x1F;
                                if ((clut_slot >= first_slot) && (clut_slot < last_slot))
                                {
                                    y = clut_slot + (frame * definition->unk10);
                                    if (packing_mode == 0)
                                    {
                                        clut = (((y >> 4) + 0x1D8) << 6) | (y & 0xF);
                                    }
                                    else
                                    {
                                        clut = (y + 0x1D8) << 6;
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
                row++;
            } while (row != grid->u.b.rows);
        }
    }
}

/**
 * @brief Blend the current animation frame with its neighboring frame.
 * @param def Animation definition describing the source frames.
 * @param anim Animation node supplying timing and scratch storage.
 * @return Pointer to the blended scratch pixel buffer.
 */
u_long* field_blend_animation_frames(FieldAnimDef* def, FieldAnim* anim)
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
    s32 new_flags;
    u16 current_pixel;
    u16 other_pixel;
    u8 range_start;

    definition_copy = def;
    header = g_field_scene.scene->header;
    duration = ((FieldTweenSpan*)field_find_count_table_span((u8*)definition_copy, anim->flags.b.keyframe, &range_start))->duration;
    keyframe_index = anim->flags.b.keyframe;
    /* The do/while(0) loop notes decide the anim/remaining register priority. */
    do
    {
        remaining = anim->timer;
        control_flags = anim->flags.word;
    } while (0);
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
        else if (keyframe_index == definition_copy->unk5)
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
        if (keyframe_index == definition_copy->unk5)
        {
            keyframe_index = 0;
        }
        else
        {
            keyframe_index = keyframe_index + 1;
        }
    }
    if (*(s32*)&def->flags & FIELD_ANIM_DEF_SPAN_INDEXED)
    {
        other_frame = (((FieldTweenSpan*)field_find_count_table_span((u8*)def, keyframe_index, &range_start))->range_start + keyframe_index) - range_start;
    }
    else
    {
        other_frame = keyframe_index;
    }
    if (definition_copy->unkC == 0)
    {
        pixel_count = definition_copy->unk10 * 0x10;
        if (anim->flags.b.state == 0)
        {
            current_pixels = (u16*)((u8*)header->pixel_data + (definition_copy->unkE << 5));
        }
        else
        {
            current_pixels = (u16*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition_copy->unk12 * 2 + ((anim->flags.b.state - 1) * pixel_count) * 2);
        }
        if (other_frame == 0)
        {
            other_pixels = (u16*)((u8*)header->pixel_data + (definition_copy->unkE << 5));
        }
        else
        {
            other_pixels = (u16*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition_copy->unk12 * 2 + ((other_frame - 1) * pixel_count) * 2);
        }
    }
    else
    {
        pixel_count = definition_copy->unk10 << 8;
        if (anim->flags.b.state == 0)
        {
            current_pixels = (u16*)((u8*)header->pixel_data + (definition_copy->unkE << 9));
        }
        else
        {
            current_pixels = (u16*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition_copy->unk12 * 2 + ((anim->flags.b.state - 1) * pixel_count) * 2);
        }
        if (other_frame == 0)
        {
            other_pixels = (u16*)((u8*)header->pixel_data + (definition_copy->unkE << 9));
        }
        else
        {
            other_pixels = (u16*)((u8*)header->pixel_data + header->pixel_stride * 2 + definition_copy->unk12 * 2 + ((other_frame - 1) * pixel_count) * 2);
        }
    }
    control_flags = anim->flags.word;
    if (control_flags & FIELD_ANIM_FLAG_SECOND_BUFFER)
    {
        destination = &anim->scratch_pixels[pixel_count];
        new_flags = control_flags & ~FIELD_ANIM_FLAG_SECOND_BUFFER;
    }
    else
    {
        destination = anim->scratch_pixels;
        new_flags = control_flags | FIELD_ANIM_FLAG_SECOND_BUFFER;
    }
    anim->flags.word = new_flags;
    pixel_count--;
    output = destination;
    while (pixel_count != -1)
    {
        current_pixel = *current_pixels++;
        other_pixel = *other_pixels++;
        if ((elapsed == 0) || (current_pixel == other_pixel))
        {
            *destination = current_pixel;
        }
        else
        {
            *destination = ((current_pixel | other_pixel) & 0x8000) | ((((current_pixel & 0x1F) * remaining) + ((other_pixel & 0x1F) * elapsed)) / duration) |
                   (((((current_pixel >> 5) & 0x1F) * remaining) + (((other_pixel >> 5) & 0x1F) * elapsed)) / duration) << 5 |
                   (((((current_pixel >> 10) & 0x1F) * remaining) + (((other_pixel >> 10) & 0x1F) * elapsed)) / duration) << 10;
        }
        pixel_count--;
        destination++;
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
void field_tint_animation_cel(FieldAnimDef* def, FieldAnimCel* cel, FieldTintSrc* src, s32 shade)
{
    FieldTileGrid* grid;
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
    func_8005AC50(palette_data + 2, palette_data[0], rgb);
    grid = cel->grid;
    record_cursor = cel->tiles;
    tile = grid->tiles;
    switch (cel->format)
    {
    case 0:
    case 2:
    case 3:
    case 4:
    case 5:
        stride = 12;
        break;
    case 1:
    case 6:
        break;
    }
    shared_code = cel->code_word;
    if (shared_code != 0)
    {
        stride -= 4;
    }
    if (cel->tpage_word != 0)
    {
        stride -= 4;
    }
    mask_bit = 1;
    row = 0;
    mask_cursor = cel->mask;
    first_slot = def->unkC;
    last_slot = first_slot + def->unkD;
    mask_word = *mask_cursor++;
    if (grid->u.b.rows != 0)
    {
        do
        {
            col = 0;
            if (grid->u.b.cols != 0)
            {
                do
                {
                    if (mask_word & mask_bit)
                    {
                        if (tile->clut_slot & 0x80)
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
                    col++;
                } while (col != grid->u.b.cols);
            }
            row++;
        } while (row != grid->u.b.rows);
    }
}

/**
 * @brief Apply a tint-table shade to every cel in a tint source.
 * @param def Animation definition selecting the tintable color range.
 * @param src Tint source whose cel list and palette are used.
 * @param shade Tint-table shade offset.
 */
void field_tint_animation_cel_list(FieldAnimDef* def, FieldTintSrc* src, s32 shade)
{
    FieldAnimCel* cel;
    FieldTileGrid* grid;
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
    func_8005AC50(palette_data + 2, palette_data[0], rgb);
    first_slot = def->unkC;
    last_slot = first_slot + def->unkD;
    for (cel = src->cels; cel != NULL; cel = cel->next)
    {
        record_cursor = cel->tiles;
        grid = cel->grid;
        tile = grid->tiles;
        switch (cel->format)
        {
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
            stride = 12;
            break;
        case 1:
        case 6:
            break;
        }
        shared_code = cel->code_word;
        if (shared_code != 0)
        {
            stride -= 4;
        }
        if (cel->tpage_word != 0)
        {
            stride -= 4;
        }
        mask_bit = 1;
        row = 0;
        mask_cursor = cel->mask;
        mask_word = *mask_cursor++;
        if (grid->u.b.rows != 0)
        {
            do
            {
                col = 0;
                if (grid->u.b.cols != 0)
                {
                    do
                    {
                        if (mask_word & mask_bit)
                        {
                            if (tile->clut_slot & 0x80)
                            {
                                color_slot = tile->color_index;
                                if ((color_slot >= first_slot) && (last_slot >= color_slot))
                                {
                                    color_entry = &palette[color_slot] + shade;
                                    if (shared_code != 0)
                                    {
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
                        col++;
                    } while (col != grid->u.b.cols);
                }
                row++;
            } while (row != grid->u.b.rows);
        }
    next_cel:;
    }
}

/**
 * @brief Advance an animation's keyframe cursor and reload its timer.
 * @param def Animation definition controlling playback and repeat behavior.
 * @param anim Animation node to advance.
 */
void field_advance_animation_keyframe(FieldAnimDef* def, FieldAnim* anim)
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
                    if (*(s32*)&def->flags & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT)
                    {
                        anim->flags.b.keyframe = anim->flags.b.keyframe + 1;
                    }
                    else if (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) - 5) < 2)) || ((*(s32*)&def->flags & 0xFF000007) == 0x01000005))
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
                    if (!(*(s32*)&def->flags & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) && (anim->flags.b.keyframe == 0) &&
                        (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) - 5) >= 2)) ||
                         ((def->handler_group == 1) && ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) != 5)) || (def->handler_group >= 2)))
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
                if (keyframe == def->unk5)
                {
                    if (!(*(s32*)&def->flags & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) && (keyframe == 0))
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
            if (current_keyframe == def->unk5)
            {
                if (!(*(s32*)&def->flags & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) &&
                    (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) - 5) < 2)) || ((*(s32*)&def->flags & 0xFF000007) == 0x01000005)))
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
                if (!(*(s32*)&def->flags & FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT) && ((u8)next_keyframe == def->unk5) &&
                    (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) - 5) >= 2)) ||
                     ((def->handler_group == 1) && ((*(s32*)&def->flags & FIELD_ANIM_KIND_MASK) != 5)) || (def->handler_group >= 2)))
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
    keyframe_span = (FieldTweenSpan*)field_find_count_table_span((u8*)def, anim->flags.b.keyframe, &range_start);
    anim->timer = keyframe_span->duration;
    if (*(s32*)&def->flags & FIELD_ANIM_DEF_SPAN_INDEXED)
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
void field_retarget_cel_list_cluts(FieldAnimDef* def, FieldTintSrc* src, s32 frame)
{
    FieldAnimCel* cel;

    cel = src->cels;
    if (cel != NULL)
    {
        do
        {
            field_retarget_cel_cluts(def, cel, frame);
            cel = cel->next;
        } while (cel != NULL);
    }
}

/**
 * @brief Find the count-table record containing a linear animation index.
 * @param table Pointer to the animation count table.
 * @param linear_index Linear animation index to resolve.
 * @param range_start_out Receives the cumulative count before the returned record.
 * @return Pointer to the count-table record containing @p linear_index.
 */
u8* field_find_count_table_span(u8* table, s32 linear_index, u8* range_start_out)
{
    u8 header_count;
    u8 count;
    u8 range_start;
    u8 range_end;

    *range_start_out = 0;
    header_count = *table & 0x7F;
    /* A single-pass loop: the original's loop notes decide the register allocation. */
    while (linear_index >= header_count)
    {
        *range_start_out = header_count;
        header_count = table[7];
        *(volatile u8*)(table + 7); /* the original reads this byte and discards it */
        if (header_count)
        {
            table += sizeof(FieldTweenKey) * 3;
        }
        else
        {
            table += sizeof(FieldTweenKey) * 3;
        }
        count = *table;
        range_start = *range_start_out;
        range_end = range_start + (count & 0x7F);
        while (linear_index >= range_end)
        {
            table += sizeof(FieldTweenSpan);
            range_start += count & 0x7F;
            *range_start_out = range_start;
            count = *table;
            range_end = range_start + (count & 0x7F);
        }
        break;
    }
    return table;
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
