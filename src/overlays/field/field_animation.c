#include "field_scene_internal.h"

/** @brief Movie/streaming control block at 0x801ED500. */
#define FIELD_MOVIE_STATE ((volatile FieldMovieState*)0x801ED500)
/** @brief Field CD/movie flag word at 0x801ED800. */
#define FIELD_CD_FLAGS (*(volatile s32*)0x801ED800)

extern u16 g_field_movie_frame_width;
extern u16 g_field_movie_frame_height;
/** @brief Pointer to the signed angle table indexed by FieldNodeDef. */
extern s16* g_field_node_angle_table;

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
 * @note Match is 98.07% (1005/1058 exact rows). The residual is a cluster of
 *       gcc 2.8.0 codegen-boundary phenomena with no known source lever: the
 *       cross-jump tail-merge topology of the case-3/4/5 @c req->data stores and
 *       the coupled branch-delay-slot fill land at different byte offsets, plus
 *       a handful of sched1 slot shifts (the 0x801ED800 address hoist) and
 *       register-coloring residue (the case-5 v/w pair, the 0x798 flags reload).
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
 * @see decomp.me (98.07%) TODO
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
                            FIELD_CD_FLAGS |= 0x40;
                        }
                        /* fallthrough */
                    case 1:
                        if (cdrom_can_queue_resource(def->unk1 * 2 + 0x16A6) != 0)
                        {
                            FIELD_MOVIE_STATE->rects[0].x = def2->unkC * 4 + 0x140;
                            FIELD_MOVIE_STATE->rects[0].y = def2->unkD * 0x10 + 0x100;
                            FIELD_MOVIE_STATE->rects[0].w = def2->unkE * 4;
                            FIELD_MOVIE_STATE->rects[0].h = def2->unkF * 0x10;
                            FIELD_CD_FLAGS &= ~0x40;
                            cel = anim->cels;
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
                                    func_80059F18();
                                }
                                anim->flags.word &= ~0x40;
                                anim->timer = 1;
                                func_80084240();
                            }
                        }
                        else
                        {
                            if (def->unk1 < 2)
                            {
                                func_800157B0(2);
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
            anim = anim->next;
        } while (anim != NULL);
    }

    one = 1;
    anim = scene->strips;
    if (anim != NULL)
    {
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
                        anim->flags.word = anim->flags.word & ~0x10;
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
                    req->rect.h = one;
                    req->rect.w = def->unk5 + 1;
                    field_queue_vram_upload(req);
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
                            v = (def->unk10 + 0xF) / 0x10;
                        }
                        else
                        {
                            req->rect.x = 0;
                            req->rect.w = 0x100;
                            req->rect.y = def->unkE + 0x1D8;
                            v = def->unk10;
                        }
                        req->rect.h = v;
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

/**
 * @brief One entry of an animation's keyframe table (FieldAnimDef::data).
 *
 * The three signed deltas are the horizontal / vertical / depth offsets the
 * keyframe ends on; they are scaled by the fraction of the keyframe elapsed so
 * far. Only bit 15 of the trailing halfword is used.
 */
typedef struct
{
    /** 0x00 horizontal end offset. */
    s16 x;
    /** 0x02 vertical end offset. */
    s16 y;
    /** 0x04 depth end offset. */
    s16 z;
    /** 0x06 bit 15 is copied to the target's visibility flag. */
    u16 visibility;
} FieldTweenKey;

/**
 * @brief Count-table record returned by field_find_count_table_span.
 *
 * Only the duration is read here; field_blit_animation_frame's caller uses the same halfword
 * to reload FieldAnim::timer.
 */
typedef struct
{
    u8 _pad0;
    /** 0x01 running total of the spans before this one, in frames. */
    u8 range_start;
    /** 0x02 length of the keyframe this record covers, in frames. */
    u16 duration;
} FieldTweenSpan;

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

/**
 * @brief 16-bit field of a FieldSfxKey, addressed as a whole or by byte.
 *
 * Word 0 is read as a byte for the entry kind and as a halfword for the flag
 * bits; word 1 as a byte for the sound's bank/index and as a halfword for its
 * flag bit and base attenuation.
 */
