#include "common.h"

struct FieldAnimDefRasterView;
struct FieldPartDef;
struct FieldTileDesc;
struct FieldPart;

/**
 * @brief Animation-definition view used while preparing tile masks.
 */
typedef struct FieldAnimDefRasterView
{
    u8 pad_00[4];
    union
    {
        u32 word;
        struct
        {
            u8 pad_04[2];
            u8 frame_count;
            u8 handler_group;
        } bytes;
    } flags;
    struct FieldAnimDefRasterView* next;
    u8 rect_x;
    u8 rect_y;
    u8 rect_width;
    u8 rect_height;
    struct FieldPartDef* part_def;
    struct FieldTileDesc* frame_tiles;
} FieldAnimDefRasterView;

/**
 * @brief Tile-grid definition referenced by a runtime field part.
 */
typedef struct FieldPartDef
{
    u8 pad_00[8];
    union
    {
        u32 word;
        struct
        {
            u8 pad_08[2];
            u8 cols;
            u8 rows;
        } bytes;
    } flags;
} FieldPartDef;

/**
 * @brief Packed four-byte source descriptor for one field tile.
 */
typedef struct FieldTileDesc
{
    u8 clut_slot;
    u8 texture_attrs;
    u8 packed_uv;
    u8 color_index;
} FieldTileDesc;

/**
 * @brief Runtime field-part view used while validating shared tile attributes.
 */
typedef struct FieldPart
{
    u8 pad_00[8];
    struct FieldPart* linked_part;
    s32* bits;
    u8 pad_10[8];
    u32 tpage_word;
    u32 code_word;
} FieldPart;
typedef struct FieldAnimDef FieldAnimDef;
typedef struct FieldAnim FieldAnim;
typedef struct FieldAnimCel FieldAnimCel;

typedef struct
{
    void *tiles; /* 0x00 */
    u8 _pad0[8 - 4];
    union
    {
        u32 word;
        struct
        {
            u8 _pad1[2];
            u8 cols; /* 0x0A */
            u8 rows; /* 0x0B */
        } b;
    } u;
} FieldTileGrid;

struct FieldAnimDef
{
    u8 unk0;  /* 0x00 */
    u8 unk1;  /* 0x01 */
    u8 unk2;  /* 0x02 */
    u8 _pad0;
    u8 flags; /* 0x04 */
    u8 unk5;  /* 0x05 */
    u8 unk6;  /* 0x06 */
    u8 handler_group; /* 0x07 */
    FieldAnimDef *next; /* 0x08 */
    u8 unkC;  /* 0x0C */
    u8 unkD;  /* 0x0D */
    u8 unkE;  /* 0x0E */
    u8 unkF;  /* 0x0F */
    void *unk10; /* 0x10 */
    s32 *data;  /* 0x14 */
};

typedef union
{
    s32 word;
    struct
    {
        u8 unk0;
        u8 state; /* 0x25 */
        u8 keyframe;      /* 0x26 */
        u8 stop_keyframe; /* 0x27 */
    } b;
} FieldAnimFlags;

struct FieldAnimCel
{
    FieldAnimCel *next;  /* 0x00 */
    FieldTileGrid *grid; /* 0x04 */
    u8 _pad0[0xC - 8];
    u32 *mask;  /* 0x0C */
    u8 *tiles;  /* 0x10 */
    u8 _pad1[0x18 - 0x14];
    s32 tpage_word; /* 0x18 */
    s32 code_word;  /* 0x1C */
    s8 active;      /* 0x20 */
    u8 format;  /* 0x21 */
};

struct FieldAnim
{
    FieldAnim *next;      /* 0x00 */
    FieldAnimDef *def;    /* 0x04 */
    u8 _pad0[0xC - 8];
    FieldAnimCel *cels;   /* 0x0C */
    s32 unk10;            /* 0x10 */
    u8 _pad1[0x20 - 0x14];
    u8 *frame_data;       /* 0x20 */
    FieldAnimFlags flags; /* 0x24 */
    u8 repeat_count;      /* 0x28 */
    u8 _pad2;
    u16 timer;            /* 0x2A */
    u16 frame_tile_count; /* 0x2C */
    u8 _pad3[0x30 - 0x2E];
};

