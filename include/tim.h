#ifndef _TIM_H
#define _TIM_H

#include "common.h"

/**
 * @file tim.h
 * @brief PSX TIM image-file format types and constants.
 */

/** Size of the TIM file header (magic word + flags word) in bytes. */
#define TIM_HEADER_SIZE 8

/** Advance to the next TIM block by skipping @p block->bnum bytes.
 * Re-reads bnum at the call site; use TIM_PIXEL_BLOCK when bnum is
 * already cached in a local variable and codegen order matters. */
#define TIM_NEXT_BLOCK(block) ((TimBlock*)((u8*)(block) + (block)->bnum))

/** Locate the pixel block given the TIM base and a pre-loaded CLUT bnum.
 * The parenthesisation (bnum + TIM_HEADER_SIZE) is required for the match: it
 * forces addiu (bnum+8) before addu (+base), matching the compiler's register
 * allocation when bnum is held in a saved register. */
#define TIM_PIXEL_BLOCK(tim, bnum) \
    ((TimBlock*)((u8*)(tim) + ((bnum) + TIM_HEADER_SIZE)))

/** Number of entries in a 256-color (8bpp) CLUT. */
#define CLUT_ENTRY_COUNT 0x100

/** PSX 16bpp mask bit: set on a CLUT entry to enable semi-transparency. */
#define GPU_STP_BIT 0x8000

/** @brief Width and height stored in a TIM block header. */
typedef struct
{
    u16 width;
    u16 height;
} TimDimensions;

/**
 * @brief Header of a PSX TIM pixel/CLUT block.
 *
 * Every TIM block - the CLUT block and the image block alike - starts with
 * this 0xC-byte header: a total byte length followed by the destination VRAM
 * rectangle. The block payload (palette entries or pixels) follows immediately
 * at offset @c sizeof(TimBlock), i.e. @c block + 1.
 */
typedef struct
{
    u32 bnum; /* 0x00 - block length in bytes, including this header */
    u16 dx;   /* 0x04 - VRAM destination X */
    u16 dy;   /* 0x06 - VRAM destination Y */
    TimDimensions dimensions; /* 0x08 - region size */
} TimBlock;

/**
 * @brief Fixed prefix of a TIM file whose CLUT length varies by bit depth.
 *
 * The CLUT payload begins at @c clut_data and contains the number of entries
 * implied by @c clut_block.bnum. The pixel block follows at that variable
 * offset; use @c TIM_PIXEL_BLOCK to locate it.
 */
typedef struct
{
    u32 magic;
    u32 flags;
    TimBlock clut_block;
    u16 clut_data[1];
} TimPrefix;

/**
 * @brief PSX TIM file header for an 8bpp (256-color) image.
 *
 * Maps the fixed portion of a TIM file: two-word file header, CLUT block
 * header, and the 256-entry CLUT palette. The pixel block follows at a
 * variable offset; use @c TIM_NEXT_BLOCK(&tim->clut_block) to reach it.
 */
typedef struct
{
    u32 magic;                       /* 0x00 - TIM magic/type word (low byte = 0x10) */
    u32 flags;                       /* 0x04 - flag word (bit3=hasCLUT, bits1:0=bit-depth) */
    TimBlock clut_block;             /* 0x08 - CLUT block header */
    u16 clut_data[CLUT_ENTRY_COUNT]; /* 0x14 - 256-entry 16bpp palette */
    /* pixel block follows clut_data */
} Tim;

#endif