typedef union
{
    u16 word;
    struct
    {
        u8 lo;
        u8 hi;
    } b;
} FieldSfxWord;

/**
 * @brief One entry of the sound keyframe table at FieldAnimDef::data.
 *
 * Shares the 8-byte stride with FieldTweenKey; the low three bits of byte 0
 * say which of the two an entry is (1 = sound).
 */
typedef struct
{
    /**
     * 0x00 bits 0-2 entry kind (1 = sound); bits 8-12 a channel-slot number
     * (zero means "use sfx_id" instead); bit 14 clear selects
     * positional playback; bit 15 clear stops the channel.
     */
    FieldSfxWord control;
    /**
     * 0x02 low byte is the sound's bank/index, bits 8-14 its base attenuation
     * and bit 15 selects one-shot playback over the a1/a3 pair.
     */
    FieldSfxWord sound;
    /** 0x04 bits 0-9 sound id, used when control carries no channel slot. */
    u16 sfx_id;
    u16 unk6; /* 0x06 */
} FieldSfxKey;

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
 * @warning **THIS FUNCTION IS NOT A MATCH (95.20%, 195/226 exact rows).** It is
 *          committed as work in progress and may not be functionally
 *          equivalent. Re-verify before building a release image. The running
 *          analysis, including thirteen measured-and-retired probe classes,
 *          lives in working/func_80058154/status.md.
 *
 * @note The residual is four instructions, all in the screen-position block.
 *       The target RELOADS `part->def` for the `row` term; gcc 2.8's cse
 *       deletes the second load here because the `/ 256` rounding expands to a
 *       branch around a single insn whose join label has `LABEL_NUSES == 1`, so
 *       `cse_end_of_basic_block` walks straight through it. The target also
 *       keeps `cam_y - cam_z` in its own pseudo and copies it into `y`, where
 *       regmove coalesces the two here. The other two instructions are the
 *       delay-slot `nop` pair that follows from the reload. Everything else in
 *       the block is register naming downstream of those two.
 * @note `cam_z` is deliberately reused to carry `row - 0xE0` (it is dead by
 *       then). It is worth 15 exact rows; every other carrier, including a
 *       fresh local, loses 17-20. Likewise the three camera divides must be
 *       spelled as explicit if/else rounding sharing one `q` temp, and `col` /
 *       `row` must be named statements rather than inline terms.
 * @note `kind` is read through a second, duplicate address expression so that
 *       `key` becomes a separate pseudo, matching the target's `addu` copy.
 *
 * @see decomp.me (95.20%) TODO
 */
