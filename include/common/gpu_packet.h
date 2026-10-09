#ifndef _GPU_PACKET_H
#define _GPU_PACKET_H

#include "common.h"

/**
 * @file gpu_packet.h
 * @brief Primitive setters and GPU constants that libgpu doesn't provide.
 *
 * The game often fills two or three neighbouring primitive fields with one
 * word or halfword store instead of a store per field. The *_PACKED and
 * *_WORD setters below do that. The SPRT and POLY_G4 ones address the packet
 * by byte offset, so they also work on a void* or u8* cursor.
 */

/** @brief Set y0, then x0 (libgpu's setXY0 writes x0 first). */
#define SET_YX0(p, _y0, _x0) \
    (p)->y0 = (_y0), (p)->x0 = (_x0)

/** @brief Set the colour of any primitive one byte at a time, blue first. The code byte is left alone. */
#define SET_BGR0(p, _b0, _g0, _r0) \
    (p)->b0 = _b0, (p)->g0 = _g0, (p)->r0 = _r0

/** @brief Write the whole r0/g0/b0/code word of any primitive at once (see GPU_COLOR_WORD). */
#define SET_BGR0_PACKED(p, _word) \
    (*(u32*)((u8*)(p) + 4) = (u32)(_word))

/** @brief Pack a colour into a primitive colour word (0x00bbggrr), with a zero code byte. */
#define GPU_COLOR_WORD(_r, _g, _b) \
    (((u32)(_b) << 16) | ((u32)(_g) << 8) | (u32)(_r))

/** @brief Draws a texture at its own brightness: 0x80 is the GPU's 1.0 for each channel. */
#define GPU_TINT_NEUTRAL GPU_COLOR_WORD(0x80, 0x80, 0x80)

/** @brief Write vertex 1, 2 or 3's colour word of a POLY_G4 at once. Vertex 0 uses SET_BGR0_PACKED. */
#define SET_POLY_G4_BGR1_PACKED(p, _word) \
    (*(u32*)((u8*)(p) + 0x0C) = (u32)(_word))
#define SET_POLY_G4_BGR2_PACKED(p, _word) \
    (*(u32*)((u8*)(p) + 0x14) = (u32)(_word))
#define SET_POLY_G4_BGR3_PACKED(p, _word) \
    (*(u32*)((u8*)(p) + 0x1C) = (u32)(_word))

/** @brief Write an SPRT's x0 and y0 from one packed word (y0 in the high half). */
#define SET_SPRT_XY0_WORD(p, _xy) \
    (*(u32*)((u8*)(p) + 0x08) = (u32)(_xy))

/** @brief Write an SPRT's w and h from one packed word (h in the high half). */
#define SET_SPRT_WH_WORD(p, _wh) \
    (*(u32*)((u8*)(p) + 0x10) = (u32)(_wh))

/** @brief Write an SPRT's w and h with one store. */
#define SET_SPRT_WH_PACKED(p, _w, _h) \
    (*(u32*)((u8*)(p) + 0x10) = ((u32)(u16)(_h) << 16) | (u32)(u16)(_w))

/** @brief Write an SPRT's u0 and v0 from one packed halfword (v0 in the high byte). */
#define SET_SPRT_UV0_PACKED(p, _uv) \
    (*(u16*)((u8*)(p) + 0x0C) = (u16)(_uv))

/** @brief Write an SPRT's u0, v0 and clut from one packed word (clut in the high half). */
#define SET_SPRT_UV_CLUT_WORD(p, _word) \
    (*(u32*)((u8*)(p) + 0x0C) = (u32)(_word))

/** @brief Set an SPRT's CLUT id directly; libgpu's setClut takes VRAM coordinates instead. */
#define SET_SPRT_CLUT(p, _clut) \
    (*(u16*)((u8*)(p) + 0x0E) = (u16)(_clut))

/** @brief GPU command codes of a POLY_G4 and a TILE, and the bit that makes a primitive semi-transparent. */
#define GPU_CODE_POLY_G4 0x38
#define GPU_CODE_TILE 0x60
#define GPU_CODE_SEMI_TRANS 0x02

/** @brief Texture colour depths (the tp argument of getTPage). */
#define GPU_TEXTURE_4BIT 0
#define GPU_TEXTURE_8BIT 1
#define GPU_TEXTURE_16BIT 2

/** @brief Colours in a 4-bit and in an 8-bit texture's CLUT. An 8-bit CLUT fills a whole VRAM row. */
#define GPU_CLUT_4BIT_COLORS 16
#define GPU_CLUT_8BIT_COLORS 256

/** @brief Semi-transparency modes (the abr argument of getTPage). */
#define GPU_BLEND_HALF 0        /**< Half the background plus half the primitive. */
#define GPU_BLEND_ADD 1         /**< Background plus primitive. */
#define GPU_BLEND_SUBTRACT 2    /**< Background minus primitive. */
#define GPU_BLEND_ADD_QUARTER 3 /**< Background plus a quarter of the primitive. */

/** @brief Size of packet type T in words, for stepping a u_long* cursor: `prim += PRIM_WORDS(SPRT);`. */
#define PRIM_WORDS(T) (sizeof(T) / sizeof(u_long))

#endif
