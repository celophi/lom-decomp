#ifndef FIELD_SCENE_INTERNAL_H
#define FIELD_SCENE_INTERNAL_H

#include "common.h"
#include "field_animation.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @brief Halve a signed value, rounding toward zero.
 *
 * Written as a conditional; gcc 2.8 expands a plain `/ 2` without the branch
 * the original code has.
 *
 * @param v Signed value to halve.
 * @return @p v divided by two, rounded toward zero.
 */
#define HALF_TOWARD_ZERO(v) ((v) >= 0 ? ((v) >> 1) : (((v) + 1) >> 1))

/**
 * @brief Arithmetic right shift that rounds toward zero instead of down.
 *
 * The generalisation of HALF_TOWARD_ZERO. The argument is expanded in each
 * arm, so it should be free of side effects.
 *
 * @param v Signed value to shift.
 * @param n Shift amount, i.e. divide by 1 << n.
 * @return @p v divided by `1 << n`, rounded toward zero.
 */
#define SHIFT_TOWARD_ZERO(v, n) ((v) >= 0 ? ((v) >> (n)) : (((v) + ((1 << (n)) - 1)) >> (n)))

/*
 * Packed tile descriptor and field texture-atlas constants.
 *
 * The field textures occupy two VRAM banks. Logical page slots 0-9 begin at
 * (320, 256); slots 10-15 continue in the bank beginning at (512, 0).
 * TPage X coordinates are stored in units of 64 VRAM halfwords.
 */
#define FIELD_TILE_PRESENT 0x80
#define FIELD_TILE_CLUT_MASK 0x1F
#define FIELD_TILE_CLUT_X_SLOT_MASK 0xF
#define FIELD_TILE_TPAGE_SLOT_MASK 0xF
#define FIELD_TILE_U_MASK 0xF
#define FIELD_TILE_V_MASK 0xF0
#define FIELD_TILE_SIZE 0x10
#define FIELD_TILE_SEMITRANS 0x40
#define FIELD_TILE_LOWER_BANK_SLOTS 0xA
#define FIELD_TILE_UPPER_BANK_VRAM_X 0x200
#define FIELD_TILE_UPPER_BANK_VRAM_X_BIAS 0x80
#define FIELD_TILE_LOWER_BANK_VRAM_X 0x140
#define FIELD_TILE_LOWER_BANK_VRAM_Y 0x100
#define FIELD_TILE_CLUT_VRAM_Y 0x1D8
#define FIELD_TILE_TPAGE_X_SHIFT 6
#define FIELD_TILE_4BIT_U_PAGE_SHIFT 4
#define FIELD_TILE_UPPER_BANK_U_CELLS_PER_SLOT 4
#define FIELD_TEXTURE_4_BIT 0
#define FIELD_TEXTURE_8_BIT 1

/*
 * A 4bpp CLUT contains 16 colors, so its low four reference bits select a
 * 16-pixel X slot. Reference bit 4 selects VRAM row 472 or 473. An 8bpp CLUT
 * consumes a complete 256-color row, so all five bits select its Y offset.
 *
 * Keep these as macros: their expansion preserves the matched mask/shift order.
 */
#define FIELD_TILE_4BIT_CLUT_X(ref) (((ref) & FIELD_TILE_CLUT_X_SLOT_MASK) << 4)
#define FIELD_TILE_4BIT_CLUT_Y(ref) (FIELD_TILE_CLUT_VRAM_Y + (((ref) & FIELD_TILE_CLUT_MASK) >> 4))
#define FIELD_TILE_8BIT_CLUT_Y(ref) (FIELD_TILE_CLUT_VRAM_Y + ((ref) & FIELD_TILE_CLUT_MASK))

/*
 * Texture-atlas address decoding. TPage X is measured in 64-halfword columns.
 * In the sprite format, U is stored in 16-pixel cells; crossing to the upper
 * bank rebases those cells onto the common TPage beginning at (512, 0).
 */
#define FIELD_TILE_ABR(attrs) ((attrs) >> 4)
#define FIELD_TILE_LOWER_BANK_PAGE_X(slot) (FIELD_TILE_LOWER_BANK_VRAM_X + ((slot) << FIELD_TILE_TPAGE_X_SHIFT))
#define FIELD_TILE_UPPER_BANK_PAGE_X(slot) ((s32)(((slot) << FIELD_TILE_TPAGE_X_SHIFT) - FIELD_TILE_UPPER_BANK_VRAM_X_BIAS))
#define FIELD_TILE_TPAGE_COLUMN(slot, u_cell, depth) ((slot) + ((u_cell) >> (FIELD_TILE_4BIT_U_PAGE_SHIFT - (depth))))
#define FIELD_TILE_UPPER_BANK_U_REBASE(slot) ((FIELD_TILE_LOWER_BANK_SLOTS - (slot)) * FIELD_TILE_UPPER_BANK_U_CELLS_PER_SLOT)

/* Per-record fields which may instead be supplied once by the owning cel. */
#define FIELD_TILE_REC_SHARED_RGB_CODE 1
#define FIELD_TILE_REC_SHARED_TPAGE 2

/* Scratchpad table containing complete RGB/primitive-code words. */
#define FIELD_TILE_COLOR_WORDS ((s32*)0x1F800000)

/**
 * @brief Packed 4-byte per-tile source descriptor consumed by field_build_sprite_tile_record.
 */
typedef struct
{
    /**
     * Bit 7 = tile present. Bits 0-4 encode the CLUT location: 4bpp splits
     * them into X/16 and Y+0/1; 8bpp uses them as the CLUT Y offset.
     */
    u8 clut_slot;
    /**
     * Bits 0-3 = logical atlas page slot; bits 4-5 = TPage ABR blend mode;
     * bit 6 enables semitransparency in the primitive code byte.
     */
    u8 texture_attrs;
    /** Low nibble = U/16; high nibble = V, already aligned to 16 pixels. */
    u8 packed_uv;
    /** Index into the scratchpad RGB/primitive-code table at 0x1F800000. */
    u8 color_index;
} FieldTileDesc;

/**
 * @brief Overlapping view of FieldObj's word at 0x0C.
 *
 * The word is tested as a whole (bit 0 = object active) while bytes 0x0E and
 * 0x0F are read separately as a magnitude and an angle, so the two views have
 * to share storage.
 */
