#include "common.h"

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
 * @brief Per-object definition record; only the flags word is read here.
 */
typedef struct
{
    u8 _pad[0xC];
    /** 0x0C bit 2 and bits 4-5 select the base multiplier; bit 3 doubles it. */
    s32 flags;
} FieldObjDef;

/**
 * @brief Element of an object's part list.
 */
typedef struct FieldPart FieldPart;
struct FieldPart
{
    FieldPart *next;        /* 0x00 */
    u8 _pad0[0x21 - 4];
    /** 0x21 selects the per-part byte cost: 0x18, 0x1C, 0x28 or 0x34 units. */
    u8 kind;
    u8 _pad1[0x26 - 0x22];
    /** 0x26 number of instances; zero means the part is skipped entirely. */
    u16 count;
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
};

typedef struct
{
    u8 _pad[4];
    FieldObj *head;         /* 0x04 head of the object list */
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

extern FieldSceneGlobals g_field_scene;

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
