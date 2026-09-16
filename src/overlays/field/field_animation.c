#include "field_scene_internal.h"

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
 * @brief Per-frame update for the field scene's animation, strip, sprite,
 *        effect and sequence lists (plus the object/part walk).
 *
 * Walks the six lists hanging off @c g_field_scene.scene and advances each:
 * object parts get a per-part hook (field_update_part_sweep); animation nodes drive a
 * CD/MDEC movie state machine and per-handler dispatch; strip nodes copy their
 * pixel source out of the scene header (hdr->pixel_data) into per-node scratch buffers
 * and queue a VRAM upload (field_queue_vram_upload); sprite/effect nodes tick their
 * counters; sequence nodes advance a small state machine keyed on flags & 3.
 *
 * @note Match is 100% (1033 exact target instructions out of 1033).
 * @note The state==1 CD-flag set is written through
 *       @c (u32)anim ^ ((u32)anim ^ 0x801ED800U): the same address and type as
 *       @c FIELD_CD_FLAGS, but forcing the constant through the @c anim register
 *       reproduces the target's codegen. The tear-down path clears the CD flag
 *       word through @c CdWordBits, and the movie rect setup writes
 *       @c rects[0].x through a non-volatile cast; both spellings are required
 *       to reproduce the target's scheduling and codegen.
 * @note The movie default-state timer tail falls through @c goto movie_timer to
 *       a single shared @c anim->timer = 1; matching the target's tail merging.
 * @note The two strip-upload tails are merged with @c goto strip_upload so the
 *       shared @c field_queue_vram_upload call is emitted once, matching the
 *       target's tail merging.
 * @note The second strip copy loop must reuse @c count (not a fresh @c i) to
 *       reproduce the target's counter/sentinel register coloring, and the
 *       @c unkD==0 stride count must be spelled @c (unk5 + 1 - state) so gcc
 *       keeps the target's @c addiu -1 reassociation.
 * @note @c one holds the literal 1 so gcc's loop.c hoists it into a saved
 *       register for the three @c req->rect.h = 1 strip stores; writing @c 1
 *       inline leaves the hoist undone. The @c do{}while(0) around the case-5
 *       body gives @c req the extra references it needs to win s1 over @c anim
 *       in the global allocator (see working notes).
 *
 * @see decomp.me (100%) TODO
 */
