/**
 * @file field_scene_build.c
 * @brief Scene build from a map object and the per-frame drawing of its objects.
 */

#include "field_scene_internal.h"
#include "scene_state.h"
#include "field_calls.h"
#include "display.h"
#include "sdk/libpress.h"

/*
 * FieldObjDef::flags bits (see also FIELD_OBJ_DEF_SCREEN_FIXED). A wrapping
 * object repeats every FIELD_OBJ_WRAP_MIN << shift pixels; the shift is bits
 * 4-5 horizontally and bits 6-7 vertically.
 */
#define FIELD_OBJ_DEF_VISIBLE 0x1
#define FIELD_OBJ_DEF_WRAP_X 0x4
#define FIELD_OBJ_DEF_WRAP_Y 0x8
#define FIELD_OBJ_DEF_WRAP_X_SIZE 0x30
#define FIELD_OBJ_DEF_WRAP_X_SHIFT(flags) (((flags) >> 4) & 3)
#define FIELD_OBJ_DEF_WRAP_Y_SHIFT(flags) (((flags) >> 6) & 3)
#define FIELD_OBJ_WRAP_MIN 0x100

/* FieldObjDef::motion: a drift of speed 0x10 at angle 0 is stored as no drift. */
#define FIELD_OBJ_DEF_DRIFT_MASK 0xFFFF0000
#define FIELD_OBJ_DEF_NO_DRIFT 0x00100000

/** FieldObjDef scroll factors: 4.4 fixed point, bit 7 negates. */
#define FIELD_SCROLL_SCALE_ONE 0x10
#define FIELD_SCROLL_SCALE_NEGATIVE 0x80
#define FIELD_SCROLL_SCALE_MAGNITUDE 0x7F

/** FieldObj tint scale of 1.0 in 8.8 fixed point. */
#define FIELD_TINT_SCALE_ONE 0x100
/** Largest CLUT id a depth-offset corner CLUT is clamped to. */
#define FIELD_DEPTH_CLUT_MAX 0x7FF
/** Start value of a running minimum over s16 coordinates. */
#define FIELD_BOUNDS_EMPTY 0x7FFF

/** First resource format version whose nodes name an owning object or part. */
#define FIELD_RESOURCE_NODE_OWNERS 0x12
/** FieldNodeDef::obj_index / part_index value for "none". */
#define FIELD_NODE_NO_OWNER 0xFF

/** Scratch colour bytes an uploading node reserves: two copies of @p colors. */
#define FIELD_ANIM_SCRATCH_BYTES(colors) (2 * (colors) * sizeof(u16))

/*
 * getTPage fields built from a part word (texture depth in bits 4-5) and from
 * a decoded shared page word (blend mode in bits 4-5).
 */
#define FIELD_TPAGE_DEPTH_BITS(part_word) (((part_word) * 8) & 0x180)
#define FIELD_TPAGE_ABR_BITS(page_word) (((page_word) * 2) & 0x60)

/*
 * FieldPart::tpage_word / code_word while the scene builds: zero for "not
 * shared", else one plus (page slot | abr << 4) or (colour index | semi << 9).
 */
#define FIELD_SHARED_TPAGE_ABR_SHIFT 4
#define FIELD_SHARED_CODE_SEMI_SHIFT 9
#define FIELD_SHARED_CODE_COLOR_MASK 0xFF

/** Bytes reserved for the MDEC VLC table (DecDCTvlcBuild). */
#define FIELD_VLC_TABLE_BYTES 0x14C00

/** field_draw_scene_objects update modes. */
#define FIELD_DRAW_ADVANCE 0
#define FIELD_DRAW_UNSCALED 2

/** Corner CLUT value meaning "no CLUT chain open yet". */
#define FIELD_CLUT_NONE 0xFFFF
/** FieldCellRec::uv_clut of a cell that draws nothing. */
#define FIELD_CELL_EMPTY -1
/** UV offsets from a cell's top-left texel to its right column and bottom row. */
#define FIELD_CELL_UV_RIGHT (FIELD_TILE_SIZE - 1)
#define FIELD_CELL_UV_BOTTOM ((FIELD_TILE_SIZE - 1) << 8)

/*
 * GPU packet tag: the address of the next packet in the chain and the
 * packet's length in words after the tag. The emitters write it as one word.
 */
#define FIELD_PRIM_TAG(next, words) (((u32)(next) & 0xFFFFFF) | ((words) << 24))
#define FIELD_DR_TPAGE_WORDS 1
#define FIELD_SPRT_WORDS 3
#define FIELD_POLY_FT4_WORDS 9

/*
 * Primitive bytes budgeted per cell of each part kind (see FieldPart::kind).
 * The sprite and quad figures are what field_draw_part emits: a SPRT_16 plus
 * a DR_TPAGE, and a POLY_FT4.
 */
#define FIELD_SPRITE_CELL_BYTES 24
#define FIELD_SINGLE_CELL_BYTES 28
#define FIELD_QUAD_CELL_BYTES 40
#define FIELD_OTHER_CELL_BYTES 52
/** field_size_work_buffer: bytes added to every half, and the smallest half. */
#define FIELD_WORK_BUFFER_RESERVE 0xA000
#define FIELD_WORK_BUFFER_MIN 0x12000

/** Last word of a cel tile record; its meaning depends on the part kind. */
typedef union
{
    /** Sprite records: a complete GP0(E1h) draw-mode command. */
    s32 draw_mode;
    /** Quad records: the second UV pair and the texture page. */
    struct
    {
        s8 u;
        s8 v;
        s16 tpage;
    } quad;
} FieldTileTail;

/**
 * @brief Cel tile record as the record builders write it.
 *
 * FieldPart::records holds one per present cell. When the cel shares its
 * rgb/code word (FIELD_TILE_REC_SHARED_RGB_CODE) the record is one word
 * shorter and the tail moves into the rgb/code slot; when it shares its
 * texture page (FIELD_TILE_REC_SHARED_TPAGE) the tail is left out. An absent
 * tile stores FIELD_CELL_EMPTY over the whole first word.
 */
typedef struct
{
    s8 u;
    s8 v;
    s16 clut;
    s32 rgb_code;
    FieldTileTail tail;
} FieldTileRec;

/**
 * @brief The same cel tile record as the emitters read it: whole words.
 *
 * Because the shorter forms drop words, rgb_code and tpage name slots rather
 * than fields: a record without its rgb/code word keeps its page in rgb_code.
 */
typedef struct
{
    /** UV pair and CLUT id, or FIELD_CELL_EMPTY. */
    s32 uv_clut;
    s32 rgb_code;
    s32 tpage;
} FieldCellRec;

/**
 * @brief SPRT_16 (or, in its first two words, DR_TPAGE) as the sprite emitter
 *        writes it.
 *
 * Declared as whole words rather than as the Psy-Q types: every field is one
 * 32-bit store, and the byte members or setaddr/setlen would turn each into
 * a read-modify-write.
 */
typedef struct
{
    u32 tag;
    u32 code;
    /** Packed (y << 16) | (x & 0xFFFF). */
    u32 xy;
    /** UV pair and CLUT id. */
    u32 uv;
} FieldPrim;

/**
 * @brief POLY_FT4 as the rotated emitter writes it, as whole words.
 *
 * The vertex words take a packed FieldPoint straight from the row buffers;
 * uv0 carries the CLUT id and uv1 the texture page, as in POLY_FT4.
 */
typedef struct
{
    u32 tag;
    u32 code;
    u32 xy0;
    u32 uv0;
    u32 xy1;
    u32 uv1;
    u32 xy2;
    u32 uv2;
    u32 xy3;
    u32 uv3;
} FieldPolyPrim;

/**
 * @brief One column edge's offset, pre-multiplied by the rotation's sine and
 *        cosine.
 *
 * The rotated emitter builds width + 1 of these once per part, so each row
 * only adds its own contribution.
 */
typedef struct
{
    s32 sin_term;
    s32 cos_term;
} FieldColStep;

/** Scratchpad table of FieldColStep entries. */
#define FIELD_ROT_COL_STEPS ((FieldColStep*)0x1F800000)

/**
 * @brief A screen point in one of the rotated emitter's row buffers.
 *
 * Compared per component for the screen reject but copied into the primitive
 * as one word, so the two views share storage.
 */
typedef union
{
    /** Packed (y << 16) | (x & 0xFFFF), as stored in a POLY_FT4 vertex. */
    s32 word;
    struct
    {
        s16 x;
        s16 y;
    } p;
} FieldPoint;

/** The two scratchpad row buffers of rotated FieldPoint corners. */
#define FIELD_ROT_ROW_A ((FieldPoint*)0x1F800200)
#define FIELD_ROT_ROW_B ((FieldPoint*)0x1F800300)

/**
 * @brief Generic view of a scene list element: every list links through offset 0.
 *
 * The build passes append through one tail pointer of this type, starting from
 * the list head inside FieldScene.
 */
typedef struct FieldLink
{
    struct FieldLink* next;
} FieldLink;

/** FieldResource::format_version, read through its own symbol. */
extern u16 g_field_resource_version;

/*
 * Main-executable number renderer (ot, cursor, value, digits, position,
 * flags). This file calls it without the trailing flags argument, so it is
 * declared without a prototype.
 */
void* func_800AD208();

static void field_prepare_animation_definitions(FieldAnimDef* def, s32 handler_group);
static void field_build_animation_list(FieldAnimDef* def, u8** arena, FieldAnim** tail);
static void field_build_sprite_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags);
static void field_build_quad_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags);
static void field_draw_marker_overlay(u8** cursor, u_long* ot);
static void field_emit_sprite_grid(FieldPart* part, u8** cursor_ptr, FieldViewport* origin, u_long* ot);
static void field_emit_rotated_sprite_grid(FieldPart* part, u8** cursor_ptr, FieldViewport* origin, u_long* ot);
static FieldPart* field_find_shareable_part(FieldScene* scene, FieldObj* obj, FieldPart* part, FieldTileDesc* tiles);
static void field_draw_part(FieldPart* part, u8** cursor, FieldViewport* origin, u_long* ot);

/**
 * @brief Clamp a depth-offset CLUT id to the valid range.
 * @param clut Corner CLUT id plus the object and part depth.
 * @return @p clut clamped to 0..FIELD_DEPTH_CLUT_MAX.
 */
static inline s16 field_depth_clut(s32 clut)
{
    s32 clamped;

    if (clut > 0)
    {
        clamped = clut;
        if (clut > FIELD_DEPTH_CLUT_MAX)
        {
            clamped = FIELD_DEPTH_CLUT_MAX;
        }
        return clamped;
    }
    return 0;
}