typedef union
{
    s32 word;
    struct
    {
        u8 unk0;
        /** 0x0D number of attached FieldNode instances, the object-level
            counterpart of FieldPart::node_count. */
        u8 node_count;
        /** 0x0E drift magnitude; zero disables the per-frame drift. */
        u8 drift_speed;
        /** 0x0F drift angle, scaled by 0x10 before rcos/rsin. */
        u8 drift_angle;
    } b;
} FieldObjFlags;

typedef struct FieldPartDef FieldPartDef;
typedef struct FieldNodeDef FieldNodeDef;
typedef struct FieldMarkerDef FieldMarkerDef;
typedef struct FieldAnimDef FieldAnimDef;

/**
 * @brief Per-object definition record.
 */
typedef struct
{
    /** 0x00 null-terminated list of the object's part definitions. */
    FieldPartDef** part_defs;
    /** 0x04 shared-source handle; two defs with the same one are compatible. */
    s32 shared_source;
    u8 _pad1[0xC - 8];
    /**
     * 0x0C bit 0 starts the object visible, bit 1 fixes it to the screen, bit 2
     * and bits 4-5 select the horizontal wrap, bit 3 and bits 6-7 the vertical
     * one. Unsigned (the wrap shifts use srl); the build reads bit 0 through
     * the low byte.
     */
    union
    {
        u32 word;
        u8 low;
    } flags;
    /** 0x10 x/y/z scale in percent, copied to the object as 8.8 fixed point. */
    u16 scale_x;
    u16 scale_y; /* 0x12 */
    u16 scale_z; /* 0x14 */
    s16 x;       /* 0x16 x offset */
    s16 y;       /* 0x18 y offset */
    s16 z;       /* 0x1A z offset; also biases depth CLUTs of the parts */
    /** 0x1C scroll factors and initial drift; the build tests the drift pair as one word. */
    union
    {
        u32 word;
        struct
        {
            /** Horizontal scroll factor; 0x10 means "unscaled", bit 7 negates. */
            u8 scroll_scale_x;
            /** Vertical scroll factor, same encoding. */
            u8 scroll_scale_y;
            /** Initial FieldObjFlags drift magnitude. */
            u8 drift_speed;
            /** Initial FieldObjFlags drift angle. */
            u8 drift_angle;
        } b;
    } motion;
} FieldObjDef;

/** FieldObjDef::flags bit: the object ignores the camera scroll. */
#define FIELD_OBJ_DEF_SCREEN_FIXED 2

/**
 * @brief Definition record hanging off a part: the part's tile grid.
 *
 * The word at 0x08 is read whole for its mode bits while bytes 0x0A and 0x0B
 * are read separately as the cell grid dimensions, so the two views have to
 * share storage - same arrangement as FieldObjFlags below. The animation code
 * reaches the same record through FieldAnimDef::u (tile or tint view).
 */
struct FieldPartDef
{
    /**
     * 0x00 packed tile descriptors, one 4-byte entry per grid cell; also the
     * identity key field_find_shareable_part matches parts on.
     */
    FieldTileDesc* tiles;
    u8 _pad0[8 - 4];
    union
    {
        /** 0x08 whole word; bit 0 is the initial visibility, bits 4-5 select
            the CLUT packing mode, bit 7 marks the part unshareable, bits 8-11
            equal to 1 mean a single cell, bits 12-15 select
            field_emit_rotated_sprite_grid's placement mode. */
        u32 word;
        struct
        {
            /** 0x08 low byte of the word, read alone for the visibility bit. */
            u8 flags;
            u8 _pad1;
            /** 0x0A grid width, in cells. */
            u8 cols;
            /** 0x0B grid height, in cells. */
            u8 rows;
        } b;
    } u;
    s16 x; /* 0x0C x offset within the object */
    s16 y; /* 0x0E y offset within the object */
    s16 z; /* 0x10 z offset within the object; also biases depth CLUTs */
    /** 0x12 initial FieldPart::sweep_period. */
    u16 sweep_period;
    u8 _pad2[0x18 - 0x14];
    /** 0x18..0x1E corner CLUT ids, copied to FieldPart::clut_bl..clut_tr;
        offsets from the object and part depth when FIELD_PART_DEF_DEPTH_CLUT
        is set. */
    s16 clut_bl;
    s16 clut_tl;
    s16 clut_br;
    s16 clut_tr;
};

/* FieldPartDef::u.word bits. */
#define FIELD_PART_DEF_VISIBLE 0x1
/** Bits 1-3: the even FieldPart::kind; a single-cell part adds one. */
#define FIELD_PART_DEF_KIND_MASK 0xE
/** Bits 4-5: texture depth of the part's tiles (FIELD_TEXTURE_4_BIT, ...). */
#define FIELD_PART_DEF_DEPTH(word) (((word) >> 4) & 3)
#define FIELD_PART_DEF_DEPTH_CLUT 0x40
#define FIELD_PART_DEF_UNSHARED 0x80
/** Bits 8-11 equal to FIELD_PART_DEF_SINGLE_CELL: the part is one cell. */
#define FIELD_PART_DEF_CELLS_MASK 0xF00
#define FIELD_PART_DEF_SINGLE_CELL 0x100

/*
 * Sweep mode in bits 12-15 of a part definition word (0 = none, 1-4 = mode).
 * It also selects the pivot of a rotated part: 1-2 the scene centre, 3 its
 * left edge, 4 its right edge, anything else the part's own centre.
 */
#define FIELD_PART_SWEEP_MASK 0xF000
#define FIELD_PART_SWEEP_MODE(def) (((def)->u.word >> 12) & 0xF)
#define FIELD_PART_SWEEP_MODE_COUNT 5

/**
 * @brief Element of an object's part list.
 */