void field_update_animation_sfx(FieldAnimDef* def, FieldAnim* anim)
{
    FieldPart* part;
    FieldObj* obj;
    FieldSfxKey* key;
    s32 sfx_id;
    s32 chan_mask;
    s32 kind;
    s32 cam_x;
    s32 cam_y;
    s32 cam_z;
    s32 x;
    s32 y;
    u32 vol;
    u32 att;
    u32 tmp;
    s32 col;
    s32 row;
    s32 q;

    part = (FieldPart*)anim->cels;
    obj = (FieldObj*)anim->unk10;
    kind = def->data[anim->flags.b.state * 8] & 7;
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
                    cam_x = 0;
                    cam_y = 0;
                    cam_z = 0;
                }
                else
                {
                    cam_x = ((FieldCamera*)0x801ED480)->x;
                    cam_y = ((FieldCamera*)0x801ED480)->y;
                    cam_z = ((FieldCamera*)0x801ED480)->z;
                }
                if (cam_x >= 0)
                {
                    q = cam_x >> 8;
                }
                else
                {
                    q = (cam_x + 0xFF) >> 8;
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
                if (cam_z >= 0)
                {
                    q = cam_z >> 9;
                }
                else
                {
                    q = (cam_z + 0x1FF) >> 9;
                }
                y = cam_y - q;
                col = part->def->u.b.cols * 8;
                x = x + (obj->x + part->x) / 256 + col;
                row = part->def->u.b.rows * 8;
                y = y + ((obj->y + part->y) * 2 - (obj->z + part->z)) / 512;
                cam_z = row - 0xE0;
                y = y - cam_z;
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
 * @brief Walk a run-length-encoded count table to locate the record covering a
 *        given linear index, returning that record and the cumulative count
 *        consumed before it.
 *
 * The first byte of @p table holds a 7-bit count (high bit ignored). If
 * @p linear_index is below that count the table does not reach the requested
 * index, so @p range_start_out is left 0 and @p table is returned unchanged.
 * Otherwise the leading count is committed to @p range_start_out, the 0x18-byte
 * header is skipped, and the function steps through the
 * following 4-byte records, accumulating each record's 7-bit count, until the
 * running total would exceed @p linear_index. The pointer to that record is
 * returned and @p range_start_out holds the cumulative count of all preceding
 * records.
 *
 * @param table Pointer to the count table (RLE header followed by 4-byte records).
 * @param linear_index Linear index to resolve against each running total.
 * @param range_start_out Receives the count consumed before the returned record.
 * @return Pointer to the record whose range contains @p linear_index.
 *
 * @note NOT MATCHED - 99.82%, one row. The dead read of byte 7 lands in v0 where
 *       the target uses v1. Both registers are dead at that point and gcc scans
 *       hard regs in numeric order (MIPS defines no REG_ALLOC_ORDER), so it takes
 *       v0; the original had v0 occupied by something this version lets die. A
 *       `register u8 unused asm("$3")` pin closes the row and reaches 100%, but
 *       register pins are not allowed in this tree, so the row stays open until
 *       the natural shape that busies v0 is found.
 * @note @p range_start_out must stay @c volatile and the otherwise-dead read of byte 7 must
 *       be preserved; both are required for the original codegen (the reload of
 *       @c *range_start_out and the stray load).
 * @note The @c do/while(0) wrapper IS required - removing it costs 6 exact rows.
 *       It emits loop notes, which lift REG_N_REFS for everything inside and
 *       change the allocation order (see [ALLOC-23]). @c for(;;) and @c while(1)
 *       measure identically, so any loop wrapper will do; a plain @c if with a
 *       trailing @c return will not.
 * @note The loop body's apparently redundant recompute of @c range_end is genuine.
 *       Folding it into the natural
 *       `while (linear_index >= (u8) (range_start + (raw_count & 0x7F)))`
 *       form costs 9 exact rows and
 *       one instruction.
 * @note Measured inert, do not retry: every spelling of the dead read (bare
 *       expression, @c (void) cast, assignment to @c raw_count / @c range_end /
 *       @c header_count / @c range_start, @c s8 / @c s32 / @c u32 destination,
 *       array-index form, a
 *       @c volatile @c u8 @c * probe pointer, the @c register keyword without
 *       @c asm), and every declaration-order permutation of @c unused. All land
 *       in v0.
 * @see decomp.me (99.82%) TODO
 */
u8* field_find_count_table_span(u8* table, s32 linear_index, volatile s8* range_start_out)
{
    u8 header_count;
    u8 raw_count;
    u8 range_start;
    u8 range_end;
    u8 unused;

    *range_start_out = 0;
    header_count = *table & 0x7F;
    do
    {
        if (linear_index >= header_count)
        {
            *range_start_out = header_count;
            unused = *(volatile u8*)(table + 7);
            table += 0x18;
            raw_count = *table;
            range_start = *range_start_out;
            range_end = range_start + (raw_count & 0x7F);
            while (linear_index >= (u8)range_end)
            {
                table += 4;
                range_end = range_start + (raw_count & 0x7F);
                *range_start_out = range_end;
                raw_count = *table;
                range_start = range_end;
                range_end = range_end + (raw_count & 0x7F);
            }
        }
        return table;
    } while (0);
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
 * Objects whose FieldObjFlags::unk1 (parts whose FieldPart::node_count) is
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
 *
 * @note NOT MATCHED - 97.47%. Two residues remain, both in the CLUT walk:
 *       - the delay slot of the `part == NULL` guard is filled here with the
 *         hoisted `(s16)delta` sign-extension, but is a `nop` in the target;
 *       - the fourth (clut_tr) clamp keeps its result in @c v1 and stores from
 *         it, where the target moves it into @c v0 through two extra copies.
 *       Everything else, including all four clamp blocks and both call
 *       sequences, is exact. See working/func_800592B4/status.md.
 * @note TODO: this function is IN the build but does not byte-match, so the
 *       field_animation segment will not either until the two residues above are
 *       closed.
 * @note @c delta must be @c s32 with an explicit @c (s16) cast at each of the
 *       four uses. Declaring it @c s16 lets gcc drop the truncation of the
 *       @c /256 result and weakens the @c pos->z load to @c lhu (-4 exact
 *       rows).
 * @note @c zpos must be read into a local before the subtraction; folding it
 *       back into the expression evaluates the divide first and schedules the
 *       @c lh out of the load-delay slot (-2 exact rows).
 * @see decomp.me (97.47%) TODO
 */
void field_set_object_position(s32 obj_index, s32 part_index, FieldPos* pos, s32 rebias_cluts)
{
    FieldObj* obj;
    FieldPart* part;
    s32 delta;
    s32 sum;
    s32 value;
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
                sum = part->clut_bl + (s16)delta;
                if (sum > 0)
                {
                    value = sum;
                    if (value >= 0x800)
                    {
                        value = 0x7FF;
                    }
                }
                else
                {
                    value = 0;
                }
                part->clut_bl = value;

                sum = part->clut_tl + (s16)delta;
                if (sum > 0)
                {
                    value = sum;
                    if (value >= 0x800)
                    {
                        value = 0x7FF;
                    }
                }
                else
                {
                    value = 0;
                }
                part->clut_tl = value;

                sum = part->clut_br + (s16)delta;
                if (sum > 0)
                {
                    value = sum;
                    if (value >= 0x800)
                    {
                        value = 0x7FF;
                    }
                }
                else
                {
                    value = 0;
                }
                part->clut_br = value;

                sum = part->clut_tr + (s16)delta;
                if (sum > 0)
                {
                    value = sum;
                    if (value >= 0x800)
                    {
                        value = 0x7FF;
                    }
                }
                else
                {
                    value = 0;
                }
                part->clut_tr = value;

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
 * @note NOT MATCHED - 99.08% (281/291 exact rows). The @c g_field_scene
 *       reload in the list_kind 3 arm is solved: the unreachable
 *       @c do/while(0) above @c case @c 3 stops cse following the dispatch
 *       branch into it, worth +11 exact rows ([CSE-11] in idioms.md). What
 *       remains is the constant 1 for the @c op test, which the target
 *       re-materialises as @c xori where cse folds it into an entry-block
 *       value. The running analysis is in working/func_800597B4/status.md.
 * @note TODO: the .rodata for this jump table is NOT wired. The @c unk1_e
 *       .rodata subsegment at 0x115 is exactly this table, so it now has two
 *       producers: the splat-generated bytes and gcc own copy in field_animation.o.
 *       Reassigning or dropping that subsegment is the outstanding config
 *       step, on top of the jump-table alignment problem already described at
 *       the top of this file.
 * @note The three per-case definition pointers are deliberately separate
 *       locals. Sharing one @c def across the op 2/0/4 bodies, the op 1 body
 *       and the list_kind 3 loop unions their live ranges into a single pseudo
 *       that has to survive the recursive call, which costs a fourth saved
 *       register and 39 exact rows.
 * @note Case 2 falls through into case 0 on purpose - that is what the target
 *       does, and it is why case 0 re-reads @c anim->def.
 *
 * @see decomp.me (99.08%) TODO
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
        /*
         * Unreachable, and required to match; see [CSE-11] in idioms.md and
         * the same construct in func_8005A84C.
         */
        do
        {
        } while (0);
    case 3:
        seq = g_field_scene.scene->seqs;
        if (op != 1)
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
        anim->repeat_count = 0;
        anim->flags.word &= ~2;
        anim->flags.word = (anim->flags.word & ~1) | ((*(u32*)&def->flags >> 3) & 1);
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
