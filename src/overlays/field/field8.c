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
 * @brief Packed 4-byte per-tile source descriptor consumed by func_8005477C.
 */
typedef struct
{
    /** Bit 7 = tile present; bits 0-4 feed the texture-page / CLUT word. */
    u8 unk0;
    /** Bit 6 = set flag 2 on the record; bits 0-3 = horizontal slot. */
    u8 unk1;
    /** High nibble is copied out verbatim; low nibble is the vertical slot. */
    u8 unk2;
    /** Index into the scratchpad word table at 0x1F800000. */
    u8 unk3;
} FieldTileDesc;

/**
 * @brief Render record built from a FieldTileDesc.
 *
 * unk0/unk1 are a texture coordinate pair and unk2 the CLUT/texture-page
 * halfword. The tail is used two ways: func_8005477C writes a single GPU
 * draw-mode word across unk8..unkA, while func_80054904 writes a second
 * coordinate pair plus its own texture-page halfword.
 *
 * @note When the descriptor is absent the whole first word is set to -1, so
 *       unk0/unk1/unk2 are also addressed as a single s32 (see the else arm).
 * @note Both writers shift the tail down by 4 bytes when flags bit 0 is set,
 *       i.e. the record is 4 bytes shorter in that mode.
 */
typedef struct
{
    s8 unk0;
    s8 unk1;
    s16 unk2;
    s32 unk4;
    s8 unk8;
    s8 unk9;
    s16 unkA;
} FieldTileRec;

/**
 * @brief Build one field tile render record from its packed descriptor.
 *
 * Writes the vertical slot, the texture-page/CLUT halfword (selected by
 * @p mode), an entry pulled from the scratchpad table at 0x1F800000, and a
 * GPU draw-mode word (0xE1000400 | attributes). If the descriptor's presence
 * bit is clear the record's first word is poisoned with -1 instead.
 *
 * @param desc  Packed 4-byte tile descriptor.
 * @param prim  Render record to fill in.
 * @param mode  Texture-page selection mode, 0-3. 0 and 1 pick different
 *              packings of desc->unk0; anything else zeroes the halfword.
 *              Also contributes bits 7-8 of the draw-mode attributes.
 * @param flags Bit 0: emit the draw-mode word at offset 4 and skip the
 *              scratchpad lookup. Bit 1: skip the draw-mode word entirely.
 *
 * @note The `switch (mode)` is required to match: the equivalent nested `if`
 *       lays the blocks out as default/case1/case0 and scores 91.89%, and an
 *       `if/else if` chain scores lower still.
 * @note The `base` and `tmp` temporaries are required to match. Written as one
 *       inline `|` chain gcc reassociates the operands and CSE-hoists
 *       `mode & 3` instead of `desc->unk1 * 2` (94.59%); merely parenthesizing
 *       without a named temp is not enough (97.81%).
 * @note `b0` must be `u32`, not `u8`: with `u8` the final `or` in the case-0
 *       arm comes out operand-swapped (99.80%).
 * @note Measured non-factors, all still 100%: `h * 0x10` vs `h << 4`, the
 *       compound vs expanded form of the `h -=` subtraction, and a redundant
 *       `(u32)` cast on the `>> 4`.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005477C(FieldTileDesc *desc, FieldTileRec *prim, s32 mode, s32 flags)
{
    u32 h;
    s32 attr;
    s32 base;
    s32 tmp;
    u8 b1;
    s32 lo;

    if (desc->unk0 & 0x80)
    {
        u8 b2 = desc->unk2;

        h = b2 & 0xF;
        prim->unk1 = b2 & 0xF0;
        switch (mode)
        {
        case 0:
            {
                u32 b0 = desc->unk0;

                prim->unk2 = ((((b0 & 0x1F) >> 4) + 0x1D8) << 6) | (b0 & 0xF);
            }
            break;
        case 1:
            prim->unk2 = (((desc->unk0 & 0x1F) + 0x1D8) << 6);
            break;
        default:
            prim->unk2 = 0;
            break;
        }
        if (!(flags & 1))
        {
            prim->unk4 = ((s32 *) 0x1F800000)[desc->unk3];
            if (desc->unk1 & 0x40)
            {
                *((u8 *) prim + 7) |= 2;
            }
        }
        if (!(flags & 2))
        {
            b1 = desc->unk1;
            lo = b1 & 0xF;
            if ((u32) (lo + (h >> (4 - mode))) >= 0xAU)
            {
                h -= (0xA - lo) * 4;
                base = ((mode & 3) << 7) | ((b1 * 2) & 0x60);
                attr = base | 8;
            }
            else
            {
                base = ((mode & 3) << 7) | ((b1 * 2) & 0x60);
                tmp = (((u32) ((lo << 6) + 0x140)) >> 6) | 0x10;
                attr = base | tmp;
            }
            if (!(flags & 1))
            {
                *(s32 *) &prim->unk8 = attr | 0xE1000400;
            }
            else
            {
                prim->unk4 = attr | 0xE1000400;
            }
        }
        prim->unk0 = h * 0x10;
    }
    else
    {
        *(s32 *) prim = -1;
    }
}

/**
 * @brief Build a two-coordinate field tile render record from its descriptor.
 *
 * Sibling of func_8005477C: same descriptor and the same CLUT/texture-page
 * selection, but it emits a second texture coordinate pair (u + 15, v) and a
 * texture-page halfword at the record tail instead of a GPU draw-mode word.
 *
 * @param desc  Packed 4-byte tile descriptor.
 * @param prim  Render record to fill in.
 * @param mode  Texture-page selection mode, 0-3. 0 and 1 pick different
 *              packings of desc->unk0; anything else zeroes the halfword.
 *              Also contributes bits 7-8 of the texture-page attributes.
 * @param flags Bit 0: skip the scratchpad lookup and shift the tail down by
 *              4 bytes. Bit 1: skip the second coordinate pair entirely.
 *
 * @note The attribute word must be built by **accumulating in place** into one
 *       variable (`base = mode & 3; base = base << 7; base = base | ...;`).
 *       Writing it as a single `|` chain reassociates the operands and flips
 *       which subexpression is shared between the two arms: 99.14% with block 1
 *       inlined, 98.28% with both blocks inlined. Same mechanism as
 *       func_8005477C's `base`/`tmp` temporaries. See [CSE-05] in idioms.md.
 * @note `base = base | tmp; prim->unkA = base;` must stay split; folding it to
 *       `prim->unkA = base | tmp;` costs an instruction (98.77%).
 * @note The tail stores must be ordered unk8, unk9, unkA. The natural-looking
 *       unk8, unkA, unk9 order leaves the load-delay slot unfilled (98.21%).
 * @note `prev` must be a block-local declared inside the `flags & 1` arm.
 *       Hoisting it above the `if` as a function-scope variable colors it into
 *       the wrong register and flips the delay-slot fill (97.84%).
 * @note The `switch (mode)` and `u32 b0` requirements are as documented on
 *       func_8005477C; measured here too (94.07% and 99.85% respectively).
 * @note The `(s32)` cast inside the `>= 0xA` arm is required: it selects an
 *       arithmetic `sra` over a logical `srl` (99.55% without).
 *
 * @see decomp.me (100%) TODO
 */
void func_80054904(FieldTileDesc *desc, FieldTileRec *prim, s32 mode, s32 flags)
{
    s32 base;
    s32 tmp;
    s32 tmp2;
    s32 attr;
    u8 b1;
    u32 lo;

    if (desc->unk0 & 0x80)
    {
        prim->unk0 = (desc->unk2 & 0xF) * 0x10;
        prim->unk1 = desc->unk2 & 0xF0;
        switch (mode)
        {
        case 0:
            {
                u32 b0 = desc->unk0;

                prim->unk2 = ((((b0 & 0x1F) >> 4) + 0x1D8) << 6) | (b0 & 0xF);
            }
            break;
        case 1:
            prim->unk2 = (((desc->unk0 & 0x1F) + 0x1D8) << 6);
            break;
        default:
            prim->unk2 = 0;
            break;
        }
        if (!(flags & 1))
        {
            prim->unk4 = ((s32 *) 0x1F800000)[desc->unk3];
            if (desc->unk1 & 0x40)
            {
                *((u8 *) prim + 7) |= 2;
            }
        }
        b1 = desc->unk1;
        lo = b1 & 0xF;
        if (lo >= 0xAU)
        {
            base = mode & 3;
            base = base << 7;
            base = base | ((b1 * 2) & 0x60);
            tmp = (s32) ((((s32) (lo << 6)) - 0x80) & 0x3FF) >> 6;
        }
        else
        {
            base = mode & 3;
            base = base << 7;
            base = base | ((b1 * 2) & 0x60);
            tmp = (((lo << 6) + 0x140) >> 6) | 0x10;
        }
        base = base | tmp;
        prim->unkA = base;
        if (!(flags & 2))
        {
            u8 c1 = desc->unk1;
            u32 lo2 = c1 & 0xF;

            if (lo2 >= 0xAU)
            {
                base = mode & 3;
                base = base << 7;
                base = base | ((c1 * 2) & 0x60);
                tmp2 = (s32) ((((s32) (lo2 << 6)) - 0x80) & 0x3FF) >> 6;
            }
            else
            {
                base = mode & 3;
                base = base << 7;
                base = base | ((c1 * 2) & 0x60);
                tmp2 = (((lo2 << 6) + 0x140) >> 6) | 0x10;
            }
            attr = base | tmp2;
            if (!(flags & 1))
            {
                prim->unk8 = ((desc->unk2 & 0xF) * 0x10) + 0xF;
                prim->unk9 = desc->unk2 & 0xF0;
                prim->unkA = attr;
            }
            else
            {
                FieldTileRec *prev = (FieldTileRec *) ((u8 *) prim - 4);

                prev->unk8 = ((desc->unk2 & 0xF) * 0x10) + 0xF;
                prev->unk9 = desc->unk2 & 0xF0;
                prev->unkA = attr;
            }
        }
    }
    else
    {
        *(s32 *) prim = -1;
    }
}

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
        u8 unk1;
        /** 0x0E drift magnitude; zero disables the per-frame drift. */
        u8 unk2;
        /** 0x0F drift angle, scaled by 0x10 before rcos/rsin. */
        u8 unk3;
    } b;
} FieldObjFlags;

/**
 * @brief Per-object definition record.
 */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 shared-source handle; two defs with the same one are compatible. */
    s32 unk4;
    u8 _pad1[0xC - 8];
    /**
     * 0x0C bit 1 zeroes the offsets; bit 2 and bits 4-5 select the horizontal
     * multiplier / wrap; bit 3 and bits 6-7 the vertical one. Must be UNSIGNED:
     * the target shifts it with `srl`, not `sra`.
     */
    u32 flags;
    u8 _pad2[0x1C - 0x10];
    /** 0x1C horizontal scale; 0x10 means "unscaled", bit 7 negates. */
    u8 unk1C;
    /** 0x1D vertical scale; same encoding as unk1C. */
    u8 unk1D;
} FieldObjDef;

/**
 * @brief Definition record hanging off a part.
 *
 * The word at 0x08 is read whole (its bits 12-15 select func_80055D20's
 * placement mode) while bytes 0x0A and 0x0B are read separately as the cell
 * grid dimensions, so the two views have to share storage - same arrangement
 * as FieldObjFlags below.
 */
typedef struct
{
    /** 0x00 identity key; func_80056824 matches parts on it. */
    s32 unk0;
    u8 _pad0[8 - 4];
    union
    {
        /** 0x08 whole word; bit 7 marks the part unshareable, bits 12-15
            select func_80055D20's placement mode. */
        u32 word;
        struct
        {
            u8 _pad1[2];
            /** 0x0A grid width, in cells. */
            u8 unkA;
            /** 0x0B grid height, in cells. */
            u8 unkB;
        } b;
    } u;
} FieldPartDef;