typedef struct FieldPart FieldPart;
struct FieldPart
{
    FieldPart* next;   /* 0x00 */
    FieldPartDef* def; /* 0x04 */
    /**
     * 0x08 part whose bit plane and records this one reuses, or NULL when it
     * owns them (field_set_color_scale then rebuilds its tint records).
     */
    FieldPart* shared;
    /** 0x0C bit plane: one bit per grid cell, row-major, LSB first. */
    u32* bits;
    /** 0x10 packed stream of FieldCellRec, one per set bit. */
    u8* records;
    /** 0x14 byte size of the bit plane at 0x0C. */
    s32 bits_size;
    /**
     * 0x18 texture-page word shared by every cell; when non-zero it is emitted
     * once as its own primitive instead of per record, shortening the stride.
     */
    s32 tpage_word;
    /** 0x1C rgb/code word shared by every cell; same stride effect as tpage. */
    s32 code_word;
    /** 0x20 zero means the part is not drawn (for an animation cel: not the current one). */
    u8 visible;
    /**
     * 0x21 record format, 0-6: selects the per-part byte cost (0x18, 0x1C,
     * 0x28 or 0x34 units) and the tile record layout (see field_blit_animation_frame).
     */
    u8 kind;
    /** 0x22 number of attached FieldNode instances field_update_part_sweep updates. */
    u8 node_count;
    u8 _pad2[0x26 - 0x23];
    /** 0x26 number of instances; zero means the part is skipped entirely. */
    u16 instance_count;
    s32 x; /* 0x28 x offset within the object */
    s32 y; /* 0x2C y offset within the object */
    s32 z; /* 0x30 z offset within the object */
    s16 unk34; /* 0x34 */
    /** 0x36 reload period for the sweep phase at 0x38. */
    u16 sweep_period;
    /** 0x38 sweep phase; counts down each frame, reloads from 0x36 at zero. */
    u16 sweep_phase;
    /** 0x3A rotation angle applied to the vertical (row) step. */
    u16 row_angle;
    /** 0x3C rotation angle applied to the horizontal (column) step. */
    u16 column_angle;
    /** 0x3E rotation angle of the grid as a whole; feeds both rsin and rcos. */
    u16 rotation_angle;
    /** 0x40 horizontal scale, 4.12 fixed point (ONE = unscaled). */
    u16 scale_x;
    /** 0x42 vertical scale, 4.12 fixed point. */
    u16 scale_y;
    /**
     * 0x44..0x4A the four corner CLUT ids, bilinearly interpolated across the
     * grid. Derived from the interpolation endpoints: the row weight resolves
     * to unk46 on the first (topmost) row and unk44 on the last, and the
     * column weight to the "left" pair on the first (leftmost) column.
     */
    s16 clut_bl; /* 0x44 bottom left */
    s16 clut_tl; /* 0x46 top left */
    s16 clut_br; /* 0x48 bottom right */
    s16 clut_tr; /* 0x4A top right */
};

/**
 * @brief Element of the scene's object list.
 */
typedef struct FieldObj FieldObj;
struct FieldObj
{
    FieldObj* next;      /* 0x00 */
    FieldObjDef* def;    /* 0x04 */
    FieldPart* parts;    /* 0x08 head of the part list */
    FieldObjFlags flags; /* 0x0C */
    /**
     * 0x10 tint multipliers, 8.8 fixed point, from the definition's
     * percentages; field_find_shareable_part compares red and green as one word.
     */
    union
    {
        struct
        {
            u16 red;
            u16 green;
        } c;
        s32 word;
    } red_green;
    u16 blue;
    /** 0x16 second tint multiplier per channel, 0x100 = unscaled. */
    u16 red_scale;
    u16 green_scale;
    u16 blue_scale;
    s32 x;       /* 0x1C x offset */
    s32 y;       /* 0x20 y offset */
    s32 z;       /* 0x24 z offset */
    s32 drift_x; /* 0x28 accumulated x drift */
    s32 drift_y; /* 0x2C accumulated y drift */
};

/**
 * @brief Singly-linked record hanging off FieldSceneHeader::records.
 *
 * @note Only the link is identified. field_header_record_at hands the caller @c body,
 *       which is read there as four halfwords.
 */
typedef struct FieldHeaderRec FieldHeaderRec;
struct FieldHeaderRec
{
    FieldHeaderRec* next; /* 0x00 */
    s32 body;             /* 0x04 first word of the record proper */
};

/**
 * @brief Header hanging off FieldScene at offset 0.
 *
 * @note field_draw_scene_objects reads only the 0x30 halfword; the streaming
 *       update in field_update_scene_animations also reads the pixel-source base at 0x04 and the
 *       column stride at 0x28.
 */
typedef struct
{
    u8 _pad0[4];
    u16* pixel_data; /* 0x04 strip pixel-source base */
    u8 _pad1[0x10 - 8];
    /** 0x10 head of the record list indexed by field_header_record_at; null when the
        scene carries no records. */
    FieldHeaderRec* records;
    u8 _pad2[0x28 - 0x14];
    u16 pixel_stride; /* 0x28 source stride, in halfwords */
    u8 _pad3[0x2C - 0x2A];
    /** 0x2C scene flags; see FIELD_SCENE_HEADER_BOUNDED. */
    s32 flags;
    s16 unk30; /* 0x30 */
    /** 0x32 counterpart of unk30; field_collision_collect_groups uses the pair as the scene's
        pixel extent when sizing its tile budget. */
    s16 unk32;
} FieldSceneHeader;

/** FieldSceneHeader::flags bit: movers are kept inside the unk30 x unk32 extent. */
#define FIELD_SCENE_HEADER_BOUNDED 0x2

/**
 * @brief Per-marker record hanging off FieldMarker::def.
 *
 * unk4/unk6 and unk8/unkA are two screen-space point pairs; unk10 is a depth
 * bias folded into the marker's vertical origin and unk14 the numeric label
 * drawn next to it.
 */
struct FieldMarkerDef
{
    FieldMarkerDef* next; /* 0x00 next definition in the scene resource */
    /** 0x04 first point, horizontal. */
    u16 x0;
    /** 0x06 first point, vertical (halved before use). */
    u16 y0;
    /** 0x08 second point, horizontal. */
    u16 x1;
    /** 0x0A second point, vertical (halved before use). */
    u16 y1;
    /** 0x0C horizontal offset from the first edge to the opposite one. */
    u16 offset_x;
    /** 0x0E vertical offset from the first edge to the opposite one. */
    u16 offset_y;
    /** 0x10 depth bias added to the fixed 0xE0 vertical origin; also the
        bottom of the vertical band the marker blocks for collision. */
    s16 depth_bias;
    /** 0x12 height of the collision band; 1 means the band has no top. */
    s16 band_height;
    /** 0x14 value rendered as the marker's numeric label; also the value
        field_collision_hit_markers returns on a hit. */
    u16 label;
};

/**
 * @brief Element of the scene's marker list (FieldScene offset 0x10).
 *
 * @note Only drawn when g_field_marker_overlay_enabled is set, so this is most likely a debug
 *       overlay rather than something the retail render path shows.
 */
