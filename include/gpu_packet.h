#ifndef _GPU_PACKET_H
#define _GPU_PACKET_H

#include "common.h"

/*
 * Custom GPU primitive-packing macros.
 *
 * These complement the stock Psy-Q libgpu setters (setRGB0, setWH, setUV0,
 * setClut, ...). The original game code frequently builds a primitive by
 * writing several adjacent fields with a single wide store (one word or one
 * halfword) instead of the per-field stores the libgpu macros expand to.
 * Reproducing that exact store width is required for a byte-for-byte match,
 * so the packed variants below exist alongside the libgpu ones.
 *
 * Naming: ALL_CAPS with underscores. Macros that target a field common to
 * every primitive (the P_TAG color word at offset 4) are primitive-agnostic.
 * Macros whose field offset is specific to one primitive type carry that type
 * in the name (e.g. "SPRT") and address the packet through a raw byte offset
 * so they can be called on a void* / u8* cursor without a typed cast.
 */

/* --- Position setters --- */

/*
 * Set x0 and y0 writing y before x, matching the store order emitted by
 * code that writes the y0 field (higher offset) first and then x0. libgpu's
 * setXY0 writes x before y; use SET_YX0 when the original instruction stream
 * stores y0 first and the reversed order is required for a matching build.
 */
#define SET_YX0(p, _y0, _x0) \
    (p)->y0 = (_y0), (p)->x0 = (_x0)

/* --- Color word (P_TAG: r0,g0,b0,code at offset 4; valid for any prim) --- */

/*
 * Set b0/g0/r0 with three separate byte stores; leaves the code byte alone.
 * Mirrors the libgpu setRGB0 ordering but in b,g,r argument order.
 */
#define SET_BGR0(p, _b0, _g0, _r0) \
    (p)->b0 = _b0, (p)->g0 = _g0, (p)->r0 = _r0

/*
 * Store a pre-packed color word at offset 4 with one 32-bit write, matching
 * the hand-written `*(u32*)(p + 4) = 0x00bbggrr;` idiom. Use this (not
 * SET_BGR0) when the original emits one word store rather than three byte
 * stores. The argument is the full 32-bit value in P_TAG layout:
 *   byte 0 (LSB) = r0, byte 1 = g0, byte 2 = b0, byte 3 (MSB) = code.
 * Pack with @c GPU_COLOR_WORD or use a named constant like @c GPU_TINT_NEUTRAL.
 */
#define SET_BGR0_PACKED(p, _word) \
    (*(u32*)((u8*)(p) + 4) = (u32)(_word))

/*
 * Build a packed P_TAG color word (code byte = 0) from r/g/b components.
 * Matches the byte layout `0x00bbggrr` so it can be passed directly to
 * @ref SET_BGR0_PACKED. Constant-folded for literal arguments.
 */
#define GPU_COLOR_WORD(_r, _g, _b) \
    (((u32)(_b) << 16) | ((u32)(_g) << 8) | (u32)(_r))

/*
 * Neutral tint for a textured primitive: each channel = 0x80, the PSX GPU's
 * 1.0x modulation factor (texel rendered as-is, no brightness change). The
 * code byte is 0; the caller fills it in via @c setcode / @c setSprt / etc.
 */
#define GPU_TINT_NEUTRAL GPU_COLOR_WORD(0x80, 0x80, 0x80)

/* --- SPRT-specific packed setters (single store) --- */

/*
 * Set an SPRT's width and height (shorts at offset 0x10 / 0x12) with a single
 * word store, matching `*(u32*)(p + 0x10) = 0x00hh00ww;`. libgpu's setWH
 * emits two short stores.
 */
#define SET_SPRT_WH_PACKED(p, _w, _h) \
    (*(u32*)((u8*)(p) + 0x10) = ((u32)(u16)(_h) << 16) | (u32)(u16)(_w))

/*
 * Set an SPRT's u0 and v0 (bytes at offset 0x0C / 0x0D) with a single
 * halfword store, matching `*(u16*)(p + 0x0C) = uv;`. libgpu's setUV0 emits
 * two byte stores.
 */
#define SET_SPRT_UV0_PACKED(p, _uv) \
    (*(u16*)((u8*)(p) + 0x0C) = (u16)(_uv))

/*
 * Store a raw CLUT id into an SPRT (halfword at offset 0x0E). libgpu's
 * setClut takes VRAM coordinates and computes the id via getClut; this sets
 * a precomputed id directly.
 */
#define SET_SPRT_CLUT(p, _clut) \
    (*(u16*)((u8*)(p) + 0x0E) = (u16)(_clut))

/*
 * Size of a GPU packet type T in u_long words, for advancing a u_long*
 * primitive cursor one packet at a time: `prim += PRIM_WORDS(SPRT);`.
 */
#define PRIM_WORDS(T) (sizeof(T) / sizeof(u_long))

#endif