/**
 * @brief Element of an object's part list.
 */
typedef struct FieldPart FieldPart;
struct FieldPart
{
    FieldPart *next;        /* 0x00 */
    FieldPartDef *def;      /* 0x04 */
    u8 _pad0[0xC - 8];
    /** 0x0C bit plane: one bit per grid cell, row-major, LSB first. */
    s32 *bits;
    /** 0x10 packed stream of FieldCellRec, one per set bit. */
    u8 *records;
    u8 _pad1[0x18 - 0x14];
    /**
     * 0x18 texture-page word shared by every cell; when non-zero it is emitted
     * once as its own primitive instead of per record, shortening the stride.
     */
    s32 tpage_word;
    /** 0x1C rgb/code word shared by every cell; same stride effect as tpage. */
    s32 code_word;
    /** 0x20 zero means the part is not drawn. */
    u8 unk20;
    /** 0x21 selects the per-part byte cost: 0x18, 0x1C, 0x28 or 0x34 units. */
    u8 kind;
    /** 0x22 number of attached FieldNode instances func_80057A28 updates. */
    u8 node_count;
    u8 _pad2[0x26 - 0x23];
    /** 0x26 number of instances; zero means the part is skipped entirely. */
    u16 count;
    s32 unk28;              /* 0x28 x offset within the object */
    s32 unk2C;              /* 0x2C y offset within the object */
    s32 unk30;              /* 0x30 z offset within the object */
    u8 _pad3[0x36 - 0x34];
    /** 0x36 reload period for the sweep phase at 0x38. */
    u16 sweep_period;
    /** 0x38 sweep phase; counts down each frame, reloads from 0x36 at zero. */
    u16 sweep_phase;
    /** 0x3A rotation angle applied to the vertical (row) step. */
    u16 unk3A;
    /** 0x3C rotation angle applied to the horizontal (column) step. */
    u16 unk3C;
    /** 0x3E rotation angle of the grid as a whole; feeds both rsin and rcos. */
    u16 unk3E;
    /** 0x40 horizontal scale, 8.8 fixed point. */
    u16 unk40;
    /** 0x42 vertical scale, 8.8 fixed point. */
    u16 unk42;
    /**
     * 0x44..0x4A the four corner CLUT ids, bilinearly interpolated across the
     * grid. Derived from the interpolation endpoints: the row weight resolves
     * to unk46 on the first (topmost) row and unk44 on the last, and the
     * column weight to the "left" pair on the first (leftmost) column.
     */
    s16 clut_bl;            /* 0x44 bottom left */
    s16 clut_tl;            /* 0x46 top left */
    s16 clut_br;            /* 0x48 bottom right */
    s16 clut_tr;            /* 0x4A top right */
};

/**
 * @brief Element of the scene's object list.
 */
typedef struct FieldObj FieldObj;
struct FieldObj
{
    FieldObj *next;         /* 0x00 */
    FieldObjDef *def;       /* 0x04 */
    FieldPart *parts;       /* 0x08 head of the part list */
    FieldObjFlags flags;    /* 0x0C */
    s32 unk10;              /* 0x10 compared when matching two objects */
    u16 unk14;              /* 0x14 compared when matching two objects */
    u8 _pad0[0x1C - 0x16];
    s32 unk1C;              /* 0x1C x offset */
    s32 unk20;              /* 0x20 y offset */
    s32 unk24;              /* 0x24 z offset */
    s32 unk28;              /* 0x28 accumulated x drift */
    s32 unk2C;              /* 0x2C accumulated y drift */
};

/**
 * @brief Header hanging off FieldScene offset 0.
 *
 * @note func_80054... reads only the 0x30 halfword; the streaming update in
 *       func_80056A04 also reads the strip pixel-source base at 0x04 and the
 *       column stride at 0x28.
 */
typedef struct
{
    u8 _pad0[4];
    u16 *unk4;             /* 0x04 strip pixel-source base */
    u8 _pad1[0x28 - 8];
    u16 unk28;             /* 0x28 column stride, in halfwords */
    u8 _pad2[0x30 - 0x2A];
    s16 unk30;             /* 0x30 */
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
    u16 unk4;
    /** 0x06 first point, vertical (halved before use). */
    u16 unk6;
    /** 0x08 second point, horizontal. */
    u16 unk8;
    /** 0x0A second point, vertical (halved before use). */
    u16 unkA;
    u8 _pad1[0x10 - 0xC];
    /** 0x10 depth bias added to the fixed 0xE0 vertical origin. */
    s16 unk10;
    u8 _pad2[0x14 - 0x12];
    /** 0x14 value rendered as the marker's numeric label. */
    u16 unk14;
} FieldMarkerDef;

/**
 * @brief Element of the scene's marker list (FieldScene offset 0x10).
 *
 * @note Only drawn when D_8003524C is set, so this is most likely a debug
 *       overlay rather than something the retail render path shows.
 */
typedef struct FieldMarker FieldMarker;
struct FieldMarker
{
    FieldMarker *next;      /* 0x00 */
    FieldMarkerDef *def;    /* 0x04 */
    /** 0x08 third point, horizontal. */
    u16 unk8;
    /** 0x0A third point, vertical (halved before use). */
    u16 unkA;
    /** 0x0C fourth point, horizontal. */
    u16 unkC;
    /** 0x0E fourth point, vertical (halved before use). */
    u16 unkE;
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
    FieldImageReq *next; /* 0x00 */
    RECT rect;           /* 0x04 destination rectangle in VRAM */
    u_long *data;        /* 0x0C source pixel data */
};

/** @brief Definition record shared by the animation and sequence lists. */
typedef struct
{
    u8 unk0;   /* 0x00 */
    u8 unk1;   /* 0x01 */
    u8 unk2;   /* 0x02 */
    u8 _pad0;
    u8 unk4;   /* 0x04 low three bits select the handler */
    u8 unk5;   /* 0x05 */
    u8 unk6;   /* 0x06 */
    u8 _pad1;
    u16 unk8;  /* 0x08 */
    u16 unkA;  /* 0x0A */
    u8 unkC;   /* 0x0C */
    u8 unkD;   /* 0x0D */
    u8 unkE;   /* 0x0E */
    u8 unkF;   /* 0x0F */
    u8 unk10;  /* 0x10 */
    u8 _pad2;
    u16 unk12; /* 0x12 */
    u8 *unk14; /* 0x14 */
} FieldAnimDef;

/**
 * @brief Tile grid referenced by a tile-blit animation definition.
 *
 * Reached two ways: through FieldTileAnimDef::grid (func_80057CA4) and through
 * FieldAnimCel::grid (func_800584DC). The word at 0x08 is read whole for its
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
    FieldTileDesc *tiles;
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
    FieldTileGrid *grid; /* 0x10 */
} FieldTileAnimDef;

/** @brief Element of an animation node's cel ring. */
/**
 * @brief One 4-byte entry of the scratchpad colour table at 0x1F800000.
 *
 * func_8005477C copies the whole entry into a tile record's rgb/code word;
 * func_800589F0 rewrites only the colour bytes, so it needs the halves named.
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

/** @brief Palette record reached through FieldTintSrc::unk4. */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 count halfword followed by the palette entries themselves. */
    u16 *unk4;
} FieldTintPal;

/**
 * @brief Colour source for the tile tint pass, hung off FieldAnim::unk10.
 *
 * The two halfword triples multiply component-wise into the three-word colour
 * func_8005AC50 expands into the scratchpad table at 0x1F800000.
 */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 record holding the palette this tint is built from. */
    FieldTintPal *unk4;
    u8 _pad1[0x10 - 8];
    u16 unk10; /* 0x10 red */
    u16 unk12; /* 0x12 green */
    u16 unk14; /* 0x14 blue */
    u16 unk16; /* 0x16 red scale */
    u16 unk18; /* 0x18 green scale */
    u16 unk1A; /* 0x1A blue scale */
} FieldTintSrc;

typedef struct FieldAnimCel FieldAnimCel;
struct FieldAnimCel
{
    FieldAnimCel *next; /* 0x00 */
    /** 0x04 grid this cel's bit plane and tile records are laid out on. */
    FieldTileGrid *grid;
    u8 _pad0[0xC - 8];
    /** 0x0C tile-presence bitmap, one bit per grid cell, LSB first. */
    u32 *mask;
    /** 0x10 packed destination tile records, advanced past every present tile. */
    u8 *tiles;
    u8 _pad1[0x18 - 0x14];
    /** 0x18 when set, the tile record is 4 bytes shorter. */
    s32 unk18;
    /** 0x1C when set, the tile record is 4 bytes shorter. */
    s32 unk1C;
    u8 unk20; /* 0x20 */
    /** 0x21 record-format selector, 0-6; see func_80057CA4. */
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
        u8 unk0;
        /** 0x25 current frame / cel index. */
        u8 state;
        u8 unk2;
        u8 unk3;
    } b;
} FieldAnimFlags;

/** @brief Element of the scene's animation lists (0x18/0x1C/0x20/0x24). */
typedef struct FieldAnim FieldAnim;
struct FieldAnim
{
    FieldAnim *next;      /* 0x00 */
    FieldAnimDef *def;    /* 0x04 */
    u8 _pad0[0xC - 8];
    FieldAnimCel *cels;   /* 0x0C */
    s32 unk10;            /* 0x10 */
    /** 0x14 last horizontal tween offset pushed to the target (see func_80057E88). */
    s32 unk14;
    /** 0x18 last vertical tween offset pushed to the target. */
    s32 unk18;
    /** 0x1C last depth tween offset pushed to the target. */
    s32 unk1C;
    /** 0x20 base of the per-frame packed tile records. */
    u8 *frames;
    FieldAnimFlags flags; /* 0x24 */
    u8 _pad2[0x2A - 0x28];
    u16 counter;          /* 0x2A */
    /** 0x2C tile records per frame, i.e. the stride from one frame to the next. */
    u16 frame_tiles;
    u8 _pad3[0x30 - 0x2E];
    FieldImageReq req;    /* 0x30 */
    u16 buf40[0x10];      /* 0x40 */
    u16 buf60[0xF0];      /* 0x60 */
    u16 buf240[1];        /* 0x240 */
};

/** @brief Element of the scene's sequence list (0x14). */
typedef struct FieldSeq FieldSeq;
struct FieldSeq
{
    FieldSeq *next;    /* 0x00 */
    FieldAnimDef *def; /* 0x04 */
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
    } rects[3];        /* 0x20 */
    u8 _pad1[0x98 - 0x38];
    u8 chunk_idx;      /* 0x98 */
    u8 _pad2[0x9D - 0x99];
    u8 frame_ready;    /* 0x9D */
    u8 _pad3;
    u8 end_state;      /* 0x9F */
} FieldMovieState;

/**
 * @brief Definition record shared by a FieldNode.
 *
 * unkA/unkC index the signed angle table pointed to by D_8018001C; unk10/unk12
 * are the horizontal/vertical base offsets (each shifted left by 8).
 */
typedef struct
{
    u8 _pad0[0xA];
    u16 unkA;  /* 0x0A angle-table index for the horizontal step */
    u16 unkC;  /* 0x0C angle-table index for the vertical step */
    u8 _pad1[0x10 - 0xE];
    s16 unk10; /* 0x10 horizontal base offset (<< 8) */
    s16 unk12; /* 0x12 vertical base offset (<< 8) */
} FieldNodeDef;

/**
 * @brief Element of the scene's attached-node list (FieldScene offset 0x08).
 *
 * Each node hangs off a FieldPart and carries a swept 2D position that
 * func_80057A28 recomputes every frame. The same list is walked by
 * field_clear_node_accumulators in field1.c.
 */