typedef struct FieldMarker FieldMarker;
struct FieldMarker
{
    FieldMarker* next;   /* 0x00 */
    FieldMarkerDef* def; /* 0x04 */
    /** 0x08 third point, horizontal. */
    u16 x2;
    /** 0x0A third point, vertical (halved before use). */
    u16 y2;
    /** 0x0C fourth point, horizontal. */
    u16 x3;
    /** 0x0E fourth point, vertical (halved before use). */
    u16 y3;
    s16 x_max; /* 0x10 bounding box of the four points */
    s16 x_min; /* 0x12 */
    s16 y_max; /* 0x14 */
    s16 y_min; /* 0x16 */
    /** 0x18 side vector, from the first point to the third. */
    s32 side_dx;
    s32 side_dy; /* 0x1C */
    /** 0x20 edge vector, from the first point to the second. */
    s32 edge_dx;
    s32 edge_dy; /* 0x24 */
    /** 0x28 lower and upper intercept of the two edges running along the
        side vector (x when the side is vertical). */
    s32 side_lo;
    s32 side_hi; /* 0x2C */
    /** 0x30 lower and upper intercept of the two edges running along the
        edge vector (x when the edge is vertical). */
    s32 edge_lo;
    s32 edge_hi; /* 0x34 */
};

/**
 * @brief Handler word at FieldAnimDef 0x04.
 *
 * The word is tested whole (the handler kind and the list group in one
 * compare, e.g. `& 0xFF000007`) while its bytes are also read and written on
 * their own, so the two views share storage.
 */
typedef union
{
    u32 word;
    struct
    {
        /** 0x04 low three bits select the handler; bits 3-7 are FIELD_ANIM_DEF_* flags. */
        u8 kind_flags;
        /** 0x05 last keyframe index; the cel ring wraps after it. */
        u8 last_frame;
        /** 0x06 number of frames in the definition's frame data. */
        u8 frame_count;
        /** 0x07 scene animation list (0-3) the definition belongs to. */
        u8 handler_group;
    } b;
} FieldAnimDefFlags;

/**
 * @brief Definition record shared by the animation and sequence lists.
 *
 * Bytes 0x0C..0x13 are handler-specific; FieldAnimDef::flags (the list group
 * and the handler kind) decides which view of @c u applies.
 */
struct FieldAnimDef
{
    /**
     * 0x00 first count-table record (see field_find_count_table_span, which
     * walks the definition from here). field_build_animation_list tests the
     * span count through the whole word.
     */
    union
    {
        u32 word;
        struct
        {
            /** 0x00 low seven bits: keyframes covered by the first span. */
            u8 span_count;
            /** 0x01 starting frame; the movie handler (list 0 kind 4) reads it as a file pair index. */
            u8 unk1;
            /** 0x02 duration of the first keyframe span, in frames. */
            u16 duration;
        } b;
    } head;
    FieldAnimDefFlags flags; /* 0x04 */
    /** 0x08 next definition in the same scene list. */
    FieldAnimDef* next;
    union
    {
        /** Tile and image handlers (lists 0 and 3). */
        struct
        {
            u8 rect_x;      /* 0x0C left edge of the source rectangle */
            u8 rect_y;      /* 0x0D top edge of the source rectangle */
            u8 rect_width;  /* 0x0E rectangle width */
            u8 rect_height; /* 0x0F rectangle height */
            /** 0x10 tile grid whose runtime cel list the handler drives. */
            FieldPartDef* grid;
        } tile;
        /** Palette handlers (list 1). */
        struct
        {
            /**
             * 0x0C CLUT packing mode, matched against FieldPartDef word bits 4-5:
             * zero packs 16-entry CLUTs 16 to a row, non-zero uses 256-entry rows.
             */
            u8 clut_mode;
            /** 0x0D non-zero reverses the strip rotation direction (kind 2). */
            u8 reverse;
            /** 0x0E CLUT slot: row and column nibbles for 16-entry CLUTs, the row otherwise. */
            u8 clut_slot;
            /** 0x0F entry offset of the first uploaded colour within the row. */
            u8 clut_offset;
            /** 0x10 CLUTs (or colour entries) uploaded per frame. */
            u8 length;
            u8 _pad0;
            /** 0x12 halfword offset of the frame colours past the header pixel stride. */
            u16 pixel_offset;
        } clut;
        /** Tint handlers (list 2). */
        struct
        {
            u8 first_slot; /* 0x0C first scratchpad colour slot rewritten */
            u8 slot_count; /* 0x0D number of colour slots rewritten */
            u8 _pad1[2];
            /** 0x10 tile grid whose runtime cel list is tinted (kind 0). */
            FieldPartDef* grid;
        } tint;
    } u;
    u8* data; /* 0x14 handler-specific data */
};

/**
 * @brief One 4-byte entry of the scratchpad colour table at 0x1F800000.
 *
 * field_build_sprite_tile_record copies the whole entry into a tile record's rgb/code word;
 * field_tint_animation_cel rewrites only the colour bytes, so it needs the halves named.
 */
typedef struct
{
    /** 0x00 red and green, the low half of a GPU rgb/code word. */
    u16 rg;
    /** 0x02 blue. */
    u8 b;
    /** 0x03 primitive code. */
    u8 code;
} FieldTintColor;

#define FIELD_TINT_COLORS ((FieldTintColor*)FIELD_TILE_COLOR_WORDS)

/**
 * @brief Colour bytes embedded in a rendered cell record.
 *
 * The record prefix occupies the first four bytes; tinting updates the
 * following red/green halfword and blue byte in place.
 */
typedef struct
{
    u8 _pad0[4];
    u16 rg;
    u8 b;
} FieldCellTint;

/** @brief Palette record reached through FieldTintSrc::palette. */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 count halfword followed by the palette entries themselves. */
    u16* data;
} FieldTintPal;

/**
 * @brief Colour source for the tile tint pass, hung off FieldAnim::owner.
 *
 * The two halfword triples multiply component-wise into the three-word colour
 * field_build_tint_colors expands into the scratchpad table at 0x1F800000.
 *
 * @note field_tint_animation_cel_list reaches this record through FieldAnim::cels instead, and
 *       walks the cel list at 0x08 rather than being handed a single cel.
 * @note The scene's object list is a list of these: field_set_color_scale walks it
 *       through @c next and treats the list at 0x08 as the object's parts.
 */
typedef struct FieldTintSrc FieldTintSrc;
struct FieldTintSrc
{
    /** 0x00 next tint source when this record is an element of the object list. */
    FieldTintSrc* next;
    /** 0x04 record holding the palette this tint is built from. */
    FieldTintPal* palette;
    /** 0x08 head of the cel list this source tints (field_tint_animation_cel_list only). */
    FieldPart* cels;
    u8 _pad1[0x10 - 0xC];
    u16 red;         /* 0x10 */
    u16 green;       /* 0x12 */
    u16 blue;        /* 0x14 */
    u16 red_scale;   /* 0x16 */
    u16 green_scale; /* 0x18 */
    u16 blue_scale;  /* 0x1A */
};

