#include "field_scene_internal.h"
#include "scene_state.h"
#include "field_calls.h"

/**
 * @brief GPU primitive as field_emit_sprite_grid writes it: four raw words.
 *
 * Layout-compatible with SPRT_16 (tag / rgb+code / x0+y0 / u0+v0+clut) and,
 * for the 8-byte form, with DR_TPAGE. It is declared as plain words rather
 * than reusing those Psy-Q types because every field is written as one whole
 * 32-bit store; going through setaddr/setlen or the byte members turns each
 * tag write into a read-modify-write and costs the match.
 */
typedef struct
{
    u32 tag;  /* 0x00 */
    u32 code; /* 0x04 */
    u32 xy;   /* 0x08 packed (y << 16) | (x & 0xFFFF) */
    u32 uv;   /* 0x0C uv pair plus CLUT id */
} FieldPrim;

/**
 * @brief One entry of FieldPart::records, consumed per set bit plane bit.
 *
 * The stride is 0xC bytes, less 4 when the part carries a global code word and
 * another 4 when it carries a global texture page, so unk4/unk8 are only
 * present in the longer forms.
 */
typedef struct
{
    /** 0x00 uv pair plus CLUT id; -1 means the cell emits nothing. */
    s32 uv_clut;
    /** 0x04 rgb/code word used when the part has no global code word. */
    s32 rgb_code;
    /** 0x08 texture-page word tested against the running page code. */
    s32 tpage;
} FieldCellRec;

/**
 * @brief POLY_FT4 as field_emit_rotated_sprite_grid writes it: ten raw words.
 *
 * Layout-compatible with Psy-Q's POLY_FT4 (tag / rgb+code / four x,y pairs each
 * followed by its u,v pair). It is declared as plain words rather than reusing
 * POLY_FT4 because every field is written as one whole 32-bit store: the vertex
 * slots take a packed (x,y) pair straight out of the point buffer, and going
 * through the byte members or setXY0 would turn each into a read-modify-write.
 */
typedef struct
{
    u32 tag;  /* 0x00 */
    u32 code; /* 0x04 */
    u32 xy0;  /* 0x08 */
    u32 uv0;  /* 0x0C uv pair plus CLUT id, straight from the record */
    u32 xy1;  /* 0x10 */
    u32 uv1;  /* 0x14 uv pair plus texture page */
    u32 xy2;  /* 0x18 */
    u32 uv2;  /* 0x1C */
    u32 xy3;  /* 0x20 */
    u32 uv3;  /* 0x24 */
} FieldPolyPrim;

/**
 * @brief One column's rotated unit step, cached in the scratchpad at 0x1F800000.
 *
 * There are width + 1 of these, one per column edge. Each holds the column
 * offset already multiplied by the grid's sine and cosine, so the per-row pass
 * only has to add the row's contribution and shift.
 */
typedef struct
{
    s32 sin_term; /* 0x00 column offset * sin */
    s32 cos_term; /* 0x04 column offset * cos */
} FieldColStep;

/** @brief Scratchpad table of FieldColStep entries (width + 1 of them). */
#define FIELD_ROT_COL_STEPS ((FieldColStep *) 0x1F800000)

/**
 * @brief A screen-space point in one of the two scratchpad row buffers.
 *
 * The pair is compared component-wise for the viewport reject but copied into
 * the primitive as a single word, so the two views have to share storage.
 */
typedef union
{
    /** Packed (y << 16) | (x & 0xFFFF), as stored into a POLY_FT4 vertex. */
    s32 word;
    struct
    {
        s16 x;
        s16 y;
    } p;
} FieldPoint;

/** @brief First scratchpad row buffer of rotated FieldPoint corners. */
#define FIELD_ROT_ROW_A ((FieldPoint *) 0x1F800200)
/** @brief Second scratchpad row buffer of rotated FieldPoint corners. */
#define FIELD_ROT_ROW_B ((FieldPoint *) 0x1F800300)


/**
 * @brief FieldObj as field_build_render_records fills it in.
 *
 * Same layout as FieldObj, but with the 0x10..0x1B block named as the tint
 * halfwords FieldTintSrc reads (FieldObj declares it as the two words
 * field_find_shareable_part compares).
 */
typedef struct FieldObjBuild FieldObjBuild;
struct FieldObjBuild
{
    FieldObjBuild* next;   /* 0x00 */
    FieldObjDef* def;      /* 0x04 */
    FieldPart* parts;      /* 0x08 */
    FieldObjFlags flags;   /* 0x0C */
    /** 0x10 tint, 8.8 fixed point, from the definition's percentages. */
    u16 red;
    u16 green; /* 0x12 */
    u16 blue;  /* 0x14 */
    /** 0x16 second tint multiplier, 0x100 = unscaled. */
    u16 red_scale;
    u16 green_scale; /* 0x18 */
    u16 blue_scale;  /* 0x1A */
    s32 x;       /* 0x1C */
    s32 y;       /* 0x20 */
    s32 z;       /* 0x24 */
    s32 drift_x; /* 0x28 */
    s32 drift_y; /* 0x2C */
};

/**
 * @brief Generic view of a scene list element: every list links through offset 0.
 *
 * The build passes append through one tail pointer of this type, starting from
 * the list head inside FieldScene.
 */
typedef struct FieldLink
{
    struct FieldLink* next; /* 0x00 */
} FieldLink;

extern void DecDCTReset(int mode);
extern void DecDCTvlcBuild(u_short *table);
void field_prepare_animation_definitions(FieldAnimDef *def, s32 handler_group);
void field_build_animation_list(FieldAnimDef *def, u8 **arena, FieldAnim **tail);
void field_build_sprite_tile_record(FieldTileDesc *desc, FieldTileRec *record, s32 texture_depth, s32 record_flags);
void field_build_quad_tile_record(FieldTileDesc *desc, FieldTileRec *record, s32 texture_depth, s32 record_flags);
FieldPart *field_find_shareable_part(FieldScene *scene, FieldObj *obj, FieldPart *part, FieldTileDesc *key);
/* Shared work-area word at 0x80180008, adjacent to g_field_scene.
   Referencing it as a symbol (rather than as an offset off a literal base) is
   what makes gcc emit the target's %hi/%lo relocation here. */
extern u16 D_80180008;

/**
 * @brief Build the scene's per-object render records.
 *
 * Allocates the scene nodes, objects, parts and their bit planes and record
 * streams out of the FIELD work arena, links them into the FieldScene lists,
 * builds the animation and sequence lists, starts the sequences whose start
 * mask includes @p object_index, and sets up the MDEC VLC table when the scene
 * has a movie. The arena top is written back to the allocator state.
 *
 * @param map Map object being built; its built flag is set on completion.
 * @param object_index Index tested against each sequence's start-group mask.
 *
 * @note Shared locals (share_flags, tpage, color, instances, base) must stay
 *       one variable each across the object and record passes; the measured
 *       levers are listed in working/field/phase2/u41a-report.md.
 * @see decomp.me (95.60%) https://decomp.me/scratch/i4GmA - earlier scratch; the
 *      100% body below came from local permuter runs plus the Wave 24/25
 *      HOMING + BARRIER handoff, not from that link.
 */