typedef struct FieldNode FieldNode;
struct FieldNode
{
    FieldNode *next;   /* 0x00 */
    FieldNodeDef *def; /* 0x04 */
    u8 _pad0[0xC - 8];
    FieldPart *part;   /* 0x0C owning part */
    u8 _pad1[0x28 - 0x10];
    s32 unk28;         /* 0x28 horizontal delta since the previous frame */
    s32 unk2C;         /* 0x2C vertical delta since the previous frame */
    u8 _pad2[0x38 - 0x30];
    s32 unk38;         /* 0x38 current horizontal position */
    s32 unk3C;         /* 0x3C current vertical position */
};

typedef struct
{
    FieldSceneHeader *unk0; /* 0x00 */
    FieldObj *head;         /* 0x04 head of the object list */
    FieldNode *nodes;       /* 0x08 head of the attached-node list */
    u8 _pad0[0x10 - 0xC];
    FieldMarker *markers;   /* 0x10 head of the marker list */
    FieldSeq *seqs;         /* 0x14 head of the sequence list */
    FieldAnim *anims;       /* 0x18 head of the animation list */
    FieldAnim *strips;      /* 0x1C head of the strip list */
    FieldAnim *sprites;     /* 0x20 head of the sprite list */
    FieldAnim *effects;     /* 0x24 head of the effect list */
    u8 _pad1[0x34 - 0x28];
    FieldImageReq *uploads; /* 0x34 head of the pending upload list */
} FieldScene;

typedef struct
{
    FieldScene *scene;
} FieldSceneGlobals;

/**
 * @brief Field memory-allocator state block at 0x801ED000.
 */
typedef struct
{
    /** 0x00 top of the allocated region. */
    u32 unk0;
    u8 _pad[0xC - 4];
    /** 0x0C base of the allocated region. */
    u32 unkC;
    /** 0x10 end of the first half of the region. */
    u32 unk10;
} FieldMemState;

/**
 * @brief Camera / scroll state block at 0x801ED480.
 *
 * The individual words are also referenced as the standalone symbols
 * D_801ED484 / D_801ED488 / D_801ED48C; func_80054CA8 uses BOTH forms and the
 * distinction is required to match, because it selects the addressing mode.
 */
typedef struct
{
    u8 _pad[4];
    s32 unk4;               /* 0x04 == D_801ED484 */
    s32 unk8;               /* 0x08 == D_801ED488 */
    s32 unkC;               /* 0x0C == D_801ED48C */
} FieldCamera;

extern FieldSceneGlobals g_field_scene;
extern s32 D_8003524C;
extern s32 D_801ED484;
extern s32 D_801ED488;
extern s32 D_801ED48C;

s32 rcos(s32);
s32 rsin(s32);
void func_8005538C(u32 *, u32 *);
void func_8005692C(FieldPart *, s32, s32 *, s32);

/**
 * @brief Size the field working buffer from the current scene's object list.
 *
 * Walks every object in the scene, derives a per-object multiplier from its
 * definition flags, then sums a per-part byte cost over each object's part
 * list. The total gets a 0xA000 header allowance, is clamped to a 0x12000
 * minimum, and is written back to the allocator state at 0x801ED000 as a
 * base / midpoint / top triple (the region is sized to twice the total).
 *
 * @note The four multipliers MUST be assigned to named locals inside the inner
 *       loop. Written inline as `part->count * (n * 0x18)`, gcc reassociates to
 *       `(part->count * 0x18) * n`, which is no longer loop-invariant, so
 *       nothing gets hoisted into the preheader and the multiplies are
 *       strength-reduced inside the loop instead (74.75%). Declaring them in
 *       the loop body lets loop.c hoist all four. See [CSE-05] in idioms.md.
 * @note The dispatch MUST be a `switch`, not an if/else chain (88.54%). gcc
 *       merges `case 2..5` into a single range node, giving a three-node
 *       decision tree that tests `== 1`, then `< 2`, then `< 6` -- and it emits
 *       the case bodies in case-label order after the tests, which an if/else
 *       chain cannot reproduce. No jump table is generated.
 * @note `kind` must be `s32`, not `u8`: `u8` compares unsigned (`sltiu`) where
 *       the target uses signed `slti` (98.23%).
 * @note `obj->def->flags` must be re-read for the `& 8` test rather than
 *       reusing the `flags` local; reusing it drops the second load (95.91%).
 * @note `total` must be accumulated in place (`total = total + 0xA000`, then
 *       clamped in place) rather than assigned to a second variable, which
 *       colors it into the wrong register (99.34%). See [ALLOC-17].
 * @note The 0x12000 limit must go through a variable. Compared directly,
 *       gcc rewrites `total < 0x12000` into the negated `0x11FFF < total`
 *       and flips the branch (99.28%).
 * @note Measured non-factor: `n * 2` vs `n << 1`, both 100%.
 *
 * @see decomp.me (100%) TODO
 */
void func_80054B1C(void)
{
    FieldMemState *state = (FieldMemState *) 0x801ED000;
    FieldObj *obj;
    FieldPart *part;
    s32 n;
    u32 total;
    u32 base;
    u32 lim;

    total = 0;
    obj = g_field_scene.scene->head;
    if (obj != 0)
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
            if (part != 0)
            {
                do
                {
                    s32 m18 = n * 0x18;
                    s32 m1C = n * 0x1C;
                    s32 m28 = n * 0x28;
                    s32 m34 = n * 0x34;

                    if (part->count != 0)
                    {
                        s32 kind = part->kind;

                        switch (kind)
                        {
                        case 0:
                            total += part->count * m18;
                            break;
                        case 1:
                            total += m1C;
                            break;
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                            total += part->count * m28;
                            break;
                        default:
                            total += part->count * m34;
                            break;
                        }
                    }
                    part = part->next;
                }
                while (part != 0);
            }
            obj = obj->next;
        }
        while (obj != 0);
    }
    total = total + 0xA000;
    base = state->unk0;
    lim = 0x12000;
    if (total < lim)
    {
        total = 0x12000;
    }
    state->unk10 = base + total;
    state->unkC = base;
    state->unk0 = base + total * 2;
}

/**
 * @brief Draw every visible part of every active field object.
 *
 * Walks the scene's object list; for each active object it derives a scroll
 * offset from the camera state (scaled per-axis by the object's definition,
 * optionally negated and wrapped to a power-of-two boundary), applies the
 * object's per-frame drift, then walks the object's part list and submits each
 * visible part to func_8005692C. Parts near a wrap boundary are submitted more
 * than once so they appear on both sides of the seam.
 *
 * @param arg0 Render target / context handle, forwarded to func_8005692C and
 *             func_8005538C.
 * @param arg1 TODO: opaque, forwarded unchanged as the 4th arg of
 *             func_8005692C and 2nd of func_8005538C.
 * @param arg2 Mode selector: 0 advances the per-frame drift; 2 forces the
 *             unscaled camera offsets.
 *
 * @warning **THIS FUNCTION IS NOT A MATCH (92.92%) AND MAY NOT BE FUNCTIONALLY
 *          EQUIVALENT.** It is committed as work in progress. Do not rely on
 *          its exact behaviour, and re-verify before building a release image.
 *          The running analysis lives in working/func_80054CA8/status.md,
 *          including eleven measured-and-retired probe classes.
 *
 * @note The signed divides come in TWO forms and the choice is per-site.
 *       `x / 256` yields the compact `bgez / addiu / sra` sequence, which is
 *       what the target uses inside the part loop and for D_801ED488 /
 *       D_801ED48C. The head divide of D_801ED484 and the px/py/pz divides use
 *       a two-block form that ONLY appears if the rounding is written out as a
 *       real `if/else`. Collapsing those to `/ 256` costs ~11 exact rows.
 * @note `params` is one 5-word array, not five locals: params[2..4] are written
 *       and never read here, and survive only because the array's address is
 *       passed to func_8005692C.
 * @note `FieldObjDef.flags` must be `u32` (`srl`, not `sra`) - worth 6 rows.
 * @note The mask must be computed BEFORE loading obj->unk28 / obj->unk2C in the
 *       two wrap blocks - worth 3 rows.
 * @note Measured and rejected, despite the target visibly doing it: writing
 *       `ox`/`oz` as accumulate-in-place (`ox = ox + obj->unk28`) scores 15-42
 *       exact rows WORSE. See status.md before retrying.
 *
 * @see decomp.me (92.92%) TODO
 */
