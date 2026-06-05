#ifndef _TIM_H
#define _TIM_H

#include "common.h"

/**
 * @file tim.h
 * @brief PSX TIM image-file format types and constants.
 */

/** Size of the TIM file header (magic word + flags word) in bytes. */
#define TIM_HEADER_SIZE     8

/** Number of entries in a 256-color (8bpp) CLUT. */
#define CLUT_ENTRY_COUNT    0x100

/** PSX 16bpp mask bit: set on a CLUT entry to enable semi-transparency. */
#define GPU_STP_BIT         0x8000

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
    u16 w;    /* 0x08 - region width in pixels */
    u16 h;    /* 0x0A - region height in lines */
} TimBlock;

#endif