void field_build_render_records(FieldMapObject *map, u16 object_index)
{
    s32 rgb[3];
    u8 code;
    struct { u8 *cur; } arena;
    FieldMemState *mem;
    FieldScene *scene;
    FieldObjBuild *prev_obj;
    s32 bit_word;
    s32 abr;
    s16 *point;
    FieldNode *node;
    u32 *bits;
    union
    {
        FieldObjBuild *obj;
        u8 *records;
    } base;
    FieldObjBuild *obj;
    FieldLink *tail;
    FieldPart *prev_part;
    FieldPartDef **part_defs;
    s16 *points;
    s32 intercept_a;
    u8 *vlc_table;
    s16 clut_value;
    s32 intercept_b;
    s32 def_word;
    s32 sprite_tpage_word;
    s32 quad_tpage_word;
    s32 code_word;
    s32 count;
    int lower_bank_bit;
    s32 bit;
    s32 tpage_state;
    s32 share_flags;
    s32 instances;
    s32 tpage;
    s32 color;
    s32 semi;
    s32 sprite_stop;
    s32 quad_stop;
    s16 hi;
    s16 lo;
    u16 *palette;
    union
    {
        FieldNodeRun *run;
        FieldMarker *marker;
        FieldObjBuild *obj;
    } rec;
    u32 slot;
    s32 clut;
    s32 stride;
    s32 words;
    FieldSeq *seq;
    FieldSeqDef *seq_def;
    s32 clamped;
    u8 attrs;
    s32 kind;
    FieldTileDesc *tiles;
    FieldPartDef *part_def;
    FieldPart *shared;
    FieldNodeDef *node_def;
    union
    {
        FieldMarkerDef *marker;
        FieldNodeDef *node;
    } def;
    FieldPart *part;
    s32 mask;
    FieldObjDef *obj_def;
    u8 *cursor;
    FieldObjDef **obj_defs;

    mem = FIELD_MEM_STATE;
    tpage = 0;
    color = 0;
    semi = 0;
    abr = 0;
    scene = FIELD_RESOURCE->scene;
    bit_word = 0;
    /* The target stores zero before the real start address. */
    arena.cur = 0;
    scene->header = (FieldSceneHeader *) map;
    scene->secondary_nodes = NULL;
    node_def = map->node_defs;
    arena.cur = (u8 *) (scene + 1);
    tail = (FieldLink *) &scene->nodes;
    points = FIELD_RESOURCE->points;
    if (node_def != NULL)
    {
        do
        {
            node = (FieldNode *) arena.cur;
            arena.cur = (u8 *) (node + 1);
            tail->next = (FieldLink *) node;
            tail = (FieldLink *) node;
            node->def = node_def;
            node->spans = 0;
            node->unk14 = 0;
            node->unk18 = *(u8 *) &(node_def)->flags >> FIELD_NODE_DEF_ENABLE_SHIFT;
            node->x_min = 0x7FFF;
            node->x_max = 0;
            node->row_end = 0;
            node->row_start = 0x7FFF;
            node->unk24 = 0;
            node->delta_x = 0;
            node->delta_y = 0;
            node->unk30 = 0;
            node->unk34 = 0;
            node->x = 0;
            node->y = 0;
            node->unk40 = 0;
            rec.run = node_def->runs;
            count = rec.run->count & FIELD_NODE_RUN_COUNT_MASK;
            while (count != 0)
            {
                point = points + rec.run->first * 2;
                while (--count != -1)
                {
                    node->x_min = (*point > node->x_min) ? node->x_min : *point;
                    node->x_max = (*point < node->x_max) ? node->x_max : *point;
                    node->row_end = (point[1] < node->row_end) ? node->row_end : point[1];
                    node->row_start = (point[1] > node->row_start) ? node->row_start : point[1];
                    point += 2;
                }
                rec.run++;
                count = rec.run->count & FIELD_NODE_RUN_COUNT_MASK;
            }
            node_def = node_def->next;
        }
        while (node_def != NULL);
    }
    tail->next = NULL;
    def.marker = map->edge_defs;
    tail = (FieldLink *) &scene->markers;
    if (def.marker != NULL)
    {
        do
        {
            rec.marker = (FieldMarker *) arena.cur;
            arena.cur = (u8 *) (rec.marker + 1);
            tail->next = (FieldLink *) rec.marker;
            tail = (FieldLink *) rec.marker;
            rec.marker->def = def.marker;
            rec.marker->x2 = def.marker->x0 + def.marker->offset_x;
            rec.marker->y2 = def.marker->y0 + def.marker->offset_y;
            rec.marker->x3 = def.marker->x1 + def.marker->offset_x;
            rec.marker->y3 = def.marker->y1 + def.marker->offset_y;
            hi = lo = def.marker->x0;
            if (hi < (s16) def.marker->x1)
            {
                hi = def.marker->x1;
            }
            if ((s16) def.marker->x1 < lo)
            {
                lo = def.marker->x1;
            }
            if (hi < (s16) rec.marker->x2)
            {
                hi = rec.marker->x2;
            }
            if ((s16) rec.marker->x2 < lo)
            {
                lo = rec.marker->x2;
            }
            if (hi < (s16) rec.marker->x3)
            {
                hi = rec.marker->x3;
            }
            if ((s16) rec.marker->x3 < lo)
            {
                lo = rec.marker->x3;
            }
            rec.marker->x_max = hi;
            rec.marker->x_min = lo;
            hi = lo = def.marker->y0;
            if (hi < (s16) def.marker->y1)
            {
                hi = def.marker->y1;
            }
            if ((s16) def.marker->y1 < lo)
            {
                lo = def.marker->y1;
            }
            if (hi < (s16) rec.marker->y2)
            {
                hi = rec.marker->y2;
            }
            if ((s16) rec.marker->y2 < lo)
            {
                lo = rec.marker->y2;
            }
            if (hi < (s16) rec.marker->y3)
            {
                hi = rec.marker->y3;
            }
            if ((s16) rec.marker->y3 < lo)
            {
                lo = rec.marker->y3;
            }
            rec.marker->y_max = hi;
            rec.marker->y_min = lo;
            rec.marker->side_dx = (s16) rec.marker->x2 - (s16) def.marker->x0;
            rec.marker->side_dy = (s16) rec.marker->y2 - (s16) def.marker->y0;
            if (rec.marker->side_dx != 0)
            {
                intercept_a = (s16) def.marker->y0 - rec.marker->side_dy * (s16) def.marker->x0 / rec.marker->side_dx;
                intercept_b = (s16) def.marker->y1 - rec.marker->side_dy * (s16) def.marker->x1 / rec.marker->side_dx;
            }
            else
            {
                intercept_a = (s16) def.marker->x0;
                intercept_b = (s16) def.marker->x1;
            }
            if (intercept_b < intercept_a)
            {
                rec.marker->side_hi = intercept_a;
                rec.marker->side_lo = intercept_b;
            }
            else
            {
                rec.marker->side_lo = intercept_a;
                rec.marker->side_hi = intercept_b;
            }
            rec.marker->edge_dx = (s16) def.marker->x1 - (s16) def.marker->x0;
            rec.marker->edge_dy = (s16) def.marker->y1 - (s16) def.marker->y0;
            if (rec.marker->edge_dx != 0)
            {
                intercept_a = (s16) def.marker->y0 - rec.marker->edge_dy * (s16) def.marker->x0 / rec.marker->edge_dx;
                intercept_b = (s16) rec.marker->y2 - rec.marker->edge_dy * (s16) rec.marker->x2 / rec.marker->edge_dx;
            }
            else
            {
                intercept_a = (s16) def.marker->x0;
                intercept_b = (s16) rec.marker->x2;
            }
            if (intercept_b < intercept_a)
            {
                rec.marker->edge_hi = intercept_a;
                rec.marker->edge_lo = intercept_b;
            }
            else
            {
                rec.marker->edge_lo = intercept_a;
                rec.marker->edge_hi = intercept_b;
            }
            def.marker = def.marker->next;
        }
        while (def.marker != NULL);
    }
    tail->next = NULL;
    obj_defs = map->object_defs;
    prev_obj = (FieldObjBuild *) &scene->objects;
    if (*obj_defs != NULL)
    {
        do
        {
            rec.obj = (FieldObjBuild *) arena.cur;
            obj_def = *obj_defs;
            arena.cur = (u8 *) (rec.obj + 1);
            prev_obj->next = rec.obj;
            rec.obj->def = obj_def;
            rec.obj->red = (obj_def->scale_x << 8) / 100;
            /* Also the record pass instance counter: one local keeps the register tie. */
            instances = 1;
            rec.obj->green = (obj_def->scale_y << 8) / 100;
            rec.obj->blue = (obj_def->scale_z << 8) / 100;
            rec.obj->red_scale = rec.obj->green_scale = rec.obj->blue_scale = 0x100;
            rec.obj->flags.word = (rec.obj->flags.word & ~1) | (*(u8 *) &obj_def->flags & 1);
            rec.obj->flags.b.node_count = 0;
            rec.obj->x = obj_def->x << 8;
            rec.obj->y = obj_def->y << 8;
            prev_obj = rec.obj;
            /* One register: the object here, the quad record base in the record pass. */
            base.obj = rec.obj;
            rec.obj->z = obj_def->z << 8;
            if ((*(s32 *) &obj_def->scroll_scale_x & 0xFFFF0000) == 0x100000)
            {
                rec.obj->flags.b.drift_speed = 0;
            }
            else
            {
                rec.obj->flags.b.drift_speed = obj_def->drift_speed;
            }
            base.obj->flags.b.drift_angle = obj_def->drift_angle;
            base.obj->drift_x = 0;
            base.obj->drift_y = 0;
            part_defs = obj_def->part_defs;
            prev_part = (FieldPart *) &base.obj->parts;
            if (*part_defs != NULL)
            {
                do
                {
                    part = (FieldPart *) arena.cur;
                    part_def = *part_defs;
                    arena.cur = (u8 *) (part + 1);
                    prev_part->next = part;
                    part->def = part_def;
                    part->visible = part_def->u.b.flags & 1;
                    def_word = part_def->u.word;
                    prev_part = part;
                    if ((def_word & 0xF00) == 0x100)
                    {
                        kind = (def_word & 0xE) + 1;
                    }
                    else
                    {
                        kind = def_word & 0xE;
                    }
                    part->kind = kind;
                    part->node_count = 0;
                    part->x = part_def->x << 8;
                    part->y = part_def->y << 8;
                    part->z = part_def->z << 8;
                    part->unk34 = 0;
                    part->sweep_period = part_def->sweep_period;
                    part->sweep_phase = instances;
                    part->row_angle = 0;
                    part->column_angle = 0;
                    part->rotation_angle = 0;
                    part->scale_x = 0x1000;
                    part->scale_y = 0x1000;
                    if (part_def->u.word & 0x40)
                    {
                        clut = obj_def->z + part_def->z + part_def->clut_bl;
                        if (clut > 0)
                        {
                            clamped = clut;
                            if (clut >= 0x800)
                            {
                                clamped = 0x7FF;
                            }
                            clut_value = clamped;
                        }
                        else
                        {
                            clut_value = 0;
                        }
                        part->clut_bl = clut_value;
                        clut = obj_def->z + part_def->z + part_def->clut_tl;
                        if (clut > 0)
                        {
                            clamped = clut;
                            if (clut >= 0x800)
                            {
                                clamped = 0x7FF;
                            }
                            clut_value = clamped;
                        }
                        else
                        {
                            clut_value = 0;
                        }
                        part->clut_tl = clut_value;
                        clut = obj_def->z + part_def->z + part_def->clut_br;
                        if (clut > 0)
                        {
                            clamped = clut;
                            if (clut >= 0x800)
                            {
                                clamped = 0x7FF;
                            }
                            clut_value = clamped;
                        }
                        else
                        {
                            clut_value = 0;
                        }
                        part->clut_br = clut_value;
                        clut = obj_def->z + part_def->z + part_def->clut_tr;
                        if (clut > 0)
                        {
                            clamped = clut;
                            if (clut >= 0x800)
                            {
                                clamped = 0x7FF;
                            }
                            clut_value = clamped;
                        }
                        else
                        {
                            clut_value = 0;
                        }
                        part->clut_tr = clut_value;
                    }
                    else
                    {
                        part->clut_bl = part_def->clut_bl;
                        part->clut_tl = part_def->clut_tl;
                        part->clut_br = part_def->clut_br;
                        part->clut_tr = part_def->clut_tr;
                    }
                    share_flags = 0;
                    tiles = part_def->tiles;
                    tpage_state = (part->kind > 0U) * 2;
                    if (tiles != NULL)
                    {
                        part->shared = field_find_shareable_part(scene, (FieldObj *) base.obj, part, tiles);
                        if (part->shared == NULL)
                        {
                            if ((part_def->u.word & 0xF00) == 0x100)
                            {
                                count = 1;
                            }
                            else
                            {
                                count = part_def->u.b.cols * part_def->u.b.rows;
                            }
                            bits = part->bits = (u32 *) arena.cur;
                            words = (count + 31) / 32;
                            part->bits_size = words * 4;
                            arena.cur = (u8 *) (bits + words);
                            bit_word = 0;
                            bit = 1;
                            while (--count != -1)
                            {
                                if (tiles->clut_slot & 0x80)
                                {
                                    bit_word |= bit;
                                    if (tpage_state == 0)
                                    {
                                        attrs = tiles->texture_attrs;
                                        tpage_state = 1;
                                        tpage = attrs & 0xF;
                                        abr = (attrs >> 4) & 3;
                                    }
                                    else if ((tpage_state == 1) && ((tpage != (tiles->texture_attrs & 0xF)) || (abr != ((tiles->texture_attrs >> 4) & 3))))
                                    {
                                        tpage_state = 2;
                                    }
                                    if (share_flags == 0)
                                    {
                                        share_flags = 1;
                                        color = tiles->color_index;
                                        semi = (tiles->texture_attrs >> 6) & 1;
                                    }
                                    else if ((share_flags == 1) && ((color != tiles->color_index) || (semi != ((tiles->texture_attrs >> 6) & 1))))
                                    {
                                        share_flags = 2;
                                    }
                                }
                                bit <<= 1;
                                if (bit == 0)
                                {
                                    *bits = bit_word;
                                    bits++;
                                    bit = 1;
                                    bit_word = 0;
                                }
                                tiles++;
                            }
                            if (bit != 1)
                            {
                                *bits = bit_word;
                            }
                            if (tpage_state == 1)
                            {
                                part->tpage_word = tpage + abr * 0x10 + 1;
                            }
                            else
                            {
                                part->tpage_word = 0;
                            }
                            if (share_flags == 1)
                            {
                                part->code_word = color + (semi << 9) + 1;
                            }
                            else
                            {
                                part->code_word = 0;
                            }
                        }
                    }
                    /* Two block levels give part_defs the loop weight to win s7 (0 or 1 level: 99.85%). */
                    do { do { part_defs++; } while (0); } while (0);
                }
                while (*part_defs != NULL);
            }
            obj_defs += 1;
            prev_part->next = NULL;
        }
        while (*obj_defs != NULL);
    }
    prev_obj->next = NULL;
    node = scene->nodes;
    if (node != NULL)
    {
        do
        {
            def.node = node->def;
            if ((D_80180008 >= 0x12) && (def.node->obj_index != 0xFF))
            {
                if (def.node->part_index != 0xFF)
                {
                    node->obj = NULL;
                    node->part = func_8005AB80(def.node->obj_index, def.node->part_index);
                    if (node->part->def->u.word & 0xF000)
                    {
                        scene->secondary_nodes = node;
                    }
                    node->part->node_count++;
                }
                else
                {
                    node->obj = func_8005AB4C(def.node->obj_index);
                    node->obj->flags.b.node_count++;
                    node->part = NULL;
                }
            }
            else
            {
                node->obj = NULL;
                node->part = NULL;
            }
            node = node->next;
        }
        while (node != NULL);
    }
    field_prepare_animation_definitions(map->anim_defs[0], 0);
    field_prepare_animation_definitions(map->anim_defs[1], 1);
    field_prepare_animation_definitions(map->anim_defs[2], 2);
    field_prepare_animation_definitions(map->anim_defs[3], 3);
    obj = (FieldObjBuild *) scene->objects;
    if (obj != NULL)
    {
        do
        {
            obj_def = obj->def;
            rgb[0] = obj->red << 8;
            rgb[1] = obj->green << 8;
            rgb[2] = obj->blue << 8;
            palette = (u16 *) obj_def->shared_source;
            func_8005AC50((u8 *) (palette + 2), *palette, rgb);
            part = obj->parts;
            code = 0;
            if (part != NULL)
            {
                do
                {
                    func_8005AD20(part->kind, *(u16 *) obj_def->shared_source, &code);
                    part_def = part->def;
                    tiles = part_def->tiles;
                    if (tiles != NULL)
                    {
                        shared = part->shared;
                        if (shared != NULL)
                        {
                            part->bits = shared->bits;
                            part->bits_size = shared->bits_size;
                            part->tpage_word = shared->tpage_word;
                            part->code_word = shared->code_word;
                            part->records = shared->records;
                            part->instance_count = shared->instance_count;
                        }
                        else
                        {
                            instances = 0;
                            switch (part->kind)
                            {
                                case 0:
                                    stride = 0xC;
                                    code_word = part->code_word;
                                    part->records = arena.cur;
                                    cursor = part->records;
                                    if (code_word != 0)
                                    {
                                        color = code_word - 1;
                                        part->code_word = FIELD_TILE_COLOR_WORDS[color & 0xFF];
                                        share_flags = FIELD_TILE_REC_SHARED_RGB_CODE;
                                        if (color & 0x200)
                                        {
                                            ((u8 *) &part->code_word)[3] |= 2;
                                        }
                                        stride = 8;
                                    }
                                    else
                                    {
                                        share_flags = 0;
                                    }
                                    sprite_tpage_word = part->tpage_word;
                                    tpage = sprite_tpage_word - 1;
                                    if (sprite_tpage_word != 0)
                                    {
                                        slot = tpage & 0xF;
                                        if (slot >= 10)
                                        {
                                            tpage = ((part_def->u.word * 8) & 0x180) | ((tpage * 2) & 0x60) | ((u32) (((slot << 6) - 0x80) & 0x3FF) >> 6);
                                        }
                                        else
                                        {
                                            /* A literal lets combine reassociate the OR; the quad case reuses it. */
                                            lower_bank_bit = 0x10;
                                            tpage = ((part_def->u.word * 8) & 0x180) | ((tpage * 2) & 0x60) | (((u32) ((slot << 6) + 0x140) >> 6) | lower_bank_bit);
                                        }
                                        part->tpage_word = (tpage & 0x9FF) | 0xE1000400;
                                        share_flags |= FIELD_TILE_REC_SHARED_TPAGE;
                                        stride -= 4;
                                    }
                                    bits = part->bits;
                                    count = part_def->u.b.cols * part_def->u.b.rows;
                                    count--;
                                    bit = 0;
                                    if (count != -1)
                                    {
                                        /* The target keeps -1 live across the call. */
                                        sprite_stop = -1;
                                        do
                                        {
                                            if (bit == 0)
                                            {
                                                bit_word = *bits;
                                                bits++;
                                                bit = 1;
                                            }
                                            if (bit_word & bit)
                                            {
                                                field_build_sprite_tile_record(tiles, (FieldTileRec *) cursor, (part_def->u.word >> 4) & 3, share_flags);
                                                cursor += stride;
                                                instances++;
                                            }
                                            bit <<= 1;
                                            count--;
                                            tiles++;
                                        }
                                        while (count != sprite_stop);
                                    }
                                    arena.cur += (u16) instances * stride;
                                    break;
                                case 1:
                                    break;
                                case 2:
                                case 3:
                                case 4:
                                case 5:
                                    code_word = part->code_word;
                                    part->records = arena.cur;
                                    base.records = part->records;
                                    cursor = base.records;
                                    stride = 0xC;
                                    if (code_word != 0)
                                    {
                                        color = code_word - 1;
                                        part->code_word = FIELD_TILE_COLOR_WORDS[color & 0xFF];
                                        share_flags = FIELD_TILE_REC_SHARED_RGB_CODE;
                                        if (color & 0x200)
                                        {
                                            ((u8 *) &part->code_word)[3] |= 2;
                                        }
                                        stride = 8;
                                    }
                                    else
                                    {
                                        share_flags = 0;
                                    }
                                    quad_tpage_word = part->tpage_word;
                                    tpage = quad_tpage_word - 1;
                                    if (quad_tpage_word != 0)
                                    {
                                        slot = tpage & 0xF;
                                        if (slot >= 10)
                                        {
                                            tpage = ((part_def->u.word * 8) & 0x180) | ((tpage * 2) & 0x60) | ((u32) (((slot << 6) - 0x80) & 0x3FF) >> 6);
                                        }
                                        else
                                        {
                                            tpage = ((part_def->u.word * 8) & 0x180) | ((tpage * 2) & 0x60) | (((u32) ((slot << 6) + 0x140) >> 6) | lower_bank_bit);
                                        }
                                        part->tpage_word = tpage;
                                        share_flags |= FIELD_TILE_REC_SHARED_TPAGE;
                                        stride -= 4;
                                    }
                                    bits = part->bits;
                                    count = part_def->u.b.cols * part_def->u.b.rows;
                                    count--;
                                    bit = 0;
                                    if (count != -1)
                                    {
                                        /* The target keeps -1 live across the call. */
                                        quad_stop = -1;
                                        do
                                        {
                                            if (bit == 0)
                                            {
                                                bit_word = *bits;
                                                bits++;
                                                bit = 1;
                                            }
                                            if (bit_word & bit)
                                            {
                                                field_build_quad_tile_record(tiles, (FieldTileRec *) cursor, (part_def->u.word >> 4) & 3, share_flags);
                                                cursor += stride;
                                                instances++;
                                            }
                                            bit <<= 1;
                                            count--;
                                            tiles++;
                                        }
                                        while (count != quad_stop);
                                    }
                                    arena.cur += (u16) instances * stride;
                                    break;
                            }
                            part->instance_count = instances;
                        }
                    }
                    else
                    {
                        part->records = NULL;
                        part->instance_count = 0;
                    }
                    part = part->next;
                }
                while (part != NULL);
            }
            obj = obj->next;
        }
        while (obj != NULL);
    }
    scene->unk38 = 0;
    *(s32 *) scene->_pad2 = 0;
    field_build_animation_list(map->anim_defs[0], &arena.cur, &scene->anims);
    field_build_animation_list(map->anim_defs[1], &arena.cur, &scene->strips);
    field_build_animation_list(map->anim_defs[2], &arena.cur, &scene->sprites);
    field_build_animation_list(map->anim_defs[3], &arena.cur, &scene->effects);
    count = FIELD_RESOURCE->seq_count;
    seq_def = FIELD_RESOURCE->seq_defs;
    tail = (FieldLink *) &scene->seqs;
    while (--count != -1)
    {
        seq = (FieldSeq *) arena.cur;
        arena.cur = (u8 *) (seq + 1);
        tail->next = (FieldLink *) seq;
        tail = (FieldLink *) seq;
        seq->def = seq_def++;
        seq->flags.word &= ~3;
    }
    /* Keeps the sllv ahead of the terminator store (plain: 98.83%). */
    do { mask = 1 << object_index; } while (0);
    tail->next = NULL;
    seq = scene->seqs;
    count = 0;
    while (seq != NULL)
    {
        if (seq->def->unk1 & mask)
        {
            func_8005A744(seq, count & 0xFF);
        }
        seq = seq->next;
        count++;
    }
    scene->uploads = NULL;
    if (scene->unk38 != 0)
    {
        vlc_table = arena.cur;
        scene->unk38 = (s32) vlc_table;
        arena.cur = vlc_table + 0x14C00;
        DecDCTReset(0);
        DecDCTvlcBuild((u_short *) scene->unk38);
    }
    mem->top = (u32) arena.cur;
    map->built = 1;
}