typedef struct
{
    u8 _pad0[4];
    u16 *data; /* 0x04 */
} FieldTintPal;

typedef struct
{
    u8 _pad0[4];
    FieldTintPal *palette; /* 0x04 */
    u8 _pad1[0x10 - 8];
    u16 red;   /* 0x10 */
    u16 green; /* 0x12 */
    u16 blue;  /* 0x14 */
} FieldTintSrc;

typedef struct
{
    u8 _pad0;
    u8 range_start; /* 0x01 */
    u16 duration;   /* 0x02 */
} FieldTweenSpan;

/** @brief Minimal view of one sound keyframe entry. */
typedef struct
{
    u8 kind;  /* 0x00 */
    u8 _pad0;
    u16 sound_flags; /* 0x02 */
} FieldSfxKey;

typedef struct
{
    u8 _pad0[0x38];
    s32 unk38; /* 0x38 */
} FieldScene;

extern FieldScene *g_field_scene;

/**
 * @brief Find the runtime part whose definition pointer matches @p part_def.
 *
 * @param part_def Part definition to find in the scene object lists.
 * @param owner_out Optional output for the part's owning object/tint-source view.
 * @return Matching runtime part, or NULL when the definition is not in use.
 */
FieldPart *func_8005ABD8(void *part_def, FieldTintSrc **owner_out);

/**
 * @brief Prepare tile-animation definitions and their runtime presence masks.
 *
 * Assigns @p handler_group to every definition. For tile handlers in groups
 * zero and three, it verifies that the runtime part's shared TPage and
 * RGB/code words agree with every present source tile, clearing either shared
 * word when the descriptors disagree. It also rasterizes the definition's
 * source rectangle into the runtime part's row-major presence bitmap.
 *
 * @param head Head of the linked animation-definition list.
 * @param handler_group Scene animation-list group, in the range 0 through 3.
 *
 * @see decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */
void field_prepare_animation_definitions(void* head, s32 handler_group)
{
    FieldAnimDefRasterView* def = (FieldAnimDefRasterView*)head;
    u32 shared_page_slot = 0;
    u32 shared_color_index = 0;
    u32 shared_semitrans = 0;
    u32 shared_blend_mode = 0;
    s32 tpage_status;
    s32 code_status;
    int minus_one;
    FieldAnimDefRasterView* rec;
    FieldPartDef* part_def;
    FieldPart* part;
    FieldTileDesc* tile;
    FieldTileDesc* mask_tile;
    s32 frame;
    s32 tile_index;
    s32 mask_bit;
    s32* mask;
    s32 row;
    s32 col;
    s32 mask_word;
    if (def != 0)
    {
        do
        {
            def->flags.bytes.handler_group = handler_group;
            if (((handler_group == 0) && ((def->flags.word & 7U) < 2U)) || (handler_group == 3))
            {
                rec = def;

                part_def = def->part_def;
                part = func_8005ABD8(part_def, 0);
                if (part->linked_part != 0)
                {
                    part = part->linked_part;
                }
                if ((def->flags.word & 7U) == 1)
                {
                    if ((part_def->flags.word & 0xF00U) == 0x100U)
                    {
                        def->rect_width = 1;
                        def->rect_height = 1;
                    }
                    else
                    {
                        def->rect_width = part_def->flags.bytes.cols;
                        def->rect_height = part_def->flags.bytes.rows;
                    }
                }
                if (part->tpage_word != 0)
                {
                    u32 temp = part->tpage_word - 1;
                    tpage_status = 1;
                    shared_blend_mode = temp >> 4;
                    shared_page_slot = temp & 0xF;
                }
                else
                {
                    tpage_status = 0;
                }
                if (part->code_word != 0)
                {
                    u32 temp = part->code_word - 1;
                    code_status = 1;
                    shared_semitrans = temp >> 9;
                    shared_color_index = temp & 0xFF;
                }
                else
                {
                    code_status = 0;
                }
                if ((tpage_status != 0) || (code_status != 0))
                {
                    frame = def->flags.bytes.frame_count - 1;
                    tile = rec->frame_tiles;
                    if (frame != (-1))
                    {
                        do
                        {
                            tile_index = ((frame = rec->rect_width) * rec->rect_height) - 1;
                            if (tile_index != (-1))
                            {
                                do
                                {
                                    if (tile->clut_slot & 0x80)
                                    {
                                        if (tpage_status == 1)
                                        {
                                            u8 texture_attrs = tile->texture_attrs;
                                            if ((shared_page_slot != (texture_attrs & 0xF)) ||
                                                (shared_blend_mode != ((texture_attrs >> 4) & 3)))
                                            {
                                                tpage_status = 2;
                                            }
                                        }
                                        if (code_status == 1)
                                        {
                                            u8 color_index = tile->color_index;
                                            u8 texture_attrs = tile->texture_attrs;
                                            if ((shared_color_index != color_index) ||
                                                (shared_semitrans != ((texture_attrs >> 6) & 1)))
                                            {
                                                code_status = 2;
                                            }
                                        }
                                    }
                                    tile++;
                                    tile_index--;
                                } while (tile_index != (-1));
                            }
                            frame--;
                        } while (frame != (-1));
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

                if (((handler_group == 0) && ((def->flags.word & 7U) == 0)) || (handler_group == 3))
                {
                    frame = def->flags.bytes.frame_count - 1;
                    tile = rec->frame_tiles;
                    if (frame != (-1))
                    {
                        do
                        {
                            mask_tile = tile;
                            mask_bit = 1;
                            mask = part->bits;
                            mask_word = *mask;
                            if (part_def->flags.bytes.rows != 0)
                            {
                                row = 0;
                                do
                                {
                                    if (row < rec->rect_y)
                                    {
                                        minus_one = -1;
                                        col = part_def->flags.bytes.cols - 1;
                                        if (col != minus_one)
                                        {
                                            do
                                            {
                                                mask_bit <<= 1;
                                                if (mask_bit == 0)
                                                {
                                                    *mask = mask_word;
                                                    mask++;
                                                    mask_bit = 1;
                                                    mask_word = *mask;
                                                }
                                                col--;
                                            } while (col != (-1));
                                        }
                                    }
                                    else if (row < (rec->rect_y + rec->rect_height))
                                    {
                                        if (part_def->flags.bytes.cols != 0)
                                        {
                                            col = 0;
                                            do
                                            {
                                                if ((col >= rec->rect_x) &&
                                                    (col < (rec->rect_x + rec->rect_width)))
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
                                                    *mask = mask_word;
                                                    mask++;
                                                    mask_bit = 1;
                                                    mask_word = *mask;
                                                }
                                                col++;
                                            } while (col != part_def->flags.bytes.cols);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    row++;
                                } while (row != part_def->flags.bytes.rows);
                            }
                            if (mask_bit != 1)
                            {
                                *mask = mask_word;
                            }
                            frame--;
                            tile += rec->rect_width * rec->rect_height;
                        } while (frame != (-1));
                    }
                }
            }
            def = def->next;
        } while (def != 0);
    }
}

FieldAnimCel *field_find_object_by_definition(void *definition);
u8 *field_find_count_table_span(FieldAnimDef *, s32, u8 *);
void field_apply_animation_tween(FieldAnimDef *, FieldAnim *, s32);
void field_blit_animation_frame(FieldAnimDef *, FieldAnim *, s32);
void func_8005AC50(void *colors, u16 color_count, s32 *rgb_scale);
void func_8005AD20(u8 format, u16 color_count, s8 *primitive_code);
void field_build_sprite_tile_record(s32 *, u8 *, s32, s32);
void field_build_quad_tile_record(s32 *, u8 *, s32, s32);

/**
 * @brief Build the scene's animation node list from a definition chain.
 *
 * Walks @p def 's chain and, for each definition, bump-allocates a 0x30-byte
 * FieldAnim out of the arena at @p arena and tail-appends it to the list at
 * @p tail. Each node is seeded from its definition: the play-mode flags at
 * FieldAnim::flags, the starting keyframe cursor, the loop counter, and the
 * keyframe length from field_find_count_table_span. The handler kind - the low three bits of
 * the word at FieldAnimDef::flags, qualified by
 * FieldAnimDef::handler_group - then selects how the node's cel list is
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
 *
 * @note NOT MATCHED - 89.12% (399/704 exact rows, 19 insns short, frame 0x88 vs
 *       0x90). This replaces an earlier 89.01% version that was raw m2c output
 *       and semantically broken (locals read before assignment, a switch with
 *       statements before its first case, a fall-through case with no break).
 *       The remaining gap is a single register-allocation flip: the target keeps
 *       @p def in t0 and the cel cursor in t3 - both caller-saved - and spills
 *       and reloads them around all 17 calls, while this version wins them
 *       callee-saved registers and so emits no spill traffic. That missing
 *       traffic is the whole 19-insn shortfall, the 8-byte frame difference and
 *       every remaining structural row. @p def needs to drop from 94 to 91
 *       weighted refs to lose s7 to the arena cursor. Raising pressure
 *       artificially is worth +51 to +66 exact rows, so the natural construct
 *       that does it is the only thing left to find. See
 *       working/func_80053C7C/status.md for the full evidence and the list of
 *       probe classes already retired.
 * @note The five @c flags masks must stay SEPARATE statements; fold-const
 *       collapses them into one @c and if written as a single expression.
 * @note Both @c cel->format switches need their empty @c case @c 1: / @c case
 *       @c 6: arms to emit the 7-entry jump tables, as in field_retarget_cel_cluts.
 * @note The three @c & @c 7 handler switches read @c def->flags as a byte; the
 *       @c & @c 0xFF000007 and @c & @c 0x40 / @c & @c 0x20 tests read the whole
 *       word. Both views of the same field are required.
 * @note @c rec is a local copy of @p def, needed twice - once in the
 *       @c handler_group
 *       @c == @c 0 arm and once before the record loop. It is what puts the
 *       definition pointer in s4 and is worth 2.8%.
 *
 * @see decomp.me (89.12%) TODO
 */
void field_build_animation_list(FieldAnimDef *def, u8 **arena, FieldAnim **tail)
{
    s32 rgb[3];       /* sp10 */
    u8 range_start;      /* sp20 */
    FieldTintSrc *tint_src;/* sp24 */
    s8 primitive_code;   /* sp28 */
    FieldScene *scene;/* sp2C */
    FieldTileGrid *grid; /* sp30 */
    u16 stagger_timer;   /* sp38 */
    s32 record_stride;   /* sp40 */
    u16 tile_count;      /* sp48 */
    FieldAnim *anim;
    FieldAnimDef *rec;
    FieldAnimCel *cel;
    FieldSfxKey *key;
    FieldTweenSpan *span;
    u8 *arena_cursor;
    u8 *tile_record;
    u16 *palette_data;
    s32 *tile_data;
    s32 *frame_descs;
    u32 *mask;
    u32 mask_word;
    u32 mask_bit;
    s32 handler_kind;
    s32 control_flags;
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
    scene = g_field_scene;
    if (def != NULL)
    {
        do
        {
            anim = (FieldAnim *) *arena;
            *arena = (u8 *) anim + 0x30;
            *tail = anim;
            tail = &anim->next;
            anim->def = def;
            if (!(*(u32 *) &def->unk0 & 0x7F))
            {
                anim->flags.word &= ~0x40;
            }
            else
            {
                anim->flags.word = (anim->flags.word & ~0x40) | ((def->flags >> 7) << 6);
            }
            anim->repeat_count = 0;
            control_flags = (anim->flags.word & ~1) | ((*(u32 *) &def->flags >> 3) & 1);
            control_flags &= ~2;
            control_flags &= ~4;
            control_flags &= ~8;
            control_flags &= ~0x10;
            control_flags &= ~0x20;
            anim->flags.word = control_flags;
            anim->flags.b.stop_keyframe = 0;
            if (*(s32 *) &def->flags & 0x40)
            {
                anim->flags.b.keyframe = 0;
                anim->flags.b.state = def->unk1;
            }
            else
            {
                initial_state = def->unk1;
                anim->flags.b.state = initial_state;
                anim->flags.b.keyframe = initial_state;
            }
            if (def->handler_group == 3)
            {
                anim->timer = 1;
            }
            else
            {
                span = (FieldTweenSpan *) field_find_count_table_span(def, anim->flags.b.keyframe, &range_start);
                if (*(s32 *) &def->flags & 0x20)
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
            switch (def->handler_group)
            {
            case 0:
                rec = def;
                switch (rec->flags & 7)
                {
                case 0:
                case 1:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    break;
                case 2:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    if ((anim->flags.word & 0x40) &&
                        (range_start = 0, frame = def->unk5, frame != -1))
                    {
                        do
                        {
                            frame -= 1;
                            cel->active = range_start == anim->flags.b.state;
                            cel = cel->next;
                            range_start += 1;
                        }
                        while (frame != -1);
                    }
                    break;
                case 3:
                    if ((anim->flags.word & 0x40) && (anim->timer != 1))
                    {
                        anim->flags.word |= 0x20;
                    }
                    break;
                case 4:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    scene->unk38 = 1;
                    break;
                case 5:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    field_apply_animation_tween(def, anim, 0);
                    break;
                case 6:
                    cel = field_find_object_by_definition(rec->unk10);
                    tint_src = (FieldTintSrc *) cel;
                    anim->cels = cel;
                    field_apply_animation_tween(def, anim, 0);
                    break;
                default:
                    grid = (FieldTileGrid *) rec->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    anim->unk10 = (s32) tint_src;
                    key = (FieldSfxKey *) rec->data;
                    if (((key->kind & 7) == 1) && (key->sound_flags & 0x8000))
                    {
                        anim->timer = 1;
                        anim->flags.word |= 8;
                    }
                    break;
                }
                break;
            case 1:
                switch (def->flags & 7)
                {
                case 0:
                    grid = (FieldTileGrid *) def->data;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    break;
                case 1:
                    cel = field_find_object_by_definition(def->data);
                    tint_src = (FieldTintSrc *) cel;
                    anim->cels = cel;
                    break;
                }
                break;
            case 2:
                switch (def->flags & 7)
                {
                case 0:
                    grid = (FieldTileGrid *) def->unk10;
                    cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                    anim->cels = cel;
                    anim->unk10 = (s32) tint_src;
                    break;
                case 1:
                    cel = field_find_object_by_definition(def->unk10);
                    tint_src = (FieldTintSrc *) cel;
                    anim->cels = cel;
                    break;
                }
                break;
            default:
                grid = (FieldTileGrid *) def->unk10;
                cel = (FieldAnimCel *) func_8005ABD8(grid, &tint_src);
                anim->cels = cel;
                break;
            }
            if (((u32) (*(u32 *) &def->flags & 0xFF000007) < 2) || (def->handler_group == 3))
            {
                rgb[0] = tint_src->red << 8;
                rgb[1] = tint_src->green << 8;
                rgb[2] = tint_src->blue << 8;
                palette_data = tint_src->palette->data;
                func_8005AC50(palette_data + 2, palette_data[0], rgb);
                primitive_code = 0;
                func_8005AD20(cel->format, tint_src->palette->data[0], &primitive_code);
                anim->frame_data = *arena;
                arena_cursor = *arena;
                switch (cel->format)
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
                frame_descs = def->data;
                rec = def;
                if ((*(u32 *) &def->flags & 0xFF000007) == 1)
                {
                    anim->unk10 = (s32) cel->tiles;
                    frame = def->unk6 - 1;
                    if (frame != -1)
                    {
                        frame -= 1;
                        do
                        {
                            frame -= 1;
                        }
                        while (frame != -1);
                    }
                }
                else
                {
                    frame = def->unk6 - 1;
                    if (frame != -1)
                    {
                        do
                        {
                            tile_data = frame_descs;
                            mask_bit = 1;
                            row = 0;
                            tile_count = 0;
                            mask = cel->mask;
                            mask_word = *mask++;
                            if (grid->u.b.rows != 0)
                            {
                                do
                                {
                                    if (row < rec->unkD)
                                    {
                                        col = grid->u.b.cols - 1;
                                        if (col != -1)
                                        {
                                            do
                                            {
                                                mask_bit *= 2;
                                                if (mask_bit == 0)
                                                {
                                                    mask_word = *mask++;
                                                    mask_bit = 1;
                                                }
                                                col -= 1;
                                            }
                                            while (col != -1);
                                        }
                                    }
                                    else if (row < rec->unkD + rec->unkF)
                                    {
                                        col = 0;
                                        if (grid->u.b.cols != 0)
                                        {
                                            do
                                            {
                                                if ((col >= rec->unkC) && (col < rec->unkC + rec->unkE))
                                                {
                                                    if (mask_word & mask_bit)
                                                    {
                                                        switch (cel->format)
                                                        {
                                                        case 0:
                                                            tile_record = arena_cursor;
                                                            arena_cursor += record_stride;
                                                            field_build_sprite_tile_record(tile_data, tile_record,
                                                                                           (grid->u.word >> 4) & 3, record_flags);
                                                            break;
                                                        case 2:
                                                        case 3:
                                                        case 4:
                                                        case 5:
                                                            tile_record = arena_cursor;
                                                            arena_cursor += record_stride;
                                                            field_build_quad_tile_record(tile_data, tile_record,
                                                                                         (grid->u.word >> 4) & 3, record_flags);
                                                            break;
                                                        case 1:
                                                        case 6:
                                                            break;
                                                        }
                                                        tile_count += 1;
                                                    }
                                                    tile_data += 1;
                                                }
                                                mask_bit *= 2;
                                                if (mask_bit == 0)
                                                {
                                                    mask_word = *mask++;
                                                    mask_bit = 1;
                                                }
                                                col += 1;
                                            }
                                            while (col != grid->u.b.cols);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    row += 1;
                                }
                                while (row != grid->u.b.rows);
                            }
                            frame -= 1;
                            frame_descs += rec->unkE * rec->unkF;
                        }
                        while (frame != -1);
                        anim->frame_tile_count = tile_count;
                        if (def->handler_group == 3)
                        {
                            if (*(u32 *) &def->flags & 0x20)
                            {
                                field_blit_animation_frame(def, anim, 0);
                            }
                        }
                        else if (anim->flags.word & 0x40)
                        {
                            field_blit_animation_frame(def, anim, 0);
                        }
                    }
                }
                *arena = arena_cursor;
            }
            handler_kind = *(u32 *) &def->flags & 0xFF000007;
            if (((u32) (handler_kind - 3) < 2) ||
                ((def->handler_group == 1) && ((u32) (def->flags & 7) >= 2)))
            {
                if (handler_kind == 0x01000002)
                {
                    if (def->unkC == 0)
                    {
                        *arena += 0x50;
                    }
                    else
                    {
                        *arena += 0x410;
                    }
                }
                else if (handler_kind == 0x01000005)
                {
                    if (def->unkC == 0)
                    {
                        *arena += (*(u8 *) &def->unk10 << 6) + 0x10;
                    }
                    else
                    {
                        *arena += (*(u8 *) &def->unk10 << 10) + 0x10;
                    }
                }
                else
                {
                    *arena += 0x10;
                }
            }
            def = def->next;
        }
        while (def != NULL);
    }
    *tail = NULL;
}
