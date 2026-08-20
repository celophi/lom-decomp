#ifndef FIELD_SCENE_INTERNAL_H
#define FIELD_SCENE_INTERNAL_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

/*
 * TODO: this file's .rodata does not byte-match yet. gcc emits `.rdata` plus
 * `.align 3` ahead of every jump table, so the four tables land at +0x00,
 * +0x18, +0x38 and +0x58 of the segment's rodata, while the original packs them
 * at +0x00, +0x14, +0x34 and +0x54 - the 5-word first table (jtbl_8004FCD4)
 * gets a 4-byte pad the original does not have, and everything after it shifts.
 * The .text of every function here is unaffected; only the %hi/%lo operands of
 * the three later jump-table loads point 4 bytes high. Needs 4-byte alignment
 * for jump tables out of the toolchain (gcc or maspsx), not a source change.
 */

/**
 * @brief Truncating divide by two, written out as the conditional GCC would
 *        NOT generate for `/ 2`.
 *
 * gcc 2.8's expmed.c refuses the branchy power-of-two divide expansion for
 * `abs_d == 2`, so `x / 2` always comes out as `srl 31 / addu / sra 1`. The
 * target uses the branch form, which means the rounding was spelled out in the
 * source. Reverting this to `/ 2` costs the whole halving block.
 *
 * @param v Signed value to halve.
 * @return @p v divided by two, rounded toward zero.
 */
#define HALF_TOWARD_ZERO(v) ((v) >= 0 ? ((v) >> 1) : (((v) + 1) >> 1))

/**
 * @brief Arithmetic right shift that rounds toward zero instead of down.
 *
 * The generalisation of HALF_TOWARD_ZERO above. It names its argument three
 * times on purpose: gcc cannot CSE the two arms of the conditional across the
 * branch, so a nested use expands to the target's triplicated multiply chains.
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

/** Format-dependent final word of a compact tile record. */
typedef union
{
    /** Sprite records store a complete GP0(E1h) command here. */
    s32 draw_mode;
    /** Quad records store their second UV and TPage tuple here. */
    struct
    {
        s8 u;
        s8 v;
        s16 tpage;
    } quad;
} FieldTileTail;

/**
 * @brief Render record built from a FieldTileDesc.
 *
 * u/v are a texture coordinate pair and clut is the CLUT halfword. The tail is
 * used two ways: field_build_sprite_tile_record writes tail.draw_mode, while
 * field_build_quad_tile_record writes tail.quad's second coordinate pair and
 * texture-page halfword.
 *
 * @note When the descriptor is absent the whole first word is set to -1, so
 *       u/v/clut are also addressed as a single s32 (see the else arm).
 * @note Both writers shift the tail down by 4 bytes when flags bit 0 is set,
 *       i.e. the record is 4 bytes shorter in that mode.
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
 * @return Nothing.
 *
 * @see decomp.me (100%) TODO
 */

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

/**
 * @brief Per-object definition record.
 */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 shared-source handle; two defs with the same one are compatible. */
    s32 shared_source;
    u8 _pad1[0xC - 8];
    /**
     * 0x0C bit 1 zeroes the offsets; bit 2 and bits 4-5 select the horizontal
     * multiplier / wrap; bit 3 and bits 6-7 the vertical one. Must be UNSIGNED:
     * the target shifts it with `srl`, not `sra`.
     */
    u32 flags;
    u8 _pad2[0x1C - 0x10];
    /** 0x1C horizontal scale; 0x10 means "unscaled", bit 7 negates. */
    u8 scroll_scale_x;
    /** 0x1D vertical scale; same encoding as unk1C. */
    u8 scroll_scale_y;
} FieldObjDef;

/**
 * @brief Definition record hanging off a part.
 *
 * The word at 0x08 is read whole (its bits 12-15 select field_emit_rotated_sprite_grid's
 * placement mode) while bytes 0x0A and 0x0B are read separately as the cell
 * grid dimensions, so the two views have to share storage - same arrangement
 * as FieldObjFlags below.
 */