/**
 * @brief Prepare tile-animation definitions and their runtime presence masks.
 *
 * Assigns @p handler_group to every definition. For tile handlers in groups
 * zero and three, it verifies that the runtime part's shared TPage and
 * RGB/code words agree with every present source tile, clearing either shared
 * word when the descriptors disagree. It also rasterizes the definition's
 * source rectangle into the runtime part's row-major presence bitmap.
 *
 * @param def Head of the linked animation-definition list.
 * @param handler_group Scene animation-list group, in the range 0 through 3.
 *
 * @see decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */
void field_prepare_animation_definitions(FieldAnimDef* def, s32 handler_group)
{
    u32 shared_page_slot = 0;
    u32 shared_color_index = 0;
    u32 shared_semitrans = 0;
    u32 shared_blend_mode = 0;
    s32 tpage_status;
    s32 code_status;
    FieldAnimDef* rec;
    FieldPartDef* part_def;
    FieldPart* part;
    FieldTileDesc* tile;
    FieldTileDesc* mask_tile;
    s32 frame;
    s32 tile_index;
    u32 mask_bit;
    u32* mask;
    s32 row;
    s32 col;
    u32 mask_word;

    for (; def != NULL; def = def->next)
    {
        def->flags.b.handler_group = handler_group;
        if (!(((handler_group == 0) && ((def->flags.word & 7) < 2)) || (handler_group == 3)))
        {
            continue;
        }
        rec = def; /* second pointer to the same record; the original keeps both live */
        part_def = def->u.tile.grid;
        part = func_8005ABD8(part_def, NULL);
        if (part->shared != NULL)
        {
            part = part->shared;
        }
        if ((def->flags.word & 7) == 1)
        {
            if ((part_def->u.word & 0xF00) == 0x100)
            {
                def->u.tile.rect_width = 1;
                def->u.tile.rect_height = 1;
            }
            else
            {
                def->u.tile.rect_width = part_def->u.b.cols;
                def->u.tile.rect_height = part_def->u.b.rows;
            }
        }
        if (part->tpage_word != 0)
        {
            u32 tpage = part->tpage_word - 1;

            tpage_status = 1;
            shared_blend_mode = tpage >> 4;
            shared_page_slot = tpage & 0xF;
        }
        else
        {
            tpage_status = 0;
        }
        if (part->code_word != 0)
        {
            u32 code = part->code_word - 1;

            code_status = 1;
            shared_semitrans = code >> 9;
            shared_color_index = code & 0xFF;
        }
        else
        {
            code_status = 0;
        }
        if ((tpage_status != 0) || (code_status != 0))
        {
            frame = def->flags.b.frame_count;
            tile = (FieldTileDesc *) rec->data;
            while (--frame != -1)
            {
                tile_index = rec->u.tile.rect_width * rec->u.tile.rect_height;
                while (--tile_index != -1)
                {
                    if (tile->clut_slot & 0x80)
                    {
                        if (tpage_status == 1)
                        {
                            u8 texture_attrs = tile->texture_attrs;

                            if ((shared_page_slot != (texture_attrs & 0xF)) || (shared_blend_mode != ((texture_attrs >> 4) & 3)))
                            {
                                tpage_status = 2;
                            }
                        }
                        if ((code_status == 1) && ((shared_color_index != tile->color_index) || (shared_semitrans != ((tile->texture_attrs >> 6) & 1))))
                        {
                            code_status = 2;
                        }
                    }
                    tile++;
                }
            }
            if (tpage_status != 1)
            {
                part->tpage_word = 0;
            }
            if (code_status != 1)
            {
                part->code_word = 0;
            }
        }
        if (((handler_group == 0) && ((def->flags.word & 7) == 0)) || (handler_group == 3))
        {
            frame = def->flags.b.frame_count;
            tile = (FieldTileDesc *) rec->data;
            while (--frame != -1)
            {
                mask_tile = tile;
                mask_bit = 1;
                mask = part->bits;
                mask_word = *mask;
                for (row = 0; row != part_def->u.b.rows; row++)
                {
                    if (row < rec->u.tile.rect_y)
                    {
                        col = part_def->u.b.cols;
                        while (--col != -1)
                        {
                            mask_bit <<= 1;
                            if (mask_bit == 0)
                            {
                                *mask++ = mask_word;
                                mask_bit = 1;
                                mask_word = *mask;
                            }
                        }
                    }
                    else if (row < rec->u.tile.rect_y + rec->u.tile.rect_height)
                    {
                        for (col = 0; col != part_def->u.b.cols; col++)
                        {
                            if ((col >= rec->u.tile.rect_x) && (col < rec->u.tile.rect_x + rec->u.tile.rect_width))
                            {
                                if (mask_tile->clut_slot & 0x80)
                                {
                                    mask_word |= mask_bit;
                                }
                                mask_tile++;
                            }
                            mask_bit <<= 1;
                            if (mask_bit == 0)
                            {
                                *mask++ = mask_word;
                                mask_bit = 1;
                                mask_word = *mask;
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                if (mask_bit != 1)
                {
                    *mask = mask_word;
                }
                tile += rec->u.tile.rect_width * rec->u.tile.rect_height;
            }
        }
    }
}

/**
 * @brief Build the scene's animation node list from a definition chain.
 *
 * Walks @p def 's chain and, for each definition, bump-allocates a 0x30-byte
 * FieldAnim out of the arena at @p arena and tail-appends it to the list at
 * @p tail. Each node is seeded from its definition: the play-mode flags at
 * FieldAnim::flags, the starting keyframe cursor, the loop counter, and the
 * keyframe length from field_find_count_table_span. The handler kind - the low three bits of
 * the word at FieldAnimDef::flags, qualified by
 * FieldAnimDef::flags.b.handler_group - then selects how the node's cel list is
 * resolved (func_8005ABD8 or field_find_object_by_definition) and what
 * extra setup runs.
 *
 * For the tinted kinds the definition's colour is expanded into the scratchpad
 * table (func_8005AC50 / func_8005AD20) and the per-frame GPU primitives are
 * built into the arena: every frame walks the cel's bit plane row-major, and
 * each set bit inside the definition's sub-rectangle emits one primitive through
 * field_build_sprite_tile_record or field_build_quad_tile_record depending on the cel's record format. The arena
 * cursor is advanced past whatever each kind consumed before moving to the next
 * definition, and the list is null-terminated on the way out.
 *
 * @param def   Head of the animation definition chain; @c next links it.
 * @param arena Bump-allocation cursor; advanced past every node and primitive.
 * @param tail  Where to store the next node pointer; walked along the list and
 *              finally cleared.
 */
void field_build_animation_list(FieldAnimDef *def, u8 **arena, FieldAnim **tail)
{
    s32 rgb[3];
    u8 range_start;
    FieldTintSrc *tint_src;
    u8 primitive_code;
    FieldScene *scene;
    FieldPartDef *grid;
    u16 stagger_timer;
    s32 record_stride;
    u16 tile_count;
    FieldAnim *anim;
    FieldAnimDef *rec;
    FieldPart *cel;
    FieldSfxKey *key;
    FieldTweenSpan *span;
    u8 *arena_cursor;
    u16 *palette_data;
    FieldTileDesc *tile_data;
    FieldTileDesc *frame_descs;
    u32 *mask;
    u32 mask_word;
    u32 mask_bit;
    s32 handler_kind;
    u32 control_flags;
    u32 def_flags;
    s32 record_flags;
    s32 frame;
    s32 row;
    s32 col;
    u8 initial_state;
    u16 duration;
    u16 timer;

    cel = NULL;
    grid = NULL;
    record_stride = 0;
    tile_count = 0;
    stagger_timer = 1;
    tint_src = NULL;
    scene = g_field_scene.scene;
    for (; def != NULL; def = def->next)
    {
        anim = (FieldAnim *) *arena;
        *arena = (u8 *) &anim->upload;
        *tail = anim;
        tail = &anim->next;
        anim->def = def;
        if (!(def->head.word & 0x7F))
        {
            anim->flags.word &= ~0x40;
        }
        else
        {
            anim->flags.word = (anim->flags.word & ~0x40) | ((def->flags.b.kind_flags >> 7) << 6);
        }
        def_flags = def->flags.word;
        anim->repeat_count = 0;
        control_flags = (anim->flags.word & ~FIELD_ANIM_FLAG_PING_PONG) | ((def_flags >> 3) & 1);
        control_flags &= ~FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
        control_flags &= ~FIELD_ANIM_FLAG_REVERSE;
        control_flags &= ~FIELD_ANIM_FLAG_START_PENDING;
        control_flags &= ~FIELD_ANIM_FLAG_SECOND_BUFFER;
        control_flags &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
        anim->flags.word = control_flags;
        anim->flags.b.stop_keyframe = 0;
        if (def->flags.word & 0x40)
        {
            anim->flags.b.state = def->head.b.unk1;
            anim->flags.b.keyframe = 0;
        }
        else
        {
            initial_state = def->head.b.unk1;
            anim->flags.b.state = initial_state;
            anim->flags.b.keyframe = initial_state;
        }
        if (def->flags.b.handler_group == 3)
        {
            anim->timer = 1;
        }
        else
        {
            span = field_find_count_table_span(def, anim->flags.b.keyframe, &range_start);
            if (def->flags.word & 0x20)
            {
                anim->timer = span->duration;
            }
            else
            {
                duration = span->duration;
                if (duration < stagger_timer)
                {
                    anim->timer = duration;
                    stagger_timer = 1;
                }
                else
                {
                    timer = stagger_timer;
                    stagger_timer = timer + 1;
                    anim->timer = timer;
                }
            }
        }
        switch (def->flags.b.handler_group)
        {
        case 0:
            rec = def; /* second pointer to the definition; the original keeps both live */
            switch (rec->flags.b.kind_flags & 7)
            {
            case 0:
            case 1:
                grid = rec->u.tile.grid;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                break;
            case 2:
                grid = rec->u.tile.grid;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                if (anim->flags.word & 0x40)
                {
                    range_start = 0;
                    frame = def->flags.b.last_frame + 1;
                    while (--frame != -1)
                    {
                        cel->visible = range_start == anim->flags.b.state;
                        cel = cel->next;
                        range_start += 1;
                    }
                }
                break;
            case 3:
                if ((anim->flags.word & 0x40) && (anim->timer != 1))
                {
                    anim->flags.word |= 0x20;
                }
                break;
            case 4:
                grid = rec->u.tile.grid;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                scene->unk38 = 1;
                break;
            case 5:
                grid = rec->u.tile.grid;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                field_apply_animation_tween(def, anim, 0);
                break;
            case 6:
                tint_src = (FieldTintSrc *) field_find_object_by_definition(rec->u.tile.grid);
                anim->cels = (FieldPart *) tint_src;
                field_apply_animation_tween(def, anim, 0);
                break;
            case 7:
            default:
                grid = rec->u.tile.grid;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                anim->owner.tint_src = tint_src;
                key = (FieldSfxKey *) rec->data;
                if (((key->control.b.lo & 7) == 1) && (key->sound.word & 0x8000))
                {
                    anim->timer = 1;
                    anim->flags.word |= 8;
                }
                break;
            }
            break;
        case 1:
            switch (def->flags.b.kind_flags & 7)
            {
            case 0:
                grid = (FieldPartDef *) def->data;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                break;
            case 1:
                tint_src = (FieldTintSrc *) field_find_object_by_definition(def->data);
                anim->cels = (FieldPart *) tint_src;
                break;
            }
            break;
        case 2:
            switch (def->flags.b.kind_flags & 7)
            {
            case 0:
                grid = def->u.tint.grid;
                cel = func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                anim->owner.tint_src = tint_src;
                break;
            case 1:
                tint_src = (FieldTintSrc *) field_find_object_by_definition(def->u.tint.grid);
                anim->cels = (FieldPart *) tint_src;
                break;
            }
            break;
        default:
            grid = def->u.tile.grid;
            cel = func_8005ABD8(grid, &tint_src);
            anim->cels = cel;
            break;
        }
        if (((u32) (def->flags.word & 0xFF000007) < 2) || (def->flags.b.handler_group == 3))
        {
            rgb[0] = tint_src->red << 8;
            rgb[1] = tint_src->green << 8;
            rgb[2] = tint_src->blue << 8;
            palette_data = tint_src->palette->data;
            func_8005AC50((u8 *) (palette_data + 2), palette_data[0], rgb);
            primitive_code = 0;
            func_8005AD20(cel->kind, tint_src->palette->data[0], &primitive_code);
            anim->frame_data = *arena;
            arena_cursor = *arena;
            switch (cel->kind) /* case 0 and 2..5 stay separate arms: merged 99.25% */
            {
            case 0:
                record_stride = 0xC;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                record_stride = 0xC;
                break;
            case 1:
            case 6:
                break;
            }
            record_flags = 1;
            if (cel->code_word != 0)
            {
                record_stride -= 4;
            }
            else
            {
                record_flags = 0;
            }
            if (cel->tpage_word != 0)
            {
                record_flags |= 2;
                record_stride -= 4;
            }
            frame_descs = (FieldTileDesc *) def->data;
            rec = def;
            if ((def->flags.word & 0xFF000007) == 1)
            {
                anim->owner.tiles = cel->records;
                frame = def->flags.b.frame_count;
                while (--frame != -1)
                {
                    /* no per-frame records for this kind; the original still runs the loop */
                }
            }
            else
            {
                frame = def->flags.b.frame_count;
                while (--frame != -1)
                {
                    tile_count = 0;
                    tile_data = frame_descs;
                    mask_bit = 1;
                    mask = cel->bits;
                    mask_word = *mask++;
                    for (row = 0; row != grid->u.b.rows; row++)
                    {
                        if (row < rec->u.tile.rect_y)
                        {
                            col = grid->u.b.cols;
                            while (--col != -1)
                            {
                                mask_bit *= 2;
                                if (mask_bit == 0)
                                {
                                    mask_word = *mask++;
                                    mask_bit = 1;
                                }
                            }
                        }
                        else if (row < rec->u.tile.rect_y + rec->u.tile.rect_height)
                        {
                            for (col = 0; col != grid->u.b.cols; col++)
                            {
                                if ((col >= rec->u.tile.rect_x) && (col < rec->u.tile.rect_x + rec->u.tile.rect_width))
                                {
                                    if (mask_word & mask_bit)
                                    {
                                        switch (cel->kind)
                                        {
                                        case 0:
                                            field_build_sprite_tile_record(tile_data, (FieldTileRec *) arena_cursor, (grid->u.word >> 4) & 3, record_flags);
                                            arena_cursor += record_stride;
                                            break;
                                        case 2:
                                        case 3:
                                        case 4:
                                        case 5:
                                            field_build_quad_tile_record(tile_data, (FieldTileRec *) arena_cursor, (grid->u.word >> 4) & 3, record_flags);
                                            arena_cursor += record_stride;
                                            break;
                                        case 1:
                                        case 6:
                                            break;
                                        }
                                        tile_count++;
                                    }
                                    tile_data++;
                                }
                                mask_bit *= 2;
                                if (mask_bit == 0)
                                {
                                    mask_word = *mask++;
                                    mask_bit = 1;
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    frame_descs += rec->u.tile.rect_width * rec->u.tile.rect_height;
                }
                anim->frame_tile_count = tile_count;
                if (def->flags.b.handler_group == 3)
                {
                    if (def->flags.word & 0x20)
                    {
                        field_blit_animation_frame(def, anim, 0);
                    }
                }
                else if (anim->flags.word & 0x40)
                {
                    field_blit_animation_frame(def, anim, 0);
                }
            }
            *arena = arena_cursor;
        }
        if ((((def->flags.word & 0xFF000007) >= 3) && ((def->flags.word & 0xFF000007) < 5)) ||
            ((def->flags.b.handler_group == 1) && ((u32) (def->flags.b.kind_flags & 7) >= 2)))
        {
            if ((def->flags.word & 0xFF000007) == 0x01000002)
            {
                if (def->u.clut.clut_mode == 0)
                {
                    *arena += 0x50;
                }
                else
                {
                    *arena += 0x410;
                }
            }
            else if ((def->flags.word & 0xFF000007) == 0x01000005)
            {
                if (def->u.clut.clut_mode == 0)
                {
                    *arena += (def->u.clut.length << 6) + 0x10;
                }
                else
                {
                    *arena += (def->u.clut.length << 10) + 0x10;
                }
            }
            else
            {
                *arena += 0x10;
            }
        }
    }
    *tail = NULL;
}

/**
 * @brief Build a compact sprite record from a packed field tile descriptor.
 *
 * Decodes the tile's UV and CLUT coordinates, copies its RGB/primitive-code
 * word when it is not shared, and emits its PSX draw-mode command. An absent
 * tile is represented by setting the record's first word to -1.
 *
 * @param desc Packed four-byte tile descriptor.
 * @param record Destination sprite record.
 * @param texture_depth PSX texture depth: 0 = 4bpp, 1 = 8bpp, 2 = 15bpp.
 * @param record_flags Combination of FIELD_TILE_REC_SHARED_RGB_CODE and
 *                     FIELD_TILE_REC_SHARED_TPAGE.
 */
void field_build_sprite_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags)
{
    u32 u_cell;
    s32 tpage;
    u8 texture_attrs;
    s32 page_slot;

    if (desc->clut_slot & FIELD_TILE_PRESENT)
    {
        u8 packed_uv = desc->packed_uv;

        u_cell = packed_uv & FIELD_TILE_U_MASK;
        record->v = packed_uv & FIELD_TILE_V_MASK;
        switch (texture_depth)
        {
        case FIELD_TEXTURE_4_BIT:
        {
            u32 clut_ref = desc->clut_slot;

            setClut(record, FIELD_TILE_4BIT_CLUT_X(clut_ref), FIELD_TILE_4BIT_CLUT_Y(clut_ref));
        }
        break;
        case FIELD_TEXTURE_8_BIT:
            setClut(record, 0, FIELD_TILE_8BIT_CLUT_Y(desc->clut_slot));
            break;
        default:
            record->clut = 0;
            break;
        }
        if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
        {
            record->rgb_code = FIELD_TILE_COLOR_WORDS[desc->color_index];
            if (desc->texture_attrs & FIELD_TILE_SEMITRANS)
            {
                setSemiTrans(record, 1);
            }
        }
        if (!(record_flags & FIELD_TILE_REC_SHARED_TPAGE))
        {
            texture_attrs = desc->texture_attrs;
            page_slot = texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK;
            if ((u32)FIELD_TILE_TPAGE_COLUMN(page_slot, u_cell, texture_depth) >= FIELD_TILE_LOWER_BANK_SLOTS)
            {
                /*
                 * U crossed the ten-slot lower bank. Rebase it onto the upper
                 * bank's first TPage: getTPage(depth, abr, 512, 0).
                 */
                u_cell -= FIELD_TILE_UPPER_BANK_U_REBASE(page_slot);
                tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_UPPER_BANK_VRAM_X, 0);
            }
            else
            {
                tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_LOWER_BANK_PAGE_X(page_slot), FIELD_TILE_LOWER_BANK_VRAM_Y);
            }
            if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
            {
                record->tail.draw_mode = _get_mode(1, 0, tpage);
            }
            else
            {
                record->rgb_code = _get_mode(1, 0, tpage);
            }
        }
        record->u = u_cell * FIELD_TILE_SIZE;
    }
    else
    {
        *(s32*)record = -1;
    }
}

/**
 * @brief Build a two-coordinate field tile render record from its descriptor.
 *
 * Sibling of field_build_sprite_tile_record: same descriptor and the same CLUT/texture-page
 * selection, but it emits a second texture coordinate pair (u + 15, v) and a
 * texture-page halfword at the record tail instead of a GPU draw-mode word.
 *
 * @param desc  Packed 4-byte tile descriptor.
 * @param record Render record to fill in.
 * @param texture_depth PSX texture depth: 0 = 4bpp, 1 = 8bpp, 2 = 15bpp.
 *              It selects the CLUT packing and supplies TPage bits 7-8.
 * @param record_flags FIELD_TILE_REC_SHARED_RGB_CODE omits the per-tile
 *              scratchpad word and shortens the record by four bytes;
 *              FIELD_TILE_REC_SHARED_TPAGE omits the second UV/TPage tuple.
 */
void field_build_quad_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags)
{
    s32 tpage;
    s32 second_tpage;
    u8 texture_attrs;
    u32 page_slot;

    if (desc->clut_slot & FIELD_TILE_PRESENT)
    {
        record->u = (desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE;
        record->v = desc->packed_uv & FIELD_TILE_V_MASK;
        switch (texture_depth)
        {
        case FIELD_TEXTURE_4_BIT:
        {
            u32 clut_ref = desc->clut_slot;

            setClut(record, FIELD_TILE_4BIT_CLUT_X(clut_ref), FIELD_TILE_4BIT_CLUT_Y(clut_ref));
        }
        break;
        case FIELD_TEXTURE_8_BIT:
            setClut(record, 0, FIELD_TILE_8BIT_CLUT_Y(desc->clut_slot));
            break;
        default:
            record->clut = 0;
            break;
        }
        if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
        {
            record->rgb_code = FIELD_TILE_COLOR_WORDS[desc->color_index];
            if (desc->texture_attrs & FIELD_TILE_SEMITRANS)
            {
                setSemiTrans(record, 1);
            }
        }
        texture_attrs = desc->texture_attrs;
        page_slot = texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK;
        if (page_slot >= FIELD_TILE_LOWER_BANK_SLOTS)
        {
            tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_UPPER_BANK_PAGE_X(page_slot), 0);
        }
        else
        {
            tpage = getTPage(texture_depth, FIELD_TILE_ABR(texture_attrs), FIELD_TILE_LOWER_BANK_PAGE_X(page_slot), FIELD_TILE_LOWER_BANK_VRAM_Y);
        }
        record->tail.quad.tpage = tpage;
        if (!(record_flags & FIELD_TILE_REC_SHARED_TPAGE))
        {
            u8 second_texture_attrs = desc->texture_attrs;
            u32 second_page_slot = second_texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK;

            if (second_page_slot >= FIELD_TILE_LOWER_BANK_SLOTS)
            {
                second_tpage = getTPage(texture_depth, FIELD_TILE_ABR(second_texture_attrs), FIELD_TILE_UPPER_BANK_PAGE_X(second_page_slot), 0);
            }
            else
            {
                second_tpage =
                    getTPage(texture_depth, FIELD_TILE_ABR(second_texture_attrs), FIELD_TILE_LOWER_BANK_PAGE_X(second_page_slot), FIELD_TILE_LOWER_BANK_VRAM_Y);
            }
            if (!(record_flags & FIELD_TILE_REC_SHARED_RGB_CODE))
            {
                record->tail.quad.u = ((desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE) + 0xF;
                record->tail.quad.v = desc->packed_uv & FIELD_TILE_V_MASK;
                record->tail.quad.tpage = second_tpage;
            }
            else
            {
                FieldTileRec* prev = (FieldTileRec*)((u8*)record - 4);

                prev->tail.quad.u = ((desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE) + 0xF;
                prev->tail.quad.v = desc->packed_uv & FIELD_TILE_V_MASK;
                prev->tail.quad.tpage = second_tpage;
            }
        }
    }
    else
    {
        *(s32*)record = -1;
    }
}

/**
 * @brief Size the field working buffer from the current scene's object list.
 *
 * Walks every object in the scene, derives a per-object multiplier from its
 * definition flags, then sums a per-part byte cost over each object's part
 * list. The total gets a 0xA000 header allowance, is clamped to a 0x12000
 * minimum, and is written back to the allocator state at 0x801ED000 as a
 * base / midpoint / top triple (the region is sized to twice the total).
 */
void field_size_work_buffer(void)
{
    FieldMemState* state = FIELD_MEM_STATE;
    FieldObj* obj;
    FieldPart* part;
    s32 n;
    u32 total;
    u32 base;
    u32 lim;

    total = 0;
    obj = g_field_scene.scene->objects;
    if (obj != NULL)
    {
        do
        {
            s32 flags = obj->def->flags;

            n = 1;
            if (flags & 4)
            {
                n = 3;
                if (flags & 0x30)
                {
                    n = 2;
                }
            }
            if (obj->def->flags & 8)
            {
                n = n * 2;
            }
            part = obj->parts;
            if (part != NULL)
            {
                do
                {
                    s32 sprite_cost = n * 0x18;
                    s32 single_cost = n * 0x1C;
                    s32 quad_cost = n * 0x28;
                    s32 other_cost = n * 0x34;

                    if (part->instance_count != 0)
                    {
                        s32 kind = part->kind;

                        switch (kind)
                        {
                        case 0:
                            total += part->instance_count * sprite_cost;
                            break;
                        case 1:
                            total += single_cost;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            total += part->instance_count * quad_cost;
                            break;
                        default:
                            total += part->instance_count * other_cost;
                            break;
                        }
                    }
                    part = part->next;
                } while (part != NULL);
            }
            obj = obj->next;
        } while (obj != NULL);
    }
    total = total + 0xA000;
    base = state->top;
    lim = 0x12000; /* a literal compare is 99.28% (register choice) */
    if (total < lim)
    {
        total = 0x12000;
    }
    state->midpoint = base + total;
    state->base = base;
    state->top = base + total * 2;
}

/**
 * @brief Draw every visible part of every active field object.
 *
 * Walks the scene's object list; for each active object it derives a scroll
 * offset from the camera state (scaled per-axis by the object's definition,
 * optionally negated and wrapped to a power-of-two boundary), applies the
 * object's per-frame drift, then walks the object's part list and submits each
 * visible part to field_draw_part. Parts near a wrap boundary are submitted more
 * than once so they appear on both sides of the seam.
 *
 * @param cursor Address of the primitive-buffer cursor, forwarded to
 *               field_draw_part and field_draw_marker_overlay.
 * @param ot Ordering-table base, forwarded unchanged to field_draw_part and
 *           field_draw_marker_overlay.
 * @param update_mode Mode selector: 0 advances the per-frame drift; 2 forces
 *                    the unscaled camera offsets.
 *
 * @note The three do/while(0) blocks around the /256 and /512 screen-Y terms
 *       end scheduling regions (loop notes) and are required; plain statements
 *       measure 98.65-99.52%. The two volatile reads are also required: plain
 *       reads measure 99.91% (rows) and 98.82% (viewport.x reload).
 */
void field_draw_scene_objects(u8** cursor, u_long* ot, s32 update_mode)
{
    FieldViewport viewport;
    FieldObj* obj;
    FieldScene* scene;
    FieldObjDef* def;
    FieldPart* part;
    s32 scroll_x;
    s32 scroll_y;
    s32 scroll_z;
    s32 wrap_size;
    s32 wrap_height;
    s32 scroll_diff;

    wrap_size = 0;
    wrap_height = 0;
    scene = g_field_scene.scene;
    viewport.width = scene->header->unk30;
    {
        s32 cam_y;
        s32 cam_z;
        s32 screen_y;

        viewport.camera_x = SHIFT_TOWARD_ZERO(g_field_camera_x, 8);
        cam_y = g_field_camera_y;
        if (cam_y < 0)
        {
            cam_y += 0xFF;
        }
        do
        {
            screen_y = cam_y >> 8;
        } while (0);
        cam_z = g_field_camera_z;
        if (cam_z < 0)
        {
            cam_z += 0x1FF;
        }
        viewport.camera_y = (screen_y - (cam_z >> 9)) + 0xE0;
    }
    for (obj = scene->objects; obj != NULL; obj = obj->next)
    {
        if (obj->flags.word & 1)
        {
            def = obj->def;
            scroll_x = 0;
            if (def->flags & 2)
            {
                scroll_y = 0;
                scroll_z = 0;
            }
            else
            {
                u8 scale_x = def->scroll_scale_x;

                if ((scale_x == 0x10) || (update_mode == 2))
                {
                    scroll_x = g_field_camera_x;
                }
                else
                {
                    if (scale_x & 0x80)
                    {
                        scroll_x = (-g_field_camera_x * (scale_x & 0x7F)) / 16;
                    }
                    else
                    {
                        scroll_x = (g_field_camera_x * def->scroll_scale_x) / 16;
                    }
                }
                {
                    u8 scale_y = def->scroll_scale_y;

                    if ((scale_y == 0x10) || (update_mode == 2))
                    {
                        scroll_y = SCENE_STATE->camera_y;
                        scroll_z = SCENE_STATE->camera_z;
                    }
                    else if (scale_y & 0x80)
                    {
                        scroll_y = SHIFT_TOWARD_ZERO(-g_field_camera_y * (scale_y & 0x7F), 4);
                        scroll_z = (-g_field_camera_z * (def->scroll_scale_y & 0x7F)) / 16;
                    }
                    else
                    {
                        scroll_y = SHIFT_TOWARD_ZERO(g_field_camera_y * def->scroll_scale_y, 4);
                        scroll_z = (g_field_camera_z * def->scroll_scale_y) / 16;
                    }
                }
            }
            if (obj->flags.b.drift_speed != 0)
            {
                if (update_mode == 0)
                {
                    obj->drift_x += (rcos(obj->flags.b.drift_angle * 0x10) * obj->flags.b.drift_speed) / 256;
                    obj->drift_y -= (rsin(obj->flags.b.drift_angle * 0x10) * obj->flags.b.drift_speed) / 256;
                }
                if (def->flags & 4)
                {
                    s32 drift;

                    wrap_size = 0x10000 << ((def->flags >> 4) & 3);
                    drift = obj->drift_x;
                    if (drift >= 0)
                    {
                        obj->drift_x = drift & (wrap_size - 1);
                    }
                    else
                    {
                        obj->drift_x = -(-drift & (wrap_size - 1));
                    }
                }
                if (def->flags & 8)
                {
                    s32 drift;

                    wrap_size = 0x20000 << ((def->flags >> 6) & 3);
                    drift = obj->drift_y;
                    if (drift >= 0)
                    {
                        obj->drift_y = drift & (wrap_size - 1);
                    }
                    else
                    {
                        obj->drift_y = -(-drift & (wrap_size - 1));
                    }
                }
            }
            {
                s32 pixel_y;
                s32 pixel_z;

                scroll_x += obj->drift_x;
                scroll_z += obj->drift_y;
                scroll_x = SHIFT_TOWARD_ZERO(scroll_x, 8);
                pixel_y = SHIFT_TOWARD_ZERO(scroll_y, 8);
                /* the subtraction stays in both arms: one shared copy is 99.64% */
                if (scroll_z >= 0)
                {
                    pixel_z = scroll_z >> 9;
                    scroll_diff = pixel_y - pixel_z;
                }
                else
                {
                    pixel_z = (scroll_z + 0x1FF) >> 9;
                    scroll_diff = pixel_y - pixel_z;
                }
            }
            scroll_z = scroll_diff; /* from here scroll_z is the screen-Y scroll */
            for (part = obj->parts; part != NULL; part = part->next)
            {
                if ((part->visible != 0) && (part->instance_count != 0))
                {
                    viewport.x = scroll_x + (obj->x + part->x) / 256;
                    {
                        s32 pixel_y;
                        s32 top_y;
                        FieldPartDef* part_def;
                        s32 height;
                        s32 z;
                        s32 screen_y;

                        do
                        {
                            pixel_y = (obj->y + part->y) / 256;
                        } while (0);
                        top_y = scroll_z + pixel_y;
                        part_def = part->def;
                        height = *(volatile u8*)&part_def->u.b.rows;
                        z = obj->z + part->z;
                        height *= 0x10;
                        do
                        {
                            screen_y = top_y - z / 512;
                        } while (0);
                        height -= 0xE0;
                        viewport.y = screen_y - height;
                    }
                    if (def->flags & 4)
                    {
                        s32 x;

                        wrap_size = 0x100 << ((def->flags >> 4) & 3);
                        x = *(volatile s32*)&viewport.x;
                        if (x >= 0)
                        {
                            viewport.x = x & (wrap_size - 1);
                        }
                        else
                        {
                            viewport.x = wrap_size - (-x & (wrap_size - 1));
                        }
                    }
                    if (def->flags & 8)
                    {
                        wrap_height = 0x100 << ((def->flags >> 6) & 3);
                        if (viewport.y >= 0)
                        {
                            viewport.y = viewport.y & (wrap_height - 1);
                        }
                        else
                        {
                            viewport.y = wrap_height - (-viewport.y & (wrap_height - 1));
                        }
                    }
                    field_draw_part(part, cursor, &viewport, ot);
                    if (def->flags & 4)
                    {
                        if (viewport.x > 0)
                        {
                            viewport.x -= wrap_size;
                            field_draw_part(part, cursor, &viewport, ot);
                            viewport.x += wrap_size;
                        }
                        if (!(def->flags & 0x30))
                        {
                            s32 x = viewport.x + wrap_size;

                            if (x < 0x140)
                            {
                                viewport.x = x;
                                field_draw_part(part, cursor, &viewport, ot);
                                viewport.x -= wrap_size;
                            }
                        }
                    }
                    if (def->flags & 8)
                    {
                        if (viewport.y > 0)
                        {
                            viewport.y -= wrap_height;
                            field_draw_part(part, cursor, &viewport, ot);
                        }
                        if (def->flags & 4)
                        {
                            if (viewport.x > 0)
                            {
                                viewport.x -= wrap_size;
                                field_draw_part(part, cursor, &viewport, ot);
                                viewport.x += wrap_size;
                            }
                            if (!(def->flags & 0x30))
                            {
                                s32 x = viewport.x + wrap_size;

                                if (x < 0x140)
                                {
                                    viewport.x = x;
                                    field_draw_part(part, cursor, &viewport, ot);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (g_field_marker_overlay_enabled[0] != 0)
    {
        field_draw_marker_overlay(cursor, ot);
    }
}

/**
 * @brief Draw the field's marker overlay: an outline and a numeric label per
 *        marker.
 *
 * Walks the scene's marker list (FieldScene offset 0x10). Each marker emits a
 * red LINE_F4 quad through its def's two points and its own two points, then a
 * red LINE_F2 closing the first point back to the third, then a numeric label
 * (func_800AD208) drawn at the first point with one digit below 10 and two
 * otherwise. All points are shifted by the camera scroll, and every vertical
 * coordinate is halved toward zero. The whole run is chained onto the ordering
 * table entry at @p ot[-1], ahead of whatever the cursor already pointed at.
 *
 * @param cursor Packet-buffer cursor; read for the first primitive address and
 *               written back with the address one past the last primitive.
 * @param ot     Ordering table pointer; the run is linked into @p ot[-1].
 *
 * @note The two do/while(0) blocks around the camera terms end scheduling
 *       regions and are required: without both 96.96%, either alone
 *       98.93% / 99.34%.
 */
void field_draw_marker_overlay(u8** cursor, u_long* ot)
{
    s16 pos[2];
    FieldScene* scene;
    FieldMarker* marker;
    FieldMarkerDef* def;
    LINE_F4* prim;
    void* prev;
    s32 sx;
    s32 sy;
    s32 cam_y;
    s32 base_y;
    s32 depth;
    s32 value;
    s32 digits;
    u_long* ot_entry;

    prim = (LINE_F4*)*cursor;
    prev = NULL;
    scene = g_field_scene.scene;
    sx = g_field_camera_x / 256;
    do
    {
        cam_y = g_field_camera_y / 256;
    } while (0);
    do
    {
        sy = cam_y - g_field_camera_z / 512;
    } while (0);
    marker = scene->markers;
    if (marker != NULL)
    {
        ot_entry = ot - 1;
        do
        {
            def = marker->def;
            depth = def->depth_bias + 0xE0;
            setLineF4(prim);
            setRGB0(prim, 0xFF, 0, 0);
            base_y = sy + depth;
            setXY4(prim,
                   def->x0 + sx, base_y - HALF_TOWARD_ZERO((s16)def->y0),
                   def->x1 + sx, base_y - HALF_TOWARD_ZERO((s16)def->y1),
                   marker->x3 + sx, base_y - HALF_TOWARD_ZERO((s16)marker->y3),
                   marker->x2 + sx, base_y - HALF_TOWARD_ZERO((s16)marker->y2));
            if (prev != NULL)
            {
                setaddr(prev, prim);
            }
            prev = prim;
            prim = prim + 1;
            setLineF2((LINE_F2*)prim);
            setRGB0(prim, 0xFF, 0, 0);
            setXY0(prim, def->x0 + sx, base_y - HALF_TOWARD_ZERO((s16)def->y0));
            prim->x1 = marker->x2 + sx;
            prim->y1 = base_y - HALF_TOWARD_ZERO((s16)marker->y2);
            setaddr(prev, prim);
            prev = prim;
            prim = (LINE_F4*)((LINE_F2*)prim + 1);
            pos[0] = def->x0 + sx;
            pos[1] = base_y - HALF_TOWARD_ZERO((s16)def->y0);
            value = def->label;
            digits = 2;
            if (def->label < 0xA)
            {
                digits = 1;
            }
            prim = (LINE_F4*)func_800AD208(ot_entry, prim, value, digits, pos);
            marker = marker->next;
        } while (marker != NULL);
    }
    if (prev != NULL)
    {
        addPrims(&ot[-1], (void*)*cursor, prev);
        *cursor = (u8*)prim;
    }
}

/**
 * @brief Emit GPU primitives for one bit-plane driven 16x16 sprite grid.
 *
 * Walks @p part 's bit plane row-major. Each set bit consumes one record from
 * the part's record stream and emits a 16-byte len-3 primitive at the current
 * grid cell, preceded by an 8-byte len-1 texture-page primitive whenever the
 * page code changes. Emitted primitives accumulate into a local chain that is
 * spliced into @p ot 's ordering-table head for the current CLUT with
 * addPrims, both whenever the interpolated CLUT changes and once at the end.
 * Rows and columns outside the 320x224 viewport are skipped by consuming their
 * bits without emitting.
 *
 * Sibling of field_emit_rotated_sprite_grid; both are reached from the field_draw_part dispatch
 * on FieldPart::kind.
 *
 * @param part Field part supplying the grid, the bit plane, the record stream
 *             and the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space origin of the grid, in pixels.
 * @param ot Base of the 8-byte-per-entry ordering-table head array, indexed
 *           by CLUT id.
 *
 * @note `step` is the record stride: 0xC, less 4 when a global code word makes
 *       the per-record copy unnecessary, less another 4 for a global page word.
 */
void field_emit_sprite_grid(FieldPart* part, u8** cursor_ptr, FieldViewport* origin, u_long* ot)
{
    s32 uv_word;
    s32 tpage_word;
    s32 code_word;
    s32 height;
    s32 interp;
    u32 clut_right;
    s32 clut_cur;
    s32 clut_left;
    s32 last_code;
    s32 row;
    s32 col;
    s32 idx;
    s32 x;
    s32 y;
    s32 step;
    s32 bits;
    s32 bit;
    s32 count;
    u32 clut;
    u32 clut_b;
    s16 val_a;
    s16 val_b;
    FieldPrim* prim;
    u8* cursor;
    u8* chain;
    u8* recp;
    s32* bitp;
    s32 width;
    FieldPartDef* info;

    last_code = 0;
    bits = 0;
    clut_cur = part->clut_tl;
    clut_left = 0;
    clut_right = 0;
    if ((clut_cur == part->clut_tr) && (clut_cur == part->clut_bl) && (clut_cur == part->clut_br))
    {
        interp = 0;
    }
    else
    {
        clut_cur = 0xFFFF;
        interp = 1;
    }
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    step = 0xC;
    if (code_word != 0)
    {
        step = 8;
    }
    if (tpage_word != 0)
    {
        step -= 4;
    }
    bit = 0;
    prim = NULL;
    bitp = part->bits;
    recp = part->records;
    info = part->def;
    cursor = *cursor_ptr;
    y = origin->y;
    height = info->u.b.rows;
    row = height;
    width = info->u.b.cols;
    chain = NULL;
    while (--row != -1)
    {
        if (y >= 0xE0)
        {
            break;
        }
        if (y < -0xF)
        {
            count = 0;
            do
            {
                for (col = width - 1; col != -1; col--)
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    bit <<= 1;
                }
                y += 0x10;
            } while ((y < -0xF) && (--row != -1));
            recp += step * count;
            if (row <= 0)
            {
                break;
            }
            row--;
        }
        if (interp != 0)
        {
            val_a = part->clut_tl;
            val_b = part->clut_bl;
            if (val_a != val_b)
            {
                clut_left = ((val_a * (row + 1)) + (val_b * ((height - row) - 1))) / height;
            }
            else
            {
                clut_left = val_a;
            }
            val_a = part->clut_tr;
            val_b = part->clut_br;
            if (val_a != val_b)
            {
                clut_right = ((val_a * (row + 1)) + (val_b * ((height - row) - 1))) / height;
            }
            else
            {
                clut_right = val_a;
            }
        }
        x = origin->x;
        col = width;
        while (--col != -1)
        {
            if (x >= 0x140)
            {
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    col--;
                    bit <<= 1;
                } while (col != -1);
                recp += step * count;
                break;
            }
            if (x < -0xF)
            {
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bits = *bitp++;
                        bit = 1;
                    }
                    if ((bits & bit) != 0)
                    {
                        count++;
                    }
                    x += 0x10;
                    bit <<= 1;
                } while ((x < -0xF) && (--col != -1));
                recp += step * count;
                if (col <= 0)
                {
                    break;
                }
                col--;
            }
            if (bit == 0)
            {
                bits = *bitp++;
                bit = 1;
            }
            if ((bits & bit) != 0)
            {
                uv_word = ((FieldCellRec*)recp)->uv_clut;
                if (uv_word != -1)
                {
                    if (interp != 0)
                    {
                        idx = width; /* split form: `width - col` is 93.10% */
                        idx -= col;
                        if (clut_left != clut_right)
                        {
                            clut = ((clut_left * (col + 1)) + (clut_right * (idx - 1))) / width;
                            clut_b = ((clut_left * col) + (clut_right * idx)) / width;
                            if (clut < clut_b)
                            {
                                clut = clut_b;
                            }
                        }
                        else
                        {
                            clut = clut_left;
                        }
                        if (clut != clut_cur)
                        {
                            if (chain != NULL)
                            {
                                addPrims(&ot[clut_cur * 2], chain, prim);
                                chain = NULL;
                            }
                            clut_cur = clut;
                        }
                    }
                    if (tpage_word != 0)
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            prim->code = tpage_word;
                            prim = (FieldPrim*)cursor;
                        }
                    }
                    else
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->rgb_code);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->tpage);
                            }
                        }
                        else if (last_code != ((FieldCellRec*)recp)->tpage)
                        {
                            cursor += 8;
                            prim->tag = ((u32)cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->rgb_code);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec*)recp)->tpage);
                            }
                        }
                        prim = (FieldPrim*)cursor;
                    }
                    cursor += 0x10;
                    prim->tag = ((u32)cursor & 0xFFFFFF) | 0x03000000;
                    if (code_word != 0)
                    {
                        prim->code = code_word;
                    }
                    else
                    {
                        prim->code = ((FieldCellRec*)recp)->rgb_code;
                    }
                    prim->xy = (x & 0xFFFF) | (y << 16);
                    prim->uv = uv_word;
                }
                recp += step;
            }
            bit <<= 1;
            x += 0x10;
        }
        y += 0x10;
    }
    if (chain != NULL)
    {
        addPrims(&ot[clut_cur * 2], chain, prim);
    }
    *cursor_ptr = cursor;
}