void field_update_scene_animations(void)
{
    FieldScene* scene;
    FieldSceneHeader* hdr;
    FieldObj* obj;
    FieldPart* part;
    FieldAnim* anim;
    FieldAnimDef* def;
    FieldAnimDef* def2;
    FieldAnimDef* def3;
    FieldAnimCel* cel;
    FieldImageReq* req;
    FieldSeq* seq;
    FieldSeq* walk;
    FieldAnimDef* rec;
    u16* src;
    u16* dst;
    s32 prev_state;
    s32 flags;
    s32 mode;
    s32 count;
    s32 count2;
    s32 i;
    s32 v;
    s32 w;
    s32 t;
    s32 y;
    s32 one;

    scene = g_field_scene.scene;

    obj = scene->objects;
    if (obj != NULL)
    {
        do
        {
            part = obj->parts;
            if (part != NULL)
            {
                do
                {
                    if (part->def->u.word & 0xF000)
                    {
                        mode = (part->def->u.word >> 12) & 0xF;
                        if ((mode != 0) && (mode < 5))
                        {
                            field_update_part_sweep(part);
                        }
                    }
                    part = part->next;
                } while (part != NULL);
            }
            obj = obj->next;
        } while (obj != NULL);
    }

    anim = scene->anims;
    hdr = scene->header;
    if (anim != NULL)
    {
        do
        {
            def = anim->def;
            def2 = anim->def;
            if (anim->flags.word & 0x20)
            {
                req = &anim->req;
                if ((*(s32*)&def->flags & 7) == 3)
                {
                    req->rect.x = def->unkC * 4 + 0x140;
                    req->rect.y = def->unkD * 0x10 + 0x100;
                    req->rect.w = def->unkE * 4;
                    req->rect.h = def->unkF * 0x10;
                    req->data = (u_long*)(def->data + ((anim->flags.b.state * def->unkE * def->unkF) << 7));
                    field_queue_vram_upload(req);
                }
                anim->flags.word &= ~0x20;
            }
            if (anim->flags.word & 0x40)
            {
                anim->timer--;
                switch (def->flags & 7)
                {
                case 4:
                    switch (anim->flags.b.state)
                    {
                    case 0:
                        if (cdrom_process_state() == 0)
                        {
                            cdrom_stream(0xB, (void*)0x80140000);
                            cdrom_queue_seek(def->unk1 * 2 + 0x16A6);
                            anim->flags.b.state = 1;
                            /* Same address/type as FIELD_CD_FLAGS, but forced
                             * through the anim register to match codegen. */
                            (*(volatile s32*)((u32)anim ^ ((u32)anim ^ 0x801ED800U))) |= 0x40;
                        }
                        /* fallthrough */
                    case 1:
                        if (cdrom_can_queue_resource(def->unk1 * 2 + 0x16A6) != 0)
                        {
                            ((FieldMovieState*)0x801ED500)->rects[0].x = def2->unkC * 4 + 0x140;
                            FIELD_MOVIE_STATE->rects[0].y = def2->unkD * 0x10 + 0x100;
                            FIELD_MOVIE_STATE->rects[0].w = def2->unkE * 4;
                            FIELD_MOVIE_STATE->rects[0].h = def2->unkF * 0x10;
                            cel = anim->cels;
                            ((volatile CdWordBits*)0x801ED800)->word &= ~0x40;
                            if (def->unk1 < 2)
                            {
                                func_80140358(def->unk1 * 2 + 0x16A6, 1, def->unk5 - 2, cel->active);
                            }
                            else
                            {
                                func_80140358(def->unk1 * 2 + 0x16A6, 1, 0x12E, 0);
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
                                if (cdrom_can_queue_resource(def->unk1 * 2 + 0x16A7) != 0)
                                {
                                    req = &anim->req;
                                    if (def->unk1 < 2)
                                    {
                                        cel = anim->cels;
                                        req->rect.x = FIELD_MOVIE_STATE->rects[cel->active].x;
                                        req->rect.y = FIELD_MOVIE_STATE->rects[cel->active].y;
                                        req->rect.w = FIELD_MOVIE_STATE->rects[cel->active].w;
                                        req->rect.h = FIELD_MOVIE_STATE->rects[cel->active].h;
                                        req->data = (u_long*)0x80140000;
                                        field_queue_vram_upload(req);
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
                                        req->rect.x = 0x140;
                                        req->rect.y = 0x100;
                                        req->data = (u_long*)0x80140004;
                                        req->rect.w = g_field_movie_frame_width;
                                        req->rect.h = g_field_movie_frame_height;
                                        field_queue_vram_upload(req);
                                        cel->active = 0;
                                        cel = cel->next;
                                        cel->active = 0;
                                    }
                                    FIELD_MOVIE_STATE->end_state = 4;
                                }
                                anim->timer = 1;
                            }
                            else
                            {
                                if (def->unk1 >= 2)
                                {
                                    field_begin_scene_fade_in();
                                }
                                anim->flags.word &= ~0x40;
                                func_80084240();
                                goto movie_timer;
                            }
                        }
                        else
                        {
                            if (def->unk1 < 2)
                            {
                                set_controller_vsync_interval(2);
                            }
                            func_801406E4();
                            func_80140D48();
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
                                cdrom_queue_read(def->unk1 * 2 + 0x16A7, (void*)0x80140000);
                                FIELD_MOVIE_STATE->end_state = 3;
                            }
                        movie_timer:
                            anim->timer = 1;
                        }
                        break;
                    }
                    break;
                case 5:
                case 6:
                    field_apply_animation_tween(def, anim, 1);
                    break;
                }
            if (anim->timer == 0)
            {
                prev_state = anim->flags.b.state;
                field_advance_animation_keyframe(def, anim);
                switch (def->flags & 7)
                {
                case 0:
                    if (prev_state != anim->flags.b.state)
                    {
                        field_blit_animation_frame(def, anim, anim->flags.b.state);
                    }
                    break;
                case 2:
                    cel = anim->cels;
                    i = prev_state - 1;
                    while (i != -1)
                    {
                        cel = cel->next;
                        i--;
                    }
                    cel->active = 0;
                    cel = anim->cels;
                    i = anim->flags.b.state;
                    i--;
                    while (i != -1)
                    {
                        cel = cel->next;
                        i--;
                    }
                    cel->active = 1;
                    break;
                case 3:
                    req = &anim->req;
                    req->rect.x = def2->unkC * 4 + 0x140;
                    req->rect.y = def2->unkD * 0x10 + 0x100;
                    req->rect.w = def2->unkE * 4;
                    req->rect.h = def2->unkF * 0x10;
                    req->data = (u_long*)(def2->data + ((anim->flags.b.state * def2->unkE * def2->unkF) << 7));
                    field_queue_vram_upload(req);
                    break;
                case 5:
                case 6:
                    while (anim->timer == 0)
                    {
                        field_apply_animation_tween(def, anim, 1);
                        field_advance_animation_keyframe(def, anim);
                    }
                    break;
                case 7:
                    field_update_animation_sfx(def, anim);
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
        one = 1;
        do
        {
            def = anim->def;
            flags = anim->flags.word;
            def3 = anim->def;
            if (flags & 0x20)
            {
                req = &anim->req;
                switch (def->flags & 7)
                {
                case 2:
                    if (flags & 0x10)
                    {
                        dst = anim->buf240;
                        if (def->unkC == 0)
                        {
                            dst = anim->buf60;
                        }
                        anim->flags.word = *(volatile s32*)&anim->flags.word & ~0x10;
                    }
                    else
                    {
                        dst = anim->buf40;
                        anim->flags.word = flags | 0x10;
                    }
                    req->data = (u_long*)dst;
                    count2 = 0;
                    if (anim->flags.b.state != 0)
                    {
                        if (def3->unkD != 0)
                        {
                            t = anim->flags.b.state - 1;
                            count2 = (def->unk5 - t) * def3->unk10;
                            count = anim->flags.b.state * def3->unk10;
                        }
                        else
                        {
                            count2 = anim->flags.b.state * def3->unk10;
                            count = (def->unk5 + 1 - anim->flags.b.state) * def3->unk10;
                        }
                    }
                    else
                    {
                        count = (def->unk5 + 1) * def3->unk10;
                    }
                    if (def3->unkC == 0)
                    {
                        src = (u16*)((u8*)hdr->pixel_data + (def3->unkE << 5) + def3->unkF * 2 + count2 * 2);
                    }
                    else
                    {
                        src = (u16*)((u8*)hdr->pixel_data + (def3->unkE << 9) + def3->unkF * 2 + count2 * 2);
                    }
                    count--;
                    while (count != -1)
                    {
                        *dst++ = *src++;
                        count--;
                    }
                    if (count2 != 0)
                    {
                        if (def3->unkC == 0)
                        {
                            src = (u16*)((u8*)hdr->pixel_data + (def3->unkE << 5) + def3->unkF * 2);
                        }
                        else
                        {
                            src = (u16*)((u8*)hdr->pixel_data + (def3->unkE << 9) + def3->unkF * 2);
                        }
                        count = count2 - 1;
                        while (count != -1)
                        {
                            *dst++ = *src++;
                            count--;
                        }
                    }
                    if (def3->unkC == 0)
                    {
                        req->rect.x = def3->unkF + ((def3->unkE & 0xF) * 0x10);
                        y = def3->unkE >> 4;
                    }
                    else
                    {
                        req->rect.x = def3->unkF;
                        y = def3->unkE;
                    }
                    req->rect.y = y + 0x1D8;
                    req->rect.w = def->unk5 + 1;
                    req->rect.h = one;
                    goto strip_upload;
                    break;
                case 3:
                    if (def->unkC == 0)
                    {
                        req->rect.x = def->unkF + ((def->unkE & 0xF) * 0x10);
                        req->rect.y = (def->unkE >> 4) + 0x1D8;
                        req->rect.w = def->unk10;
                        req->rect.h = one;
                        if (anim->flags.b.state == 0)
                        {
                            req->data = (u_long*)((u8*)hdr->pixel_data + (def->unkE << 5) + (def->unkF & 0xE) * 2);
                        }
                        else
                        {
                            req->data = (u_long*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + def->unk12 * 2 + ((anim->flags.b.state - 1) * def->unk10) * 2);
                        }
                    }
                    else
                    {
                        req->rect.x = def->unkF;
                        req->rect.y = def->unkE + 0x1D8;
                        req->rect.w = def->unk10;
                        req->rect.h = one;
                        if (anim->flags.b.state == 0)
                        {
                            req->data = (u_long*)((u8*)hdr->pixel_data + (def->unkE << 9) + def->unkF * 2);
                        }
                        else
                        {
                            req->data = (u_long*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + def->unk12 * 2 + ((anim->flags.b.state - 1) * def->unk10) * 2);
                        }
                    }
                    field_queue_vram_upload(req);
                    break;
                case 4:
                    if (def->unkC == 0)
                    {
                        req->rect.x = (def->unkE & 0xF) * 0x10;
                        req->rect.y = (def->unkE >> 4) + 0x1D8;
                        v = def->unk10 * 0x10;
                        w = 0x100;
                        if (v < 0x101)
                        {
                            w = v;
                        }
                        req->rect.w = w;
                        req->rect.h = (def->unk10 + 0xF) / 0x10;
                        if (anim->flags.b.state == 0)
                        {
                            req->data = (u_long*)((u8*)hdr->pixel_data + (def->unkE << 5));
                        }
                        else
                        {
                            req->data =
                                (u_long*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + def->unk12 * 2 + (((anim->flags.b.state - 1) * def->unk10) << 5));
                        }
                    }
                    else
                    {
                        req->rect.x = 0;
                        req->rect.y = def->unkE + 0x1D8;
                        req->rect.w = 0x100;
                        req->rect.h = def->unk10;
                        if (anim->flags.b.state == 0)
                        {
                            req->data = (u_long*)((u8*)hdr->pixel_data + (def->unkE << 9));
                        }
                        else
                        {
                            req->data =
                                (u_long*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + def->unk12 * 2 + (((anim->flags.b.state - 1) * def->unk10) << 9));
                        }
                    }
                    field_queue_vram_upload(req);
                    break;
                case 5:
                    req->data = field_blend_animation_frames(def, anim);
                    do
                    {
                        if (def->unkC == 0)
                        {
                            req->rect.x = (def->unkE & 0xF) * 0x10;
                            req->rect.y = (def->unkE >> 4) + 0x1D8;
                            v = def->unk10 * 0x10;
                            w = 0x100;
                            if (v < 0x101)
                            {
                                w = v;
                            }
                            req->rect.w = w;
                            y = (def->unk10 + 0xF) / 0x10;
                        }
                        else
                        {
                            v = 0x100;
                            req->rect.x = 0;
                            req->rect.y = def->unkE + 0x1D8;
                            req->rect.w = v;
                            y = def->unk10;
                        }
                        req->rect.h = y;
                    strip_upload:
                        field_queue_vram_upload(req);
                    } while (0);
                    break;
                }
                anim->flags.word &= ~0x20;
            }
            if (anim->flags.word & 0x40)
            {
                anim->timer--;
                if ((*(s32*)&def->flags & 7) == 5)
                {
                    anim->flags.word |= 0x20;
                }
                if (anim->timer == 0)
                {
                    field_advance_animation_keyframe(def, anim);
                    switch (def->flags & 7)
                    {
                    case 0:
                        field_retarget_cel_cluts(def, anim->cels, anim->flags.b.state);
                        break;
                    case 1:
                        field_retarget_cel_list_cluts(def, (FieldTintSrc*)anim->cels, anim->flags.b.state);
                        break;
                    default:
                        anim->flags.word |= 0x20;
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
            def = anim->def;
            if (anim->flags.word & 0x40)
            {
                if (--anim->timer == 0)
                {
                    field_advance_animation_keyframe(def, anim);
                    switch (def->flags & 7)
                    {
                    case 0:
                        field_tint_animation_cel(def, anim->cels, (FieldTintSrc*)anim->unk10, anim->flags.b.state);
                        break;
                    case 1:
                        field_tint_animation_cel_list(def, (FieldTintSrc*)anim->cels, anim->flags.b.state);
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
            def = anim->def;
            if (anim->flags.word & 0x40)
            {
                if (--anim->timer == 0)
                {
                    field_advance_animation_keyframe(def, anim);
                    field_blit_animation_frame(def, anim, anim->flags.b.state);
                }
            }
            anim = anim->next;
        } while (anim != NULL);
    }

    seq = scene->seqs;
    if (seq != NULL)
    {
        do
        {
            if ((seq->flags & 3) != 0)
            {
                rec = seq->def;
                if ((seq->flags & 3) == 1)
                {
                    if ((rec->unk5 != 0xFF) && (rec->unk8 == seq->unkC))
                    {
                        walk = scene->seqs;
                        i = rec->unk5;
                        i--;
                        while (i != -1)
                        {
                            walk = walk->next;
                            i--;
                        }
                        func_8005A744(walk, ((u8*)&seq->flags)[1]);
                    }
                    if (func_8005A84C(rec->unk0, rec->unk2) == 2)
                    {
                        if (rec->unk6 != 0xFF)
                        {
                            seq->unkC = 0;
                            seq->flags = (seq->flags & ~3) | 2;
                        }
                        else
                        {
                            seq->flags = seq->flags & ~3;
                        }
                    }
                }
                if (((seq->flags & 3) == 2) && (rec->unkA == seq->unkC))
                {
                    walk = scene->seqs;
                    i = rec->unk6;
                    i--;
                    while (i != -1)
                    {
                        walk = walk->next;
                        i--;
                    }
                    func_8005A744(walk, ((u8*)&seq->flags)[1]);
                    if (walk != seq)
                    {
                        seq->flags &= ~3;
                    }
                }
                seq->unkC = seq->unkC + 1;
            }
            seq = seq->next;
        } while (seq != NULL);
    }
}

/**
 * @brief Advance the swept 2D positions of a part's attached FieldNode list.
 *
 * Decrements the part's sweep phase (0x38) and turns it into an angle - rsin of
 * phase * 0x1000 / period, divided by a per-mode divisor - stored at 0x3E. That
 * angle feeds a second rsin whose negation scales every attached node: for each
 * node on the scene list (0x08) whose owner is @p part, the horizontal and
 * vertical steps are (angle_table[def index] - base) * -rsin, rounded toward
 * zero, offset-clamped, and written as the node's absolute position (0x38/0x3C)
 * plus its per-frame delta (0x28/0x2C). The walk stops after node_count matching
 * nodes. When the phase underflows to zero it reloads from the period (0x36).
 *
 * @param part Part whose attached nodes are advanced.
 *
 * @note Built -G4 WITHOUT --expand-div: the target has bare div/divu, so
 *       field_animation.c lives in overlay_field_gcc_g4_noexpand_srcs.
 * @note Both per-mode selects are `switch` statements, and the trailing
 *       `case N: default:` on each is required to match. gcc 2.8 balances the
 *       case list into a decision tree (stmt.c balance_case_nodes): with exactly
 *       three case nodes the middle one becomes the root, which is what puts the
 *       equality test on 2 (resp. 3) first, followed by the `> root` bound test
 *       to the default. Drop the extra case and only two nodes remain, so gcc
 *       emits a flat compare chain instead (-8 exact rows). Adding a `case 0:`
 *       instead adds a fourth node and re-roots the tree (-9). The extra case
 *       may equally be spelled with its own duplicated body, or use any value
 *       above the last distinguished one (`case 4:`/`case 5:` in the divisor
 *       select also match) - the target cannot distinguish those.
 * @note The two `>= 3` / `>= 4` guards are gcc's `bgt root` bound test, and the
 *       `mode == 0` guard in the base select is the low-bound test `mode < 1`
 *       that combine narrows to `beqz` because `(word >> 12) & 0xF` is known
 *       non-negative. Neither is written in the source.
 * @note Nested if/else does NOT match: it emits the case bodies in the wrong
 *       order (X, D, A instead of X, A, D), costing 4 exact rows on the divisor
 *       select and 3 on the base select.
 * @note `val = base;` before the first arm's subtraction is required: it steers
 *       the global allocator so val/y take a0/a1 (not a1/a0) across BOTH arms.
 *       Dropping it costs 14 exact rows.
 * @note The angle-table element is taken by address (`ep = &arr[i]`) so the base
 *       register leads the index in the address `addu` (target order); the plain
 *       subscript reverses the operands and costs one row per arm.
 * @note `arr = g_field_node_angle_table` is read at the top of the node block so its load fills
 *       the load-delay slot after the mode reload and hoists above the base
 *       select, matching the target scheduling.
 * @note The `(s16)(u16)` cast on the halved unk30 read is a non-factor here: the
 *       plain `scene->header->unk30 / 2` (s16 field) also matches.
 *
 * @see decomp.me (100%) TODO
 */
void field_update_part_sweep(FieldPart* part)
{
    FieldScene* scene;
    FieldNode* node;
    FieldNodeDef* def;
    s32 mode;
    u32 divisor;
    s32 sin_val;
    s32 neg_sin;
    s32 base;
    s32 count;
    s32 x;
    s32 val;
    s32 y;
    s32 old;
    s16* arr;
    s16* ep;

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
    sin_val = rsin((part->sweep_phase << 12) / part->sweep_period);
    if (sin_val >= 0)
    {
        part->rotation_angle = sin_val / divisor;
    }
    else
    {
        part->rotation_angle = 0x1000 - ((u32)-sin_val / divisor);
    }
    neg_sin = -rsin(part->rotation_angle);
    if (part->node_count != 0)
    {
        arr = g_field_node_angle_table;
        mode = (part->def->u.word >> 12) & 0xF;
        switch (mode)
        {
        case 1:
        case 2:
            base = scene->header->unk30 / 2;
            break;
        case 3:
            base = 0;
            break;
        case 4:
        default:
            base = scene->header->unk30;
            break;
        }
        node = scene->nodes;
        count = part->node_count;
        if (node != NULL)
        {
            do
            {
                if (node->part == part)
                {
                    val = base;
                    def = node->def;
                    ep = &arr[def->x_angle_index * 2];
                    x = (*ep - val) * neg_sin;
                    val = x >> 4;
                    if (x < 0)
                    {
                        val = (x + 0xF) >> 4;
                    }
                    y = def->base_x << 8;
                    if ((val + y) < 0)
                    {
                        val = -y;
                    }
                    old = node->x;
                    node->x = val;
                    node->delta_x = val - old;
                    ep = &arr[def->y_angle_index * 2];
                    x = (*ep - base) * neg_sin;
                    val = x >> 4;
                    if (x < 0)
                    {
                        val = (x + 0xF) >> 4;
                    }
                    y = def->base_y << 8;
                    if ((val + y) < 0)
                    {
                        val = -y;
                    }
                    old = node->y;
                    count -= 1;
                    node->y = val;
                    node->delta_y = val - old;
                    if (count == 0)
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
 * @brief Blit one frame of an animation into the cel's packed tile array.
 *
 * The cel keeps its tiles packed: only grid cells whose bit is set in
 * FieldAnimCel::mask have a record in FieldAnimCel::tiles, and each record is
 * `stride` bytes wide. This walks the whole grid in raster order, tracking the
 * destination cursor across every present tile, and copies the frame's records
 * over the sub-rectangle (@p def unkC/unkD origin, unkE/unkF extent). Rows above
 * the sub-rectangle are skipped by advancing the cursor only; the walk returns
 * as soon as it passes the bottom row.
 *
 * The record width is 12 bytes, less 4 for each of the cel's two optional
 * fields, and 0 for the formats that have no records at all.
 *
 * @param def   Animation definition; supplies the sub-rectangle and the grid.
 * @param anim  Animation node holding the frame data and the target cel.
 * @param frame Frame index into FieldAnim::frame_data.
 *
 * @note Built -G4 WITHOUT --expand-div, like the rest of field_animation.c.
 */
void field_blit_animation_frame(FieldAnimDef* def, FieldAnim* anim, s32 frame)
{
    FieldAnimCel* cel;
    FieldTileGrid* grid;
    u8* dst;
    u32* src;
    u32* mask;
    u32 word;
    u32 bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 i;

    cel = anim->cels;
    grid = ((FieldTileAnimDef*)def)->grid;
    dst = cel->tiles;
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
    src = (u32*)(anim->frame_data + anim->frame_tile_count * stride * frame);
    bit = 1;
    mask = cel->mask;
    word = *mask++;
    for (row = 0; row != grid->u.b.rows; row++)
    {
        if (row < def->unkD)
        {
            /* Above the sub-rectangle: step the cursor over the whole row. */
            col = grid->u.b.cols;
            col--;
            while (col != -1)
            {
                if (word & bit)
                {
                    dst += stride;
                }
                bit <<= 1;
                if (bit == 0)
                {
                    word = *mask++;
                    bit = 1;
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
                if (word & bit)
                {
                    if ((col >= def->unkC) && (col < def->unkC + def->unkE))
                    {
                        i = stride >> 2;
                        i--;
                        while (i != -1)
                        {
                            *(u32*)dst = *src++;
                            dst += 4;
                            i--;
                        }
                    }
                    else
                    {
                        dst += stride;
                    }
                }
                bit <<= 1;
                if (bit == 0)
                {
                    word = *mask++;
                    bit = 1;
                }
            }
        }
    }
}





u8* field_find_count_table_span(u8*, s32, volatile s8*);
void func_8005A984(FieldPart*, s32, s32);
void func_8005AA68(FieldObj*, s32, s32);

/**
 * @brief Apply the current keyframe's tweened offsets to an animation's target.
 *
 * Resolves the keyframe indexed by FieldAnim::flags.b.state and the keyframe's
 * duration (field_find_count_table_span), then interpolates each of the key's three offsets by
 * the fraction of the keyframe already elapsed: `(elapsed * end << 8) / duration`.
 *
 * The target is FieldAnim::cels reinterpreted according to the definition's
 * handler kind: kind 5 drives a FieldPart (offsets 0x28/0x2C/0x30, visibility
 * byte 0x20), every other kind drives a FieldObj (offsets 0x1C/0x20/0x24,
 * visibility bit 0 of the flags word).
 *
 * @param def    Animation definition; supplies the handler kind and keyframe table.
 * @param anim   Animation node holding the frame state, counter and target.
 * @param apply_to_target When zero the tweened values are only recorded; when
 *               non-zero the delta since the previous frame is also added to the
 *               target and pushed through func_8005A984 / func_8005AA68. The
 *               record is cleared instead of stored on the keyframe's last frame
 *               (counter == 0), so the next keyframe starts from zero.
 *
 * @note The keyframe table is read through @c rec rather than @c def. Both hold
 *       the same pointer, but folding the access onto @c def costs the match.
 * @note @c obj and @c part must be cleared by two separate statements; the
 *       chained @c obj = part = NULL form does not match.
 * @note The handler kind is read as a whole word (@c *(s32 *) &def->flags): the
 *       byte access gcc emits for @c def->flags is an @c lbu, the target an @c lw.
 *       Repeating the test at each of the four sites is also required - hoisting
 *       it into a local reorders the blocks.
 * @note @c visibility must stay unsigned so its bit comes out as @c srl 15.
 *
 * @see decomp.me (100%) TODO
 */
void field_apply_animation_tween(FieldAnimDef* def, FieldAnim* anim, s32 apply_to_target)
{
    FieldAnimDef* rec;
    FieldObj* obj;
    FieldPart* part;
    FieldTweenKey* key;
    s32 duration;
    s32 elapsed;
    s32 value;
    s32 delta;
    volatile s8 base;

    rec = def;
    obj = NULL;
    part = NULL;
    if ((*(s32*)&def->flags & 7) == 5)
    {
        part = (FieldPart*)anim->cels;
    }
    else
    {
        obj = (FieldObj*)anim->cels;
    }
    key = (FieldTweenKey*)(rec->data + anim->flags.b.state * 8);
    duration = ((FieldTweenSpan*)field_find_count_table_span((u8*)def, anim->flags.b.keyframe, &base))->duration;
    if (duration == 0)
    {
        duration = 1;
    }
    elapsed = duration - anim->timer;
    if ((*(s32*)&def->flags & 7) == 5)
    {
        part->visible = key->visibility >> 15;
    }
    else
    {
        obj->flags.word = (obj->flags.word & ~1) | (key->visibility >> 15);
    }

    value = ((elapsed * key->x) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_x;
        if ((*(s32*)&def->flags & 7) == 5)
        {
            part->x += delta;
            func_8005A984(part, delta, 0);
        }
        else
        {
            obj->x += delta;
            func_8005AA68(obj, delta, 0);
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

    value = ((elapsed * key->y) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_y;
        if ((*(s32*)&def->flags & 7) == 5)
        {
            part->y += delta;
            func_8005A984(part, delta, 1);
        }
        else
        {
            obj->y += delta;
            func_8005AA68(obj, delta, 1);
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

    value = ((elapsed * key->z) << 8) / duration;
    if (apply_to_target != 0)
    {
        delta = value - anim->tween_z;
        if ((*(s32*)&def->flags & 7) == 5)
        {
            part->z += delta;
            func_8005A984(part, delta, 2);
        }
        else
        {
            obj->z += delta;
            func_8005AA68(obj, delta, 2);
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





void akao_play_sfx(s32, s32, s32, s32);
void akao_cmd_21(s32, s32);
void akao_cmd_a1(s32, s32, s32, s32);
void akao_cmd_a3(s32, s32, s32, s32);

/**
 * @brief Play or update the sound attached to an animation's current keyframe.
 *
 * Reads the keyframe indexed by FieldAnim::flags.b.state out of the table at
 * FieldAnimDef::data and does nothing unless it is a sound entry (kind 1).
 * The entry names either a channel slot (1 << (slot - 1), sound id 0) or a
 * sound id, and its flag bits pick one of three actions: stop the channel
 * (akao_cmd_21), play without positioning, or position the sound in the scene
 * first.
 *
 * The positional path projects the owning object and part into screen space --
 * camera offsets at 0x801ED480 (suppressed when the object definition's
 * "no offsets" bit is set), plus the part's own offsets and its grid origin --
 * and maps the result to a volume and an attenuation. Horizontally, x below
 * -0x20 or above 0x160 falls off in steps of four toward 0 / 0xFF, and the
 * range between them is a linear 0x40..0xBF ramp. Vertically, y outside
 * -0x20..0x100 subtracts the same quarter-step from the entry's base
 * attenuation, clamped at zero.
 *
 * @param def  Animation definition; supplies the keyframe table.
 * @param anim Animation node; supplies the frame index, the owning part
 *             (FieldAnim::cels) and object (FieldAnim::unk10), the repeat
 *             counter, and the retrigger flag (bit 3 of FieldAnim::flags).
 *
 */
void field_update_animation_sfx(FieldAnimDef* def, FieldAnim* anim)
{
    FieldPart* part;
    FieldObj* obj;
    FieldSfxKey* key;
    s32 sfx_id;
    s32 chan_mask;
    s32 kind;
    s32 cam_y;
    s32 grid_y_offset;
    s32 x;
    s32 y;
    u32 vol;
    u32 att;
    u32 tmp;
    s32 col;
    s32 q;
    s32 obj_y;
    s32 key_offset;
    FieldPartDef* row_def;

    part = (FieldPart*)anim->cels;
    obj = (FieldObj*)anim->unk10;
    key_offset = anim->flags.b.state * 8;
    kind = def->data[key_offset] & 7;
    if (kind == 1)
    {
        key = (FieldSfxKey*)(def->data + anim->flags.b.state * 8);
        if (key->control.word & 0x1F00)
        {
            chan_mask = kind << (((key->control.word >> 8) & 0x1F) - 1);
            sfx_id = 0;
        }
        else
        {
            chan_mask = 0;
            sfx_id = key->sfx_id & 0x3FF;
        }
        if (key->control.word & 0x8000)
        {
            if (key->control.word & 0x4000)
            {
                if (key->sound.word & 0x8000)
                {
                    if (anim->flags.word & 8)
                    {
                        akao_play_sfx(sfx_id, chan_mask, key->sound.b.lo, (key->sound.word >> 8) & 0x7F);
                        anim->flags.word &= ~8;
                    }
                }
                else
                {
                    akao_play_sfx(sfx_id, chan_mask, key->sound.b.lo, (key->sound.word >> 8) & 0x7F);
                }
            }
            else
            {
                if (obj->def->flags & 2)
                {
                    x = 0;
                    cam_y = 0;
                    y = 0;
                }
                else
                {
                    x = ((FieldCamera*)0x801ED480)->x;
                    cam_y = ((FieldCamera*)0x801ED480)->y;
                    y = ((FieldCamera*)0x801ED480)->z;
                }
                if (x >= 0)
                {
                    q = x >> 8;
                }
                else
                {
                    q = (x + 0xFF) >> 8;
                }
                x = q;
                if (cam_y >= 0)
                {
                    cam_y = cam_y >> 8;
                }
                else
                {
                    cam_y = (cam_y + 0xFF) >> 8;
                }
                if (y >= 0)
                {
                    q = y >> 9;
                    cam_y = cam_y - q;
                }
                else
                {
                    q = (y + 0x1FF) >> 9;
                    cam_y = cam_y - q;
                }
                y = cam_y;
                {
                    s32 position;
                    s32 mid;
                    s32 columns;
                    FieldPartDef* part_def;

                    part_def = part->def;
                    columns = part_def->u.b.cols;
                    position = obj->x + part->x;
                    col = columns * 8;
                    do
                    {
                        mid = x + position / 256;
                    } while (0);
                    x = mid + col;
                    do
                    {
                        row_def = *(FieldPartDef* volatile*)&part->def;
                    } while (0);
                }
                obj_y = obj->y;
                {
                    s32 coordinate;
                    s32 part_value;
                    s32 y_sum;
                    s32 mid;

                    part_value = row_def->u.b.rows;
                    coordinate = part->y;
                    att = part_value * 8;
                    y_sum = obj_y + coordinate;
                    coordinate = obj->z;
                    part_value = part->z;
                    mid = y + (y_sum * 2 - (coordinate + part_value)) / 512;
                    grid_y_offset = att - 0xE0;
                    y = mid - grid_y_offset;
                }
                if (x < -0x20)
                {
                    vol = (-0x20 - x) >> 2;
                    if (vol < 0x3F)
                    {
                        vol = 0x3F - vol;
                    }
                    else
                    {
                        vol = 0;
                    }
                }
                else if (x > 0x160)
                {
                    vol = (x - 0x160) >> 2;
                    tmp = vol + 0xC0;
                    if (tmp < 0x100)
                    {
                        vol = tmp;
                    }
                    else
                    {
                        vol = 0xFF;
                    }
                }
                else
                {
                    vol = ((x + 0x20) * 0x7F) / 384 + 0x40;
                }
                if (y < -0x20)
                {
                    att = (-0x20 - y) >> 2;
                    tmp = (key->sound.word >> 8) & 0x7F;
                    if (att < tmp)
                    {
                        att = tmp - att;
                    }
                    else
                    {
                        att = 0;
                    }
                }
                else if (y > 0x100)
                {
                    att = (y - 0x100) >> 2;
                    tmp = (key->sound.word >> 8) & 0x7F;
                    if (att < tmp)
                    {
                        att = tmp - att;
                    }
                    else
                    {
                        att = 0;
                    }
                }
                else
                {
                    att = (key->sound.word >> 8) & 0x7F;
                }
                if (key->sound.word & 0x8000)
                {
                    if (anim->flags.word & 8)
                    {
                        akao_play_sfx(sfx_id, chan_mask, vol, att);
                        anim->flags.word &= ~8;
                    }
                    else
                    {
                        akao_cmd_a1(sfx_id, chan_mask, anim->timer * 2, att);
                        akao_cmd_a3(sfx_id, chan_mask, anim->timer * 2, vol);
                    }
                }
                else
                {
                    akao_play_sfx(sfx_id, chan_mask, vol, att);
                }
            }
        }
        else
        {
            akao_cmd_21(sfx_id, chan_mask);
        }
    }
}

/**
 * @brief Re-point every visible tile of a cel at the frame's VRAM band.
 *
 * Walks @p cel 's bit plane row-major, consuming one tile record per set bit,
 * and rewrites the CLUT halfword at offset 2 of each record so it addresses the
 * band of VRAM holding frame @p frame. Only tiles whose descriptor row falls
 * inside the definition's sub-rectangle (@c unkE for @c unk10 rows) are
 * touched; the rest keep whatever CLUT they were built with. The whole call is
 * skipped unless the definition's packing mode agrees with the grid's.
 *
 * The CLUT id is the usual `(y << 6) | (x >> 4)` packing with the tile page
 * based at VRAM y = 0x1D8: mode 0 splits the descriptor's row/column out of one
 * byte, any other mode treats the whole byte as the row.
 *
 * @param anim_def Animation definition; supplies the packing mode (@c unkC),
 *                 the first row of the sub-rectangle (@c unkE) and its height
 *                 (@c unk10), which doubles as the per-frame band stride.
 * @param cel      Cel whose bit plane, tile records and record format are used.
 * @param frame    Frame index; scales the band stride to reach that frame.
 *
 * @note @c def is a local copy of the parameter rather than the parameter
 *       itself. The extra reference is what pushes @p frame out to s0 and keeps
 *       the definition pointer in a0; using the parameter directly rotates
 *       def/last/mode through the wrong three registers (99.00%).
 * @note The switch needs the otherwise-empty `case 1:` and `case 6:`. They
 *       widen the case range to 0..6, which is what makes gcc emit the
 *       seven-entry jump table instead of a compare tree (89.30%).
 * @note @c grid and @c mode must be locals; re-reading @c cel->grid or
 *       @c def->unkC at the point of use costs the match (93.54% / 92.33%).
 * @note @c first and @c mode must be `s32`. As `u8` gcc re-truncates them after
 *       the byte loads (97.75%).
 * @note @c slot is computed inside the presence test, not before it; hoisting it
 *       out unfills the branch delay slot the `andi` belongs in (97.84%).
 * @note @c tile is advanced before @c col so the column reload lands in the
 *       load-delay slot ahead of the increment (98.40%).
 * @note Measured non-factors, all still 100%: `bit`/`word` as `s32` instead of
 *       `u32`, `0xC` instead of `12`, and `* 64` instead of `<< 6`.
 *
 * @see decomp.me (100%) TODO
 */
void field_retarget_cel_cluts(FieldAnimDef* anim_def, FieldAnimCel* cel, s32 frame)
{
    FieldAnimDef* def;
    FieldTileGrid* grid;
    FieldTileDesc* tile;
    u8* dst;
    s16* clut_ptr;
    u32* mask;
    u32 word;
    u32 bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 slot;
    s32 y;
    s32 first;
    s32 last;
    s32 mode;
    s16 clut;

    def = anim_def;
    grid = cel->grid;
    stride = 0;
    if (def->unkC == ((grid->u.word >> 4) & 3))
    {
        tile = grid->tiles;
        dst = cel->tiles;
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
        bit = 1;
        if (cel->tpage_word != 0)
        {
            stride -= 4;
        }
        row = 0;
        mask = cel->mask;
        first = def->unkE;
        last = first + def->unk10;
        mode = def->unkC;
        word = *mask++;
        if (grid->u.b.rows != 0)
        {
            do
            {
                col = 0;
                if (grid->u.b.cols != 0)
                {
                    clut_ptr = (s16*)(dst + 2);
                    do
                    {
                        if (word & bit)
                        {
                            u8 packed = tile->clut_slot;

                            if (packed & 0x80)
                            {
                                slot = packed & 0x1F;
                                if ((slot >= first) && (slot < last))
                                {
                                    y = slot + (frame * def->unk10);
                                    if (mode == 0)
                                    {
                                        clut = (((y >> 4) + 0x1D8) << 6) | (y & 0xF);
                                    }
                                    else
                                    {
                                        clut = (y + 0x1D8) << 6;
                                    }
                                    *clut_ptr = clut;
                                }
                            }
                            clut_ptr = (s16*)((u8*)clut_ptr + stride);
                            dst += stride;
                        }
                        bit <<= 1;
                        if (bit == 0)
                        {
                            word = *mask++;
                            bit = 1;
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
 * @brief Cross-fade an animation's two neighbouring frames into a scratch buffer.
 *
 * Picks the frame to blend against - the next or previous one, depending on
 * FieldAnim::flags bits 0 and 2, wrapping at the definition's frame count - then
 * blends it with the current frame into one of the two halves of the node's
 * scratch buffer at offset 0x40, alternating halves each call (flags bit 4).
 * Each RGB555 pixel is interpolated component-wise by the fraction of the
 * keyframe elapsed so far; pixels that are equal in both frames, and every pixel
 * while nothing has elapsed yet, are copied straight across. Bit 15 is the OR of
 * the two sources.
 *
 * @param def  Animation definition; supplies the frame count (@c unk5), the
 *             packing mode (@c unkC), the source row (@c unkE), the per-frame
 *             row count (@c unk10) and the frame-table offset (@c unk12).
 * @param anim Animation node; supplies the keyframe index, the countdown, the
 *             flags and the destination scratch buffer.
 * @return Base of the half of the scratch buffer just written, ready to be
 *         handed to a FieldImageReq as its source data.
 *
 * @note @c rec is a local copy of @p def, as in field_apply_animation_tween: the extra
 *       reference is what keeps the definition pointer's allocno ahead of the
 *       argument registers. Reading everything through @p def costs 57 rows.
 * @note The @c do/while(0) around the countdown and flag reads is required. It
 *       emits loop notes, so flow.c counts those two @p anim references at the
 *       deeper @c loop_depth; that is worth +2 to @p anim 's REG_N_REFS, which
 *       is exactly what lifts it past @c elapsed in the global.c priority
 *       formula and swaps the two into s4/s5. A plain braced block does NOT
 *       work - it emits block notes, not loop notes, and measures inert.
 *       Without the wrapper the whole s4/s5 pair is exchanged (99.55%).
 *       See [ALLOC-23] in idioms.md.
 * @note @c c and @c p must be @c u16. The zero-extends gcc emits to compare two
 *       HImode locals are what feed the shifted component reads; as @c u32 the
 *       masking collapses and 49 rows go with it.
 * @note @c step is reused in place as the loop counter. Counting down a separate
 *       variable leaves @c step live, so gcc folds the loop guard to
 *       @c step != 0 instead of comparing the decremented value against -1
 *       (98.20%).
 * @note Each index update tests the value BEFORE it is changed, so the delayed
 *       branch pass can hoist the increment out of the else arm into the delay
 *       slot. Computing the new value first and then testing the old costs an
 *       instruction (99.24%).
 * @note @c base must be @c volatile @c u8; as @c s8 the read after the second
 *       call sign-extends (98.98%).
 * @note Measured non-factors, both still 100%: @c unk10 @c * @c 0x10 vs
 *       @c << @c 4, and a separate local for the second flags read.
 *
 * @see decomp.me (100%) TODO
 */
u_long* field_blend_animation_frames(FieldAnimDef* def, FieldAnim* anim)
{
    FieldAnimDef* rec;
    FieldSceneHeader* hdr;
    u16* cur;
    u16* prev;
    u16* dst;
    u16* out;
    s32 total;
    s32 remain;
    s32 elapsed;
    s32 idx;
    s32 other;
    s32 step;
    s32 flags;
    s32 newflags;
    u16 c;
    u16 p;
    volatile u8 base;

    rec = def;
    hdr = g_field_scene.scene->header;
    total = ((FieldTweenSpan*)field_find_count_table_span((u8*)rec, anim->flags.b.keyframe, (volatile s8*)&base))->duration;
    idx = anim->flags.b.keyframe;
    do
    {
        remain = anim->timer;
        flags = anim->flags.word;
    } while (0);
    elapsed = total - remain;
    if (flags & 1)
    {
        if (flags & 4)
        {
            if (idx == 0)
            {
                idx = 1;
            }
            else
            {
                idx = idx - 1;
            }
        }
        else if (idx == rec->unk5)
        {
            idx = idx - 1;
        }
        else
        {
            idx = idx + 1;
        }
    }
    else
    {
        if (idx == rec->unk5)
        {
            idx = 0;
        }
        else
        {
            idx = idx + 1;
        }
    }
    if (*(s32*)&def->flags & 0x40)
    {
        other = (((FieldTweenSpan*)field_find_count_table_span((u8*)def, idx, (volatile s8*)&base))->range_start + idx) - base;
    }
    else
    {
        other = idx;
    }
    if (rec->unkC == 0)
    {
        step = rec->unk10 * 0x10;
        if (anim->flags.b.state == 0)
        {
            cur = (u16*)((u8*)hdr->pixel_data + (rec->unkE << 5));
        }
        else
        {
            cur = (u16*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + rec->unk12 * 2 + ((anim->flags.b.state - 1) * step) * 2);
        }
        if (other == 0)
        {
            prev = (u16*)((u8*)hdr->pixel_data + (rec->unkE << 5));
        }
        else
        {
            prev = (u16*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + rec->unk12 * 2 + ((other - 1) * step) * 2);
        }
    }
    else
    {
        step = rec->unk10 << 8;
        if (anim->flags.b.state == 0)
        {
            cur = (u16*)((u8*)hdr->pixel_data + (rec->unkE << 9));
        }
        else
        {
            cur = (u16*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + rec->unk12 * 2 + ((anim->flags.b.state - 1) * step) * 2);
        }
        if (other == 0)
        {
            prev = (u16*)((u8*)hdr->pixel_data + (rec->unkE << 9));
        }
        else
        {
            prev = (u16*)((u8*)hdr->pixel_data + hdr->pixel_stride * 2 + rec->unk12 * 2 + ((other - 1) * step) * 2);
        }
    }
    flags = anim->flags.word;
    if (flags & 0x10)
    {
        dst = &anim->buf40[step];
        newflags = flags & ~0x10;
    }
    else
    {
        dst = anim->buf40;
        newflags = flags | 0x10;
    }
    anim->flags.word = newflags;
    step--;
    out = dst;
    while (step != -1)
    {
        c = *cur++;
        p = *prev++;
        if ((elapsed == 0) || (c == p))
        {
            *dst = c;
        }
        else
        {
            *dst = ((c | p) & 0x8000) | ((((c & 0x1F) * remain) + ((p & 0x1F) * elapsed)) / total) |
                   (((((c >> 5) & 0x1F) * remain) + (((p >> 5) & 0x1F) * elapsed)) / total) << 5 |
                   (((((c >> 10) & 0x1F) * remain) + (((p >> 10) & 0x1F) * elapsed)) / total) << 10;
        }
        step--;
        dst++;
    }
    return (u_long*)out;
}

/**
 * @brief Tint every visible tile of a cel from the scratchpad colour table.
 *
 * Expands @p src 's colour into the scratchpad table at 0x1F800000 (via
 * func_8005AC50), then walks @p cel 's bit plane row-major, consuming one tile
 * record per set bit, and copies the table entry selected by each tile's
 * descriptor into that record's rgb/code word. @p shade offsets the table
 * lookup, so successive frames step through the table's brightness ramp. Only
 * tiles whose descriptor slot falls inside the definition's band
 * (@c unkC for @c unkD entries) are tinted.
 *
 * When the cel carries a shared code word (@c unk1C) the colour belongs to the
 * whole cel rather than to individual records, so the first tile that resolves
 * writes it there and the function returns immediately.
 *
 * @param def   Animation definition; supplies the first slot (@c unkC) and the
 *              slot count (@c unkD) of the band this cel may tint.
 * @param cel   Cel whose bit plane, tile records and record format are used.
 * @param src   Colour source; its two halfword triples multiply into the
 *              three-word colour handed to func_8005AC50.
 * @param shade Table offset in entries, i.e. the brightness step to sample.
 *
 * @note @c pal must be assigned BEFORE the func_8005AC50 call. Materialising
 *       0x1F800000 later leaves it in a caller-saved register that the call
 *       would clobber, so gcc rebuilds it inside the loop and the whole
 *       preheader shifts (91.75% assigned after the loop setup, 99.51%
 *       assigned just before the switch).
 * @note `bit = 1;` must sit AFTER both stride tests. One statement earlier its
 *       live range is one insn longer, which drops its allocno priority just
 *       below the record cursor's (13061 vs 13125) and exchanges a0/a1 across
 *       the whole loop (99.56%). See [ALLOC-19] for the formula.
 * @note There is deliberately no cursor local for the record colour: writing
 *       through @c dst lets gcc build the induction variable itself and base it
 *       on the blue byte, matching the target's `sh -0x2(a1)` / `sb 0x0(a1)`
 *       pair. This is the opposite of field_retarget_cel_cluts, which needs an explicit
 *       cursor. A `FieldCellTint *` cursor initialised from @c dst also
 *       measures 100%; one initialised from @c dst @c + @c 4 does not.
 * @note The switch needs the otherwise-empty `case 1:` and `case 6:` to emit a
 *       jump table rather than a compare tree (84.91%), as in field_retarget_cel_cluts.
 * @note @c code must be read into a local before the loop; testing
 *       @c cel->code_word at each of its three sites reloads it (94.89%).
 * @note @c entry must be one expression; splitting it into
 *       `entry = &pal[slot]; entry += shade;` costs the hoisted `shade * 4`
 *       (99.92%).
 * @note @c first must be a local; inlining @c def->unkC into the range test
 *       costs an instruction (98.07%).
 * @note Measured non-factors, all still 100%: `slot <= last` vs `last >= slot`,
 *       @c slot as @c u8, `0xC` vs `12`, and inlining @c tab into the call.
 *
 * @see decomp.me (100%) TODO
 */
void field_tint_animation_cel(FieldAnimDef* def, FieldAnimCel* cel, FieldTintSrc* src, s32 shade)
{
    FieldTileGrid* grid;
    FieldTileDesc* tile;
    FieldTintColor* pal;
    FieldTintColor* entry;
    u8* dst;
    u16* tab;
    u32* mask;
    u32 word;
    u32 bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 first;
    s32 last;
    s32 slot;
    s32 code;
    s32 rgb[3];

    rgb[0] = src->red * src->red_scale;
    rgb[1] = src->green * src->green_scale;
    rgb[2] = src->blue * src->blue_scale;
    stride = 0;
    pal = (FieldTintColor*)0x1F800000;
    tab = src->palette->data;
    func_8005AC50(tab + 2, tab[0], rgb);
    grid = cel->grid;
    dst = cel->tiles;
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
    code = cel->code_word;
    if (code != 0)
    {
        stride -= 4;
    }
    if (cel->tpage_word != 0)
    {
        stride -= 4;
    }
    bit = 1;
    row = 0;
    mask = cel->mask;
    first = def->unkC;
    last = first + def->unkD;
    word = *mask++;
    if (grid->u.b.rows != 0)
    {
        do
        {
            col = 0;
            if (grid->u.b.cols != 0)
            {
                do
                {
                    if (word & bit)
                    {
                        if (tile->clut_slot & 0x80)
                        {
                            slot = tile->color_index;
                            if ((slot >= first) && (last >= slot))
                            {
                                entry = &pal[slot] + shade;
                                if (code != 0)
                                {
                                    ((FieldTintColor*)&cel->code_word)->rg = entry->rg;
                                    ((FieldTintColor*)&cel->code_word)->b = entry->b;
                                    return;
                                }
                                ((FieldCellTint*)dst)->rg = entry->rg;
                                ((FieldCellTint*)dst)->b = entry->b;
                            }
                            else
                            {
                                if (code != 0)
                                {
                                    return;
                                }
                            }
                        }
                        dst += stride;
                    }
                    bit <<= 1;
                    if (bit == 0)
                    {
                        word = *mask++;
                        bit = 1;
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
 * @brief Tint every visible tile of a whole cel list from the scratchpad table.
 *
 * The list variant of field_tint_animation_cel: instead of being handed one cel, it takes
 * the tint source itself and walks the cel list hanging off @c src->cels.
 * @p src 's colour is expanded once into the scratchpad table at 0x1F800000 (via
 * func_8005AC50), then each cel's bit plane is walked row-major, consuming one
 * tile record per set bit and copying the table entry selected by that tile's
 * descriptor into the record's rgb/code word. @p shade offsets the table lookup,
 * so successive frames step through the table's brightness ramp. Only tiles
 * whose descriptor slot falls inside the definition's band (@c unkC for @c unkD
 * entries) are tinted.
 *
 * When a cel carries a shared code word (@c unk1C) the colour belongs to the
 * whole cel rather than to its individual records, so the first tile that
 * resolves writes it there and the walk moves straight on to the next cel.
 *
 * @param def   Animation definition; supplies the first slot (@c unkC) and the
 *              slot count (@c unkD) of the band these cels may tint.
 * @param src   Colour source; its two halfword triples multiply into the
 *              three-word colour handed to func_8005AC50, and its @c unk8 is the
 *              head of the cel list to walk.
 * @param shade Table offset in entries, i.e. the brightness step to sample.
 *
 * @note @c stride is deliberately initialised once, outside the cel loop. The
 *       switch's empty `case 1:` / `case 6:` arms leave it at whatever the
 *       previous cel computed, and the two `-= 4` adjustments accumulate across
 *       the list.
 *
 * @see decomp.me (100%) TODO
 */
void field_tint_animation_cel_list(FieldAnimDef* def, FieldTintSrc* src, s32 shade)
{
    FieldAnimCel* cel;
    FieldTileGrid* grid;
    FieldTileDesc* tile;
    FieldTintColor* pal;
    FieldTintColor* entry;
    u8* dst;
    u16* tab;
    u32* mask;
    u32 word;
    u32 bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 first;
    s32 last;
    s32 slot;
    s32 code;
    s32 rgb[3];

    rgb[0] = src->red * src->red_scale;
    rgb[1] = src->green * src->green_scale;
    rgb[2] = src->blue * src->blue_scale;
    stride = 0;
    pal = (FieldTintColor*)0x1F800000;
    tab = src->palette->data;
    func_8005AC50(tab + 2, tab[0], rgb);
    first = def->unkC;
    last = first + def->unkD;
    for (cel = src->cels; cel != NULL; cel = cel->next)
    {
        dst = cel->tiles;
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
        code = cel->code_word;
        if (code != 0)
        {
            stride -= 4;
        }
        if (cel->tpage_word != 0)
        {
            stride -= 4;
        }
        bit = 1;
        row = 0;
        mask = cel->mask;
        word = *mask++;
        if (grid->u.b.rows != 0)
        {
            do
            {
                col = 0;
                if (grid->u.b.cols != 0)
                {
                    do
                    {
                        if (word & bit)
                        {
                            if (tile->clut_slot & 0x80)
                            {
                                slot = tile->color_index;
                                if ((slot >= first) && (last >= slot))
                                {
                                    entry = &pal[slot] + shade;
                                    if (code != 0)
                                    {
                                        ((FieldTintColor*)&cel->code_word)->rg = entry->rg;
                                        ((FieldTintColor*)&cel->code_word)->b = entry->b;
                                        goto next_cel;
                                    }
                                    ((FieldCellTint*)dst)->rg = entry->rg;
                                    ((FieldCellTint*)dst)->b = entry->b;
                                }
                                else
                                {
                                    if (code != 0)
                                    {
                                        goto next_cel;
                                    }
                                }
                            }
                            dst += stride;
                        }
                        bit <<= 1;
                        if (bit == 0)
                        {
                            word = *mask++;
                            bit = 1;
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
 * @brief Step an animation node to its next keyframe and refresh its counter.
 *
 * Advances FieldAnim::flags.b.keyframe - the keyframe cursor - according to the
 * node's play mode, then resolves the new keyframe through field_find_count_table_span and
 * reloads FieldAnim::timer with its length.
 *
 * The mode comes from the low bits of the flags word. Bit 0 selects ping-pong
 * play, in which bit 2 records that the cursor is currently walking backwards:
 * the cursor counts down to 0, turns around, counts up to FieldAnimDef::unk5,
 * and turns around again. Without bit 0 the cursor simply counts up and wraps
 * to 0. Either way, each time the cursor reaches an end of the range the loop
 * counter @c unk28 is spent; when it runs out, bit 6 (keep playing) is cleared
 * and the animation stops. Bit 3 is a one-shot skip request: it is consumed and
 * the cursor is left alone.
 *
 * Whether an end-of-range actually spends a repeat depends on the definition's
 * handler: bit 4 of the word at FieldAnimDef::flags exempts it entirely, and
 * otherwise only the @c handler_group / kind pairs 0/5, 0/6 and 1/5 are counted.
 *
 * The published frame index (FieldAnim::flags.b.state) is normally the cursor
 * itself; for handlers with bit 6 set it is instead the keyframe's running span
 * total plus the cursor, minus the offset field_find_count_table_span reports.
 *
 * @param def  Animation definition; supplies the handler kind (the word at
 *             @c flags, plus @c handler_group) and the last keyframe index
 *             (@c unk5).
 * @param anim Animation node whose cursor, loop counter, flags and frame
 *             counter are updated in place.
 *
 * @note The four handler-kind tests read the definition as a whole word
 *       (@c *(s32 @c *) @c &def->flags) and are repeated at each site, as in
 *       field_apply_animation_tween; hoisting the word into a local reorders the blocks.
 * @note The two end-of-range predicates are spelled differently on purpose, and
 *       both spellings are required. Where the test is positive it uses the
 *       packed @c (kind @c & @c 0xFF000007) @c == @c 0x01000005 form; splitting
 *       that into @c handler_group @c == @c 1 @c &&
 *       @c (kind @c & @c 7) @c == @c 5 costs
 *       16 rows. Where it is negative it is written as a three-way OR ending in
 *       @c handler_group @c >= @c 2, which is what makes gcc emit the shared
 *       @c sltiu @c handler_group, @c 2 tail; spelling it as the negation of the positive
 *       form costs 19 rows.
 * @note The outer test is `if (!(flags & 8))` with the long body first and the
 *       bit-3 clear as the `else`. Writing it the other way round puts the clear
 *       inline and stops the two flag stores cross-jumping (97.12%).
 * @note In the ping-pong arm the cursor is read straight from the field, with no
 *       local; a local costs 19 rows. In the wrap arm the @c def->unk5 test
 *       compares the cursor and the stepped value through two separate @c s32
 *       locals, and the stepped one needs the @c (u8) cast (99.59% without).
 * @note The forward arm tests `!(kind & 0x10) && cursor == 0` for the stop case
 *       and takes the step-back in the `else`; the opposite arm order costs 23
 *       rows.
 * @note Measured non-factors, all still 100%: a @c u8 local for the cursor in
 *       the forward arm, a local for the handler word in the ping-pong start
 *       arm, @c base as plain @c s8 or @c volatile @c u8, the parenthesisation
 *       of the published index, and `unk2++` vs `unk2 = unk2 + 1`.
 *
 * @see decomp.me (100%) TODO
 */
void field_advance_animation_keyframe(FieldAnimDef* def, FieldAnim* anim)
{
    FieldTweenSpan* span;
    s32 cur;
    s32 nxt;
    u8 idx;
    volatile s8 base;

    if (!(anim->flags.word & 8))
    {
        if (anim->flags.word & 1)
        {
            if (anim->flags.word & 4)
            {
                if (anim->flags.b.keyframe == 0)
                {
                    if (*(s32*)&def->flags & 0x10)
                    {
                        anim->flags.b.keyframe = anim->flags.b.keyframe + 1;
                    }
                    else if (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & 7) - 5) < 2)) || ((*(s32*)&def->flags & 0xFF000007) == 0x01000005))
                    {
                        if (anim->repeat_count == 0)
                        {
                            anim->flags.word &= ~0x40;
                        }
                        else
                        {
                            anim->repeat_count--;
                            anim->flags.b.keyframe++;
                        }
                    }
                    anim->flags.word &= ~4;
                }
                else
                {
                    anim->flags.b.keyframe = anim->flags.b.keyframe - 1;
                    if (!(*(s32*)&def->flags & 0x10) && (anim->flags.b.keyframe == 0) &&
                        (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & 7) - 5) >= 2)) ||
                         ((def->handler_group == 1) && ((*(s32*)&def->flags & 7) != 5)) || (def->handler_group >= 2)))
                    {
                        if (anim->repeat_count == 0)
                        {
                            anim->flags.word &= ~0x40;
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
                idx = anim->flags.b.keyframe;
                if (idx == def->unk5)
                {
                    if (!(*(s32*)&def->flags & 0x10) && (idx == 0))
                    {
                        anim->flags.word &= ~0x40;
                    }
                    else
                    {
                        anim->flags.b.keyframe--;
                        anim->flags.word |= 4;
                    }
                }
                else
                {
                    anim->flags.b.keyframe = idx + 1;
                }
            }
        }
        else
        {
            cur = anim->flags.b.keyframe;
            if (cur == def->unk5)
            {
                if (!(*(s32*)&def->flags & 0x10) &&
                    (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & 7) - 5) < 2)) || ((*(s32*)&def->flags & 0xFF000007) == 0x01000005)))
                {
                    if (anim->repeat_count == 0)
                    {
                        anim->flags.word &= ~0x40;
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
                nxt = cur + 1;
                anim->flags.b.keyframe = nxt;
                if (!(*(s32*)&def->flags & 0x10) && ((u8)nxt == def->unk5) &&
                    (((def->handler_group == 0) && ((u32)((*(s32*)&def->flags & 7) - 5) >= 2)) ||
                     ((def->handler_group == 1) && ((*(s32*)&def->flags & 7) != 5)) || (def->handler_group >= 2)))
                {
                    if (anim->repeat_count == 0)
                    {
                        anim->flags.word &= ~0x40;
                    }
                    else
                    {
                        anim->repeat_count--;
                    }
                }
            }
        }
        if ((anim->flags.word & 0x40) && (anim->repeat_count == 0) && (anim->flags.word & 2) && (anim->flags.b.stop_keyframe == anim->flags.b.keyframe))
        {
            anim->flags.word &= ~0x40;
        }
    }
    else
    {
        anim->flags.word &= ~8;
    }
    span = (FieldTweenSpan*)field_find_count_table_span((u8*)def, anim->flags.b.keyframe, &base);
    anim->timer = span->duration;
    if (*(s32*)&def->flags & 0x40)
    {
        anim->flags.b.state = (span->range_start + anim->flags.b.keyframe) - base;
    }
    else
    {
        anim->flags.b.state = anim->flags.b.keyframe;
    }
}

/**
 * @brief Re-point every cel of an animation node at the frame's VRAM band.
 *
 * Walks the cel list hanging off @p src and runs field_retarget_cel_cluts on each one, so
 * the whole node is retargeted at frame @p frame in one call. Same list shape as
 * field_tint_animation_cel_list: the record reached through FieldAnim::cels carries its cels at
 * offset 0x08.
 *
 * @param def   Animation definition, forwarded to field_retarget_cel_cluts unchanged.
 * @param src   Record whose @c unk8 is the head of the cel list to walk.
 * @param frame Frame index, forwarded to field_retarget_cel_cluts unchanged.
 *
 * @see decomp.me (100%) TODO
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
u8* field_find_count_table_span(u8* table, s32 linear_index, volatile s8* range_start_out)
{
    u8 header_count;
    u8 raw_count;
    u8 range_start;
    u8 range_end;

    *range_start_out = 0;
    header_count = *table & 0x7F;
    while (linear_index >= header_count)
    {
        *range_start_out = header_count;
        header_count = table[7];
        *(volatile u8*)(table + 7);
        if (header_count)
        {
            table += sizeof(FieldTweenKey) * 3;
        }
        else
        {
            table += sizeof(FieldTweenKey) * 3;
        }
        raw_count = *table;
        range_start = *range_start_out;
        range_end = range_start + (raw_count & 0x7F);
        while (linear_index >= (u8)range_end)
        {
            table += sizeof(FieldTweenSpan);
            range_end = range_start + (raw_count & 0x7F);
            *range_start_out = range_end;
            raw_count = *table;
            range_start = range_end;
            range_end = range_end + (raw_count & 0x7F);
        }
        break;
    }
    return table;
}

/**
 * @brief Push a VRAM upload request onto the scene's pending-upload list.
 *
 * @param req Request to link in; its next pointer takes the old list head.
 * @see decomp.me (100%) https://decomp.me/scratch/kYAFh
 */
void field_queue_vram_upload(FieldImageReq* req)
{
    FieldScene* scene;

    scene = g_field_scene.scene;
    req->next = (FieldImageReq*)scene->uploads;
    scene->uploads = req;
}