/**
 * @brief Build the runtime scene of a map object.
 *
 * Allocates the scene's nodes, markers, objects and parts from the arena after
 * the FieldScene, with each part's bit plane of present tiles and its tile
 * records, binds the nodes to their owners, builds the four animation lists
 * and the sequence list, starts the sequences whose start mask selects
 * @p object_index, and reserves the MDEC VLC table when a movie needs one.
 *
 * @param map Map object whose scene is built; marked built afterwards.
 * @param object_index Map object index tested against FieldSeqDef::start_mask.
 * @see decomp.me (95.60%) https://decomp.me/scratch/i4GmA
 */
void field_build_render_records(FieldMapObject* map, u16 object_index)
{
    s32 rgb[3];
    u8 prim_code;
    struct
    {
        u8* cur;
    } arena = {NULL};
    FieldMemState* mem;
    FieldScene* scene;
    FieldObj* prev_obj;
    s32 bit_word;
    s32 abr;
    DVECTOR* point;
    FieldNode* node;
    u32* bits;
    u8* quad_records;
    FieldObj* obj;
    FieldLink* tail;
    FieldPart* prev_part;
    FieldPartDef** part_defs;
    DVECTOR* points;
    s32 intercept_a;
    u8* vlc_table;
    s32 intercept_b;
    s32 def_word;
    s32 sprite_tpage_word;
    s32 quad_tpage_word;
    s32 code_word;
    s32 count;
    s32 lower_bank_y_bit;
    s32 bit;
    s32 tpage_state;
    /* share_flags, tpage and color are the object pass's tile scan and then the record pass's record flags and decoded words. */
    s32 share_flags;
    s32 instances;
    s32 tpage;
    s32 color;
    s32 semi;
    s16 high;
    s16 low;
    u16* palette;
    /* One pointer walks the node runs and then allocates the markers. */
    union
    {
        FieldNodeRun* run;
        FieldMarker* marker;
    } item;
    u32 page_slot;
    s32 clut;
    s32 stride;
    s32 words;
    FieldSeq* seq;
    FieldSeqDef* seq_def;
    u8 attrs;
    s32 kind;
    FieldTileDesc* tiles;
    FieldPartDef* part_def;
    FieldPart* shared;
    FieldNodeDef* node_def;
    FieldMarkerDef* marker_def;
    FieldPart* part;
    u8 mask;
    FieldObjDef* obj_def;
    u8* cursor;
    FieldObjDef** obj_defs;

    mem = FIELD_MEM_STATE;
    tpage = 0;
    color = 0;
    semi = 0;
    abr = 0;
    scene = FIELD_RESOURCE->scene;
    bit_word = 0;
    scene->header = (FieldSceneHeader*)map;
    scene->secondary_nodes = NULL;
    node_def = map->node_defs;
    arena.cur = (u8*)(scene + 1);
    tail = (FieldLink*)&scene->nodes;
    points = FIELD_RESOURCE->points;
    if (node_def != NULL)
    {
        do
        {
            node = (FieldNode*)arena.cur;
            arena.cur = (u8*)(node + 1);
            tail->next = (FieldLink*)node;
            tail = (FieldLink*)node;
            node->def = node_def;
            node->spans = 0;
            node->unk14 = 0;
            node->unk18 = *(u8*)&(node_def)->flags >> FIELD_NODE_DEF_ENABLE_SHIFT;
            node->x_min = FIELD_BOUNDS_EMPTY;
            node->x_max = 0;
            node->row_end = 0;
            node->row_start = FIELD_BOUNDS_EMPTY;
            node->unk24 = 0;
            node->delta_x = 0;
            node->delta_y = 0;
            node->unk30 = 0;
            node->unk34 = 0;
            node->x = 0;
            node->y = 0;
            node->unk40 = 0;
            item.run = node_def->runs;
            count = item.run->count & FIELD_NODE_RUN_COUNT_MASK;
            while (count != 0)
            {
                point = &points[item.run->first];
                while (--count != -1)
                {
                    node->x_min = (point->vx > node->x_min) ? node->x_min : point->vx;
                    node->x_max = (point->vx < node->x_max) ? node->x_max : point->vx;
                    node->row_end = (point->vy < node->row_end) ? node->row_end : point->vy;
                    node->row_start = (point->vy > node->row_start) ? node->row_start : point->vy;
                    point++;
                }
                item.run++;
                count = item.run->count & FIELD_NODE_RUN_COUNT_MASK;
            }
            node_def = node_def->next;
        } while (node_def != NULL);
    }
    tail->next = NULL;
    marker_def = map->edge_defs;
    tail = (FieldLink*)&scene->markers;
    if (marker_def != NULL)
    {
        do
        {
            item.marker = (FieldMarker*)arena.cur;
            arena.cur = (u8*)(item.marker + 1);
            tail->next = (FieldLink*)item.marker;
            tail = (FieldLink*)item.marker;
            item.marker->def = marker_def;
            item.marker->x2 = marker_def->x0 + marker_def->offset_x;
            item.marker->y2 = marker_def->y0 + marker_def->offset_y;
            item.marker->x3 = marker_def->x1 + marker_def->offset_x;
            item.marker->y3 = marker_def->y1 + marker_def->offset_y;
            high = low = marker_def->x0;
            if (high < (s16)marker_def->x1)
            {
                high = marker_def->x1;
            }
            if ((s16)marker_def->x1 < low)
            {
                low = marker_def->x1;
            }
            if (high < (s16)item.marker->x2)
            {
                high = item.marker->x2;
            }
            if ((s16)item.marker->x2 < low)
            {
                low = item.marker->x2;
            }
            if (high < (s16)item.marker->x3)
            {
                high = item.marker->x3;
            }
            if ((s16)item.marker->x3 < low)
            {
                low = item.marker->x3;
            }
            item.marker->x_max = high;
            item.marker->x_min = low;
            high = low = marker_def->y0;
            if (high < (s16)marker_def->y1)
            {
                high = marker_def->y1;
            }
            if ((s16)marker_def->y1 < low)
            {
                low = marker_def->y1;
            }
            if (high < (s16)item.marker->y2)
            {
                high = item.marker->y2;
            }
            if ((s16)item.marker->y2 < low)
            {
                low = item.marker->y2;
            }
            if (high < (s16)item.marker->y3)
            {
                high = item.marker->y3;
            }
            if ((s16)item.marker->y3 < low)
            {
                low = item.marker->y3;
            }
            item.marker->y_max = high;
            item.marker->y_min = low;
            item.marker->side_dx = (s16)item.marker->x2 - (s16)marker_def->x0;
            item.marker->side_dy = (s16)item.marker->y2 - (s16)marker_def->y0;
            if (item.marker->side_dx != 0)
            {
                intercept_a = (s16)marker_def->y0 - item.marker->side_dy * (s16)marker_def->x0 / item.marker->side_dx;
                intercept_b = (s16)marker_def->y1 - item.marker->side_dy * (s16)marker_def->x1 / item.marker->side_dx;
            }
            else
            {
                intercept_a = (s16)marker_def->x0;
                intercept_b = (s16)marker_def->x1;
            }
            if (intercept_b < intercept_a)
            {
                item.marker->side_hi = intercept_a;
                item.marker->side_lo = intercept_b;
            }
            else
            {
                item.marker->side_lo = intercept_a;
                item.marker->side_hi = intercept_b;
            }
            item.marker->edge_dx = (s16)marker_def->x1 - (s16)marker_def->x0;
            item.marker->edge_dy = (s16)marker_def->y1 - (s16)marker_def->y0;
            if (item.marker->edge_dx != 0)
            {
                intercept_a = (s16)marker_def->y0 - item.marker->edge_dy * (s16)marker_def->x0 / item.marker->edge_dx;
                intercept_b = (s16)item.marker->y2 - item.marker->edge_dy * (s16)item.marker->x2 / item.marker->edge_dx;
            }
            else
            {
                intercept_a = (s16)marker_def->x0;
                intercept_b = (s16)item.marker->x2;
            }
            if (intercept_b < intercept_a)
            {
                item.marker->edge_hi = intercept_a;
                item.marker->edge_lo = intercept_b;
            }
            else
            {
                item.marker->edge_lo = intercept_a;
                item.marker->edge_hi = intercept_b;
            }
            marker_def = marker_def->next;
        } while (marker_def != NULL);
    }
    tail->next = NULL;
    obj_defs = map->object_defs;
    prev_obj = (FieldObj*)&scene->objects;
    if (*obj_defs != NULL)
    {
        do
        {
            obj = (FieldObj*)arena.cur;
            obj_def = *obj_defs;
            arena.cur = (u8*)(obj + 1);
            prev_obj->next = obj;
            obj->def = obj_def;
            /* Percentages to 8.8 fixed point. */
            obj->red_green.c.red = (obj_def->scale_x << 8) / 100;
            obj->red_green.c.green = (obj_def->scale_y << 8) / 100;
            obj->blue = (obj_def->scale_z << 8) / 100;
            obj->red_scale = obj->green_scale = obj->blue_scale = FIELD_TINT_SCALE_ONE;
            obj->flags.word = (obj->flags.word & ~FIELD_OBJ_VISIBLE) | (obj_def->flags.low & FIELD_OBJ_DEF_VISIBLE);
            obj->flags.b.node_count = 0;
            obj->x = obj_def->x << 8;
            obj->y = obj_def->y << 8;
            prev_obj = obj;
            obj->z = obj_def->z << 8;
            if ((obj_def->motion.word & FIELD_OBJ_DEF_DRIFT_MASK) == FIELD_OBJ_DEF_NO_DRIFT)
            {
                obj->flags.b.drift_speed = 0;
            }
            else
            {
                obj->flags.b.drift_speed = obj_def->motion.b.drift_speed;
            }
            obj->flags.b.drift_angle = obj_def->motion.b.drift_angle;
            obj->drift_x = 0;
            obj->drift_y = 0;
            part_defs = obj_def->part_defs;
            prev_part = (FieldPart*)&obj->parts;
            if (*part_defs != NULL)
            {
                do
                {
                    part = (FieldPart*)arena.cur;
                    part_def = *part_defs;
                    arena.cur = (u8*)(part + 1);
                    prev_part->next = part;
                    part->def = part_def;
                    part->visible = part_def->u.b.flags & FIELD_PART_DEF_VISIBLE;
                    def_word = part_def->u.word;
                    prev_part = part;
                    if ((def_word & FIELD_PART_DEF_CELLS_MASK) == FIELD_PART_DEF_SINGLE_CELL)
                    {
                        kind = (def_word & FIELD_PART_DEF_KIND_MASK) + 1;
                    }
                    else
                    {
                        kind = def_word & FIELD_PART_DEF_KIND_MASK;
                    }
                    part->kind = kind;
                    part->node_count = 0;
                    part->x = part_def->x << 8;
                    part->y = part_def->y << 8;
                    part->z = part_def->z << 8;
                    part->unk34 = 0;
                    part->sweep_period = part_def->sweep_period;
                    part->sweep_phase = 1;
                    part->row_angle = 0;
                    part->column_angle = 0;
                    part->rotation_angle = 0;
                    part->scale_x = ONE;
                    part->scale_y = ONE;
                    /* Depth CLUTs are offset by the object and part depth and clamped. */
                    if (part_def->u.word & FIELD_PART_DEF_DEPTH_CLUT)
                    {
                        part->clut_bl = field_depth_clut(obj_def->z + part_def->z + part_def->clut_bl);
                        part->clut_tl = field_depth_clut(obj_def->z + part_def->z + part_def->clut_tl);
                        part->clut_br = field_depth_clut(obj_def->z + part_def->z + part_def->clut_br);
                        part->clut_tr = field_depth_clut(obj_def->z + part_def->z + part_def->clut_tr);
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
                    /* 0 = no page seen yet, 1 = one page so far, 2 = no shared page (always for non-sprite kinds). */
                    tpage_state = (part->kind != 0) ? 2 : 0;
                    if (tiles != NULL)
                    {
                        part->shared = field_find_shareable_part(scene, obj, part, tiles);
                        if (part->shared == NULL)
                        {
                            if ((part_def->u.word & FIELD_PART_DEF_CELLS_MASK) == FIELD_PART_DEF_SINGLE_CELL)
                            {
                                count = 1;
                            }
                            else
                            {
                                count = part_def->u.b.cols * part_def->u.b.rows;
                            }
                            bits = part->bits = (u32*)arena.cur;
                            words = (count + 31) / 32;
                            part->bits_size = words * 4;
                            arena.cur = (u8*)(bits + words);
                            bit_word = 0;
                            bit = 1;
                            while (--count != -1)
                            {
                                if (tiles->clut_slot & FIELD_TILE_PRESENT)
                                {
                                    bit_word |= bit;
                                    if (tpage_state == 0)
                                    {
                                        attrs = tiles->texture_attrs;
                                        tpage_state = 1;
                                        tpage = attrs & FIELD_TILE_TPAGE_SLOT_MASK;
                                        abr = (attrs >> 4) & 3;
                                    }
                                    else if ((tpage_state == 1) &&
                                             ((tpage != (tiles->texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK)) || (abr != ((tiles->texture_attrs >> 4) & 3))))
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
                            /* The shared words are stored plus one, so zero means "none". */
                            if (tpage_state == 1)
                            {
                                part->tpage_word = tpage + abr * (1 << FIELD_SHARED_TPAGE_ABR_SHIFT) + 1;
                            }
                            else
                            {
                                part->tpage_word = 0;
                            }
                            if (share_flags == 1)
                            {
                                part->code_word = color + (semi << FIELD_SHARED_CODE_SEMI_SHIFT) + 1;
                            }
                            else
                            {
                                part->code_word = 0;
                            }
                        }
                    }
                    part_defs++;
                } while (*part_defs != NULL);
            }
            obj_defs += 1;
            prev_part->next = NULL;
        } while (*obj_defs != NULL);
    }
    prev_obj->next = NULL;
    node = scene->nodes;
    if (node != NULL)
    {
        do
        {
            node_def = node->def;
            if ((g_field_resource_version >= FIELD_RESOURCE_NODE_OWNERS) && (node_def->obj_index != FIELD_NODE_NO_OWNER))
            {
                if (node_def->part_index != FIELD_NODE_NO_OWNER)
                {
                    node->obj = NULL;
                    node->part = field_get_object_part(node_def->obj_index, node_def->part_index);
                    if (node->part->def->u.word & FIELD_PART_SWEEP_MASK)
                    {
                        scene->secondary_nodes = node;
                    }
                    node->part->node_count++;
                }
                else
                {
                    node->obj = field_get_object(node_def->obj_index);
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
        } while (node != NULL);
    }
    field_prepare_animation_definitions(map->anim_defs[0], FIELD_ANIM_GROUP_TILE);
    field_prepare_animation_definitions(map->anim_defs[1], FIELD_ANIM_GROUP_PALETTE);
    field_prepare_animation_definitions(map->anim_defs[2], FIELD_ANIM_GROUP_TINT);
    field_prepare_animation_definitions(map->anim_defs[3], FIELD_ANIM_GROUP_EFFECT);
    /* getTPage's Y bit for the lower texture bank. */
    lower_bank_y_bit = getTPage(0, 0, 0, FIELD_TILE_LOWER_BANK_VRAM_Y);
    obj = scene->objects;
    if (obj != NULL)
    {
        do
        {
            obj_def = obj->def;
            rgb[0] = obj->red_green.c.red << 8;
            rgb[1] = obj->red_green.c.green << 8;
            rgb[2] = obj->blue << 8;
            palette = (u16*)obj_def->shared_source;
            field_build_tint_colors((u8*)(palette + 2), *palette, rgb);
            part = obj->parts;
            prim_code = 0;
            if (part != NULL)
            {
                do
                {
                    field_set_tint_primitive_code(part->kind, *(u16*)obj_def->shared_source, &prim_code);
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
                                stride = FIELD_CEL_RECORD_SIZE;
                                code_word = part->code_word;
                                part->records = arena.cur;
                                cursor = part->records;
                                if (code_word != 0)
                                {
                                    color = code_word - 1;
                                    part->code_word = FIELD_TILE_COLOR_WORDS[color & FIELD_SHARED_CODE_COLOR_MASK];
                                    share_flags = FIELD_TILE_REC_SHARED_RGB_CODE;
                                    if (color & (1 << FIELD_SHARED_CODE_SEMI_SHIFT))
                                    {
                                        /* Semi-transparency bit of the word's GPU code byte (setSemiTrans). */
                                        ((u8*)&part->code_word)[3] |= 2;
                                    }
                                    stride = FIELD_CEL_RECORD_SIZE - FIELD_CEL_SHARED_WORD_SIZE;
                                }
                                else
                                {
                                    share_flags = 0;
                                }
                                sprite_tpage_word = part->tpage_word;
                                tpage = sprite_tpage_word - 1;
                                if (sprite_tpage_word != 0)
                                {
                                    page_slot = tpage & FIELD_TILE_TPAGE_SLOT_MASK;
                                    if (page_slot >= FIELD_TILE_LOWER_BANK_SLOTS)
                                    {
                                        tpage = FIELD_TPAGE_DEPTH_BITS(part_def->u.word) | FIELD_TPAGE_ABR_BITS(tpage) |
                                                ((u32)(FIELD_TILE_UPPER_BANK_PAGE_X(page_slot) & 0x3FF) >> FIELD_TILE_TPAGE_X_SHIFT);
                                    }
                                    else
                                    {
                                        tpage = FIELD_TPAGE_DEPTH_BITS(part_def->u.word) | FIELD_TPAGE_ABR_BITS(tpage) |
                                                (((u32)FIELD_TILE_LOWER_BANK_PAGE_X(page_slot) >> FIELD_TILE_TPAGE_X_SHIFT) | lower_bank_y_bit);
                                    }
                                    part->tpage_word = _get_mode(1, 0, tpage);
                                    share_flags |= FIELD_TILE_REC_SHARED_TPAGE;
                                    stride -= FIELD_CEL_SHARED_WORD_SIZE;
                                }
                                bits = part->bits;
                                count = part_def->u.b.cols * part_def->u.b.rows;
                                bit = 0;
                                while (--count != -1)
                                {
                                    if (bit == 0)
                                    {
                                        bit_word = *bits;
                                        bits++;
                                        bit = 1;
                                    }
                                    if (bit_word & bit)
                                    {
                                        field_build_sprite_tile_record(tiles, (FieldTileRec*)cursor, FIELD_PART_DEF_DEPTH(part_def->u.word), share_flags);
                                        cursor += stride;
                                        instances++;
                                    }
                                    bit <<= 1;
                                    tiles++;
                                }
                                arena.cur += (u16)instances * stride;
                                break;
                            case 1:
                                break;
                            case 2:
                            case 3:
                            case 4:
                            case 5:
                                code_word = part->code_word;
                                part->records = arena.cur;
                                quad_records = part->records;
                                cursor = quad_records;
                                stride = FIELD_CEL_RECORD_SIZE;
                                if (code_word != 0)
                                {
                                    color = code_word - 1;
                                    part->code_word = FIELD_TILE_COLOR_WORDS[color & FIELD_SHARED_CODE_COLOR_MASK];
                                    share_flags = FIELD_TILE_REC_SHARED_RGB_CODE;
                                    if (color & (1 << FIELD_SHARED_CODE_SEMI_SHIFT))
                                    {
                                        /* Semi-transparency bit of the word's GPU code byte (setSemiTrans). */
                                        ((u8*)&part->code_word)[3] |= 2;
                                    }
                                    stride = FIELD_CEL_RECORD_SIZE - FIELD_CEL_SHARED_WORD_SIZE;
                                }
                                else
                                {
                                    share_flags = 0;
                                }
                                quad_tpage_word = part->tpage_word;
                                tpage = quad_tpage_word - 1;
                                if (quad_tpage_word != 0)
                                {
                                    page_slot = tpage & FIELD_TILE_TPAGE_SLOT_MASK;
                                    if (page_slot >= FIELD_TILE_LOWER_BANK_SLOTS)
                                    {
                                        tpage = FIELD_TPAGE_DEPTH_BITS(part_def->u.word) | FIELD_TPAGE_ABR_BITS(tpage) |
                                                ((u32)(FIELD_TILE_UPPER_BANK_PAGE_X(page_slot) & 0x3FF) >> FIELD_TILE_TPAGE_X_SHIFT);
                                    }
                                    else
                                    {
                                        tpage = FIELD_TPAGE_DEPTH_BITS(part_def->u.word) | FIELD_TPAGE_ABR_BITS(tpage) |
                                                (((u32)FIELD_TILE_LOWER_BANK_PAGE_X(page_slot) >> FIELD_TILE_TPAGE_X_SHIFT) | lower_bank_y_bit);
                                    }
                                    part->tpage_word = tpage;
                                    share_flags |= FIELD_TILE_REC_SHARED_TPAGE;
                                    stride -= FIELD_CEL_SHARED_WORD_SIZE;
                                }
                                bits = part->bits;
                                count = part_def->u.b.cols * part_def->u.b.rows;
                                bit = 0;
                                while (--count != -1)
                                {
                                    if (bit == 0)
                                    {
                                        bit_word = *bits;
                                        bits++;
                                        bit = 1;
                                    }
                                    if (bit_word & bit)
                                    {
                                        field_build_quad_tile_record(tiles, (FieldTileRec*)cursor, FIELD_PART_DEF_DEPTH(part_def->u.word), share_flags);
                                        cursor += stride;
                                        instances++;
                                    }
                                    bit <<= 1;
                                    tiles++;
                                }
                                arena.cur += (u16)instances * stride;
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
                } while (part != NULL);
            }
            obj = obj->next;
        } while (obj != NULL);
    }
    scene->vlc_table = 0;
    scene->unk3C = 0;
    field_build_animation_list(map->anim_defs[0], &arena.cur, &scene->anims);
    field_build_animation_list(map->anim_defs[1], &arena.cur, &scene->strips);
    field_build_animation_list(map->anim_defs[2], &arena.cur, &scene->sprites);
    field_build_animation_list(map->anim_defs[3], &arena.cur, &scene->effects);
    count = FIELD_RESOURCE->seq_count;
    seq_def = FIELD_RESOURCE->seq_defs;
    tail = (FieldLink*)&scene->seqs;
    while (--count != -1)
    {
        seq = (FieldSeq*)arena.cur;
        arena.cur = (u8*)(seq + 1);
        tail->next = (FieldLink*)seq;
        tail = (FieldLink*)seq;
        seq->def = seq_def++;
        seq->flags.word &= ~FIELD_SEQ_PHASE_MASK;
    }
    mask = 1 << object_index;
    tail->next = NULL;
    seq = scene->seqs;
    count = 0;
    while (seq != NULL)
    {
        if (seq->def->start_mask & mask)
        {
            field_start_sequence(seq, count);
        }
        seq = seq->next;
        count++;
    }
    scene->uploads = NULL;
    if (scene->vlc_table != 0)
    {
        vlc_table = arena.cur;
        scene->vlc_table = (s32)vlc_table;
        arena.cur = vlc_table + FIELD_VLC_TABLE_BYTES;
        DecDCTReset(0);
        DecDCTvlcBuild((u_short*)scene->vlc_table);
    }
    mem->top = (u32)arena.cur;
    map->built = 1;
}

/**
 * @brief Prepare one scene list's animation definitions for building.
 *
 * Tags every definition with @p handler_group. For tile blits (and every
 * effect) the cel's shared texture page and rgb/code words are kept only if
 * every present tile of every frame agrees with them, and the frames' source
 * rectangle is rasterized into the cel's bit plane so the cel has a record
 * for every tile any frame uses.
 *
 * @param def Head of the definition list.
 * @param handler_group FIELD_ANIM_GROUP_* of the list.
 * @see decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */
static void field_prepare_animation_definitions(FieldAnimDef* def, s32 handler_group)
{
    u32 page_slot = 0;
    u32 color_index = 0;
    u32 semitrans = 0;
    u32 blend_mode = 0;
    s32 tpage_status;
    s32 code_status;
    FieldAnimDef* tile_def;
    FieldPartDef* grid;
    FieldPart* cel;
    FieldTileDesc* tile;
    FieldTileDesc* frame_tile;
    s32 frame;
    s32 tile_index;
    u32 bit;
    u32* bit_words;
    s32 row;
    s32 col;
    u32 bit_word;

    for (; def != NULL; def = def->next)
    {
        def->flags.b.handler_group = handler_group;
        if (!(((handler_group == FIELD_ANIM_GROUP_TILE) && (FIELD_ANIM_KIND(def) <= FIELD_TILE_ANIM_CEL_RECORDS)) ||
              (handler_group == FIELD_ANIM_GROUP_EFFECT)))
        {
            continue;
        }
        /* Kept apart from def for the rectangle reads; one pointer allocates differently. */
        tile_def = def;
        grid = def->u.tile.grid;
        cel = field_find_grid_part(grid, NULL);
        if (cel->shared != NULL)
        {
            cel = cel->shared;
        }
        if (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_CEL_RECORDS)
        {
            if ((grid->u.word & FIELD_PART_DEF_CELLS_MASK) == FIELD_PART_DEF_SINGLE_CELL)
            {
                def->u.tile.rect_width = 1;
                def->u.tile.rect_height = 1;
            }
            else
            {
                def->u.tile.rect_width = grid->u.b.cols;
                def->u.tile.rect_height = grid->u.b.rows;
            }
        }
        /* Status 0 = no shared word, 1 = every tile agrees, 2 = a tile differs. */
        if (cel->tpage_word != 0)
        {
            u32 tpage = cel->tpage_word - 1;

            tpage_status = 1;
            blend_mode = tpage >> FIELD_SHARED_TPAGE_ABR_SHIFT;
            page_slot = tpage & FIELD_TILE_TPAGE_SLOT_MASK;
        }
        else
        {
            tpage_status = 0;
        }
        if (cel->code_word != 0)
        {
            u32 code = cel->code_word - 1;

            code_status = 1;
            semitrans = code >> FIELD_SHARED_CODE_SEMI_SHIFT;
            color_index = code & FIELD_SHARED_CODE_COLOR_MASK;
        }
        else
        {
            code_status = 0;
        }
        if ((tpage_status != 0) || (code_status != 0))
        {
            frame = def->flags.b.frame_count;
            tile = (FieldTileDesc*)tile_def->data;
            while (--frame != -1)
            {
                tile_index = tile_def->u.tile.rect_width * tile_def->u.tile.rect_height;
                while (--tile_index != -1)
                {
                    if (tile->clut_slot & FIELD_TILE_PRESENT)
                    {
                        if (tpage_status == 1)
                        {
                            u8 texture_attrs = tile->texture_attrs;

                            if ((page_slot != (texture_attrs & FIELD_TILE_TPAGE_SLOT_MASK)) || (blend_mode != ((texture_attrs >> 4) & 3)))
                            {
                                tpage_status = 2;
                            }
                        }
                        if ((code_status == 1) && ((color_index != tile->color_index) || (semitrans != ((tile->texture_attrs >> 6) & 1))))
                        {
                            code_status = 2;
                        }
                    }
                    tile++;
                }
            }
            if (tpage_status != 1)
            {
                cel->tpage_word = 0;
            }
            if (code_status != 1)
            {
                cel->code_word = 0;
            }
        }
        if (((handler_group == FIELD_ANIM_GROUP_TILE) && (FIELD_ANIM_KIND(def) == FIELD_TILE_ANIM_BLIT)) || (handler_group == FIELD_ANIM_GROUP_EFFECT))
        {
            frame = def->flags.b.frame_count;
            tile = (FieldTileDesc*)tile_def->data;
            while (--frame != -1)
            {
                frame_tile = tile;
                bit = 1;
                bit_words = cel->bits;
                bit_word = *bit_words;
                for (row = 0; row != grid->u.b.rows; row++)
                {
                    if (row < tile_def->u.tile.rect_y)
                    {
                        col = grid->u.b.cols;
                        while (--col != -1)
                        {
                            bit <<= 1;
                            if (bit == 0)
                            {
                                *bit_words++ = bit_word;
                                bit = 1;
                                bit_word = *bit_words;
                            }
                        }
                    }
                    else if (row < tile_def->u.tile.rect_y + tile_def->u.tile.rect_height)
                    {
                        for (col = 0; col != grid->u.b.cols; col++)
                        {
                            if ((col >= tile_def->u.tile.rect_x) && (col < tile_def->u.tile.rect_x + tile_def->u.tile.rect_width))
                            {
                                if (frame_tile->clut_slot & FIELD_TILE_PRESENT)
                                {
                                    bit_word |= bit;
                                }
                                frame_tile++;
                            }
                            bit <<= 1;
                            if (bit == 0)
                            {
                                *bit_words++ = bit_word;
                                bit = 1;
                                bit_word = *bit_words;
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                if (bit != 1)
                {
                    *bit_words = bit_word;
                }
                tile += tile_def->u.tile.rect_width * tile_def->u.tile.rect_height;
            }
        }
    }
}

/**
 * @brief Build one scene animation list from its definitions.
 *
 * Allocates a FieldAnim per definition from @p arena, appends it through
 * @p tail and seeds its flags, keyframe and timer. The list group and handler
 * kind then select the node's cel list (a grid's cels or a whole object) and
 * any extra setup. Tile blits and effects expand their cel's tint into the
 * scratchpad colour table and build one set of tile records per frame, one
 * record per set bit of the cel inside the frame rectangle. Uploading kinds
 * also reserve their upload request and scratch colours after the node.
 *
 * @param def Head of the definition list.
 * @param arena Allocation cursor; advanced past every node and record.
 * @param tail Link to store the next node in; cleared after the last one.
 */
static void field_build_animation_list(FieldAnimDef* def, u8** arena, FieldAnim** tail)
{
    s32 rgb[3];
    u8 range_start;
    FieldTintSrc* tint_src;
    u8 primitive_code;
    FieldScene* scene;
    FieldPartDef* grid;
    u16 stagger_timer;
    s32 record_stride;
    u16 tile_count;
    FieldAnim* anim;
    FieldAnimDef* tile_def;
    FieldPart* cel;
    FieldSfxKey* key;
    FieldTweenSpan* span;
    u8* arena_cursor;
    u16* palette_data;
    FieldTileDesc* tile_data;
    FieldTileDesc* frame_descs;
    u32* mask;
    u32 mask_word;
    u32 mask_bit;
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
        anim = (FieldAnim*)*arena;
        *arena = (u8*)&anim->upload;
        *tail = anim;
        tail = &anim->next;
        anim->def = def;
        if (!(def->head.word & FIELD_SPAN_COUNT_MASK))
        {
            anim->flags.word &= ~FIELD_ANIM_FLAG_ACTIVE;
        }
        else
        {
            /* FIELD_ANIM_DEF_ACTIVE (bit 7) becomes FIELD_ANIM_FLAG_ACTIVE (bit 6). */
            anim->flags.word = (anim->flags.word & ~FIELD_ANIM_FLAG_ACTIVE) | ((def->flags.b.kind_flags >> 7) << 6);
        }
        def_flags = def->flags.word;
        anim->repeat_count = 0;
        /* FIELD_ANIM_DEF_PING_PONG (bit 3) becomes FIELD_ANIM_FLAG_PING_PONG (bit 0). */
        control_flags = (anim->flags.word & ~FIELD_ANIM_FLAG_PING_PONG) | ((def_flags >> 3) & 1);
        control_flags &= ~FIELD_ANIM_FLAG_STOP_AT_KEYFRAME;
        control_flags &= ~FIELD_ANIM_FLAG_REVERSE;
        control_flags &= ~FIELD_ANIM_FLAG_START_PENDING;
        control_flags &= ~FIELD_ANIM_FLAG_SECOND_BUFFER;
        control_flags &= ~FIELD_ANIM_FLAG_UPLOAD_PENDING;
        anim->flags.word = control_flags;
        anim->flags.b.stop_keyframe = 0;
        if (def->flags.word & FIELD_ANIM_DEF_SPAN_INDEXED)
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
        if (def->flags.b.handler_group == FIELD_ANIM_GROUP_EFFECT)
        {
            anim->timer = 1;
        }
        else
        {
            span = field_find_count_table_span(def, anim->flags.b.keyframe, &range_start);
            if (def->flags.word & FIELD_ANIM_DEF_TIMED)
            {
                anim->timer = span->duration;
            }
            else
            {
                /* Successive nodes start one frame apart, so they do not all advance together. */
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
        case FIELD_ANIM_GROUP_TILE:
            /* A second pointer to the definition; reading through def alone allocates differently. */
            tile_def = def;
            switch (tile_def->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
            {
            case FIELD_TILE_ANIM_BLIT:
            case FIELD_TILE_ANIM_CEL_RECORDS:
                grid = tile_def->u.tile.grid;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                break;
            case FIELD_TILE_ANIM_CEL_CYCLE:
                grid = tile_def->u.tile.grid;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
                {
                    /* Only the cel of the starting frame is visible. */
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
            case FIELD_TILE_ANIM_UPLOAD:
                if ((anim->flags.word & FIELD_ANIM_FLAG_ACTIVE) && (anim->timer != 1))
                {
                    anim->flags.word |= FIELD_ANIM_FLAG_UPLOAD_PENDING;
                }
                break;
            case FIELD_TILE_ANIM_MOVIE:
                grid = tile_def->u.tile.grid;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                scene->vlc_table = 1;
                break;
            case FIELD_TILE_ANIM_TWEEN_PART:
                grid = tile_def->u.tile.grid;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                field_apply_animation_tween(def, anim, 0);
                break;
            case FIELD_TILE_ANIM_TWEEN_OBJECT:
                tint_src = (FieldTintSrc*)field_find_object_by_definition(tile_def->u.tile.grid);
                anim->cels = (FieldPart*)tint_src;
                field_apply_animation_tween(def, anim, 0);
                break;
            case FIELD_TILE_ANIM_SOUND:
            default:
                grid = tile_def->u.tile.grid;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                anim->owner.tint_src = tint_src;
                key = (FieldSfxKey*)tile_def->data;
                if (((key->control.b.lo & FIELD_SFX_KEY_KIND_MASK) == FIELD_SFX_KEY_SOUND) && (key->sound.word & FIELD_SFX_ONE_SHOT))
                {
                    anim->timer = 1;
                    anim->flags.word |= FIELD_ANIM_FLAG_START_PENDING;
                }
                break;
            }
            break;
        case FIELD_ANIM_GROUP_PALETTE:
            switch (def->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
            {
            case FIELD_PALETTE_ANIM_CEL_CLUT:
                grid = (FieldPartDef*)def->data;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                break;
            case FIELD_PALETTE_ANIM_CEL_LIST_CLUT:
                tint_src = (FieldTintSrc*)field_find_object_by_definition(def->data);
                anim->cels = (FieldPart*)tint_src;
                break;
            }
            break;
        case FIELD_ANIM_GROUP_TINT:
            switch (def->flags.b.kind_flags & FIELD_ANIM_KIND_MASK)
            {
            case FIELD_TINT_ANIM_CEL:
                grid = def->u.tint.grid;
                cel = field_find_grid_part(grid, &tint_src);
                anim->cels = cel;
                anim->owner.tint_src = tint_src;
                break;
            case FIELD_TINT_ANIM_CEL_LIST:
                tint_src = (FieldTintSrc*)field_find_object_by_definition(def->u.tint.grid);
                anim->cels = (FieldPart*)tint_src;
                break;
            }
            break;
        default:
            grid = def->u.tile.grid;
            cel = field_find_grid_part(grid, &tint_src);
            anim->cels = cel;
            break;
        }
        /* Tile blits and effects get their cel's tint and one record set per frame. */
        if (((u32)(def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) <= FIELD_TILE_ANIM_CEL_RECORDS) || (def->flags.b.handler_group == FIELD_ANIM_GROUP_EFFECT))
        {
            rgb[0] = tint_src->red << 8;
            rgb[1] = tint_src->green << 8;
            rgb[2] = tint_src->blue << 8;
            palette_data = tint_src->palette->data;
            field_build_tint_colors((u8*)(palette_data + 2), palette_data[0], rgb);
            primitive_code = 0;
            field_set_tint_primitive_code(cel->kind, tint_src->palette->data[0], &primitive_code);
            anim->frame_data = *arena;
            arena_cursor = *arena;
            /* Sprite and quad records happen to be the same size. */
            switch (cel->kind)
            {
            case 0:
                record_stride = FIELD_CEL_RECORD_SIZE;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                record_stride = FIELD_CEL_RECORD_SIZE;
                break;
            case 1:
            case 6:
                break;
            }
            record_flags = FIELD_TILE_REC_SHARED_RGB_CODE;
            if (cel->code_word != 0)
            {
                record_stride -= FIELD_CEL_SHARED_WORD_SIZE;
            }
            else
            {
                record_flags = 0;
            }
            if (cel->tpage_word != 0)
            {
                record_flags |= FIELD_TILE_REC_SHARED_TPAGE;
                record_stride -= FIELD_CEL_SHARED_WORD_SIZE;
            }
            frame_descs = (FieldTileDesc*)def->data;
            tile_def = def;
            if ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) == FIELD_ANIM_GROUP_KIND(FIELD_ANIM_GROUP_TILE, FIELD_TILE_ANIM_CEL_RECORDS))
            {
                anim->owner.tiles = cel->records;
                frame = def->flags.b.frame_count;
                while (--frame != -1)
                {
                    /* This kind builds no per-frame records; the loop only counts the frames. */
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
                        if (row < tile_def->u.tile.rect_y)
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
                        else if (row < tile_def->u.tile.rect_y + tile_def->u.tile.rect_height)
                        {
                            for (col = 0; col != grid->u.b.cols; col++)
                            {
                                if ((col >= tile_def->u.tile.rect_x) && (col < tile_def->u.tile.rect_x + tile_def->u.tile.rect_width))
                                {
                                    if (mask_word & mask_bit)
                                    {
                                        switch (cel->kind)
                                        {
                                        case 0:
                                            field_build_sprite_tile_record(tile_data, (FieldTileRec*)arena_cursor, FIELD_PART_DEF_DEPTH(grid->u.word),
                                                                           record_flags);
                                            arena_cursor += record_stride;
                                            break;
                                        case 2:
                                        case 3:
                                        case 4:
                                        case 5:
                                            field_build_quad_tile_record(tile_data, (FieldTileRec*)arena_cursor, FIELD_PART_DEF_DEPTH(grid->u.word),
                                                                         record_flags);
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
                    frame_descs += tile_def->u.tile.rect_width * tile_def->u.tile.rect_height;
                }
                anim->frame_tile_count = tile_count;
                if (def->flags.b.handler_group == FIELD_ANIM_GROUP_EFFECT)
                {
                    if (def->flags.word & FIELD_ANIM_DEF_TIMED)
                    {
                        field_blit_animation_frame(def, anim, 0);
                    }
                }
                else if (anim->flags.word & FIELD_ANIM_FLAG_ACTIVE)
                {
                    field_blit_animation_frame(def, anim, 0);
                }
            }
            *arena = arena_cursor;
        }
        /* Uploading kinds reserve an upload request, plus scratch colours for the cycle and the blend. */
        if ((((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) >= FIELD_TILE_ANIM_UPLOAD) &&
             ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) <= FIELD_TILE_ANIM_MOVIE)) ||
            ((def->flags.b.handler_group == FIELD_ANIM_GROUP_PALETTE) && ((u32)(def->flags.b.kind_flags & FIELD_ANIM_KIND_MASK) >= FIELD_PALETTE_ANIM_CYCLE)))
        {
            if ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) == FIELD_ANIM_GROUP_KIND(FIELD_ANIM_GROUP_PALETTE, FIELD_PALETTE_ANIM_CYCLE))
            {
                if (def->u.clut.clut_mode == 0)
                {
                    *arena += sizeof(FieldImageReq) + FIELD_ANIM_SCRATCH_BYTES(FIELD_CLUT_4BIT_COLORS);
                }
                else
                {
                    *arena += sizeof(FieldImageReq) + FIELD_ANIM_SCRATCH_BYTES(FIELD_CLUT_8BIT_COLORS);
                }
            }
            else if ((def->flags.word & FIELD_ANIM_GROUP_KIND_MASK) == FIELD_ANIM_PALETTE_BLEND_WORD)
            {
                if (def->u.clut.clut_mode == 0)
                {
                    *arena += def->u.clut.length * FIELD_ANIM_SCRATCH_BYTES(FIELD_CLUT_4BIT_COLORS) + sizeof(FieldImageReq);
                }
                else
                {
                    *arena += def->u.clut.length * FIELD_ANIM_SCRATCH_BYTES(FIELD_CLUT_8BIT_COLORS) + sizeof(FieldImageReq);
                }
            }
            else
            {
                *arena += sizeof(FieldImageReq);
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
static void field_build_sprite_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags)
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
                /* The shorter record keeps its draw-mode word in the rgb/code slot. */
                record->rgb_code = _get_mode(1, 0, tpage);
            }
        }
        record->u = u_cell * FIELD_TILE_SIZE;
    }
    else
    {
        *(s32*)record = FIELD_CELL_EMPTY;
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
static void field_build_quad_tile_record(FieldTileDesc* desc, FieldTileRec* record, s32 texture_depth, s32 record_flags)
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
                record->tail.quad.u = ((desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE) + FIELD_CELL_UV_RIGHT;
                record->tail.quad.v = desc->packed_uv & FIELD_TILE_V_MASK;
                record->tail.quad.tpage = second_tpage;
            }
            else
            {
                /* Without its rgb/code word the record is shorter: the tail moves up one word. */
                FieldTileRec* shifted = (FieldTileRec*)((u8*)record - FIELD_CEL_SHARED_WORD_SIZE);

                shifted->tail.quad.u = ((desc->packed_uv & FIELD_TILE_U_MASK) * FIELD_TILE_SIZE) + FIELD_CELL_UV_RIGHT;
                shifted->tail.quad.v = desc->packed_uv & FIELD_TILE_V_MASK;
                shifted->tail.quad.tpage = second_tpage;
            }
        }
    }
    else
    {
        *(s32*)record = FIELD_CELL_EMPTY;
    }
}

/**
 * @brief Size the double-buffered primitive area from the scene's objects.
 *
 * Adds up the primitive bytes each part emits per frame (times the number of
 * copies a wrapping object draws), adds a fixed reserve, clamps to a minimum,
 * and allocates two such halves at the allocator top.
 */
void field_size_work_buffer(void)
{
    FieldMemState* state = FIELD_MEM_STATE;
    FieldObj* obj;
    FieldPart* part;
    s32 copies;
    u32 total;
    u32 base;
    u32 min_size;

    total = 0;
    obj = g_field_scene.scene->objects;
    if (obj != NULL)
    {
        do
        {
            s32 flags = obj->def->flags.word;

            /* A wrapping object is drawn up to three times across, twice down. */
            copies = 1;
            if (flags & FIELD_OBJ_DEF_WRAP_X)
            {
                copies = 3;
                if (flags & FIELD_OBJ_DEF_WRAP_X_SIZE)
                {
                    copies = 2;
                }
            }
            if (obj->def->flags.word & FIELD_OBJ_DEF_WRAP_Y)
            {
                copies = copies * 2;
            }
            part = obj->parts;
            if (part != NULL)
            {
                do
                {
                    s32 sprite_cost = copies * FIELD_SPRITE_CELL_BYTES;
                    s32 single_cost = copies * FIELD_SINGLE_CELL_BYTES;
                    s32 quad_cost = copies * FIELD_QUAD_CELL_BYTES;
                    s32 other_cost = copies * FIELD_OTHER_CELL_BYTES;

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
    total = total + FIELD_WORK_BUFFER_RESERVE;
    base = state->top;
    min_size = FIELD_WORK_BUFFER_MIN;
    if (total < min_size)
    {
        total = min_size;
    }
    state->midpoint = base + total;
    state->base = base;
    state->top = base + total * 2;
}

/**
 * @brief Draw every visible part of every active field object.
 *
 * For each active object the camera position is scaled by the definition's
 * scroll factors (parallax), the object's drift is advanced and wrapped, and
 * each visible part is submitted to field_draw_part. On a wrapping object a
 * part near the seam is submitted again one wrap period to the left, to the
 * right and/or above, so it shows on both sides.
 *
 * @param cursor Primitive-buffer cursor, forwarded to the part emitters.
 * @param ot Ordering-table base, forwarded to the part emitters.
 * @param update_mode FIELD_DRAW_ADVANCE advances the drift; FIELD_DRAW_UNSCALED
 *                    ignores the scroll factors.
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

    wrap_size = 0;
    wrap_height = 0;
    scene = g_field_scene.scene;
    viewport.width = scene->header->unk30;
    viewport.camera_x = SHIFT_TOWARD_ZERO(g_field_camera_x, 8);
    viewport.camera_y = SHIFT_TOWARD_ZERO(g_field_camera_y, 8) - SHIFT_TOWARD_ZERO(g_field_camera_z, 9) + VRAM_DRAW_HEIGHT;
    for (obj = scene->objects; obj != NULL; obj = obj->next)
    {
        if (obj->flags.word & FIELD_OBJ_VISIBLE)
        {
            def = obj->def;
            scroll_x = 0;
            if (def->flags.word & FIELD_OBJ_DEF_SCREEN_FIXED)
            {
                scroll_y = 0;
                scroll_z = 0;
            }
            else
            {
                u8 scale_x = def->motion.b.scroll_scale_x;

                if ((scale_x == FIELD_SCROLL_SCALE_ONE) || (update_mode == FIELD_DRAW_UNSCALED))
                {
                    scroll_x = g_field_camera_x;
                }
                else
                {
                    if (scale_x & FIELD_SCROLL_SCALE_NEGATIVE)
                    {
                        scroll_x = (-g_field_camera_x * (scale_x & FIELD_SCROLL_SCALE_MAGNITUDE)) / FIELD_SCROLL_SCALE_ONE;
                    }
                    else
                    {
                        scroll_x = (g_field_camera_x * def->motion.b.scroll_scale_x) / FIELD_SCROLL_SCALE_ONE;
                    }
                }
                {
                    u8 scale_y = def->motion.b.scroll_scale_y;

                    if ((scale_y == FIELD_SCROLL_SCALE_ONE) || (update_mode == FIELD_DRAW_UNSCALED))
                    {
                        scroll_y = SCENE_STATE->camera_y;
                        scroll_z = SCENE_STATE->camera_z;
                    }
                    else if (scale_y & FIELD_SCROLL_SCALE_NEGATIVE)
                    {
                        scroll_y = SHIFT_TOWARD_ZERO(-g_field_camera_y * (scale_y & FIELD_SCROLL_SCALE_MAGNITUDE), 4);
                        scroll_z = (-g_field_camera_z * (def->motion.b.scroll_scale_y & FIELD_SCROLL_SCALE_MAGNITUDE)) / FIELD_SCROLL_SCALE_ONE;
                    }
                    else
                    {
                        scroll_y = SHIFT_TOWARD_ZERO(g_field_camera_y * def->motion.b.scroll_scale_y, 4);
                        scroll_z = (g_field_camera_z * def->motion.b.scroll_scale_y) / FIELD_SCROLL_SCALE_ONE;
                    }
                }
            }
            if (obj->flags.b.drift_speed != 0)
            {
                if (update_mode == FIELD_DRAW_ADVANCE)
                {
                    obj->drift_x += (rcos(obj->flags.b.drift_angle * 16) * obj->flags.b.drift_speed) / 256;
                    obj->drift_y -= (rsin(obj->flags.b.drift_angle * 16) * obj->flags.b.drift_speed) / 256;
                }
                if (def->flags.word & FIELD_OBJ_DEF_WRAP_X)
                {
                    s32 drift;

                    wrap_size = (FIELD_OBJ_WRAP_MIN << 8) << FIELD_OBJ_DEF_WRAP_X_SHIFT(def->flags.word);
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
                if (def->flags.word & FIELD_OBJ_DEF_WRAP_Y)
                {
                    s32 drift;

                    wrap_size = (FIELD_OBJ_WRAP_MIN << 9) << FIELD_OBJ_DEF_WRAP_Y_SHIFT(def->flags.word);
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
            /* From here scroll_x/scroll_z are the object's screen scroll in pixels. */
            scroll_x += obj->drift_x;
            scroll_z += obj->drift_y;
            scroll_x = SHIFT_TOWARD_ZERO(scroll_x, 8);
            scroll_z = SHIFT_TOWARD_ZERO(scroll_y, 8) - SHIFT_TOWARD_ZERO(scroll_z, 9);
            for (part = obj->parts; part != NULL; part = part->next)
            {
                if ((part->visible != 0) && (part->instance_count != 0))
                {
                    s32 top_y;
                    s32 height;
                    s32 screen_y;

                    viewport.x = scroll_x + (obj->x + part->x) / 256;
                    top_y = scroll_z + SHIFT_TOWARD_ZERO(obj->y + part->y, 8);
                    height = part->def->u.b.rows * FIELD_TILE_SIZE;
                    screen_y = top_y - SHIFT_TOWARD_ZERO(obj->z + part->z, 9);
                    height -= VRAM_DRAW_HEIGHT;
                    viewport.y = screen_y - height;
                    if (def->flags.word & FIELD_OBJ_DEF_WRAP_X)
                    {
                        wrap_size = FIELD_OBJ_WRAP_MIN << FIELD_OBJ_DEF_WRAP_X_SHIFT(def->flags.word);
                        if (viewport.x >= 0)
                        {
                            viewport.x = viewport.x & (wrap_size - 1);
                        }
                        else
                        {
                            viewport.x = wrap_size - (-viewport.x & (wrap_size - 1));
                        }
                    }
                    if (def->flags.word & FIELD_OBJ_DEF_WRAP_Y)
                    {
                        wrap_height = FIELD_OBJ_WRAP_MIN << FIELD_OBJ_DEF_WRAP_Y_SHIFT(def->flags.word);
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
                    if (def->flags.word & FIELD_OBJ_DEF_WRAP_X)
                    {
                        if (viewport.x > 0)
                        {
                            viewport.x -= wrap_size;
                            field_draw_part(part, cursor, &viewport, ot);
                            viewport.x += wrap_size;
                        }
                        if (!(def->flags.word & FIELD_OBJ_DEF_WRAP_X_SIZE))
                        {
                            s32 x = viewport.x + wrap_size;

                            if (x < SCREEN_WIDTH)
                            {
                                viewport.x = x;
                                field_draw_part(part, cursor, &viewport, ot);
                                viewport.x -= wrap_size;
                            }
                        }
                    }
                    if (def->flags.word & FIELD_OBJ_DEF_WRAP_Y)
                    {
                        if (viewport.y > 0)
                        {
                            viewport.y -= wrap_height;
                            field_draw_part(part, cursor, &viewport, ot);
                        }
                        if (def->flags.word & FIELD_OBJ_DEF_WRAP_X)
                        {
                            if (viewport.x > 0)
                            {
                                viewport.x -= wrap_size;
                                field_draw_part(part, cursor, &viewport, ot);
                                viewport.x += wrap_size;
                            }
                            if (!(def->flags.word & FIELD_OBJ_DEF_WRAP_X_SIZE))
                            {
                                s32 x = viewport.x + wrap_size;

                                if (x < SCREEN_WIDTH)
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
 * @brief Draw the marker debug overlay: an outline and a numeric label per marker.
 *
 * Each marker emits a red LINE_F4 through its four points, a red LINE_F2
 * closing the first point to the third, and its label (one or two digits) at
 * the first point. Points are shifted by the camera scroll and every vertical
 * coordinate is halved toward zero. The lines are chained into @p ot[-1].
 *
 * @param cursor Primitive-buffer cursor; advanced past everything emitted.
 * @param ot Ordering table; the run is linked into @p ot[-1].
 */
static void field_draw_marker_overlay(u8** cursor, u_long* ot)
{
    s16 label_pos[2];
    FieldScene* scene;
    FieldMarker* marker;
    FieldMarkerDef* def;
    LINE_F4* prim;
    void* prev;
    s32 scroll_x;
    s32 scroll_y;
    s32 origin_y;
    s32 baseline;
    s32 label;
    s32 digits;
    u_long* label_ot;

    prim = (LINE_F4*)*cursor;
    prev = NULL;
    scene = g_field_scene.scene;
    scroll_x = SHIFT_TOWARD_ZERO(g_field_camera_x, 8);
    scroll_y = SHIFT_TOWARD_ZERO(g_field_camera_y, 8) - SHIFT_TOWARD_ZERO(g_field_camera_z, 9);
    marker = scene->markers;
    if (marker != NULL)
    {
        label_ot = ot - 1;
        do
        {
            def = marker->def;
            baseline = def->depth_bias + VRAM_DRAW_HEIGHT;
            setLineF4(prim);
            setRGB0(prim, 0xFF, 0, 0);
            origin_y = scroll_y + baseline;
            setXY4(prim, def->x0 + scroll_x, origin_y - HALF_TOWARD_ZERO((s16)def->y0), def->x1 + scroll_x, origin_y - HALF_TOWARD_ZERO((s16)def->y1),
                   marker->x3 + scroll_x, origin_y - HALF_TOWARD_ZERO((s16)marker->y3), marker->x2 + scroll_x, origin_y - HALF_TOWARD_ZERO((s16)marker->y2));
            if (prev != NULL)
            {
                setaddr(prev, prim);
            }
            prev = prim;
            prim = prim + 1;
            /* The closing edge is a LINE_F2; its x1/y1 overlay LINE_F4's. */
            setLineF2((LINE_F2*)prim);
            setRGB0(prim, 0xFF, 0, 0);
            setXY0(prim, def->x0 + scroll_x, origin_y - HALF_TOWARD_ZERO((s16)def->y0));
            prim->x1 = marker->x2 + scroll_x;
            prim->y1 = origin_y - HALF_TOWARD_ZERO((s16)marker->y2);
            setaddr(prev, prim);
            prev = prim;
            prim = (LINE_F4*)((LINE_F2*)prim + 1);
            label_pos[0] = def->x0 + scroll_x;
            label_pos[1] = origin_y - HALF_TOWARD_ZERO((s16)def->y0);
            label = def->label;
            digits = 2;
            if (def->label < 10)
            {
                digits = 1;
            }
            prim = func_800AD208(label_ot, prim, label, digits, label_pos);
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
 * @brief Emit the 16x16 sprites of one axis-aligned part.
 *
 * Walks @p part 's bit plane row-major; each set bit consumes one FieldCellRec
 * and emits a sprite at its grid cell, preceded by a texture-page packet
 * whenever the page changes. The packets are chained and spliced into the
 * ordering-table entry of the current CLUT, again whenever the interpolated
 * CLUT changes. Rows and columns outside the screen only consume their bits.
 *
 * @param part Part supplying the grid, bit plane, records and corner CLUTs.
 * @param cursor_ptr Primitive-buffer cursor; advanced past everything emitted.
 * @param origin Screen position of the grid's top-left cell.
 * @param ot Ordering table with one two-word entry per CLUT id.
 */
static void field_emit_sprite_grid(FieldPart* part, u8** cursor_ptr, FieldViewport* origin, u_long* ot)
{
    s32 uv_clut;
    s32 tpage_word;
    s32 code_word;
    s32 height;
    s32 interpolate;
    u32 clut_right;
    s32 current_clut;
    s32 clut_left;
    s32 last_page;
    s32 row;
    s32 col;
    s32 done;
    s32 x;
    s32 y;
    s32 stride;
    s32 bit_word;
    s32 bit;
    s32 count;
    u32 clut;
    u32 clut_next;
    s16 clut_top;
    s16 clut_bottom;
    FieldPrim* prim;
    u8* cursor;
    u8* chain;
    u8* record;
    s32* bit_words;
    s32 width;
    FieldPartDef* part_def;

    last_page = 0;
    bit_word = 0;
    current_clut = part->clut_tl;
    clut_left = 0;
    clut_right = 0;
    if ((current_clut == part->clut_tr) && (current_clut == part->clut_bl) && (current_clut == part->clut_br))
    {
        interpolate = 0;
    }
    else
    {
        current_clut = FIELD_CLUT_NONE;
        interpolate = 1;
    }
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    stride = FIELD_CEL_RECORD_SIZE;
    if (code_word != 0)
    {
        stride = FIELD_CEL_RECORD_SIZE - FIELD_CEL_SHARED_WORD_SIZE;
    }
    if (tpage_word != 0)
    {
        stride -= FIELD_CEL_SHARED_WORD_SIZE;
    }
    bit = 0;
    prim = NULL;
    bit_words = part->bits;
    record = part->records;
    part_def = part->def;
    cursor = *cursor_ptr;
    y = origin->y;
    height = part_def->u.b.rows;
    row = height;
    width = part_def->u.b.cols;
    chain = NULL;
    while (--row != -1)
    {
        if (y >= VRAM_DRAW_HEIGHT)
        {
            break;
        }
        if (y <= -FIELD_TILE_SIZE)
        {
            /* Skip the rows above the screen, counting the records they use. */
            count = 0;
            do
            {
                for (col = width - 1; col != -1; col--)
                {
                    if (bit == 0)
                    {
                        bit_word = *bit_words++;
                        bit = 1;
                    }
                    if ((bit_word & bit) != 0)
                    {
                        count++;
                    }
                    bit <<= 1;
                }
                y += FIELD_TILE_SIZE;
            } while ((y <= -FIELD_TILE_SIZE) && (--row != -1));
            record += stride * count;
            if (row <= 0)
            {
                break;
            }
            row--;
        }
        if (interpolate != 0)
        {
            clut_top = part->clut_tl;
            clut_bottom = part->clut_bl;
            if (clut_top != clut_bottom)
            {
                clut_left = ((clut_top * (row + 1)) + (clut_bottom * ((height - row) - 1))) / height;
            }
            else
            {
                clut_left = clut_top;
            }
            clut_top = part->clut_tr;
            clut_bottom = part->clut_br;
            if (clut_top != clut_bottom)
            {
                clut_right = ((clut_top * (row + 1)) + (clut_bottom * ((height - row) - 1))) / height;
            }
            else
            {
                clut_right = clut_top;
            }
        }
        x = origin->x;
        col = width;
        while (--col != -1)
        {
            if (x >= SCREEN_WIDTH)
            {
                /* Past the right edge: count the rest of the row's records. */
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bit_word = *bit_words++;
                        bit = 1;
                    }
                    if ((bit_word & bit) != 0)
                    {
                        count++;
                    }
                    col--;
                    bit <<= 1;
                } while (col != -1);
                record += stride * count;
                break;
            }
            if (x <= -FIELD_TILE_SIZE)
            {
                /* Skip the columns left of the screen. */
                count = 0;
                do
                {
                    if (bit == 0)
                    {
                        bit_word = *bit_words++;
                        bit = 1;
                    }
                    if ((bit_word & bit) != 0)
                    {
                        count++;
                    }
                    x += FIELD_TILE_SIZE;
                    bit <<= 1;
                } while ((x <= -FIELD_TILE_SIZE) && (--col != -1));
                record += stride * count;
                if (col <= 0)
                {
                    break;
                }
                col--;
            }
            if (bit == 0)
            {
                bit_word = *bit_words++;
                bit = 1;
            }
            if ((bit_word & bit) != 0)
            {
                uv_clut = ((FieldCellRec*)record)->uv_clut;
                if (uv_clut != FIELD_CELL_EMPTY)
                {
                    if (interpolate != 0)
                    {
                        /* Two statements: `done = width - col` allocates differently. */
                        done = width;
                        done -= col;
                        if (clut_left != clut_right)
                        {
                            clut = ((clut_left * (col + 1)) + (clut_right * (done - 1))) / width;
                            clut_next = ((clut_left * col) + (clut_right * done)) / width;
                            if (clut < clut_next)
                            {
                                clut = clut_next;
                            }
                        }
                        else
                        {
                            clut = clut_left;
                        }
                        if (clut != current_clut)
                        {
                            if (chain != NULL)
                            {
                                addPrims(&ot[current_clut * 2], chain, prim);
                                chain = NULL;
                            }
                            current_clut = clut;
                        }
                    }
                    if (tpage_word != 0)
                    {
                        prim = (FieldPrim*)cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += sizeof(DR_TPAGE);
                            prim->tag = FIELD_PRIM_TAG(cursor, FIELD_DR_TPAGE_WORDS);
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
                            cursor += sizeof(DR_TPAGE);
                            prim->tag = FIELD_PRIM_TAG(cursor, FIELD_DR_TPAGE_WORDS);
                            if (code_word != 0)
                            {
                                last_page = (prim->code = ((FieldCellRec*)record)->rgb_code);
                            }
                            else
                            {
                                last_page = (prim->code = ((FieldCellRec*)record)->tpage);
                            }
                        }
                        else if (last_page != ((FieldCellRec*)record)->tpage)
                        {
                            cursor += sizeof(DR_TPAGE);
                            prim->tag = FIELD_PRIM_TAG(cursor, FIELD_DR_TPAGE_WORDS);
                            if (code_word != 0)
                            {
                                last_page = (prim->code = ((FieldCellRec*)record)->rgb_code);
                            }
                            else
                            {
                                last_page = (prim->code = ((FieldCellRec*)record)->tpage);
                            }
                        }
                        prim = (FieldPrim*)cursor;
                    }
                    cursor += sizeof(FieldPrim);
                    prim->tag = FIELD_PRIM_TAG(cursor, FIELD_SPRT_WORDS);
                    if (code_word != 0)
                    {
                        prim->code = code_word;
                    }
                    else
                    {
                        prim->code = ((FieldCellRec*)record)->rgb_code;
                    }
                    prim->xy = (x & 0xFFFF) | (y << 16);
                    prim->uv = uv_clut;
                }
                record += stride;
            }
            bit <<= 1;
            x += FIELD_TILE_SIZE;
        }
        y += FIELD_TILE_SIZE;
    }
    if (chain != NULL)
    {
        addPrims(&ot[current_clut * 2], chain, prim);
    }
    *cursor_ptr = cursor;
}

/**
 * @brief Emit the rotated and scaled quads of one part.
 *
 * Rotated sibling of field_emit_sprite_grid. The column steps are rotated
 * once into FIELD_ROT_COL_STEPS; each row then fills one of two scratchpad
 * point buffers, and every set bit emits a POLY_FT4 between the previous and
 * the current row's points. Cells entirely off one side of the screen are
 * skipped. The pivot is chosen by the part's sweep mode.
 *
 * @param part Part supplying the grid, bit plane, records, angles, scales and
 *             corner CLUTs.
 * @param cursor_ptr Primitive-buffer cursor; advanced past everything emitted.
 * @param origin Screen placement; its width and camera words place the pivot.
 * @param ot Ordering table with one two-word entry per CLUT id.
 */
static void field_emit_rotated_sprite_grid(FieldPart* part, u8** cursor_ptr, FieldViewport* origin, u_long* ot)
{
    u8* record;
    s32* bit_words;
    s32 tpage_word;
    s32 code_word;
    s32 bit_word;
    s32 bit;
    s32 height;
    s32 interpolate;
    s32 stride;
    s32 sin_rot;
    s32 cos_rot;
    s32 use_row_b;
    s32 current_clut;
    s32 clut_left;
    u32 clut_right;
    s32 corner_clut;
    s32 row;
    s32 col;
    s32 done;
    s32 width;
    s32 offset_x;
    s32 offset_y;
    s32 pivot_x;
    s32 pivot_y;
    s32 row_cos;
    s32 col_cos;
    s32 scaled;
    s32 row_sin_term;
    s32 row_cos_term;
    s32 on_screen;
    s32 uv_clut;
    u32 clut;
    u32 clut_next;
    FieldColStep* steps;
    FieldPoint* point;
    FieldPoint* prev_row;
    FieldPoint* this_row;
    FieldPolyPrim* prim;
    u8* cursor;
    u8* chain;

    bit_word = 0;
    clut_left = 0;
    corner_clut = part->clut_tl;
    clut_right = 0;
    if ((corner_clut == part->clut_tr) && (corner_clut == part->clut_bl) && (corner_clut == part->clut_br))
    {
        current_clut = corner_clut;
        interpolate = 0;
    }
    else
    {
        current_clut = FIELD_CLUT_NONE;
        interpolate = 1;
    }
    stride = FIELD_CEL_RECORD_SIZE;
    code_word = part->code_word;
    tpage_word = part->tpage_word;
    if (code_word != 0)
    {
        stride = FIELD_CEL_RECORD_SIZE - FIELD_CEL_SHARED_WORD_SIZE;
    }
    if (tpage_word != 0)
    {
        stride -= FIELD_CEL_SHARED_WORD_SIZE;
    }
    bit = 0;
    bit_words = part->bits;
    record = part->records;
    cursor = *cursor_ptr;
    prim = NULL;
    width = part->def->u.b.cols;
    height = part->def->u.b.rows;
    row_cos = rcos(part->row_angle);
    col_cos = rcos(part->column_angle);
    sin_rot = rsin(part->rotation_angle);
    chain = NULL;
    cos_rot = rcos(part->rotation_angle);
    switch (FIELD_PART_SWEEP_MODE(part->def))
    {
    case 1:
    case 2:
        pivot_y = origin->camera_y;
        pivot_x = origin->camera_x + (origin->width / 2);
        offset_x = origin->x - pivot_x;
        offset_y = origin->y - pivot_y;
        break;
    case 3:
        pivot_x = origin->camera_x;
        pivot_y = origin->camera_y;
        offset_x = origin->x - pivot_x;
        offset_y = origin->y - pivot_y;
        break;
    case 4:
        pivot_x = origin->width + origin->camera_x;
        pivot_y = origin->camera_y;
        offset_x = origin->x - pivot_x;
        offset_y = origin->y - pivot_y;
        break;
    case 5:
    default:
        offset_x = -width * (FIELD_TILE_SIZE / 2);
        offset_y = -height * (FIELD_TILE_SIZE / 2);
        pivot_x = origin->x + (width * (FIELD_TILE_SIZE / 2));
        pivot_y = origin->y + (height * (FIELD_TILE_SIZE / 2));
        break;
    }
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(offset_y * part->scale_y, 8) * row_cos, 12);
    steps = FIELD_ROT_COL_STEPS;
    for (col = width; col != -1; col--)
    {
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(offset_x * part->scale_x, 8) * col_cos, 12);
        offset_x += FIELD_TILE_SIZE;
        steps->sin_term = scaled * sin_rot;
        steps->cos_term = scaled * cos_rot;
        steps++;
    }
    steps = FIELD_ROT_COL_STEPS;
    point = FIELD_ROT_ROW_A;
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(offset_y * part->scale_y, 8) * row_cos, 12);
    row_sin_term = scaled * sin_rot;
    row_cos_term = scaled * cos_rot;
    for (col = width; col != -1; col--)
    {
        point->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - row_sin_term, 16) + pivot_x;
        point->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + row_cos_term, 16) + pivot_y;
        steps++;
        point++;
    }
    use_row_b = 0;
    row = height;
    while (--row != -1)
    {
        if (interpolate != 0)
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
        if (use_row_b == 0)
        {
            prev_row = FIELD_ROT_ROW_A;
            this_row = FIELD_ROT_ROW_B;
            use_row_b = 1;
        }
        else
        {
            prev_row = FIELD_ROT_ROW_B;
            this_row = FIELD_ROT_ROW_A;
            use_row_b = 0;
        }
        offset_y += FIELD_TILE_SIZE;
        steps = FIELD_ROT_COL_STEPS;
        point = this_row;
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(offset_y * part->scale_y, 8) * row_cos, 12);
        row_sin_term = scaled * sin_rot;
        row_cos_term = scaled * cos_rot;
        for (col = width; col != -1; col--)
        {
            point->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - row_sin_term, 16) + pivot_x;
            point->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + row_cos_term, 16) + pivot_y;
            steps++;
            point++;
        }
        for (col = width - 1; col != -1; col--)
        {
            if (bit == 0)
            {
                bit_word = *bit_words++;
                bit = 1;
            }
            if ((bit_word & bit) != 0)
            {
                if (!((prev_row[0].p.x >= 0) || (prev_row[1].p.x >= 0) || (this_row[0].p.x >= 0) || (this_row[1].p.x >= 0)))
                {
                    on_screen = 0;
                }
                else if (!((prev_row[0].p.y >= 0) || (prev_row[1].p.y >= 0) || (this_row[0].p.y >= 0) || (this_row[1].p.y >= 0)))
                {
                    on_screen = 0;
                }
                else if (!((prev_row[0].p.x < SCREEN_WIDTH) || (prev_row[1].p.x < SCREEN_WIDTH) || (this_row[0].p.x < SCREEN_WIDTH) ||
                           (this_row[1].p.x < SCREEN_WIDTH)))
                {
                    on_screen = 0;
                }
                else if ((prev_row[0].p.y < VRAM_DRAW_HEIGHT) || (prev_row[1].p.y < VRAM_DRAW_HEIGHT) || (this_row[0].p.y < VRAM_DRAW_HEIGHT) ||
                         (this_row[1].p.y < VRAM_DRAW_HEIGHT))
                {
                    on_screen = 1;
                }
                else
                {
                    on_screen = 0;
                }
                if (on_screen != 0)
                {
                    uv_clut = ((FieldCellRec*)record)->uv_clut;
                    if (uv_clut != FIELD_CELL_EMPTY)
                    {
                        if (interpolate != 0)
                        {
                            done = width - col;
                            if (clut_left != clut_right)
                            {
                                clut = ((clut_left * (col + 1)) + (clut_right * (done - 1))) / width;
                                clut_next = ((clut_left * col) + (clut_right * done)) / width;
                                if (clut < clut_next)
                                {
                                    clut = clut_next;
                                }
                            }
                            else
                            {
                                clut = clut_left;
                            }
                            if (clut != current_clut)
                            {
                                if (chain != NULL)
                                {
                                    addPrims(&ot[current_clut * 2], chain, prim);
                                    chain = NULL;
                                }
                                current_clut = clut;
                            }
                        }
                        if (chain == NULL)
                        {
                            prim = (FieldPolyPrim*)cursor;
                            chain = cursor;
                        }
                        else
                        {
                            prim = (FieldPolyPrim*)cursor;
                        }
                        cursor += sizeof(FieldPolyPrim);
                        prim->tag = FIELD_PRIM_TAG(cursor, FIELD_POLY_FT4_WORDS);
                        if (code_word != 0)
                        {
                            prim->code = code_word;
                        }
                        else
                        {
                            prim->code = ((FieldCellRec*)record)->rgb_code;
                        }
                        if (tpage_word != 0)
                        {
                            prim->uv1 = ((uv_clut & 0xFFFF) + FIELD_CELL_UV_RIGHT) | tpage_word;
                        }
                        else if (code_word != 0)
                        {
                            prim->uv1 = ((FieldCellRec*)record)->rgb_code;
                        }
                        else
                        {
                            prim->uv1 = ((FieldCellRec*)record)->tpage;
                        }
                        prim->uv0 = uv_clut;
                        prim->uv2 = (uv_clut & 0xFFFF) + FIELD_CELL_UV_BOTTOM;
                        prim->uv3 = (uv_clut & 0xFFFF) + (FIELD_CELL_UV_BOTTOM + FIELD_CELL_UV_RIGHT);
                        prim->xy0 = prev_row[0].word;
                        prim->xy1 = prev_row[1].word;
                        prim->xy2 = this_row[0].word;
                        prim->xy3 = this_row[1].word;
                    }
                }
                record += stride;
            }
            prev_row++;
            this_row++;
            bit <<= 1;
        }
    }
    if (chain != NULL)
    {
        addPrims(&ot[current_clut * 2], chain, prim);
    }
    *cursor_ptr = cursor;
}

/**
 * @brief Find an already-built part whose bit plane and records can be reused.
 *
 * A candidate uses the same tile descriptors as @p part and belongs to an
 * object with the same definition, or with the same shared source and tint.
 * Parts whose definition is FIELD_PART_DEF_UNSHARED never share.
 *
 * @param scene Scene whose objects are searched.
 * @param obj Object that owns @p part.
 * @param part Part being built.
 * @param tiles Tile descriptors of @p part (FieldPartDef::tiles).
 * @return The part to share, or NULL; the search stops at @p part itself, so
 *         only parts built before it are found.
 */
static FieldPart* field_find_shareable_part(FieldScene* scene, FieldObj* obj, FieldPart* part, FieldTileDesc* tiles)
{
    FieldObj* other_obj;
    FieldPart* other;
    FieldObjDef* obj_def;
    FieldObjDef* other_def;

    if (!(part->def->u.word & FIELD_PART_DEF_UNSHARED))
    {
        for (other_obj = scene->objects; other_obj != NULL; other_obj = other_obj->next)
        {
            for (other = other_obj->parts; other != NULL; other = other->next)
            {
                if (tiles == other->def->tiles)
                {
                    if ((obj == other_obj) && (part == other))
                    {
                        return NULL;
                    }
                    if (!(other->def->u.word & FIELD_PART_DEF_UNSHARED))
                    {
                        obj_def = obj->def;
                        other_def = other_obj->def;
                        if ((obj_def == other_def) || ((obj_def->shared_source == other_def->shared_source) &&
                                                       (obj->red_green.word == other_obj->red_green.word) && (obj->blue == other_obj->blue)))
                        {
                            return other;
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Draw one part with the emitter its kind selects.
 *
 * Kind 0 is an axis-aligned sprite grid and kinds 2 to 5 a rotated one; the
 * other kinds draw nothing here.
 *
 * @param part Part to draw.
 * @param cursor Primitive-buffer cursor.
 * @param origin Screen placement of the part.
 * @param ot Ordering table with one two-word entry per CLUT id.
 */
static void field_draw_part(FieldPart* part, u8** cursor, FieldViewport* origin, u_long* ot)
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
 * @brief Issue the scene's pending VRAM uploads and empty the list.
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
 * @brief Empty function; nothing references it.
 */
void func_800569F4(void)
{
}

/**
 * @brief Empty function; nothing references it.
 */
void func_800569FC(void)
{
}