/**
 * @brief Emit rotated, scaled POLY_FT4 primitives for one bit-plane sprite grid.
 *
 * Rotated sibling of field_emit_sprite_grid, reached from the same field_draw_part
 * dispatch on FieldPart::kind. Walks @p part 's bit plane row-major and emits
 * one 40-byte POLY_FT4 per set bit, taking the quad's four corners from two
 * ping-pong row buffers of pre-rotated points.
 *
 * The rotation is precomputed in the PSX scratchpad: 0x1F800000 holds width + 1
 * FieldColStep entries (one per column edge), and 0x1F800200 / 0x1F800300 hold
 * width + 1 points each for the previous and current row. Each row advances the
 * vertical offset by 16, rebuilds the current row's points, then walks the
 * columns emitting a quad per set bit. Cells whose four corners all fall off one
 * side of the 320x224 viewport are skipped. Primitives accumulate into a local
 * chain spliced into @p ot 's ordering-table head for the current CLUT with
 * addPrims, both when the interpolated CLUT changes and once at the end.
 *
 * @param part Field part: def gives the grid size and the placement mode, bits
 *             the bit plane, records the record stream, row_angle/column_angle/rotation_angle the rotation
 *             angles, scale_x/scale_y the scales, clut_bl..clut_tr the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space placement; the mode selects which of its words
 *               form the rotation centre.
 * @param ot Base of the 8-byte-per-entry ordering-table head array, indexed
 *           by CLUT id.
 */