/** Handler kinds of the tile animation list (FieldScene::anims). */
enum
{
    FIELD_TILE_ANIM_BLIT = 0,         /**< copy a frame into the cel's tile records */
    FIELD_TILE_ANIM_CEL_RECORDS = 1,  /**< frames use the cel's own tile records; none are built */
    FIELD_TILE_ANIM_CEL_CYCLE = 2,    /**< show the cel of the current frame */
    FIELD_TILE_ANIM_UPLOAD = 3,       /**< upload the frame's pixels to the tile rect */
    FIELD_TILE_ANIM_MOVIE = 4,        /**< stream a movie into the tile rect */
    FIELD_TILE_ANIM_TWEEN_PART = 5,   /**< move a part along the keyframe offsets */
    FIELD_TILE_ANIM_TWEEN_OBJECT = 6, /**< move an object along the keyframe offsets */
    FIELD_TILE_ANIM_SOUND = 7         /**< play the keyframe's sound */
};

/** Handler kinds of the palette animation list (FieldScene::strips). */
enum
{
    FIELD_PALETTE_ANIM_CEL_CLUT = 0,      /**< point a cel's tiles at the frame's CLUTs */
    FIELD_PALETTE_ANIM_CEL_LIST_CLUT = 1, /**< the same for every cel of a tint source */
    FIELD_PALETTE_ANIM_CYCLE = 2,         /**< rotate the colours of one CLUT row */
    FIELD_PALETTE_ANIM_CLUT_ROW = 3,      /**< upload the frame's colours into a CLUT row */
    FIELD_PALETTE_ANIM_CLUT_BLOCK = 4,    /**< upload the frame's colours into a block of CLUTs */
    FIELD_PALETTE_ANIM_BLEND = 5          /**< upload a blend of two frames */
};

/** Handler kinds of the tint animation list (FieldScene::sprites). */
enum
{
    FIELD_TINT_ANIM_CEL = 0,     /**< tint one cel */
    FIELD_TINT_ANIM_CEL_LIST = 1 /**< tint every cel of a tint source */
};

/** Handler kind of a definition (low three bits of its flag word). */
#define FIELD_ANIM_KIND(def) ((def)->flags.word & FIELD_ANIM_KIND_MASK)
/** True for the two tween kinds of the tile list. */
#define FIELD_ANIM_IS_TWEEN(def) ((FIELD_ANIM_KIND(def) >= FIELD_TILE_ANIM_TWEEN_PART) && (FIELD_ANIM_KIND(def) <= FIELD_TILE_ANIM_TWEEN_OBJECT))

/** FieldAnimDef::flags.b.handler_group: which scene list a definition is on. */
#define FIELD_ANIM_GROUP_TILE 0    /**< FieldScene::anims */
#define FIELD_ANIM_GROUP_PALETTE 1 /**< FieldScene::strips */
#define FIELD_ANIM_GROUP_TINT 2    /**< FieldScene::sprites */
#define FIELD_ANIM_GROUP_EFFECT 3  /**< FieldScene::effects: every frame is blitted */
/** Masks the handler group byte and the handler kind in FieldAnimDef::flags.word. */
#define FIELD_ANIM_GROUP_KIND_MASK 0xFF000007
/** FieldAnimDef::flags.word value (under FIELD_ANIM_GROUP_KIND_MASK) of one group and kind. */
#define FIELD_ANIM_GROUP_KIND(group, kind) (((group) << 24) | (kind))
/** FieldAnimDef::flags.word value of a palette-list blend definition. */
#define FIELD_ANIM_PALETTE_BLEND_WORD FIELD_ANIM_GROUP_KIND(FIELD_ANIM_GROUP_PALETTE, FIELD_PALETTE_ANIM_BLEND)

/** Bytes of a cel tile record; each word the cel shares (code, tpage) is left out. */
#define FIELD_CEL_RECORD_SIZE 12
#define FIELD_CEL_SHARED_WORD_SIZE 4
/** FieldTileDesc::clut_slot bit marking a tile the palette and tint handlers rewrite. */
#define FIELD_TILE_ANIMATED 0x80

/** Sound keyframe (FieldSfxKey) fields. */
#define FIELD_SFX_KEY_KIND_MASK 7
#define FIELD_SFX_KEY_SOUND 1
#define FIELD_SFX_CHANNEL_MASK 0x1F00 /* channel slot, 1-31 */
#define FIELD_SFX_FIXED_PAN 0x4000
#define FIELD_SFX_PLAY 0x8000
#define FIELD_SFX_ID_MASK 0x3FF
#define FIELD_SFX_ONE_SHOT 0x8000
#define FIELD_SFX_VOLUME(key) (((key)->sound.word >> 8) & 0x7F)

/** Axis argument of field_move_part_nodes / field_move_object_nodes. */
#define FIELD_AXIS_X 0
#define FIELD_AXIS_Y 1
#define FIELD_AXIS_Z 2

/** FieldObj::flags.word bit holding the object's visibility. */
#define FIELD_OBJ_VISIBLE 1

/** FieldSeq::flags bits 0-1: the sequence phase. */
#define FIELD_SEQ_PHASE_MASK 3
#define FIELD_SEQ_PHASE_RUNNING 1
#define FIELD_SEQ_PHASE_FINISHED 2
/** FieldSeqDef link value for "no sequence". */
#define FIELD_SEQ_NO_LINK 0xFF
/** field_get_animation_state results: running, running until its stop keyframe, stopped, and a movie still on its first two frames. */
#define FIELD_ANIM_STATE_RUNNING 0
#define FIELD_ANIM_STATE_STOPPING 1
#define FIELD_ANIM_STATE_FINISHED 2
#define FIELD_ANIM_STATE_MOVIE_STARTING 3

/**
 * @brief Animation node flags at 0x24.
 *
 * The word is tested and rewritten as a whole while byte 0x25 is read and
 * written separately as the node's frame index, so the two views share storage.
 */
#define FIELD_ANIM_FLAG_PING_PONG 0x01
#define FIELD_ANIM_FLAG_STOP_AT_KEYFRAME 0x02
#define FIELD_ANIM_FLAG_REVERSE 0x04
#define FIELD_ANIM_FLAG_START_PENDING 0x08
#define FIELD_ANIM_FLAG_SECOND_BUFFER 0x10
#define FIELD_ANIM_FLAG_UPLOAD_PENDING 0x20
#define FIELD_ANIM_FLAG_ACTIVE 0x40

