#ifndef _TIM_H
#define _TIM_H

#include "common.h"

#include "common/gpu_packet.h"

/**
 * @file tim.h
 * @brief PSX TIM image-file format types and constants.
 */

/** Size of the TIM file header (magic word + flags word) in bytes. */
#define TIM_HEADER_SIZE 8

/** The TIM file contains a CLUT block before its pixel block. */
#define TIM_FLAG_HAS_CLUT 8

/** Flag bits holding the pixel mode, which is also the GPU_TEXTURE_* texture mode. */
#define TIM_FLAG_PIXEL_MODE_MASK 7

/** @brief Advance to the next TIM block by skipping @p block->bnum bytes. */
#define TIM_NEXT_BLOCK(block) ((TimBlock*)((u8*)(block) + (block)->bnum))

/** @brief The TIM block after @p block, given its already-loaded length @p bnum. */
#define TIM_SKIP_BLOCK(block, bnum) ((TimBlock*)((u8*)(block) + (bnum)))

/** @brief Payload of a TIM block (palette entries or pixels), as the LoadImage source. */
#define TIM_BLOCK_DATA(block) ((u_long*)((block) + 1))

/** @brief The first block of a TIM file, right after its header. */
#define TIM_FIRST_BLOCK(tim) ((TimBlock*)((u8*)(tim) + TIM_HEADER_SIZE))

/** @brief The VRAM rectangle of a TIM block: its dx, dy, width and height read as a RECT. */
#define TIM_BLOCK_RECT(block) (*(RECT*)&(block)->dx)

/** @brief Nonzero when a TIM file has a CLUT block; tests the low byte of its flag word. */
#define TIM_HAS_CLUT(tim) (*(u8*)&(tim)->flags & TIM_FLAG_HAS_CLUT)

/** @brief Pixel mode of a TIM file; reads the low byte of its flag word. */
#define TIM_PIXEL_MODE(tim) (*(u8*)&(tim)->flags & TIM_FLAG_PIXEL_MODE_MASK)

/** @brief Locate the pixel block given the TIM base and a pre-loaded CLUT bnum. */
#define TIM_PIXEL_BLOCK(tim, bnum) ((TimBlock*)((u8*)(tim) + ((bnum) + TIM_HEADER_SIZE)))

/** Number of color entries in a 256-entry palette block. */
#define CLUT_ENTRY_COUNT 0x100

/** @brief VRAM destinations of a TIM's pixel block and CLUT block. */
typedef struct
{
    s16 pixel_x;
    s16 pixel_y;
    s16 clut_x;
    s16 clut_y;
} TimUploadDestination;

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
    u32 bnum;                 /* block length in bytes, including this header */
    u16 dx;                   /* VRAM destination X */
    u16 dy;                   /* VRAM destination Y */
    TimDimensions dimensions; /* region size */
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
 * @brief Upload a TIM's optional CLUT and its pixel data to the given VRAM positions.
 * @param destinations VRAM destinations for the pixel and CLUT blocks.
 * @param tim TIM resource to upload.
 */
void upload_tim_to_vram(TimUploadDestination* destinations, TimPrefix* tim);

/**
 * @brief PSX TIM file header for an 8bpp (256-color) image.
 *
 * Maps the fixed portion of a TIM file: two-word file header, CLUT block
 * header, and the 256-entry CLUT palette. The pixel block follows at a
 * variable offset; use @c TIM_NEXT_BLOCK(&tim->clut_block) to reach it.
 */
typedef struct
{
    u32 magic; /* TIM magic/type word (low byte = 0x10) */
    u32 flags; /* flag word (bit3=hasCLUT, bits1:0=bit-depth) */
    TimBlock clut_block;
    u16 clut_data[CLUT_ENTRY_COUNT]; /* 256-entry 16bpp palette */
    /* pixel block follows clut_data */
} Tim;

#endif