void func_80054CA8(s32 arg0, s32 arg1, s32 arg2)
{
    s32 params[5];
    FieldObj *obj;
    FieldObjDef *def;
    FieldPart *part;
    s32 ox;
    s32 oy;
    s32 oz;
    s32 modX;
    s32 modY;
    s32 px;
    s32 py;
    s32 pz;

    modX = 0;
    modY = 0;
    params[2] = g_field_scene.scene->unk0->unk30;
    {
        s32 t = D_801ED484;
        s32 q;

        if (t >= 0)
        {
            q = t >> 8;
        }
        else
        {
            q = (t + 0xFF) >> 8;
        }
        params[3] = q;
    }
    params[4] = (D_801ED488 / 256 - D_801ED48C / 512) + 0xE0;
    obj = g_field_scene.scene->head;
    if (obj != 0)
    {
        do
        {
            if (obj->flags.word & 1)
            {
                def = obj->def;
                ox = 0;
                if (def->flags & 2)
                {
                    oy = 0;
                    oz = 0;
                }
                else
                {
                    u8 sx = def->unk1C;

                    if ((sx == 0x10) || (arg2 == 2))
                    {
                        ox = D_801ED484;
                    }
                    else
                    {
                        u8 mag = sx & 0x7F;
                        s32 v;

                        if (sx & 0x80)
                        {
                            v = -D_801ED484;
                        }
                        else
                        {
                            mag = def->unk1C;
                            v = D_801ED484;
                        }
                        ox = (v * mag) / 16;
                    }
                    {
                        u8 sy = def->unk1D;

                        if ((sy == 0x10) || (arg2 == 2))
                        {
                            oy = ((FieldCamera *) 0x801ED480)->unk8;
                            oz = ((FieldCamera *) 0x801ED480)->unkC;
                        }
                        else
                        {
                            s32 a;
                            s32 b;

                            if (sy & 0x80)
                            {
                                oy = (-D_801ED488 * (sy & 0x7F)) / 16;
                                a = -D_801ED48C;
                                b = def->unk1D & 0x7F;
                            }
                            else
                            {
                                a = def->unk1D;
                                oy = (D_801ED488 * a) / 16;
                                b = D_801ED48C;
                            }
                            oz = (b * a) / 16;
                        }
                    }
                }
                if (obj->flags.b.unk2 != 0)
                {
                    if (arg2 == 0)
                    {
                        obj->unk28 += (rcos(obj->flags.b.unk3 * 0x10) * obj->flags.b.unk2) / 256;
                        obj->unk2C -= (rsin(obj->flags.b.unk3 * 0x10) * obj->flags.b.unk2) / 256;
                    }
                    if (def->flags & 4)
                    {
                        s32 t;

                        modX = 0x10000 << ((def->flags >> 4) & 3);
                        t = obj->unk28;
                        if (t >= 0)
                        {
                            obj->unk28 = t & (modX - 1);
                        }
                        else
                        {
                            obj->unk28 = -(-t & (modX - 1));
                        }
                    }
                    if (def->flags & 8)
                    {
                        s32 t;

                        modX = 0x20000 << ((def->flags >> 6) & 3);
                        t = obj->unk2C;
                        if (t >= 0)
                        {
                            obj->unk2C = t & (modX - 1);
                        }
                        else
                        {
                            obj->unk2C = -(-t & (modX - 1));
                        }
                    }
                }
                {
                    s32 tx = ox + obj->unk28;
                    s32 tz = oz + obj->unk2C;

                    if (tx >= 0)
                    {
                        px = tx >> 8;
                    }
                    else
                    {
                        px = (tx + 0xFF) >> 8;
                    }
                    if (oy >= 0)
                    {
                        py = oy >> 8;
                    }
                    else
                    {
                        py = (oy + 0xFF) >> 8;
                    }
                    if (tz >= 0)
                    {
                        pz = tz >> 9;
                    }
                    else
                    {
                        pz = (tz + 0x1FF) >> 9;
                    }
                }
                part = obj->parts;
                if (part != 0)
                {
                    do
                    {
                        if ((part->unk20 != 0) && (part->count != 0))
                        {
                            params[0] = px + (obj->unk1C + part->unk28) / 256;
                            {
                                s32 a = (py - pz) + (obj->unk20 + part->unk2C) / 256;
                                s32 d = part->def->u.b.unkB * 0x10 - 0xE0;

                                a = a - (obj->unk24 + part->unk30) / 512;
                                params[1] = a - d;
                            }
                            if (def->flags & 4)
                            {
                                modX = 0x100 << ((def->flags >> 4) & 3);
                                if (params[0] >= 0)
                                {
                                    params[0] = params[0] & (modX - 1);
                                }
                                else
                                {
                                    params[0] = modX - (-params[0] & (modX - 1));
                                }
                            }
                            if (def->flags & 8)
                            {
                                modY = 0x100 << ((def->flags >> 6) & 3);
                                if (params[1] >= 0)
                                {
                                    params[1] = params[1] & (modY - 1);
                                }
                                else
                                {
                                    params[1] = modY - (-params[1] & (modY - 1));
                                }
                            }
                            func_8005692C(part, arg0, params, arg1);
                            if (def->flags & 4)
                            {
                                if (params[0] > 0)
                                {
                                    params[0] -= modX;
                                    func_8005692C(part, arg0, params, arg1);
                                    params[0] += modX;
                                }
                                if (!(def->flags & 0x30))
                                {
                                    s32 t = params[0] + modX;

                                    if (t < 0x140)
                                    {
                                        params[0] = t;
                                        func_8005692C(part, arg0, params, arg1);
                                        params[0] -= modX;
                                    }
                                }
                            }
                            if (def->flags & 8)
                            {
                                if (params[1] > 0)
                                {
                                    params[1] -= modY;
                                    func_8005692C(part, arg0, params, arg1);
                                }
                                if (def->flags & 4)
                                {
                                    if (params[0] > 0)
                                    {
                                        params[0] -= modX;
                                        func_8005692C(part, arg0, params, arg1);
                                        params[0] += modX;
                                    }
                                    if (!(def->flags & 0x30))
                                    {
                                        s32 t = params[0] + modX;

                                        if (t < 0x140)
                                        {
                                            params[0] = t;
                                            func_8005692C(part, arg0, params, arg1);
                                        }
                                    }
                                }
                            }
                        }
                        part = part->next;
                    }
                    while (part != 0);
                }
            }
            obj = obj->next;
        }
        while (obj != 0);
    }
    if (D_8003524C != 0)
    {
        func_8005538C((u32 *)arg0, (u32 *)arg1);
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
 * @note `prim` must be ONE pointer variable advanced in place, not a separate
 *       LINE_F4 and LINE_F2 local: two locals allocate two registers (a3/a1)
 *       where the target carries everything in t0. Splitting them costs 36
 *       exact rows.
 * @note The advance past the LINE_F2 must be its own statement before the
 *       label block, not an argument expression at the call (`prim + 1`):
 *       folding it into the call costs 3 rows.
 * @note `depth` must be assigned BEFORE setLineF4/setRGB0, not after. That one
 *       move is worth 8 exact rows (82.49% -> 92.51%): it lets sched1 hoist the
 *       `lh` above the primitive stores.
 * @note `depth` must exist at all. Written inline as `sy + (def->unk10 + 0xE0)`
 *       GCC reassociates to `(sy + 0xE0) + def->unk10` and hoists the constant
 *       add out of the loop; that costs 18 rows.
 * @note `scene` must be split out of the marker-list read: `g_field_scene.scene`
 *       and `scene->markers` are two statements straddling the scroll divides,
 *       which is what puts the `lw 0x10(a0)` after them. Merging them costs 7
 *       rows.
 * @note `shadow = shadow->next` belongs AFTER the call, not before it (2 rows).
 * @note The camera-Y divide needs its own `cam_y` temp and BOTH statements need
 *       the do/while(0) wrapper. The wrappers are not decoration: the loop
 *       notes they leave in the RTL stop the next global load from being
 *       hoisted across the divide's compare, which is what keeps `sx` in v0 and
 *       duplicates the shift into the delay slot. Dropping the second wrapper
 *       costs 17 rows; dropping the `cam_y` temp costs 6.
 * @note The tail is `addPrims`, not two hand-written `setaddr` calls, even
 *       though they expand to the same stores - the macro form gets the two
 *       mask constants into a0/a1 the way the target has them (7 rows).
 * @note Measured NON-factors, all 100% either way: `sy + depth` vs
 *       `depth + sy`, `count`/`mode` statement order, a plain `{ }` block or a
 *       block-local temp in place of either do/while(0), and `addPrims` spelled
 *       out as `setaddr(prev, getaddr(&ot[-1]))`.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005538C(u32 *cursor, u32 *ot)
{
    s16 pos[2];
    FieldScene *scene;
    FieldMarker *marker;
    FieldMarkerDef *def;
    LINE_F4 *prim;
    void *prev;
    s32 sx;
    s32 sy;
    s32 cam_y;
    s32 base_y;
    s32 depth;
    s32 value;
    s32 digits;
    u32 *ot_entry;

    prim = (LINE_F4 *)*cursor;
    prev = NULL;
    scene = g_field_scene.scene;
    sx = D_801ED484 / 256;
    do
    {
        cam_y = D_801ED488 / 256;
    }
    while (0);
    do
    {
        sy = cam_y - D_801ED48C / 512;
    }
    while (0);
    marker = scene->markers;
    if (marker != NULL)
    {
        ot_entry = ot - 1;
        do
        {
            def = marker->def;
            depth = def->unk10 + 0xE0;
            setLineF4(prim);
            setRGB0(prim, 0xFF, 0, 0);
            base_y = sy + depth;
            prim->x0 = def->unk4 + sx;
            prim->y0 = base_y - HALF_TOWARD_ZERO((s16)def->unk6);
            prim->x1 = def->unk8 + sx;
            prim->y1 = base_y - HALF_TOWARD_ZERO((s16)def->unkA);
            prim->x2 = marker->unkC + sx;
            prim->y2 = base_y - HALF_TOWARD_ZERO((s16)marker->unkE);
            prim->x3 = marker->unk8 + sx;
            prim->y3 = base_y - HALF_TOWARD_ZERO((s16)marker->unkA);
            if (prev != NULL)
            {
                setaddr(prev, prim);
            }
            prev = prim;
            prim = prim + 1;
            setLineF2((LINE_F2 *)prim);
            setRGB0(prim, 0xFF, 0, 0);
            prim->x0 = def->unk4 + sx;
            prim->y0 = base_y - HALF_TOWARD_ZERO((s16)def->unk6);
            prim->x1 = marker->unk8 + sx;
            prim->y1 = base_y - HALF_TOWARD_ZERO((s16)marker->unkA);
            setaddr(prev, prim);
            prev = prim;
            prim = (LINE_F4 *)((LINE_F2 *)prim + 1);
            pos[0] = def->unk4 + sx;
            pos[1] = base_y - HALF_TOWARD_ZERO((s16)def->unk6);
            value = def->unk14;
            digits = 2;
            if (def->unk14 < 0xA)
            {
                digits = 1;
            }
            prim = (LINE_F4 *)func_800AD208(ot_entry, prim, value, digits, pos);
            marker = marker->next;
        }
        while (marker != NULL);
    }
    if (prev != NULL)
    {
        addPrims(&ot[-1], (void *)*cursor, prev);
        *cursor = (u32)prim;
    }
}

/**
 * @brief Screen-space placement of the grid being drawn.
 *
 * func_8005571C only needs the origin; func_80055D20 also reads unk8/unkC/unk10
 * to derive the rotation centre for its non-default placement modes.
 */
typedef struct
{
    s32 x;     /* 0x00 */
    s32 y;     /* 0x04 */
    s32 unk8;  /* 0x08 */
    s32 unkC;  /* 0x0C */
    s32 unk10; /* 0x10 */
} FieldViewport;

/**
 * @brief GPU primitive as func_8005571C writes it: four raw words.
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
    s32 unk0;
    /** 0x04 rgb/code word used when the part has no global code word. */
    s32 unk4;
    /** 0x08 texture-page word tested against the running page code. */
    s32 unk8;
} FieldCellRec;

/**
 * @brief Colour view of a FieldCellRec, used by the tint pass.
 *
 * Names the two halves of FieldCellRec::unk4 that func_800589F0 writes on their
 * own: the rgb/code word's low halfword and its blue byte.
 */
typedef struct
{
    u8 _pad0[4];
    /** 0x04 red and green. */
    u16 rg;
    /** 0x06 blue. */
    u8 b;
} FieldCellTint;

/**
 * @brief Emit GPU primitives for one bit-plane driven 16x16 sprite grid.
 *
 * Walks @p part 's bit plane row-major. Each set bit consumes one record from
 * the part's record stream and emits a 16-byte len-3 primitive at the current
 * grid cell, preceded by an 8-byte len-1 texture-page primitive whenever the
 * page code changes. Emitted primitives accumulate into a local chain that is
 * spliced into @p ot_base 's ordering-table head for the current CLUT with
 * addPrims, both whenever the interpolated CLUT changes and once at the end.
 * Rows and columns outside the 320x224 viewport are skipped by consuming their
 * bits without emitting.
 *
 * Sibling of func_80055D20; both are reached from the func_8005692C dispatch
 * on FieldPart::kind.
 *
 * @param part Field part supplying the grid, the bit plane, the record stream
 *             and the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space origin of the grid, in pixels.
 * @param ot_base Base of the 8-byte-per-entry ordering-table head array,
 *                indexed by CLUT id.
 *
 * @note `step` is the record stride: 0xC, less 4 when a global code word makes
 *       the per-record copy unnecessary, less another 4 for a global page word.
 * @note The 0xFFFFFF masks and the `-1` loop sentinels are written as literals
 *       on purpose; loop.c hoists them into the loop preheaders, which is where
 *       the target's s2 and s7 come from. Naming them costs the match.
 * @note The two CLUT multiplies must be written out inline. The `col * clut`
 *       accumulators m2c reconstructs are loop.c strength reduction, not source.
 * @note `bit = 0;` must sit below the tpage test so that it shares a CSE block
 *       with the two NULL inits; that is what makes them copy from `bit`'s
 *       register instead of materialising zero again.
 * @note Both loop counters are seeded in two statements (`row = height;
 *       row = row - 1;` and the same for `col`/`width`) rather than one
 *       `height - 1`. The extra ref crosses the floor_log2 step in global.c's
 *       priority formula, which is what puts `row` in t9 and `col` in t0
 *       ([ALLOC-19]). Collapsing the row pair costs -29 exact rows; collapsing
 *       the col pair costs -21.
 * @note `idx = width; idx -= col;` must stay split for the same reason, and
 *       only pays off while `width` is a full-width `s32`: `u8 width` with the
 *       split is -52 exact, `s32 width` without it is -254. The two are one
 *       change, not two. SImode `width` needs no separate zero_extend, which
 *       shortens `clut`'s live range by exactly the insn that made it conflict
 *       with a1; the split then restores `col`'s ref count so it still outranks
 *       `clut` and keeps t0.
 * @note `width` may be `s32` or `u32` (both 100%); `s32` matches `height`.
 * @note `prim->code = last_code;` is duplicated into both arms, and the record
 *       loads are written as `last_code = (prim->code = ...)`, for the same
 *       allocation-priority reason.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005571C(FieldPart *part, s32 **cursor_ptr, FieldViewport *origin, s32 ot_base)
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
    FieldPrim *prim;
    u8 *cursor;
    u8 *chain;
    u8 *recp;
    s32 *bitp;
    s32 width;
    FieldPartDef *info;

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
    cursor = (u8 *) *cursor_ptr;
    y = origin->y;
    height = info->u.b.unkB;
    row = height;
    row = row - 1;
    width = info->u.b.unkA;
    chain = NULL;
    while (row != -1)
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
        col = col - 1;
        while (col != -1)
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
                uv_word = ((FieldCellRec *) recp)->unk0;
                if (uv_word != -1)
                {
                    if (interp != 0)
                    {
                        idx = width;
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
                                addPrims((FieldPrim *) ((clut_cur * 8) + ot_base), chain, prim);
                                chain = NULL;
                            }
                            clut_cur = clut;
                        }
                    }
                    if (tpage_word != 0)
                    {
                        prim = (FieldPrim *) cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32) cursor & 0xFFFFFF) | 0x01000000;
                            prim->code = tpage_word;
                            prim = (FieldPrim *) cursor;
                        }
                    }
                    else
                    {
                        prim = (FieldPrim *) cursor;
                        if (chain == NULL)
                        {
                            chain = cursor;
                            cursor += 8;
                            prim->tag = ((u32) cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec *) recp)->unk4);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec *) recp)->unk8);
                            }
                        }
                        else if (last_code != ((FieldCellRec *) recp)->unk8)
                        {
                            cursor += 8;
                            prim->tag = ((u32) cursor & 0xFFFFFF) | 0x01000000;
                            if (code_word != 0)
                            {
                                last_code = (prim->code = ((FieldCellRec *) recp)->unk4);
                            }
                            else
                            {
                                last_code = (prim->code = ((FieldCellRec *) recp)->unk8);
                            }
                        }
                        prim = (FieldPrim *) cursor;
                    }
                    cursor += 0x10;
                    prim->tag = ((u32) cursor & 0xFFFFFF) | 0x03000000;
                    if (code_word != 0)
                    {
                        prim->code = code_word;
                    }
                    else
                    {
                        prim->code = ((FieldCellRec *) recp)->unk4;
                    }
                    prim->xy = (x & 0xFFFF) | (y << 16);
                    prim->uv = uv_word;
                }
                recp += step;
            }
            bit <<= 1;
            x += 0x10;
            col--;
        }
        row--;
        y += 0x10;
    }
    if (chain != NULL)
    {
        addPrims((FieldPrim *) ((clut_cur * 8) + ot_base), chain, prim);
    }
    *cursor_ptr = (s32 *) cursor;
}

/**
 * @brief Arithmetic right shift that rounds toward zero instead of down.
 *
 * The generalisation of HALF_TOWARD_ZERO above. It names its argument three
 * times on purpose: gcc cannot CSE the two arms of the conditional across the
 * branch, so a nested use expands to the target's triplicated multiply chains.
 * Spelling these as `/ 256`, `/ 4096` and `/ 65536` instead gives a compact
 * two-branch expansion and loses 107 instructions in func_80055D20.
 *
 * @param v Signed value to shift.
 * @param n Shift amount, i.e. divide by 1 << n.
 * @return @p v divided by `1 << n`, rounded toward zero.
 */
#define SHIFT_TOWARD_ZERO(v, n) ((v) >= 0 ? ((v) >> (n)) : (((v) + ((1 << (n)) - 1)) >> (n)))

/**
 * @brief POLY_FT4 as func_80055D20 writes it: ten raw words.
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

/**
 * @brief Emit rotated, scaled POLY_FT4 primitives for one bit-plane sprite grid.
 *
 * Rotated sibling of func_8005571C, reached from the same func_8005692C
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
 * chain spliced into @p ot_base 's ordering-table head for the current CLUT with
 * addPrims, both when the interpolated CLUT changes and once at the end.
 *
 * @param part Field part: def gives the grid size and the placement mode, bits
 *             the bit plane, records the record stream, unk3A/3C/3E the rotation
 *             angles, unk40/42 the scales, unk44..4A the four corner CLUT ids.
 * @param cursor_ptr In/out primitive-buffer cursor; advanced past everything
 *                   emitted.
 * @param origin Screen-space placement; the mode selects which of its words
 *               form the rotation centre.
 * @param ot_base Base of the 8-byte-per-entry ordering-table head array,
 *                indexed by CLUT id.
 *
 * @note The placement mode is `(def->u.word >> 12) & 0xF`, and the switch needs
 *       an explicit `case 5:` falling into `default:` - that is what makes gcc
 *       emit a five-entry jump table whose last slot points at the default arm.
 * @note `scaled` is assigned the vertical term BEFORE the column-fill loop, and
 *       the loop immediately overwrites it. Those 14 instructions are dead, but
 *       gcc cannot delete them because `mult` clobbers HI/LO and is not a
 *       deletable dead insn. Removing the assignment costs the whole block.
 * @note `origin->unk8 / 2` really is a plain `/ 2` here (`srl 31 / addu / sra 1`),
 *       NOT the HALF_TOWARD_ZERO branch form used elsewhere in this file.
 * @note `prim = NULL; chain = NULL;` must sit BEFORE the four rcos/rsin calls.
 *       Placed after them, prim's live range starts after the calls, it crosses
 *       none, and it takes a caller-saved register; placed before, it crosses
 *       four calls and lands in s0 like the target (+26 exact rows).
 * @note The three point-fill loops reuse `col` rather than a separate index;
 *       the target allocates a2 for both them and the column loop.
 * @note Declaration order sets the spill-slot order (see func_8005571C's note):
 *       the locals are declared in the target's slot order, 0x10 recp through
 *       0x48 clut_right.
 * @note The visibility test is assigned to `visible` first rather than tested
 *       inline; the target materialises 0/1 in a register before branching.
 * @note `part->def` is re-read rather than cached in a local because the
 *       rcos/rsin calls clobber memory for gcc's alias model and kill the CSE.
 *
 * @see working/func_80055D20/status.md for the full match log and the residue.
 * @see decomp.me (92.08%) TODO
 */
void func_80055D20(FieldPart *part, s32 **cursor_ptr, FieldViewport *origin, s32 ot_base)
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
    s32 clut_right;
    /* TODO: not recovered source. A second pseudo for `height` is what pushes
       height out of a saved register and into its stack slot, which the target
       reads back twice per interpolation block. Removing it costs 27 rows.
       The real cause is register pressure; see status.md. */
    s32 hdiv;
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
    clut_cur = part->clut_tl;
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
    cursor = (u8 *) *cursor_ptr;
    prim = NULL;
    chain = NULL;
    width = part->def->u.b.unkA;
    height = part->def->u.b.unkB;
    cos_a = rcos(part->unk3A);
    cos_b = rcos(part->unk3C);
    sin_c = rsin(part->unk3E);
    cos_c = rcos(part->unk3E);
    hdiv = height;
    switch ((part->def->u.word >> 12) & 0xF)
    {
    case 1:
    case 2:
        cy = origin->unk10;
        cx = origin->unkC + (origin->unk8 / 2);
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 3:
        cx = origin->unkC;
        cy = origin->unk10;
        x_off = origin->x - cx;
        y_off = origin->y - cy;
        break;
    case 4:
        cx = origin->unk8 + origin->unkC;
        cy = origin->unk10;
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
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->unk42, 8) * cos_a, 12);
    steps = (FieldColStep *) 0x1F800000;
    for (col = width; col != -1; col--)
    {
        scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(x_off * part->unk40, 8) * cos_b, 12);
        x_off += 0x10;
        steps->sin_term = scaled * sin_c;
        steps->cos_term = scaled * cos_c;
        steps++;
    }
    scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->unk42, 8) * cos_a, 12);
    dx = scaled * sin_c;
    dy = scaled * cos_c;
    steps = (FieldColStep *) 0x1F800000;
    pt = (FieldPoint *) 0x1F800200;
    for (col = width; col != -1; col--)
    {
        pt->p.x = SHIFT_TOWARD_ZERO(steps->cos_term - dx, 16) + cx;
        pt->p.y = SHIFT_TOWARD_ZERO(steps->sin_term + dy, 16) + cy;
        steps++;
        pt++;
    }
    flip = 0;
    if (height != 0)
    {
        row = height - 1;
        do
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
                    clut_right = ((part->clut_tr * (row + 1)) + (part->clut_br * ((hdiv - row) - 1))) / hdiv;
                }
                else
                {
                    clut_right = part->clut_tr;
                }
            }
            if (flip == 0)
            {
                prev_row = (FieldPoint *) 0x1F800200;
                this_row = (FieldPoint *) 0x1F800300;
                flip = 1;
            }
            else
            {
                prev_row = (FieldPoint *) 0x1F800300;
                this_row = (FieldPoint *) 0x1F800200;
                flip = 0;
            }
            y_off += 0x10;
            scaled = SHIFT_TOWARD_ZERO(SHIFT_TOWARD_ZERO(y_off * part->unk42, 8) * cos_a, 12);
            dx = scaled * sin_c;
            dy = scaled * cos_c;
            steps = (FieldColStep *) 0x1F800000;
            pt = this_row;
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
                    visible = ((prev_row[0].p.x >= 0) || (prev_row[1].p.x >= 0) || (this_row[0].p.x >= 0) || (this_row[1].p.x >= 0))
                           && ((prev_row[0].p.y >= 0) || (prev_row[1].p.y >= 0) || (this_row[0].p.y >= 0) || (this_row[1].p.y >= 0))
                           && ((prev_row[0].p.x < 0x140) || (prev_row[1].p.x < 0x140) || (this_row[0].p.x < 0x140) || (this_row[1].p.x < 0x140))
                           && ((prev_row[0].p.y < 0xE0) || (prev_row[1].p.y < 0xE0) || (this_row[0].p.y < 0xE0) || (this_row[1].p.y < 0xE0));
                    if (visible != 0)
                    {
                        uv_word = ((FieldCellRec *) recp)->unk0;
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
                                        addPrims((FieldPolyPrim *) ((clut_cur * 8) + ot_base), chain, prim);
                                        chain = NULL;
                                    }
                                    clut_cur = clut;
                                }
                            }
                            prim = (FieldPolyPrim *) cursor;
                            if (chain == NULL)
                            {
                                chain = cursor;
                            }
                            cursor += 0x28;
                            prim->tag = ((u32) cursor & 0xFFFFFF) | 0x09000000;
                            if (code_word != 0)
                            {
                                prim->code = code_word;
                            }
                            else
                            {
                                prim->code = ((FieldCellRec *) recp)->unk4;
                            }
                            if (tpage_word != 0)
                            {
                                prim->uv1 = ((uv_word & 0xFFFF) + 0xF) | tpage_word;
                            }
                            else if (code_word != 0)
                            {
                                prim->uv1 = ((FieldCellRec *) recp)->unk4;
                            }
                            else
                            {
                                prim->uv1 = ((FieldCellRec *) recp)->unk8;
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
            row--;
        } while (row != -1);
    }
    if (chain != NULL)
    {
        addPrims((FieldPolyPrim *) ((clut_cur * 8) + ot_base), chain, prim);
    }
    *cursor_ptr = (s32 *) cursor;
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
 * @param key   Definition key to match on (FieldPartDef::unk0).
 * @return The matching FieldPart, or NULL if none qualifies - including when
 *         the only candidate found is @p part itself on @p obj.
 *
 * @note The whole body must be wrapped in `if (!(part->def->u.word & 0x80))`
 *       with ONE trailing `return NULL;`. Spelling it as an early
 *       `if (...) { return NULL; }` guard makes gcc emit a second `jr ra` tail
 *       and merges the in-loop return into it - the exact opposite of the
 *       target, which shares the guard's exit with the final return and keeps
 *       the in-loop one separate (89.70%).
 * @note The success test must be one `||` expression. Splitting it into two
 *       consecutive `if`s costs the shared tail (92.12%).
 * @note `obj->unk14` must be `u16`; `s16` turns the `lhu` pair into `lh`
 *       (98.18%).
 * @note Operand order is required on both equality tests: `key == p->def->unk0`
 *       (99.85% reversed) and `(obj == o) && (part == p)` (99.70% reversed).
 * @note Measured non-factor: the `want`/`have` temporaries are cosmetic -
 *       repeating `obj->def` and `o->def` inline is also 100%.
 *
 * @see decomp.me (100%) TODO
 */