#define FIELD_ANIM_KIND_MASK 0x07
/* FieldAnimDef::flags.b.kind_flags bits above the handler kind. */
/** Copied to FIELD_ANIM_FLAG_PING_PONG. */
#define FIELD_ANIM_DEF_PING_PONG 0x08
#define FIELD_ANIM_DEF_IGNORE_REPEAT_COUNT 0x10
/** Start on the first keyframe for its whole span rather than a staggered short one. */
#define FIELD_ANIM_DEF_TIMED 0x20
#define FIELD_ANIM_DEF_SPAN_INDEXED 0x40
/** The node starts active (FIELD_ANIM_FLAG_ACTIVE) when its first span has keyframes. */
#define FIELD_ANIM_DEF_ACTIVE 0x80

/** Colours in one 4 bpp CLUT; one 8 bpp CLUT fills a whole 256-colour row. */
#define FIELD_CLUT_4BIT_COLORS 16
#define FIELD_CLUT_8BIT_COLORS 256

typedef union
{
    s32 word;
    struct
    {
        u8 control;
        /** 0x25 current frame / cel index. */
        u8 state;
        u8 keyframe;
        u8 stop_keyframe;
    } b;
} FieldAnimFlags;

/** @brief Element of the scene's animation lists (0x18/0x1C/0x20/0x24). */
typedef struct FieldAnim FieldAnim;
struct FieldAnim
{
    FieldAnim* next;   /* 0x00 */
    FieldAnimDef* def; /* 0x04 */
    u8 _pad0[0xC - 8];
    FieldPart* cels; /* 0x0C */
    /**
     * 0x10 owner of the cel list: the object (read as its tint source by the
     * tint handlers) that field_find_grid_part found the grid in; list 0 kind 1
     * stores the first cel's tile records here instead.
     */
    union
    {
        FieldTintSrc* tint_src;
        FieldObj* object;
        u8* tiles;
    } owner;
    /** 0x14 last horizontal tween offset pushed to the target (see field_apply_animation_tween). */
    s32 tween_x;
    /** 0x18 last vertical tween offset pushed to the target. */
    s32 tween_y;
    /** 0x1C last depth tween offset pushed to the target. */
    s32 tween_z;
    /** 0x20 base of the per-frame packed tile records. */
    u8* frame_data;
    FieldAnimFlags flags; /* 0x24 */
    /** 0x28 remaining loop repeats; decremented each time the cel ring wraps. */
    u8 repeat_count;
    u8 _pad2;
    u16 timer; /* 0x2A */
    /** 0x2C tile records per frame, i.e. the stride from one frame to the next. */
    u16 frame_tile_count;
    u8 _pad3[0x30 - 0x2E];
    FieldImageReq upload; /* 0x30 */
    /** Scratch pixels used by strip copies and frame blending. */
    u16 scratch_pixels[257]; /* 0x40 */
};

/**
 * @brief Sequence command record, one 12-byte entry of the scene's command table.
 *
 * A command names one animation node (list and index) and chains to the
 * sequences it starts when it begins and when it finishes.
 */
typedef struct
{
    /** 0x00 scene animation list: 0 anims, 1 strips, otherwise sprites. */
    u8 list_kind;
    /** 0x01 bit n set: the sequence starts when the scene is built for map object n. */
    u8 start_mask;
    /** 0x02 index of the animation node within that list. */
    u8 anim_index;
    /** 0x03 repeat count given to the node when it starts. */
    u8 repeat_count;
    /** 0x04 keyframe the node stops at, or 0xFF for none. */
    u8 stop_keyframe;
    /** 0x05 sequence started together with this one, or 0xFF for none. */
    u8 start_link;
    /** 0x06 sequence started when this one finishes, or 0xFF for none. */
    u8 end_link;
    u8 _pad7;
    /** 0x08 frames after the start before start_link is triggered. */
    u16 start_delay;
    /** 0x0A frames after the finish before end_link is triggered. */
    u16 end_delay;
} FieldSeqDef;

/** @brief Element of the scene's sequence list (0x14). */
typedef struct FieldSeq FieldSeq;
struct FieldSeq
{
    FieldSeq* next;   /* 0x00 */
    FieldSeqDef* def; /* 0x04 */
    /** 0x08 bits 0-1 phase (0 idle, 1 running, 2 finished), bits 2-3 saved phase. */
    union
    {
        s32 word;
        struct
        {
            u8 state;
            /** 0x09 index handed on to the sequences this one starts. */
            u8 index;
            u8 _pad[2];
        } b;
    } flags;
    /** 0x0C frames since the current phase began. */
    u16 phase_frames;
};

/** @brief View of the movie/streaming control block at 0x801ED500. */
typedef struct
{
    u8 _pad0[0x20];
    struct
    {
        u16 x;
        u16 y;
        u16 w;
        u16 h;
    } rects[3]; /* 0x20 */
    u8 _pad1[0x98 - 0x38];
    u8 chunk_idx; /* 0x98 */
    u8 _pad2[0x9D - 0x99];
    u8 frame_ready; /* 0x9D */
    u8 _pad3;
    u8 end_state; /* 0x9F */
} FieldMovieState;

/**
 * @brief One run of consecutive points in g_field_node_angle_table.
 */
typedef struct
{
    /** 0x00 low 15 bits = number of points; zero ends the run list. */
    u16 count;
    /** 0x02 index of the run's first (x, y) point pair. */
    u16 first;
} FieldNodeRun;

/** Mask of FieldNodeRun::count that holds the point count. */
#define FIELD_NODE_RUN_COUNT_MASK 0x7FFF

/** Bit of the FieldNodeDef::flags low byte that enables the node. */
#define FIELD_NODE_DEF_ENABLE_SHIFT 7

/**
 * @brief Definition record shared by a FieldNode.
 *
 * x_angle_index/y_angle_index select entries in g_field_node_angle_table;
 * base_x/base_y are the horizontal/vertical base offsets (each shifted by 8).
 */