void field_emit_rotated_sprite_grid(FieldPart *part, u8 **cursor_ptr, FieldViewport *origin, u_long *ot)
{
    u8 *recp;
    s32 *bitp;
    s32 tpage_word;
    s32 code_word;
    s32 bits;
    s32 bit;
    s32 height;
    s32 interp;
    s32 step;
    s32 sin_c;
    s32 cos_c;
    s32 flip;
    s32 clut_cur;
    s32 clut_left;
    u32 clut_right;
    s32 clut_init;
    s32 row;
    s32 col;
    s32 idx;
    s32 width;
    s32 x_off;
    s32 y_off;
    s32 cx;
    s32 cy;
    s32 cos_a;
    s32 cos_b;
    s32 scaled;
    s32 dx;
    s32 dy;
    s32 visible;
    s32 uv_word;
    u32 clut;
    u32 clut_b;
    FieldColStep *steps;
    FieldPoint *pt;
    FieldPoint *prev_row;
    FieldPoint *this_row;
    FieldPolyPrim *prim;
    u8 *cursor;
    u8 *chain;

    bits = 0;
    clut_left = 0;
    clut_init = part->clut_tl;
    clut_right = 0;
    if ((clut_init == part->clut_tr) && (clut_init == part->clut_bl) && (clut_init == part->clut_br))
    {
        clut_cur = clut_init;
        interp = 0;
    }
    else
    {
        clut_cur = 0xFFFF;
        interp = 1;
    }
    step = 0xC;
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    if (code_word != 0)
    {
        step = 8;
    }
    if (tpage_word != 0)
    {
        step -= 4;
    }
    bit = 0;
    bitp = part->bits;
    recp = part->records;
    cursor = *cursor_ptr;
    prim = NULL;
    width = part->def->u.b.cols;
    height = part->def->u.b.rows;
    cos_a = rcos(part->row_angle);
    cos_b = rcos(part->column_angle);
    sin_c = rsin(part->rotation_angle);
    chain = NULL;
    cos_c = rcos(part->rotation_angle);
    switch ((part->def->u.word >> 12) & 0xF)
    {
    case 1:
    case 2:
        cy = origin->camera_y;
        cx = origin->camera_x + (origin->width / 2);
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 3:
        cx = origin->camera_x;
        cy = origin->camera_y;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 4:
        cx = origin->width + origin->camera_x;
        cy = origin->camera_y;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 5:
    default:
        x_off = -width * 8;
        y_off = -height * 8;
        cx = origin->x + (width * 8);
        cy = origin->y + (height * 8);
        break;
    }
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
    steps = FIELD_ROT_COL_STEPS;
    for (col = width; col != -1; col--)
    {
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(x_off * part->scale_x, 8) * cos_b, 12);
        x_off += 0x10;
        steps->sin_term = scaled * sin_c;
        steps->cos_term = scaled * cos_c;
        steps++;
    }
    steps = FIELD_ROT_COL_STEPS;
    pt = FIELD_ROT_ROW_A;
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
    dx = scaled * sin_c;
    dy = scaled * cos_c;
    for (col = width; col != -1; col--)
    {
        pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
        pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
        steps++;
        pt++;
    }
    flip = 0;
    row = height;
    while (--row != -1)
    {
        if (interp != 0)
        {
            if (part->clut_tl != part->clut_bl)
            {
                clut_left = ((part->clut_tl * (row + 1)) + (part->clut_bl * ((height - row) - 1))) / height;
            }
            else
            {
                clut_left = part->clut_tl;
            }
            if (part->clut_tr != part->clut_br)
            {
                clut_right = ((part->clut_tr * (row + 1)) + (part->clut_br * ((height - row) - 1))) / height;
            }
            else
            {
                clut_right = part->clut_tr;
            }
        }
        if (flip == 0)
        {
            prev_row = FIELD_ROT_ROW_A;
            this_row = FIELD_ROT_ROW_B;
            flip = 1;
        }
        else
        {
            prev_row = FIELD_ROT_ROW_B;
            this_row = FIELD_ROT_ROW_A;
            flip = 0;
        }
        y_off += 0x10;
        steps = FIELD_ROT_COL_STEPS;
        pt = this_row;
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->scale_y, 8) * cos_a, 12);
        dx = scaled * sin_c;
        dy = scaled * cos_c;
        for (col = width; col != -1; col--)
        {
            pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
            pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
            steps++;
            pt++;
        }
        for (col = width - 1; col != -1; col--)
        {
            if (bit == 0)
            {
                bits = *bitp++;
                bit = 1;
            }
            if ((bits & bit) != 0)
            {
                if (!((prev_row[0].p.x >= 0) || (prev_row[1].p.x >= 0) || (this_row[0].p.x >= 0) || (this_row[1].p.x >= 0)))
                {
                    visible = 0;
                }
                else if (!((prev_row[0].p.y >= 0) || (prev_row[1].p.y >= 0) || (this_row[0].p.y >= 0) || (this_row[1].p.y >= 0)))
                {
                    visible = 0;
                }
                else if (!((prev_row[0].p.x < 0x140) || (prev_row[1].p.x < 0x140) || (this_row[0].p.x < 0x140) || (this_row[1].p.x < 0x140)))
                {
                    visible = 0;
                }
                else if ((prev_row[0].p.y < 0xE0) || (prev_row[1].p.y < 0xE0) || (this_row[0].p.y < 0xE0) || (this_row[1].p.y < 0xE0))
                {
                    visible = 1;
                }
                else
                {
                    visible = 0;
                }
                if (visible != 0)
                {
                    uv_word = ((FieldCellRec *) recp)->uv_clut;
                    if (uv_word != -1)
                    {
                        if (interp != 0)
                        {
                            idx = width - col;
                            if (clut_left != clut_right)
                            {
                                clut = ((clut_left * (col + 1)) + (clut_right * (idx - 1))) / width;
                                clut_b = ((clut_left * col) + (clut_right * idx)) / width;
                                if (clut < clut_b)
                                {
                                    clut = clut_b;
                                }
                            }
                            else
                            {
                                clut = clut_left;
                            }
                            if (clut != clut_cur)
                            {
                                if (chain != NULL)
                                {
                                    addPrims(&ot[clut_cur * 2], chain, prim);
                                    chain = NULL;
                                }
                                clut_cur = clut;
                            }
                        }
                        if (chain == NULL)
                        {
                            prim = (FieldPolyPrim *) cursor;
                            chain = cursor;
                        }
                        else
                        {
                            prim = (FieldPolyPrim *) cursor;
                        }
                        cursor += sizeof(FieldPolyPrim);
                        prim->tag = ((u32) cursor & 0xFFFFFF) | 0x09000000;
                        if (code_word != 0)
                        {
                            prim->code = code_word;
                        }
                        else
                        {
                            prim->code = ((FieldCellRec *) recp)->rgb_code;
                        }
                        if (tpage_word != 0)
                        {
                            prim->uv1 = ((uv_word & 0xFFFF) + 0xF) | tpage_word;
                        }
                        else if (code_word != 0)
                        {
                            prim->uv1 = ((FieldCellRec *) recp)->rgb_code;
                        }
                        else
                        {
                            prim->uv1 = ((FieldCellRec *) recp)->tpage;
                        }
                        prim->uv0 = uv_word;
                        prim->uv2 = (uv_word & 0xFFFF) + 0xF00;
                        prim->uv3 = (uv_word & 0xFFFF) + 0xF0F;
                        prim->xy0 = prev_row[0].word;
                        prim->xy1 = prev_row[1].word;
                        prim->xy2 = this_row[0].word;
                        prim->xy3 = this_row[1].word;
                    }
                }
                recp += step;
            }
            prev_row++;
            this_row++;
            bit <<= 1;
        }
    }
    if (chain != NULL)
    {
        addPrims(&ot[clut_cur * 2], chain, prim);
    }
    *cursor_ptr = cursor;
}

