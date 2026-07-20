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
 * @brief Render record built by func_8005477C.
 * @note When the descriptor is absent the whole first word is set to -1, so
 *       unk0/unk1/unk2 are also addressed as a single s32 (see the else arm).
 */
typedef struct
{
    s8 unk0;
    s8 unk1;
    s16 unk2;
    s32 unk4;
    s32 unk8;
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
                prim->unk8 = attr | 0xE1000400;
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