struct FieldNodeDef
{
    FieldNodeDef* next; /* 0x00 next definition in the scene resource */
    /** 0x04 flag word. Bit 2 excludes the node from the group scan; the low
        two bits select the group mode (0 = single, 1 = pair). field_collision_collect_groups
        reads the whole word for the bit-2 test and only the low byte for the
        mode, which is why both a word and a byte access appear. */
    s32 flags;
    /** 0x08 index of the owning object (field_get_object), or 0xFF for none. */
    u8 obj_index;
    /** 0x09 index of the owning part within that object (field_get_object_part),
        or 0xFF when the node hangs off the object itself. */
    u8 part_index;
    u16 x_angle_index; /* 0x0A angle-table index for the horizontal step */
    u16 y_angle_index; /* 0x0C angle-table index for the vertical step */
    u8 _pad1[0x10 - 0xE];
    s16 base_x; /* 0x10 horizontal base offset (<< 8) */
    s16 base_y; /* 0x12 vertical base offset (<< 8) */
    /** 0x14 lowest group id this definition applies to; field_collision_rasterize_groups skips
        the node when the group id being rasterised is below it. */
    s16 id_min;
    u8 _pad2[0x18 - 0x16];
    /** 0x18 point runs, terminated by a run whose count is zero. */
    FieldNodeRun runs[1];
};

/**
 * @brief Span-pair count per row, stored in byte 2 of FieldNodeDef::flags.
 *
 * @c flags is read as a whole word for the bit-2 / mode tests, but
 * field_collision_rasterize_groups also reads offset 0x06 as an @c lbu - the two accesses
 * genuinely overlap in the original. Mirrors the @c *(s32*)&def->flags idiom
 * field_animation.c uses for the same struct.
 */
#define FIELD_NODE_DEF_ROWS(d) (((u8*)&(d)->flags)[2])

/**
 * @brief Element of the scene's attached-node list (FieldScene offset 0x08).
 *
 * Each node hangs off a FieldPart and carries a swept 2D position that
 * field_update_part_sweep recomputes every frame. The same list is walked by
 * field_clear_node_accumulators in field_scene_load.c.
 */
typedef struct FieldNode FieldNode;
struct FieldNode
{
    FieldNode* next;   /* 0x00 */
    FieldNodeDef* def; /* 0x04 */
    /** 0x08 owning object; set instead of @c part when the node hangs off an
        object rather than one of its parts. field_move_object_nodes matches on this one,
        field_move_part_nodes on @c part. */
    FieldObj* obj;
    FieldPart* part; /* 0x0C owning part */
    /** 0x10 base of the node's span table: for each row, @c
        FIELD_NODE_DEF_ROWS(def) pairs of (x0, x1) shorts. Walked by
        field_collision_rasterize_groups. */
    u16* spans;
    s32 unk14; /* 0x14 */
    /** 0x18 when zero the node is skipped by the group scan in field_collision_collect_groups. */
    u8 unk18;
    u8 _pad2[0x1C - 0x19];
    /** 0x1C smallest point x of the definition's runs. */
    s16 x_min;
    /** 0x1E largest point x of the definition's runs. */
    s16 x_max;
    /** 0x20 last tile row this node covers (inclusive). */
    s16 row_end;
    /** 0x22 first tile row this node covers; also the sort key
        field_collision_rasterize_groups orders the scratch node list by. */
    s16 row_start;
    /** 0x24 horizontal offset accumulator; the axis-0 half of the pair the two
        node shift helpers move. */
    s32 unk24;
    s32 delta_x; /* 0x28 horizontal delta since the previous frame */
    s32 delta_y; /* 0x2C vertical delta since the previous frame */
    /** 0x30 depth offset accumulator, the axis-2 counterpart of unk24. */
    s32 unk30;
    /** 0x34 second bank of unk24; every shift writes both banks. */
    s32 unk34;
    s32 x; /* 0x38 current horizontal position */
    s32 y; /* 0x3C current vertical position */
    /** 0x40 second bank of unk30. */
    s32 unk40;
};

typedef struct
{
    FieldSceneHeader* header; /* 0x00 */
    FieldObj* objects;        /* 0x04 head of the object list */
    FieldNode* nodes;         /* 0x08 head of the attached-node list */
    /** 0x0C secondary node chain consulted by the collision resolver. */
    FieldNode* secondary_nodes;
    FieldMarker* markers; /* 0x10 head of the marker list */
    FieldSeq* seqs;       /* 0x14 head of the sequence list */
    FieldAnim* anims;     /* 0x18 head of the animation list */
    FieldAnim* strips;    /* 0x1C head of the strip list */
    FieldAnim* sprites;   /* 0x20 head of the sprite list */
    FieldAnim* effects;   /* 0x24 head of the effect list */
    /** 0x28 base of the per-group tile bitmask rows, or 0 when no groups
        are active (then group_count holds a FIELD_COLLISION_GROUP_ERROR_*
        code); field_set_node_enabled gates its field_collision_rasterize_groups call on this. */
    s32 group_work;
    /** 0x2C base of the per-group byte tile maps. */
    s32 group_tiles;
    /** 0x30 end of the per-group work area. */
    s32 group_work_end;
    FieldImageReq* uploads; /* 0x34 head of the pending upload list */
    /** 0x38 set by a movie animation while the scene builds, then the MDEC VLC table it gets. */
    s32 vlc_table;
    /** 0x3C cleared by the scene build. */
    s32 unk3C;
    /** 0x40 tile edge in pixels, 4 or 8. */
    u8 tile_size;
    /** 0x41 number of active groups; 0 or 1 when the scan found nothing. */
    u8 group_count;
    /** 0x42 bitmask words per group in the work area. */
    s16 group_stride;
    /** 0x44 tiles per group (tile_cols * tile_rows). */
    u16 group_tile_count;
    /** 0x46 tile columns. */
    u16 tile_cols;
    /** 0x48 tile rows. */
    u16 tile_rows;
    /** 0x4A group ids (floor heights), sorted ascending by field_collision_collect_groups. */
    s16 group_ids[10];
    /** 0x5E per-group counters, zeroed alongside group_ids. */
    s16 group_counters[10];
} FieldScene;

typedef struct
{
    FieldScene* scene;
} FieldSceneGlobals;

/** @brief Background colour word of a FieldMapObject, tested whole and read by byte. */
typedef union
{
    u32 word;
    struct
    {
        /** Bit 0: the colour below is used; bit 1: copied to FieldMapBounds::unk4. */
        u8 flags;
        u8 r;
        u8 g;
        u8 b;
    } b;
} FieldMapColor;

/**
 * @brief One object of the loaded field map.
 *
 * g_field_objects is a NULL-terminated array of pointers to these.
 * field_select_object uploads the object's image and background colour, and
 * field_build_render_records expands its definition lists into the scene arena.
 */