typedef struct
{
    /** 0x00 identity key; field_find_shareable_part matches parts on it. */
    s32 key;
    u8 _pad0[8 - 4];
    union
    {
        /** 0x08 whole word; bit 7 marks the part unshareable, bits 12-15
            select field_emit_rotated_sprite_grid's placement mode. */
        u32 word;
        struct
        {
            u8 _pad1[2];
            /** 0x0A grid width, in cells. */
            u8 cols;
            /** 0x0B grid height, in cells. */
            u8 rows;
        } b;
    } u;
} FieldPartDef;

/**
 * @brief Element of an object's part list.
 */
typedef struct FieldPart FieldPart;
struct FieldPart
{
    FieldPart* next;   /* 0x00 */
    FieldPartDef* def; /* 0x04 */
    /** 0x08 when zero the part still needs its tint records rebuilt; see func_8005A0D0. */
    s32 unk8;
    /** 0x0C bit plane: one bit per grid cell, row-major, LSB first. */
    s32* bits;
    /** 0x10 packed stream of FieldCellRec, one per set bit. */
    u8* records;
    u8 _pad1[0x18 - 0x14];
    /**
     * 0x18 texture-page word shared by every cell; when non-zero it is emitted
     * once as its own primitive instead of per record, shortening the stride.
     */
    s32 tpage_word;
    /** 0x1C rgb/code word shared by every cell; same stride effect as tpage. */
    s32 code_word;
    /** 0x20 zero means the part is not drawn. */
    u8 visible;
    /** 0x21 selects the per-part byte cost: 0x18, 0x1C, 0x28 or 0x34 units. */
    u8 kind;
    /** 0x22 number of attached FieldNode instances field_update_part_sweep updates. */
    u8 node_count;
    u8 _pad2[0x26 - 0x23];
    /** 0x26 number of instances; zero means the part is skipped entirely. */
    u16 instance_count;
    s32 x; /* 0x28 x offset within the object */
    s32 y; /* 0x2C y offset within the object */
    s32 z; /* 0x30 z offset within the object */
    u8 _pad3[0x36 - 0x34];
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
    /** 0x40 horizontal scale, 8.8 fixed point. */
    u16 scale_x;
    /** 0x42 vertical scale, 8.8 fixed point. */
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
    s32 unk10;           /* 0x10 compared when matching two objects */
    u16 unk14;           /* 0x14 compared when matching two objects */
    u8 _pad0[0x1C - 0x16];
    s32 x;       /* 0x1C x offset */
    s32 y;       /* 0x20 y offset */
    s32 z;       /* 0x24 z offset */
    s32 drift_x; /* 0x28 accumulated x drift */
    s32 drift_y; /* 0x2C accumulated y drift */
};

/**
 * @brief Singly-linked record hanging off FieldSceneHeader::records.
 *
 * @note Only the link is identified. func_800630BC hands the caller @c body,
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
    /** 0x10 head of the record list indexed by func_800630BC; null when the
        scene carries no records. */
    FieldHeaderRec* records;
    u8 _pad2[0x28 - 0x14];
    u16 pixel_stride; /* 0x28 source stride, in halfwords */
    u8 _pad3[0x30 - 0x2A];
    s16 unk30; /* 0x30 */
    /** 0x32 counterpart of unk30; func_8005F158 uses the pair as the scene's
        pixel extent when sizing its tile budget. */
    s16 unk32;
} FieldSceneHeader;

