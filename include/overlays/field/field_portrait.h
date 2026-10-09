#ifndef FIELD_PORTRAIT_H
#define FIELD_PORTRAIT_H

#include "common.h"
#include "common/gpu_packet.h"

/**
 * @file field_portrait.h
 * @brief Character portraits: FIELD's text windows, GOSUB and the card menus
 *        (ADDHERO, CARDA, CLOAD, NIKI) all draw the same 48x48 4-bit images.
 */

/** @brief Side of a square portrait, in pixels (4-bit, so a quarter of that in VRAM halfwords). */
#define FIELD_PORTRAIT_SIZE 48

/** @brief Bytes of one portrait: its CLUT followed by its pixels. */
#define FIELD_PORTRAIT_BYTES (GPU_CLUT_4BIT_COLORS * 2 + FIELD_PORTRAIT_SIZE * FIELD_PORTRAIT_SIZE / 2)

/** @brief One portrait: a 16-colour CLUT followed by 48x48 4-bit pixels (FIELD_PORTRAIT_BYTES in all). */
typedef struct
{
    u16 clut[GPU_CLUT_4BIT_COLORS];
    u8 pixels[FIELD_PORTRAIT_SIZE][FIELD_PORTRAIT_SIZE / 2];
} FieldPortrait;

/** @brief Byte offset of a portrait's pixels, after its CLUT. */
#define FIELD_PORTRAIT_PIXELS_OFFSET (GPU_CLUT_4BIT_COLORS * 2)

/**
 * @brief VRAM home of the party portraits (g_field_party_portraits): one CLUT
 *        row each from FIELD_PARTY_PORTRAIT_CLUT_Y, and their pixels at
 *        (VRAM_X, VRAM_Y0) for slot 0, (VRAM_X, VRAM_Y1) for slot 1 and
 *        (VRAM_X2, VRAM_Y1) for slot 2.
 */
#define FIELD_PARTY_PORTRAIT_COUNT 3
#define FIELD_PARTY_PORTRAIT_CLUT_X 272
#define FIELD_PARTY_PORTRAIT_CLUT_Y 472
#define FIELD_PARTY_PORTRAIT_VRAM_X 1012
#define FIELD_PARTY_PORTRAIT_VRAM_X2 1000
#define FIELD_PARTY_PORTRAIT_VRAM_Y0 288
#define FIELD_PARTY_PORTRAIT_VRAM_Y1 336

/**
 * @brief Copy one of the portrait palettes into a portrait image.
 * @param dest Palette strip at the start of the portrait image.
 * @param index Palette index.
 */
void field_copy_portrait_palette(void* dest, s32 index);

/**
 * @brief Copy a golem portrait palette into a portrait image.
 * @param destination Palette strip at the start of the portrait image.
 * @param palette Golem palette index (0-31).
 */
void field_copy_golem_portrait_palette(u8* destination, s32 palette);

#endif
