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
    u8 _pad0[0xC];
    /**
     * 0x0C bit 1 zeroes the offsets; bit 2 and bits 4-5 select the horizontal
     * multiplier / wrap; bit 3 and bits 6-7 the vertical one. Must be UNSIGNED:
     * the target shifts it with `srl`, not `sra`.
     */
    u32 flags;
    u8 _pad1[0x1C - 0x10];
    /** 0x1C horizontal scale; 0x10 means "unscaled", bit 7 negates. */
    u8 unk1C;
    /** 0x1D vertical scale; same encoding as unk1C. */
    u8 unk1D;
} FieldObjDef;

/**
 * @brief Definition record hanging off a part; only the 0x0B byte is read.
 */
typedef struct
{
    u8 _pad[0xB];
    u8 unkB;
} FieldPartDef;

/**
 * @brief Element of an object's part list.
 */
typedef struct FieldPart FieldPart;
struct FieldPart
{
    FieldPart *next;        /* 0x00 */
    FieldPartDef *def;      /* 0x04 */
    u8 _pad0[0x20 - 8];
    /** 0x20 zero means the part is not drawn. */
    u8 unk20;
    /** 0x21 selects the per-part byte cost: 0x18, 0x1C, 0x28 or 0x34 units. */
    u8 kind;
    u8 _pad1[0x26 - 0x22];
    /** 0x26 number of instances; zero means the part is skipped entirely. */
    u16 count;
    s32 unk28;              /* 0x28 x offset within the object */
    s32 unk2C;              /* 0x2C y offset within the object */
    s32 unk30;              /* 0x30 z offset within the object */
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
    u8 _pad0[0x1C - 0x10];
    s32 unk1C;              /* 0x1C x offset */
    s32 unk20;              /* 0x20 y offset */
    s32 unk24;              /* 0x24 z offset */
    s32 unk28;              /* 0x28 accumulated x drift */
    s32 unk2C;              /* 0x2C accumulated y drift */
};

/**
 * @brief Header hanging off FieldScene offset 0; only the 0x30 halfword is read.
 */
typedef struct
{
    u8 _pad[0x30];
    s16 unk30;
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

typedef struct
{
    FieldSceneHeader *unk0; /* 0x00 */
    FieldObj *head;         /* 0x04 head of the object list */
    u8 _pad0[0x10 - 8];
    FieldMarker *markers;   /* 0x10 head of the marker list */
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
                                s32 d = part->def->unkB * 0x10 - 0xE0;

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
