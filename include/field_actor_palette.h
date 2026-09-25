#ifndef FIELD_ACTOR_PALETTE_H
#define FIELD_ACTOR_PALETTE_H

#include "common.h"

/** @brief Bytes of CLUT data owned by each of the two actor palette owners. */
#define FIELD_ACTOR_CLUT_BUFFER_SIZE 0x400

/** @brief VRAM row of player 0's actor CLUT; player 1 uses the row two below. */
#define FIELD_ACTOR_CLUT_VRAM_Y 0x1EE
/** @brief VRAM row of the CLUT shared by the other actors. */
#define FIELD_SHARED_CLUT_VRAM_Y 0x1F2

/** @brief VRAM position of player 0's actor texture; player 1's follows it. */
#define FIELD_ACTOR_TEXTURE_VRAM_X 0x340
#define FIELD_ACTOR_TEXTURE_VRAM_Y 0x100
/** @brief VRAM columns of one player's actor texture. */
#define FIELD_ACTOR_TEXTURE_VRAM_WIDTH 64
/** @brief VRAM position of the texture shared by the other actors. */
#define FIELD_SHARED_TEXTURE_VRAM_X 0x140
#define FIELD_SHARED_TEXTURE_VRAM_Y 0

/** @brief VRAM row of the first of the effect CLUT rows loaded with the common texture. */
#define FIELD_EFFECT_CLUT_VRAM_Y 0x1EA
/** @brief Number of 256-entry effect CLUT rows. */
#define FIELD_EFFECT_CLUT_ROWS 4
/** @brief VRAM position of the common effect texture. */
#define FIELD_EFFECT_TEXTURE_VRAM_X 0x180
#define FIELD_EFFECT_TEXTURE_VRAM_Y 0

extern u8 g_field_actor_clut_buffers[];
extern u8 g_field_shared_clut_buffer[];

#endif