typedef struct
{
    /** NULL-terminated array of object definitions. */
    FieldObjDef** object_defs;
    /** Texture and CLUT image uploaded to VRAM; also the key field_load_map deduplicates on. */
    u_long* image;
    /** Head of the node definition list. */
    FieldNodeDef* node_defs;
    /** Head of the edge definition list. */
    FieldMarkerDef* edge_defs;
    u8 _pad0[4];
    /** Heads of the four animation definition lists, one per handler group. */
    FieldAnimDef* anim_defs[4];
    u8 _pad1[2];
    /** Set once the render records are built; cleared at every map load. */
    u16 built;
    /** High byte: texture rows at (0, 472); low byte: CLUT width below them. */
    u16 image_size;
    /** Pixel count handed to field_apply_pixel_lookup. */
    u16 pixel_count;
    FieldMapColor background;
    /** Map size, copied to FIELD_MAP_BOUNDS when the object is selected. */
    u16 width;
    u16 depth;
} FieldMapObject;

extern FieldMapObject** g_field_objects;

void field_build_render_records(FieldMapObject* object, u16 object_index);
void field_collision_rebuild_spans(void);

/**
 * @brief Header words of the scene resource block loaded at 0x80180000.
 *
 * The same words are also reachable as the standalone symbols g_field_resource_version,
 * g_field_dyn_count, g_field_scene, D_80180018 and g_field_node_angle_table.
 */
typedef struct
{
    u8 _pad0[8];
    u16 format_version; /* 0x08 */
    u8 _pad1[0x10 - 0xA];
    s32 seq_count;         /* 0x10 number of FieldSeqDef entries */
    FieldScene* scene;     /* 0x14 */
    FieldSeqDef* seq_defs; /* 0x18 sequence command table */
    /** 0x1C points referenced by FieldNodeRun. */
    DVECTOR* points;
} FieldResource;

/** The scene resource block; the scene's runtime records follow it. */
#define FIELD_RESOURCE ((FieldResource*)0x80180000)

/** Field allocator state block. */
#define FIELD_MEM_STATE ((FieldMemState*)0x801ED000)

/**
 * @brief Field memory-allocator state block at 0x801ED000.
 *
 * The scene-transition fade shares the block: field_begin_scene_fade_out arms it by setting
 * @c fade_mode to 1 and @c fade_level to 0x100, and field_update_scene_fade
 * ticks it from there.
 */
typedef struct
{
    /** 0x00 top of the allocated region. */
    u32 top;
    /** 0x04 text configuration save area (g_field_text_saved_configs). */
    u32 text_configs;
    u8 _pad0[0xC - 8];
    /** 0x0C base of the allocated region. */
    u32 base;
    /** 0x10 end of the first half of the region. */
    u32 midpoint;
    u8 _pad1[0x2C - 0x14];
    /**
     * 0x2C fade state: 1 fading out, 2 held out, 3 fading in, 0 idle. Also
     * reachable as the global g_field_scene_fade_mode, and the two spellings
     * are not interchangeable - see field_update_scene_fade.
     */
    s32 fade_mode;
    /** 0x30 fade level, 0x100 is fully lit; stepped by 8 per frame. */
    s32 fade_level;
} FieldMemState;

/*
 * Words of the allocator block that the code also reaches as plain globals.
 * The original mixes both spellings; each compiles differently (one lui per
 * global access, a shared base register through FIELD_MEM_STATE).
 */
/** @brief Top of the allocated region (FieldMemState.top). */
extern s32 g_field_mem_top;
/** @brief Base of the allocated region (FieldMemState.base). */
extern s32 g_field_mem_base;
/** @brief End of the first half of the region (FieldMemState.midpoint). */
extern s32 g_field_mem_midpoint;
/** @brief Scene fade state (FieldMemState.fade_mode). */
extern s32 g_field_scene_fade_mode;

extern FieldSceneGlobals g_field_scene;
extern s32 g_field_marker_overlay_enabled[2];
/*
 * Standalone symbols for SCENE_STATE->camera_x/y/z (scene_state.h, 0x801ED484..0x801ED48C).
 * field_draw_scene_objects uses both forms; the choice selects the addressing mode and is
 * required to match.
 */
extern s32 g_field_camera_x;
extern s32 g_field_camera_y;
extern s32 g_field_camera_z;
/** @brief Pixel lookup table field_load_map applies to the next map, plus one; 0 for none. */
extern s32 g_field_pixel_lookup_selector;
extern s16* g_field_node_angle_table;

s32 rcos(s32);
s32 rsin(s32);

/**
 * @brief Screen-space placement of the grid being drawn.
 *
 * field_emit_sprite_grid only needs the origin; field_emit_rotated_sprite_grid also reads the
 * width and camera position to derive the rotation centre for its non-default placement modes.
 */
typedef struct
{
    /** Screen-space origin of the grid. */
    s32 x;
    s32 y;
    /** Scene width in pixels, from FieldSceneHeader::unk30. */
    s32 width;
    /** Camera position in screen pixels. */
    s32 camera_x;
    s32 camera_y;
} FieldViewport;

/* Scene object/part/sequence helpers shared across the FIELD scene TUs.
   Defined in field_scene_control.c; documented at their definitions. */
void field_start_sequence(FieldSeq* seq, u8 index);
void field_move_part_nodes(FieldPart* part, s32 delta, s32 axis);
void field_move_object_nodes(FieldObj* obj, s32 delta, s32 axis);
FieldObj* field_get_object(s32 index);
FieldPart* field_get_object_part(s32 obj_index, s32 part_index);
FieldPart* field_find_grid_part(FieldPartDef* grid, FieldTintSrc** out_src);
void field_build_tint_colors(u8* colors, s32 count, s32* rgb_scale);
void field_set_tint_primitive_code(u8 format, s32 count, u8* primitive_code);
FieldObj* field_find_object_by_definition(void* definition);

/* MOVIE.BIN entry points, called in place after FIELD streams MOVIE.BIN to 0x80140000. */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx);
void movie_update(void);

/** Size of the full-screen movie still image, in pixels. */
extern u16 g_field_movie_frame_width;
extern u16 g_field_movie_frame_height;

/* Shared animation key formats used by updates and control APIs. */
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
typedef struct FieldTweenSpan
{
    /** 0x00 low seven bits: keyframes covered by this span. */
    u8 count;
    /** 0x01 running total of the spans before this one, in frames. */
    u8 range_start;
    /** 0x02 length of the keyframe this record covers, in frames. */
    u16 duration;
} FieldTweenSpan;

/** FieldTweenSpan::count bits holding the keyframe count. */
#define FIELD_SPAN_COUNT_MASK 0x7F

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

#endif