FieldPart *func_80056824(FieldScene *scene, FieldObj *obj, FieldPart *part, s32 key)
{
    FieldObj *o;
    FieldPart *p;
    FieldObjDef *want;
    FieldObjDef *have;

    if (!(part->def->u.word & 0x80))
    {
        for (o = scene->head; o != NULL; o = o->next)
        {
            for (p = o->parts; p != NULL; p = p->next)
            {
                if (key == p->def->unk0)
                {
                    if ((obj == o) && (part == p))
                    {
                        return NULL;
                    }
                    if (!(p->def->u.word & 0x80))
                    {
                        want = obj->def;
                        have = o->def;
                        if ((want == have)
                         || ((want->unk4 == have->unk4) && (obj->unk10 == o->unk10) && (obj->unk14 == o->unk14)))
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
 * @param arg1 Primitive-buffer cursor, forwarded as func_8005571C's 2nd param.
 * @param arg2 Screen-space placement, forwarded as the 3rd param.
 * @param arg3 Ordering-table head array base, forwarded as the 4th param.
 *
 * @note **`case 1: break;` must be written out** even though it does nothing.
 *       gcc balances the switch's comparison tree around the median case node,
 *       so the presence of a do-nothing case 1 is what makes `beq v1, 1` the
 *       ROOT test; without it the tree re-balances around case 0 and the whole
 *       cascade changes (52.59%). The case set is readable straight off the
 *       tree: adding a case 6 also breaks it (47.41%).
 * @note It must be a `switch`. The equivalent
 *       `if (kind == 0) ... else if (kind >= 2 && kind < 6)` chain folds the
 *       range test into a single unsigned compare (47.22%) - see [EXPAND-09].
 * @note `kind` must stay `u8`; `s8` costs the zero-extend shape (97.78%).
 * @note Measured non-factor: adding `default:` alongside `case 1:` is also 100%.
 * @note The parameters keep the loose `(FieldPart *, s32, s32 *, s32)` shape of
 *       the forward declaration above, which func_80054CA8's six call sites are
 *       matched against; the casts at the two calls are free.
 *
 * @see decomp.me (100%) TODO
 */
void func_8005692C(FieldPart *part, s32 arg1, s32 *arg2, s32 arg3)
{
    switch (part->kind)
    {
    case 0:
        func_8005571C(part, (s32 **) arg1, (FieldViewport *) arg2, arg3);
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        func_80055D20(part, (s32 **) arg1, (FieldViewport *) arg2, arg3);
        break;
    }
}

/**
 * @brief Flush the scene's pending VRAM uploads.
 *
 * Walks the scene's upload list, issues each node's LoadImage, then empties the
 * list. Nodes are not freed - the list head is simply cleared.
 *
 * @note The `scene` local is required to match: LoadImage is an ordinary call,
 *       so gcc's alias model treats it as clobbering memory. Writing
 *       `g_field_scene.scene->uploads = NULL;` inline after the loop forces a
 *       reload of the global that the target does not have - it keeps the scene
 *       pointer in s1 across every call (68.17%).
 * @note Measured non-factors, all still 100%: `while` and guarded `do/while`
 *       loop forms, declaring `data` as `void *` and casting at the call, and
 *       replacing the inline `RECT rect;` member with a raw `(RECT *)(p + 4)`
 *       cast. The inline RECT member is kept because it is what makes
 *       `&req->rect` read naturally.
 *
 * @see decomp.me (100%) TODO
 */
void func_80056998(void)
{
    FieldScene *scene;
    FieldImageReq *req;

    scene = g_field_scene.scene;
    for (req = scene->uploads; req != NULL; req = req->next)
    {
        LoadImage(&req->rect, req->data);
    }
    scene->uploads = NULL;
}

/**
 * @brief Does nothing.
 *
 * @note The original is `jr $ra; nop` with no frame and no body. Nothing in the
 *       decompiled tree references it, and no data table holds its address, so
 *       neither its purpose nor its parameter list can be recovered; a `void`
 *       signature is a placeholder that happens to be codegen-correct, since an
 *       empty body ignores its arguments either way. func_800569FC directly
 *       below it is a second, identical stub - the pair is most likely two
 *       unused slots in a per-part hook set whose siblings do real work.
 *
 * @see decomp.me (100%) TODO
 */
void func_800569F4(void)
{
}

/**
 * @brief Does nothing.
 *
 * @note Byte-identical twin of func_800569F4 above; the same caveats apply.
 *       Unreferenced and unrecoverable as to purpose or parameters.
 *
 * @see decomp.me (100%) TODO
 */
void func_800569FC(void)
{
}

/** @brief Movie/streaming control block at 0x801ED500. */
#define FIELD_MOVIE ((volatile FieldMovieState *) 0x801ED500)
/** @brief Field CD/movie flag word at 0x801ED800. */
#define FIELD_CD_FLAGS (*(volatile s32 *) 0x801ED800)

extern u16 D_80140000;
extern u16 D_80140002;
/** @brief Pointer to the signed angle table indexed by FieldNodeDef::unkA/unkC. */
extern s16 *D_8018001C;

void func_800157B0(s32);
s32 cdrom_process_state(void);
void cdrom_reset(void);
void cdrom_stream(s32, void *);
void cdrom_queue_seek(s32);
void cdrom_queue_read(s32, void *);
s32 cdrom_can_queue_resource(s32);
void func_80057A28(FieldPart *);
void func_80057CA4(FieldAnimDef *, FieldAnim *, s32);
void func_80057E88(FieldAnimDef *, FieldAnim *, s32);
void func_80058154(FieldAnimDef *, FieldAnim *);
void func_800584DC(FieldAnimDef *, FieldAnimCel *, s32);
u_long *func_8005866C(FieldAnimDef *, FieldAnim *);
void func_800589F0(FieldAnimDef *, FieldAnimCel *, FieldTintSrc *, s32);
void func_80058C00(FieldAnimDef *, FieldAnimCel *, s32);
void func_80058E28(FieldAnimDef *, FieldAnim *);
void func_800591C4(FieldAnimDef *, FieldAnimCel *, s32);
void func_80059294(FieldImageReq *);
void func_80059F18(void);
void func_8005A744(FieldSeq *, s32);
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
 * object parts get a per-part hook (func_80057A28); animation nodes drive a
 * CD/MDEC movie state machine and per-handler dispatch; strip nodes copy their
 * pixel source out of the scene header (hdr->unk4) into per-node scratch buffers
 * and queue a VRAM upload (func_80059294); sprite/effect nodes tick their
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
void func_80056A04(void)
{
    FieldScene *scene;
    FieldSceneHeader *hdr;
    FieldObj *obj;
    FieldPart *part;
    FieldAnim *anim;
    FieldAnimDef *def;
    FieldAnimDef *def2;
    FieldAnimDef *def3;
    FieldAnimCel *cel;
    FieldImageReq *req;
    FieldSeq *seq;
    FieldSeq *walk;
    FieldAnimDef *rec;
    u16 *src;
    u16 *dst;
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

    obj = scene->head;
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
                            func_80057A28(part);
                        }
                    }
                    part = part->next;
                } while (part != NULL);
            }
            obj = obj->next;
        } while (obj != NULL);
    }

    anim = scene->anims;
    hdr = scene->unk0;
    if (anim != NULL)
    {
        do
        {
            def = anim->def;
            def2 = anim->def;
            if (anim->flags.word & 0x20)
            {
                req = &anim->req;
                if ((*(s32 *) &def->unk4 & 7) == 3)
                {
                    req->rect.x = def->unkC * 4 + 0x140;
                    req->rect.y = def->unkD * 0x10 + 0x100;
                    req->rect.w = def->unkE * 4;
                    req->rect.h = def->unkF * 0x10;
                    req->data = (u_long *) (def->unk14 + ((anim->flags.b.state * def->unkE * def->unkF) << 7));
                    func_80059294(req);
                }
                anim->flags.word &= ~0x20;
            }
            if (anim->flags.word & 0x40)
            {
                anim->counter--;
                switch (def->unk4 & 7)
                {
                case 4:
                    switch (anim->flags.b.state)
                    {
                    case 0:
                        if (cdrom_process_state() == 0)
                        {
                            cdrom_stream(0xB, (void *) 0x80140000);
                            cdrom_queue_seek(def->unk1 * 2 + 0x16A6);
                            anim->flags.b.state = 1;
                            FIELD_CD_FLAGS |= 0x40;
                        }
                        /* fallthrough */
                    case 1:
                        if (cdrom_can_queue_resource(def->unk1 * 2 + 0x16A6) != 0)
                        {
                            FIELD_MOVIE->rects[0].x = def2->unkC * 4 + 0x140;
                            FIELD_MOVIE->rects[0].y = def2->unkD * 0x10 + 0x100;
                            FIELD_MOVIE->rects[0].w = def2->unkE * 4;
                            FIELD_MOVIE->rects[0].h = def2->unkF * 0x10;
                            FIELD_CD_FLAGS &= ~0x40;
                            cel = anim->cels;
                            if (def->unk1 < 2)
                            {
                                func_80140358(def->unk1 * 2 + 0x16A6, 1, def->unk5 - 2, cel->unk20);
                            }
                            else
                            {
                                func_80140358(def->unk1 * 2 + 0x16A6, 1, 0x12E, 0);
                            }
                            anim->flags.b.state = 2;
                        }
                        anim->counter = 1;
                        break;
                    default:
                        if (FIELD_MOVIE->end_state >= 3)
                        {
                            if (FIELD_MOVIE->end_state == 3)
                            {
                                if (cdrom_can_queue_resource(def->unk1 * 2 + 0x16A7) != 0)
                                {
                                    req = &anim->req;
                                    if (def->unk1 < 2)
                                    {
                                        cel = anim->cels;
                                        req->rect.x = FIELD_MOVIE->rects[cel->unk20].x;
                                        req->rect.y = FIELD_MOVIE->rects[cel->unk20].y;
                                        req->rect.w = FIELD_MOVIE->rects[cel->unk20].w;
                                        req->rect.h = FIELD_MOVIE->rects[cel->unk20].h;
                                        req->data = (u_long *) 0x80140000;
                                        func_80059294(req);
                                        if (cel->unk20 == 1)
                                        {
                                            cel->unk20 = 0;
                                            cel = cel->next;
                                            cel->unk20 = 1;
                                        }
                                        else
                                        {
                                            cel->unk20 = 1;
                                            cel = cel->next;
                                            cel->unk20 = 0;
                                        }
                                    }
                                    else
                                    {
                                        cel = anim->cels;
                                        req->rect.x = 0x140;
                                        req->rect.y = 0x100;
                                        req->data = (u_long *) 0x80140004;
                                        req->rect.w = D_80140000;
                                        req->rect.h = D_80140002;
                                        func_80059294(req);
                                        cel->unk20 = 0;
                                        cel = cel->next;
                                        cel->unk20 = 0;
                                    }
                                    FIELD_MOVIE->end_state = 4;
                                }
                                anim->counter = 1;
                            }
                            else
                            {
                                if (def->unk1 >= 2)
                                {
                                    func_80059F18();
                                }
                                anim->flags.word &= ~0x40;
                                anim->counter = 1;
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
                            if (FIELD_MOVIE->frame_ready == 1)
                            {
                                cel = anim->cels;
                                if (FIELD_MOVIE->chunk_idx == 1)
                                {
                                    cel->unk20 = 1;
                                    cel = cel->next;
                                    cel->unk20 = 0;
                                }
                                else
                                {
                                    cel->unk20 = 0;
                                    cel = cel->next;
                                    cel->unk20 = 1;
                                }
                                FIELD_MOVIE->frame_ready = 0;
                            }
                            if (FIELD_MOVIE->end_state == 2)
                            {
                                cdrom_reset();
                                cdrom_queue_read(def->unk1 * 2 + 0x16A7, (void *) 0x80140000);
                                FIELD_MOVIE->end_state = 3;
                            }
                            anim->counter = 1;
                        }
                        break;
                    }
                    break;
                case 5:
                case 6:
                    func_80057E88(def, anim, 1);
                    break;
                }
            }
            if (anim->counter == 0)
            {
                prev_state = anim->flags.b.state;
                func_80058E28(def, anim);
                switch (def->unk4 & 7)
                {
                case 0:
                    if (prev_state != anim->flags.b.state)
                    {
                        func_80057CA4(def, anim, anim->flags.b.state);
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
                    cel->unk20 = 0;
                    cel = anim->cels;
                    i = anim->flags.b.state;
                    i--;
                    while (i != -1)
                    {
                        cel = cel->next;
                        i--;
                    }
                    cel->unk20 = 1;
                    break;
                case 3:
                    req = &anim->req;
                    req->rect.x = def2->unkC * 4 + 0x140;
                    req->rect.y = def2->unkD * 0x10 + 0x100;
                    req->rect.w = def2->unkE * 4;
                    req->rect.h = def2->unkF * 0x10;
                    req->data = (u_long *) (def2->unk14 + ((anim->flags.b.state * def2->unkE * def2->unkF) << 7));
                    func_80059294(req);
                    break;
                case 5:
                case 6:
                    while (anim->counter == 0)
                    {
                        func_80057E88(def, anim, 1);
                        func_80058E28(def, anim);
                    }
                    break;
                case 7:
                    func_80058154(def, anim);
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
                switch (def->unk4 & 7)
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
                    req->data = (u_long *) dst;
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
                        src = (u16 *) ((u8 *) hdr->unk4 + (def3->unkE << 5) + def3->unkF * 2 + count2 * 2);
                    }
                    else
                    {
                        src = (u16 *) ((u8 *) hdr->unk4 + (def3->unkE << 9) + def3->unkF * 2 + count2 * 2);
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
                            src = (u16 *) ((u8 *) hdr->unk4 + (def3->unkE << 5) + def3->unkF * 2);
                        }
                        else
                        {
                            src = (u16 *) ((u8 *) hdr->unk4 + (def3->unkE << 9) + def3->unkF * 2);
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
                    func_80059294(req);
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
                            req->data = (u_long *) ((u8 *) hdr->unk4 + (def->unkE << 5) + (def->unkF & 0xE) * 2);
                        }
                        else
                        {
                            req->data = (u_long *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + def->unk12 * 2 +
                                                    ((anim->flags.b.state - 1) * def->unk10) * 2);
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
                            req->data = (u_long *) ((u8 *) hdr->unk4 + (def->unkE << 9) + def->unkF * 2);
                        }
                        else
                        {
                            req->data = (u_long *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + def->unk12 * 2 +
                                                    ((anim->flags.b.state - 1) * def->unk10) * 2);
                        }
                    }
                    func_80059294(req);
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
                            req->data = (u_long *) ((u8 *) hdr->unk4 + (def->unkE << 5));
                        }
                        else
                        {
                            req->data = (u_long *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + def->unk12 * 2 +
                                                    (((anim->flags.b.state - 1) * def->unk10) << 5));
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
                            req->data = (u_long *) ((u8 *) hdr->unk4 + (def->unkE << 9));
                        }
                        else
                        {
                            req->data = (u_long *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + def->unk12 * 2 +
                                                    (((anim->flags.b.state - 1) * def->unk10) << 9));
                        }
                    }
                    func_80059294(req);
                    break;
                case 5:
                    req->data = func_8005866C(def, anim);
                    do {
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
                    func_80059294(req);
                    } while (0);
                    break;
                }
                anim->flags.word &= ~0x20;
            }
            if (anim->flags.word & 0x40)
            {
                anim->counter--;
                if ((*(s32 *) &def->unk4 & 7) == 5)
                {
                    anim->flags.word |= 0x20;
                }
                if (anim->counter == 0)
                {
                    func_80058E28(def, anim);
                    switch (def->unk4 & 7)
                    {
                    case 0:
                        func_800584DC(def, anim->cels, anim->flags.b.state);
                        break;
                    case 1:
                        func_800591C4(def, anim->cels, anim->flags.b.state);
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
                if (--anim->counter == 0)
                {
                    func_80058E28(def, anim);
                    switch (def->unk4 & 7)
                    {
                    case 0:
                        func_800589F0(def, anim->cels, (FieldTintSrc *) anim->unk10, anim->flags.b.state);
                        break;
                    case 1:
                        func_80058C00(def, anim->cels, anim->flags.b.state);
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
                if (--anim->counter == 0)
                {
                    func_80058E28(def, anim);
                    func_80057CA4(def, anim, anim->flags.b.state);
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
                        func_8005A744(walk, ((u8 *) &seq->flags)[1]);
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
                    func_8005A744(walk, ((u8 *) &seq->flags)[1]);
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
 *       field8.c lives in overlay_field_gcc_g4_noexpand_srcs.
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
 * @note `arr = D_8018001C` is read at the top of the node block so its load fills
 *       the load-delay slot after the mode reload and hoists above the base
 *       select, matching the target scheduling.
 * @note The `(s16)(u16)` cast on the halved unk30 read is a non-factor here: the
 *       plain `scene->unk0->unk30 / 2` (s16 field) also matches.
 *
 * @see decomp.me (100%) TODO
 */
void func_80057A28(FieldPart *part)
{
    FieldScene *scene;
    FieldNode *node;
    FieldNodeDef *def;
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
    s16 *arr;
    s16 *ep;

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
        part->unk3E = sin_val / divisor;
    }
    else
    {
        part->unk3E = 0x1000 - ((u32) -sin_val / divisor);
    }
    neg_sin = -rsin(part->unk3E);
    if (part->node_count != 0)
    {
        arr = D_8018001C;
        mode = (part->def->u.word >> 12) & 0xF;
        switch (mode)
        {
        case 1:
        case 2:
            base = scene->unk0->unk30 / 2;
            break;
        case 3:
            base = 0;
            break;
        case 4:
        default:
            base = scene->unk0->unk30;
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
                    ep = &arr[def->unkA * 2];
                    x = (*ep - val) * neg_sin;
                    val = x >> 4;
                    if (x < 0)
                    {
                        val = (x + 0xF) >> 4;
                    }
                    y = def->unk10 << 8;
                    if ((val + y) < 0)
                    {
                        val = -y;
                    }
                    old = node->unk38;
                    node->unk38 = val;
                    node->unk28 = val - old;
                    ep = &arr[def->unkC * 2];
                    x = (*ep - base) * neg_sin;
                    val = x >> 4;
                    if (x < 0)
                    {
                        val = (x + 0xF) >> 4;
                    }
                    y = def->unk12 << 8;
                    if ((val + y) < 0)
                    {
                        val = -y;
                    }
                    old = node->unk3C;
                    count -= 1;
                    node->unk3C = val;
                    node->unk2C = val - old;
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
 * @param state Frame index into FieldAnim::frames.
 *
 * @note Built -G4 WITHOUT --expand-div, like the rest of field8.c.
 */
void func_80057CA4(FieldAnimDef *def, FieldAnim *anim, s32 state)
{
    FieldAnimCel *cel;
    FieldTileGrid *grid;
    u8 *dst;
    u32 *src;
    u32 *mask;
    u32 word;
    u32 bit;
    s32 stride;
    s32 row;
    s32 col;
    s32 i;

    cel = anim->cels;
    grid = ((FieldTileAnimDef *) def)->grid;
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
    if (cel->unk1C != 0)
    {
        stride -= 4;
    }
    if (cel->unk18 != 0)
    {
        stride -= 4;
    }
    src = (u32 *) (anim->frames + anim->frame_tiles * stride * state);
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
                            *(u32 *) dst = *src++;
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
 * @brief One entry of an animation's keyframe table (FieldAnimDef::unk14).
 *
 * The three signed deltas are the horizontal / vertical / depth offsets the
 * keyframe ends on; they are scaled by the fraction of the keyframe elapsed so
 * far. Only bit 15 of the trailing halfword is used.
 */
typedef struct
{
    /** 0x00 horizontal end offset. */
    s16 unk0;
    /** 0x02 vertical end offset. */
    s16 unk2;
    /** 0x04 depth end offset. */
    s16 unk4;
    /** 0x06 bit 15 is copied to the target's visibility flag. */
    u16 unk6;
} FieldTweenKey;

/**
 * @brief Count-table record returned by func_80059224.
 *
 * Only the duration is read here; func_80057CA4's caller uses the same halfword
 * to reload FieldAnim::counter.
 */
typedef struct
{
    u8 _pad0;
    /** 0x01 running total of the spans before this one, in frames. */
    u8 unk1;
    /** 0x02 length of the keyframe this record covers, in frames. */
    u16 unk2;
} FieldTweenSpan;

u8 *func_80059224(FieldAnimDef *, s32, volatile s8 *);
void func_8005A984(FieldPart *, s32, s32);
void func_8005AA68(FieldObj *, s32, s32);

/**
 * @brief Apply the current keyframe's tweened offsets to an animation's target.
 *
 * Resolves the keyframe indexed by FieldAnim::flags.b.state and the keyframe's
 * duration (func_80059224), then interpolates each of the key's three offsets by
 * the fraction of the keyframe already elapsed: `(elapsed * end << 8) / duration`.
 *
 * The target is FieldAnim::cels reinterpreted according to the definition's
 * handler kind: kind 5 drives a FieldPart (offsets 0x28/0x2C/0x30, visibility
 * byte 0x20), every other kind drives a FieldObj (offsets 0x1C/0x20/0x24,
 * visibility bit 0 of the flags word).
 *
 * @param def    Animation definition; supplies the handler kind and keyframe table.
 * @param anim   Animation node holding the frame state, counter and target.
 * @param commit When zero the tweened values are only recorded on the node; when
 *               non-zero the delta since the previous frame is also added to the
 *               target and pushed through func_8005A984 / func_8005AA68. The
 *               record is cleared instead of stored on the keyframe's last frame
 *               (counter == 0), so the next keyframe starts from zero.
 *
 * @note The keyframe table is read through @c rec rather than @c def. Both hold
 *       the same pointer, but folding the access onto @c def costs the match.
 * @note @c obj and @c part must be cleared by two separate statements; the
 *       chained @c obj = part = NULL form does not match.
 * @note The handler kind is read as a whole word (@c *(s32 *) &def->unk4): the
 *       byte access gcc emits for @c def->unk4 is an @c lbu, the target an @c lw.
 *       Repeating the test at each of the four sites is also required - hoisting
 *       it into a local reorders the blocks.
 * @note @c unk6 must stay unsigned so the visibility bit comes out as @c srl 15.
 *
 * @see decomp.me (100%) TODO
 */
void func_80057E88(FieldAnimDef *def, FieldAnim *anim, s32 commit)
{
    FieldAnimDef *rec;
    FieldObj *obj;
    FieldPart *part;
    FieldTweenKey *key;
    s32 duration;
    s32 elapsed;
    s32 value;
    s32 delta;
    volatile s8 base;

    rec = def;
    obj = NULL;
    part = NULL;
    if ((*(s32 *) &def->unk4 & 7) == 5)
    {
        part = (FieldPart *) anim->cels;
    }
    else
    {
        obj = (FieldObj *) anim->cels;
    }
    key = (FieldTweenKey *) (rec->unk14 + anim->flags.b.state * 8);
    duration = ((FieldTweenSpan *) func_80059224(def, anim->flags.b.unk2, &base))->unk2;
    if (duration == 0)
    {
        duration = 1;
    }
    elapsed = duration - anim->counter;
    if ((*(s32 *) &def->unk4 & 7) == 5)
    {
        part->unk20 = key->unk6 >> 15;
    }
    else
    {
        obj->flags.word = (obj->flags.word & ~1) | (key->unk6 >> 15);
    }

    value = ((elapsed * key->unk0) << 8) / duration;
    if (commit != 0)
    {
        delta = value - anim->unk14;
        if ((*(s32 *) &def->unk4 & 7) == 5)
        {
            part->unk28 += delta;
            func_8005A984(part, delta, 0);
        }
        else
        {
            obj->unk1C += delta;
            func_8005AA68(obj, delta, 0);
        }
        if (anim->counter == 0)
        {
            anim->unk14 = 0;
        }
        else
        {
            anim->unk14 = value;
        }
    }
    else
    {
        anim->unk14 = value;
    }

    value = ((elapsed * key->unk2) << 8) / duration;
    if (commit != 0)
    {
        delta = value - anim->unk18;
        if ((*(s32 *) &def->unk4 & 7) == 5)
        {
            part->unk2C += delta;
            func_8005A984(part, delta, 1);
        }
        else
        {
            obj->unk20 += delta;
            func_8005AA68(obj, delta, 1);
        }
        if (anim->counter == 0)
        {
            anim->unk18 = 0;
        }
        else
        {
            anim->unk18 = value;
        }
    }
    else
    {
        anim->unk18 = value;
    }

    value = ((elapsed * key->unk4) << 8) / duration;
    if (commit != 0)
    {
        delta = value - anim->unk1C;
        if ((*(s32 *) &def->unk4 & 7) == 5)
        {
            part->unk30 += delta;
            func_8005A984(part, delta, 2);
        }
        else
        {
            obj->unk24 += delta;
            func_8005AA68(obj, delta, 2);
        }
        if (anim->counter == 0)
        {
            anim->unk1C = 0;
        }
        else
        {
            anim->unk1C = value;
        }
    }
    else
    {
        anim->unk1C = value;
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
 * @brief One entry of the sound keyframe table at FieldAnimDef::unk14.
 *
 * Shares the 8-byte stride with FieldTweenKey; the low three bits of byte 0
 * say which of the two an entry is (1 = sound).
 */
typedef struct
{
    /**
     * 0x00 bits 0-2 entry kind (1 = sound); bits 8-12 a channel-slot number
     * (zero means "use the sound id at unk4" instead); bit 14 clear selects
     * positional playback; bit 15 clear stops the channel.
     */
    FieldSfxWord unk0;
    /**
     * 0x02 low byte is the sound's bank/index, bits 8-14 its base attenuation
     * and bit 15 selects one-shot playback over the a1/a3 pair.
     */
    FieldSfxWord unk2;
    /** 0x04 bits 0-9 sound id, used when unk0 carries no channel slot. */
    u16 unk4;
    u16 unk6;  /* 0x06 */
} FieldSfxKey;

void akao_play_sfx(s32, s32, s32, s32);
void akao_cmd_21(s32, s32);
void akao_cmd_a1(s32, s32, s32, s32);
void akao_cmd_a3(s32, s32, s32, s32);

/**
 * @brief Play or update the sound attached to an animation's current keyframe.
 *
 * Reads the keyframe indexed by FieldAnim::flags.b.state out of the table at
 * FieldAnimDef::unk14 and does nothing unless it is a sound entry (kind 1).
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
void func_80058154(FieldAnimDef *def, FieldAnim *anim)
{
    FieldPart *part;
    FieldObj *obj;
    FieldSfxKey *key;
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

    part = (FieldPart *) anim->cels;
    obj = (FieldObj *) anim->unk10;
    kind = def->unk14[anim->flags.b.state * 8] & 7;
    if (kind == 1)
    {
        key = (FieldSfxKey *) (def->unk14 + anim->flags.b.state * 8);
        if (key->unk0.word & 0x1F00)
        {
            chan_mask = kind << (((key->unk0.word >> 8) & 0x1F) - 1);
            sfx_id = 0;
        }
        else
        {
            chan_mask = 0;
            sfx_id = key->unk4 & 0x3FF;
        }
        if (key->unk0.word & 0x8000)
        {
            if (key->unk0.word & 0x4000)
            {
                if (key->unk2.word & 0x8000)
                {
                    if (anim->flags.word & 8)
                    {
                        akao_play_sfx(sfx_id, chan_mask, key->unk2.b.lo, (key->unk2.word >> 8) & 0x7F);
                        anim->flags.word &= ~8;
                    }
                }
                else
                {
                    akao_play_sfx(sfx_id, chan_mask, key->unk2.b.lo, (key->unk2.word >> 8) & 0x7F);
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
                    cam_x = ((FieldCamera *) 0x801ED480)->unk4;
                    cam_y = ((FieldCamera *) 0x801ED480)->unk8;
                    cam_z = ((FieldCamera *) 0x801ED480)->unkC;
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
                col = part->def->u.b.unkA * 8;
                x = x + (obj->unk1C + part->unk28) / 256 + col;
                row = part->def->u.b.unkB * 8;
                y = y + ((obj->unk20 + part->unk2C) * 2 - (obj->unk24 + part->unk30)) / 512;
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
                    tmp = (key->unk2.word >> 8) & 0x7F;
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
                    tmp = (key->unk2.word >> 8) & 0x7F;
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
                    att = (key->unk2.word >> 8) & 0x7F;
                }
                if (key->unk2.word & 0x8000)
                {
                    if (anim->flags.word & 8)
                    {
                        akao_play_sfx(sfx_id, chan_mask, vol, att);
                        anim->flags.word &= ~8;
                    }
                    else
                    {
                        akao_cmd_a1(sfx_id, chan_mask, anim->counter * 2, att);
                        akao_cmd_a3(sfx_id, chan_mask, anim->counter * 2, vol);
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
 * band of VRAM holding frame @p state. Only tiles whose descriptor row falls
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
 * @param state    Frame index; scales the band stride to reach frame @p state.
 *
 * @note @c def is a local copy of the parameter rather than the parameter
 *       itself. The extra reference is what pushes @p state out to s0 and keeps
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
void func_800584DC(FieldAnimDef *anim_def, FieldAnimCel *cel, s32 state)
{
    FieldAnimDef *def;
    FieldTileGrid *grid;
    FieldTileDesc *tile;
    u8 *dst;
    s16 *clut_ptr;
    u32 *mask;
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
        if (cel->unk1C != 0)
        {
            stride -= 4;
        }
        bit = 1;
        if (cel->unk18 != 0)
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
                    clut_ptr = (s16 *) (dst + 2);
                    do
                    {
                        if (word & bit)
                        {
                            u8 packed = tile->unk0;

                            if (packed & 0x80)
                            {
                                slot = packed & 0x1F;
                                if ((slot >= first) && (slot < last))
                                {
                                    y = slot + (state * def->unk10);
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
                            clut_ptr = (s16 *) ((u8 *) clut_ptr + stride);
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
                    }
                    while (col != grid->u.b.cols);
                }
                row++;
            }
            while (row != grid->u.b.rows);
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
 * @note @c rec is a local copy of @p def, as in func_80057E88: the extra
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
u_long *func_8005866C(FieldAnimDef *def, FieldAnim *anim)
{
    FieldAnimDef *rec;
    FieldSceneHeader *hdr;
    u16 *cur;
    u16 *prev;
    u16 *dst;
    u16 *out;
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
    hdr = g_field_scene.scene->unk0;
    total = ((FieldTweenSpan *) func_80059224(rec, anim->flags.b.unk2, (volatile s8 *) &base))->unk2;
    idx = anim->flags.b.unk2;
    do
    {
        remain = anim->counter;
        flags = anim->flags.word;
    }
    while (0);
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
    if (*(s32 *) &def->unk4 & 0x40)
    {
        other = (((FieldTweenSpan *) func_80059224(def, idx, (volatile s8 *) &base))->unk1 + idx) - base;
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
            cur = (u16 *) ((u8 *) hdr->unk4 + (rec->unkE << 5));
        }
        else
        {
            cur = (u16 *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + rec->unk12 * 2 +
                           ((anim->flags.b.state - 1) * step) * 2);
        }
        if (other == 0)
        {
            prev = (u16 *) ((u8 *) hdr->unk4 + (rec->unkE << 5));
        }
        else
        {
            prev = (u16 *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + rec->unk12 * 2 +
                            ((other - 1) * step) * 2);
        }
    }
    else
    {
        step = rec->unk10 << 8;
        if (anim->flags.b.state == 0)
        {
            cur = (u16 *) ((u8 *) hdr->unk4 + (rec->unkE << 9));
        }
        else
        {
            cur = (u16 *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + rec->unk12 * 2 +
                           ((anim->flags.b.state - 1) * step) * 2);
        }
        if (other == 0)
        {
            prev = (u16 *) ((u8 *) hdr->unk4 + (rec->unkE << 9));
        }
        else
        {
            prev = (u16 *) ((u8 *) hdr->unk4 + hdr->unk28 * 2 + rec->unk12 * 2 +
                            ((other - 1) * step) * 2);
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
            *dst = ((c | p) & 0x8000) |
                   ((((c & 0x1F) * remain) + ((p & 0x1F) * elapsed)) / total) |
                   (((((c >> 5) & 0x1F) * remain) + (((p >> 5) & 0x1F) * elapsed)) / total) << 5 |
                   (((((c >> 10) & 0x1F) * remain) + (((p >> 10) & 0x1F) * elapsed)) / total) << 10;
        }
        step--;
        dst++;
    }
    return (u_long *) out;
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
 *       pair. This is the opposite of func_800584DC, which needs an explicit
 *       cursor. A `FieldCellTint *` cursor initialised from @c dst also
 *       measures 100%; one initialised from @c dst @c + @c 4 does not.
 * @note The switch needs the otherwise-empty `case 1:` and `case 6:` to emit a
 *       jump table rather than a compare tree (84.91%), as in func_800584DC.
 * @note @c code must be read into a local before the loop; testing
 *       @c cel->unk1C at each of its three sites reloads it (94.89%).
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
void func_800589F0(FieldAnimDef *def, FieldAnimCel *cel, FieldTintSrc *src, s32 shade)
{
    FieldTileGrid *grid;
    FieldTileDesc *tile;
    FieldTintColor *pal;
    FieldTintColor *entry;
    u8 *dst;
    u16 *tab;
    u32 *mask;
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

    rgb[0] = src->unk10 * src->unk16;
    rgb[1] = src->unk12 * src->unk18;
    rgb[2] = src->unk14 * src->unk1A;
    stride = 0;
    pal = (FieldTintColor *) 0x1F800000;
    tab = src->unk4->unk4;
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
    code = cel->unk1C;
    if (code != 0)
    {
        stride -= 4;
    }
    if (cel->unk18 != 0)
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
                        if (tile->unk0 & 0x80)
                        {
                            slot = tile->unk3;
                            if ((slot >= first) && (last >= slot))
                            {
                                entry = &pal[slot] + shade;
                                if (code != 0)
                                {
                                    ((FieldTintColor *) &cel->unk1C)->rg = entry->rg;
                                    ((FieldTintColor *) &cel->unk1C)->b = entry->b;
                                    return;
                                }
                                ((FieldCellTint *) dst)->rg = entry->rg;
                                ((FieldCellTint *) dst)->b = entry->b;
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
                }
                while (col != grid->u.b.cols);
            }
            row++;
        }
        while (row != grid->u.b.rows);
    }
}
