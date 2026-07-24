#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

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
    u8 _pad2[0x26 - 0x22];
    /** 0x26 number of instances; zero means the part is skipped entirely. */
    u16 count;
    s32 unk28;              /* 0x28 x offset within the object */
    s32 unk2C;              /* 0x2C y offset within the object */
    s32 unk30;              /* 0x30 z offset within the object */
    u8 _pad3[0x3A - 0x34];
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

/** @brief Element of an animation node's cel ring. */
typedef struct FieldAnimCel FieldAnimCel;
struct FieldAnimCel
{
    FieldAnimCel *next; /* 0x00 */
    u8 _pad0[0x20 - 4];
    u8 unk20; /* 0x20 */
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
    u8 _pad1[0x24 - 0x14];
    FieldAnimFlags flags; /* 0x24 */
    u8 _pad2[0x2A - 0x28];
    u16 counter;          /* 0x2A */
    u8 _pad3[0x30 - 0x2C];
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

typedef struct
{
    FieldSceneHeader *unk0; /* 0x00 */
    FieldObj *head;         /* 0x04 head of the object list */
    u8 _pad0[0x10 - 8];
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
 * distinction is load-bearing for the addressing mode.
 */
typedef struct
{
    u8 _pad[8];
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
void func_800589F0(FieldAnimDef *, FieldAnimCel *, s32, s32);
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
                        func_800589F0(def, anim->cels, anim->unk10, anim->flags.b.state);
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