/**
 * @brief Find an already-built part in the scene that this one can share.
 *
 * Scans every part of every object in @p scene for one whose definition key
 * matches @p key and whose owning object is interchangeable with @p obj - either
 * literally the same definition, or one with the same shared-source handle and
 * the same 0x10/0x14 pair. The caller uses the result to reuse an existing
 * part's build instead of doing the work twice.
 *
 * @param scene Scene whose object list is searched.
 * @param obj   Object the candidate must be interchangeable with.
 * @param part  Part being built; excluded from its own search, and skipped
 *              entirely when its definition is marked unshareable (bit 7).
 * @param key   Definition key to match on (FieldPartDef::tiles).
 * @return The matching FieldPart, or NULL if none qualifies - including when
 *         the only candidate found is @p part itself on @p obj.
 */
FieldPart* field_find_shareable_part(FieldScene* scene, FieldObj* obj, FieldPart* part, FieldTileDesc* key)
{
    FieldObj* o;
    FieldPart* p;
    FieldObjDef* want;
    FieldObjDef* have;

    if (!(part->def->u.word & 0x80))
    {
        for (o = scene->objects; o != NULL; o = o->next)
        {
            for (p = o->parts; p != NULL; p = p->next)
            {
                if (key == p->def->tiles)
                {
                    if ((obj == o) && (part == p))
                    {
                        return NULL;
                    }
                    if (!(p->def->u.word & 0x80))
                    {
                        want = obj->def;
                        have = o->def;
                        if ((want == have) || ((want->shared_source == have->shared_source) && (obj->unk10 == o->unk10) && (obj->unk14 == o->unk14)))
                        {
                            return p;
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Dispatch one field part to the emitter its kind selects.
 *
 * Kind 0 draws an axis-aligned grid, kinds 2 through 5 a rotated/scaled one.
 * Kind 1 and anything from 6 up draw nothing. All four arguments are forwarded
 * verbatim.
 *
 * @param part Field part to draw; its kind byte selects the emitter.
 * @param cursor Primitive-buffer cursor, forwarded as the emitter's 2nd param.
 * @param origin Screen-space placement, forwarded as the 3rd param.
 * @param ot Ordering-table head array base, forwarded as the 4th param.
 */
void field_draw_part(FieldPart* part, u8** cursor, FieldViewport* origin, u_long* ot)
{
    switch (part->kind)
    {
    case 0:
        field_emit_sprite_grid(part, cursor, origin, ot);
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        field_emit_rotated_sprite_grid(part, cursor, origin, ot);
        break;
    }
}

/**
 * @brief Flush the scene's pending VRAM uploads.
 *
 * Walks the scene's upload list, issues each node's LoadImage, then empties the
 * list. Nodes are not freed - the list head is simply cleared.
 */
void field_flush_vram_uploads(void)
{
    FieldScene* scene;
    FieldImageReq* req;

    scene = g_field_scene.scene;
    for (req = scene->uploads; req != NULL; req = req->next)
    {
        LoadImage(&req->rect, req->data);
    }
    scene->uploads = NULL;
}

/**
 * @brief Empty stub; nothing references it.
 */
void func_800569F4(void)
{
}

/**
 * @brief Empty stub, identical to func_800569F4; nothing references it.
 */
void func_800569FC(void)
{
}