/**
 * @brief Per-marker record hanging off FieldMarker::def.
 *
 * unk4/unk6 and unk8/unkA are two screen-space point pairs; unk10 is a depth
 * bias folded into the marker's vertical origin and unk14 the numeric label
 * drawn next to it.
 */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 first point, horizontal. */
    u16 x0;
    /** 0x06 first point, vertical (halved before use). */
    u16 y0;
    /** 0x08 second point, horizontal. */
    u16 x1;
    /** 0x0A second point, vertical (halved before use). */
    u16 y1;
    u8 _pad1[0x10 - 0xC];
    /** 0x10 depth bias added to the fixed 0xE0 vertical origin. */
    s16 depth_bias;
    u8 _pad2[0x14 - 0x12];
    /** 0x14 value rendered as the marker's numeric label. */
    u16 label;
} FieldMarkerDef;

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
};

/**
 * @brief Element of the scene's pending VRAM upload list.
 *
 * Each node carries a ready-made LoadImage argument pair: the destination
 * rectangle sits inline at 0x04 so its address can be taken directly.
 */
typedef struct FieldImageReq FieldImageReq;
struct FieldImageReq
{
    FieldImageReq* next; /* 0x00 */
    RECT rect;           /* 0x04 destination rectangle in VRAM */
    u_long* data;        /* 0x0C source pixel data */
};

/** @brief Definition record shared by the animation and sequence lists. */
typedef struct
{
    u8 unk0; /* 0x00 */
    u8 unk1; /* 0x01 */
    u8 unk2; /* 0x02 */
    u8 unk3; /* 0x03 */
    u8 flags; /* 0x04 low three bits select the handler */
    u8 unk5;  /* 0x05 */
    u8 unk6;  /* 0x06 */
    /** 0x07 handler sub-kind; the high byte of the word read at 0x04. */
    u8 handler_group;
    u16 unk8; /* 0x08 */
    u16 unkA; /* 0x0A */
    u8 unkC;  /* 0x0C */
    u8 unkD;  /* 0x0D */
    u8 unkE;  /* 0x0E */
    u8 unkF;  /* 0x0F */
    u8 unk10; /* 0x10 */
    u8 _pad2;
    u16 unk12; /* 0x12 */
    u8* data;  /* 0x14 handler-specific data */
} FieldAnimDef;

/**
 * @brief Tile grid referenced by a tile-blit animation definition.
 *
 * Reached two ways: through FieldTileAnimDef::grid (field_blit_animation_frame) and through
 * FieldAnimCel::grid (field_retarget_cel_cluts). The word at 0x08 is read whole for its
 * packing-mode bits while bytes 0x0A and 0x0B are read separately as the grid
 * dimensions, so the two views have to share storage - same arrangement as
 * FieldPartDef.
 *
 * @note The dimensions line up with the low and high halves of some other
 *       record's `unk0A` halfword, so this may well be a view of a second
 *       FieldAnimDef rather than a struct of its own.
 */
typedef struct
{
    /** 0x00 packed tile descriptors, one 4-byte entry per grid cell. */
    FieldTileDesc* tiles;
    u8 _pad0[8 - 4];
    union
    {
        /** 0x08 whole word; bits 4-5 select the CLUT packing mode. */
        u32 word;
        struct
        {
            u8 _pad1[2];
            /** 0x0A grid width in tiles. */
            u8 cols;
            /** 0x0B grid height in tiles. */
            u8 rows;
        } b;
    } u;
} FieldTileGrid;

/**
 * @brief Tile-blit view of FieldAnimDef.
 *
 * The `unk4 & 7` handler kind decides what lives at offset 0x10: the image-DMA
 * handlers read it as the byte `FieldAnimDef::unk10`, while the tile-blit
 * handler reads the whole word as a pointer to the grid dimensions. The two
 * uses never overlap, so they are kept as separate types rather than a union.
 */
typedef struct
{
    u8 _pad0[0x10];
    FieldTileGrid* grid; /* 0x10 */
} FieldTileAnimDef;

/** @brief Element of an animation node's cel ring. */
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

typedef struct FieldAnimCel FieldAnimCel;

