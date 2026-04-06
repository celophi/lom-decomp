#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/memory.h"

/**
 * The maximum number of glyph entries in the character cache
 */
#define MAX_GLYPH_ENTRIES 256

/**
 * Flag indicating that a glyph is currently cached and valid in the character cache.
 */
#define GLYPH_CACHED_FLAG 0x10000

typedef struct {
    u8  deviceState;     // 0x00 - status / mode flag
    u8  _pad1;
    u16 buttonData;      // 0x02 - raw 16-bit input (pre-remap)

    u8  _pad2[0x28];     // 0x04–0x2B - unused here

    s16 axisX;           // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY;           // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

/**
 * Represents a single glyph's entry in the text cache, storing its ID and validity flag
 */
typedef union {
    u32 raw;
    struct {
        u16 charId; 
        struct {
            u16 isCached : 1;  // Bit 16
            u16 reserved : 15; // Bits 17-31
        } flags;
    } data;
} GlyphCacheEntry;

extern s32 g_previousGameState;
extern s32 g_textBufferAddr;
extern s8 g_TextBuffer[];

/**
 * Global cursor pointing to the current position in the text buffer for glyph rendering.
 */
extern s32 g_glyphBufferCursor;

/**
 * Global character cache for text rendering, storing up to 256 glyph entries. 
 * Each entry contains a character ID and a validity flag indicating if the glyph is currently cached.
 */
extern GlyphCacheEntry g_characterCache[MAX_GLYPH_ENTRIES];

/**
 * X coordinate in the glyph atlas texture, used for placing or retrieving glyphs.
 */
extern s32 g_glyphAtlasX;

/**
 * Y coordinate in the glyph atlas texture, used for placing or retrieving glyphs.
 */
extern s32 g_glyphAtlasY;

extern s32 D_8005D060;
extern u32 D_80052428;
extern s32 D_80061088;
extern u8 D_8005D088;
extern s32 D_8005D068[4];
extern s32 D_8005D078[3];
extern s32 D_800610A0;
extern s32 D_80061094;
extern s32 D_80061098;
extern u8  D_8005B744[];
extern u8 D_801ED600;
extern s32 D_80061090;
extern s32 D_800610A4;
extern s32 D_800610A8;
extern s32 D_8005CFE8;
extern s32 D_800894C0;
extern s32 D_800894C4;
extern s32 D_800894CC;
extern u16 D_8005D030[];

typedef struct
{
  u16 sp20;
  u16 sp22;
  s16 sp24;
} Sp20Data;

typedef struct {
    union {
        s32 unk0;
        struct {
            u8 _pad0[3];
            u8 unk3;
        } byte;
    } u;
    u8  unk4;
    u8  unk5;
    u8  unk6;
    u8  unk7;
    u16 unk8;
    u16 unkA;
    s8  unkC;
    s8  unkD;
    u16 unkE;
} SomeStruct;

void func_80050080(void);
void func_8004FEE8(int param_1);
void func_8004FD68(int param_1);

/**
 * @brief Prepares the text renderer for a new frame by resetting the write cursor and invalidating glyphs.
 * 
 * @details This function performs a "soft reset" of the text system:
 * 1. Resets the global glyph buffer write pointer (g_glyphBufferCursor) to the start 
 *    of the text buffer, allowing glyphs to be overwritten from the beginning.
 * 2. Strips the 'isCached' flag from all entries in the glyph cache. This preserves 
 *    the character ID mappings but forces the renderer to re-rasterize the actual 
 *    pixels for every glyph upon the next request.
 * 
 * @note This is less destructive than ResetTextRenderer, as it keeps the glyph 
 * cache slots assigned to their respective characters.
 * 
 * @param void No parameters.
 * @return void No return value.
 * 
 * @see decomp.me (100%) https://decomp.me/scratch/9wpJn
 */
void InvalidateGlyphCache(void);

/**
 * @brief Clears all invalid or unflagged entries from the glyph cache.
 * 
 * @details Iterates through the glyph cache and zeros out any entry that 
 * does not have the 'isCached' flag (0x10000) set. This ensures that 
 * only active, valid glyphs remain in the cache.
 * 
 * @param void No parameters.
 * @return void No return value.
 * 
 * @see decomp.me (100%) https://decomp.me/scratch/ox3LP
 */
void ClearInvalidGlyphs(void);

/**
 * @brief Resets the text renderer state and buffers.
 * 
 * @details This function initializes the text rendering system by clearing the 
 * character cache (256 entries) and zeroing out the global text buffer (32KB). 
 * It also resets the image loading state by loading a minimal 1x16 rectangle 
 * at the bottom of the screen area to clear the GPU's current text-related 
 * texture state.
 * 
 * @param void No parameters.
 * @return void No return value.
 * 
 * @see decomp.me (100%) https://decomp.me/scratch/Bdkvp
 */
void ResetTextRenderer(void);

#endif