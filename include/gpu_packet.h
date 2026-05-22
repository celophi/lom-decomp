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

/* --- Color word (P_TAG: r0,g0,b0,code at offset 4; valid for any prim) --- */

/*
 * Set b0/g0/r0 with three separate byte stores; leaves the code byte alone.
 * Mirrors the libgpu setRGB0 ordering but in b,g,r argument order.
 */
#define SET_BGR0(p, _b0, _g0, _r0) \
    (p)->b0 = _b0, (p)->g0 = _g0, (p)->r0 = _r0

/*
 * Pack r0/g0/b0 (code byte = 0) into the color word and store it with a
 * single 32-bit write, matching `*(u32*)(p + 4) = 0x00bbggrr;`. Use this
 * (not SET_BGR0) when the original emits one word store rather than three
 * byte stores.
 */
#define SET_BGR0_PACKED(p, _b0, _g0, _r0) \
    (*(u32*)((u8*)(p) + 4) = ((u32)(_b0) << 16) | ((u32)(_g0) << 8) | (u32)(_r0))

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

#endif