/**
 * @brief Colour source for the tile tint pass, hung off FieldAnim::unk10.
 *
 * The two halfword triples multiply component-wise into the three-word colour
 * func_8005AC50 expands into the scratchpad table at 0x1F800000.
 *
 * @note field_tint_animation_cel_list reaches this record through FieldAnim::cels instead, and
 *       walks the cel list at 0x08 rather than being handed a single cel.
 * @note The scene's object list is a list of these: func_8005A0D0 walks it
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
    FieldAnimCel* cels;
    u8 _pad1[0x10 - 0xC];
    u16 red;         /* 0x10 */
    u16 green;       /* 0x12 */
    u16 blue;        /* 0x14 */
    u16 red_scale;   /* 0x16 */
    u16 green_scale; /* 0x18 */
    u16 blue_scale;  /* 0x1A */
};

struct FieldAnimCel
{
    FieldAnimCel* next; /* 0x00 */
    /** 0x04 grid this cel's bit plane and tile records are laid out on. */
    FieldTileGrid* grid;
    u8 _pad0[0xC - 8];
    /** 0x0C tile-presence bitmap, one bit per grid cell, LSB first. */
    u32* mask;
    /** 0x10 packed destination tile records, advanced past every present tile. */
    u8* tiles;
    u8 _pad1[0x18 - 0x14];
    /** 0x18 when set, the tile record is 4 bytes shorter. */
    s32 tpage_word;
    /** 0x1C when set, the tile record is 4 bytes shorter. */
    s32 code_word;
    u8 active; /* 0x20 */
    /** 0x21 record-format selector, 0-6; see field_blit_animation_frame. */
    u8 format;
};

/**
 * @brief Animation node flags at 0x24.
 *
 * The word is tested and rewritten as a whole while byte 0x25 is read and
 * written separately as the node's frame index, so the two views share storage.
 */
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
    FieldAnimCel* cels; /* 0x0C */
    s32 unk10;          /* 0x10 */
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
    FieldImageReq req; /* 0x30 */
    u16 buf40[0x10];   /* 0x40 */
    u16 buf60[0xF0];   /* 0x60 */
    u16 buf240[1];     /* 0x240 */
};

/** @brief Element of the scene's sequence list (0x14). */
typedef struct FieldSeq FieldSeq;
struct FieldSeq
{
    FieldSeq* next;    /* 0x00 */
    FieldAnimDef* def; /* 0x04 */
    s32 flags;         /* 0x08 */
    u16 unkC;          /* 0x0C */
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
 * @brief Definition record shared by a FieldNode.
 *
 * x_angle_index/y_angle_index select entries in g_field_node_angle_table;
 * base_x/base_y are the horizontal/vertical base offsets (each shifted by 8).
 */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 flag word. Bit 2 excludes the node from the group scan; the low
        two bits select the group mode (0 = single, 1 = pair). func_8005F158
        reads the whole word for the bit-2 test and only the low byte for the
        mode, which is why both a word and a byte access appear. */
    s32 flags;
    u8 _pad0b[0xA - 8];
    u16 x_angle_index; /* 0x0A angle-table index for the horizontal step */
    u16 y_angle_index; /* 0x0C angle-table index for the vertical step */
    u8 _pad1[0x10 - 0xE];
    s16 base_x; /* 0x10 horizontal base offset (<< 8) */
    s16 base_y; /* 0x12 vertical base offset (<< 8) */
    /** 0x14 lowest group id this definition applies to; func_8005F5BC skips
        the node when the group id being rasterised is below it. */
    s16 id_min;
} FieldNodeDef;

/**
 * @brief Span-pair count per row, stored in byte 2 of FieldNodeDef::flags.
 *
 * @c flags is read as a whole word for the bit-2 / mode tests, but
 * func_8005F5BC also reads offset 0x06 as an @c lbu - the two accesses
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
        object rather than one of its parts. func_8005AA68 matches on this one,
        func_8005A984 on @c part. */
    FieldObj* obj;
    FieldPart* part; /* 0x0C owning part */
    /** 0x10 base of the node's span table: for each row, @c
        FIELD_NODE_DEF_ROWS(def) pairs of (x0, x1) shorts. Walked by
        func_8005F5BC. */
    u16* spans;
    u8 _pad1[0x18 - 0x14];
    /** 0x18 when zero the node is skipped by the group scan in func_8005F158. */
    u8 unk18;
    u8 _pad2[0x20 - 0x19];
    /** 0x20 last tile row this node covers (inclusive). */
    s16 row_end;
    /** 0x22 first tile row this node covers; also the sort key
        func_8005F5BC orders the scratch node list by. */
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
    /** 0x28 base of the per-group work area, or 0 when no groups are active;
        func_8005B228 gates its func_8005F5BC call on this. */
    s32 unk28;
    /** 0x2C base of the per-group tile area. */
    s32 unk2C;
    /** 0x30 end of the per-group work area. */
    s32 unk30;
    FieldImageReq* uploads; /* 0x34 head of the pending upload list */
    s32 unk38; /* 0x38 scene-build state */
    u8 _pad2[0x40 - 0x3C];
    /** 0x40 tile edge in pixels, 4 or 8. */
    u8 unk40;
    /** 0x41 number of active groups; 0 or 1 when the scan found nothing. */
    u8 unk41;
    /** 0x42 per-group stride of the work area. */
    s16 unk42;
    /** 0x44 tiles per group (unk46 * unk48). */
    s16 unk44;
    /** 0x46 tile columns. */
    s16 unk46;
    /** 0x48 tile rows. */
    s16 unk48;
    /** 0x4A group ids, sorted descending by func_8005F158. */
    s16 unk4A[10];
    /** 0x5E per-group counters, zeroed alongside unk4A. */
    s16 unk5E[10];
} FieldScene;

typedef struct
{
    FieldScene* scene;
} FieldSceneGlobals;

/**
 * @brief Field memory-allocator state block at 0x801ED000.
 *
 * The scene-transition fade shares the block: func_8005B1EC arms it by setting
 * @c fade_mode to 1 and @c fade_level to 0x100, and field_update_scene_fade
 * ticks it from there.
 */
typedef struct
{
    /** 0x00 top of the allocated region. */
    u32 top;
    u8 _pad0[0xC - 4];
    /** 0x0C base of the allocated region. */
    u32 base;
    /** 0x10 end of the first half of the region. */
    u32 midpoint;
    u8 _pad1[0x2C - 0x14];
    /**
     * 0x2C fade state: 1 fading out, 2 held out, 3 fading in, 0 idle. Also
     * reachable as the standalone symbol D_801ED02C, and the two spellings are
     * not interchangeable - see field_update_scene_fade.
     */
    s32 fade_mode;
    /** 0x30 fade level, 0x100 is fully lit; stepped by 8 per frame. */
    s32 fade_level;
} FieldMemState;

/**
 * @brief Camera / scroll state block at 0x801ED480.
 *
 * The individual words are also referenced as the standalone symbols
 * g_field_camera_x / g_field_camera_y / g_field_camera_z; field_draw_scene_objects uses BOTH forms and the
 * distinction is required to match, because it selects the addressing mode.
 */
typedef struct
{
    u8 _pad[4];
    s32 x; /* 0x04 == g_field_camera_x */
    s32 y; /* 0x08 == g_field_camera_y */
    s32 z; /* 0x0C == g_field_camera_z */
} FieldCamera;

extern FieldSceneGlobals g_field_scene;
extern s32 g_field_marker_overlay_enabled[2];
extern s32 g_field_camera_x;
extern s32 g_field_camera_y;
extern s32 g_field_camera_z;
extern s16* g_field_node_angle_table;

s32 rcos(s32);
s32 rsin(s32);
void field_draw_marker_overlay(u32*, u32*);
void field_draw_part(FieldPart*, s32, s32*, s32);


#endif
