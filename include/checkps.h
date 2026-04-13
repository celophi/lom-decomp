#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/memory.h"
#include "psyq/strings.h"

/**
 * The maximum number of glyph entries in the character cache
 */
#define MAX_GLYPH_ENTRIES 256

/**
 * Flag indicating that a glyph is currently cached and valid in the character cache.
 */
#define GLYPH_CACHED_FLAG 0x10000

typedef struct
{
    u8 deviceState; // 0x00 - status / mode flag
    u8 _pad1;
    u16 buttonData; // 0x02 - raw 16-bit input (pre-remap)

    u8 _pad2[0x28]; // 0x04–0x2B - unused here

    s16 axisX; // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY; // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

typedef struct
{
    u8 unk0;
    u8 unk1;
} D_8005CFE0_t;

extern D_8005CFE0_t D_8005CFE0;

/**
 * Represents a single glyph's entry in the text cache, storing its ID and validity flag
 */
typedef union
{
    u32 raw;
    struct
    {
        u16 charId;
        struct
        {
            u16 isCached : 1;  // Bit 16
            u16 reserved : 15; // Bits 17-31
        } flags;
    } data;
} GlyphCacheEntry;

extern s32 g_previousGameState;
extern s32 g_textBufferAddr;

/**
 * Lookup table for test pattern sizes, used in DrawSymmetricTestPattern. Contains 16 pairs of width/height values.
 */
extern u8 g_testPatternSizeTable[][2];

/**
 * Y coordinate in the glyph atlas texture, used for placing or retrieving glyphs.
 */
extern s32 g_glyphAtlasY;

const u32 D_8004FCC4[15] = {
    0xA790AD8B, 0xB997498F, 0xDC82B582, 0xBD82B582, 0x960A4281, 0x82CC917B, 0x91FC89AA, 0x82B382A2,
    0x82C482EA, 0x0AE982A2, 0xBB82A882, 0xAA82EA82, 0xE882A082, 0xB782DC82, 0x00004281,
};

extern u32 D_80052428;
extern u8 D_8005B744[];
extern u8 D_801ED600;
extern s32 D_8005CFE8;
extern u16 D_8005D030[];
extern u16 D_8005D018[];
extern s8* D_8005CFC8;
extern s8* D_8005CFD4;
extern u8* D_8005CFD0;
extern u8 D_8005CF91[];
extern u8 D_8005CFD8[];
extern u8* D_8005CFCC;
extern u8 D_8005CF90[];
extern u8 D_8005CF92[];
extern s32 D_8005CFEC;
extern u8 D_8005CF93[];
extern u8 D_8005CFE2;
extern u8 D_8005CFE1[2];

typedef struct
{
    u16 sp20;
    u16 sp22;
    s16 sp24;
} Sp20Data;

typedef struct
{
    s16 a;
    s16 b;
    s16 c;
    s16 d;
} FourShorts;

typedef struct
{
    s16 xy;
    s16 unk2;
    u32 wh;
} Arg0Struct;

typedef struct
{
    s16 unk0;
    u16 unk2;
} arg1struct;

typedef struct
{
    union
    {
        s32 unk0;
        struct
        {
            u8 _pad0[3];
            u8 unk3;
        } byte;
    } u;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u16 positionX;
    u16 positionY;
    s8 unkC;
    s8 unkD;
    u16 unkE;
} GlyphInstance;

void func_80050080(void);
void func_8004FEE8(int param_1);
void func_8004FD68(int param_1);
void func_80051908(void* arg0, u8* arg1, s32 arg2);
void DrawSymmetricTestPattern(void);

/**
 * @brief Creates a new glyph instance and links it into the active text stream.
 *
 * @details This function initializes a GlyphInstance with screen coordinates and
 * UV atlas offsets derived from the glyph's cache slot. It then links the instance
 * into a doubly-linked list of characters and advances the global text cursor.
 * If the cursor exceeds the right margin, it performs a line wrap.
 *
 * @param instance Pointer to the memory for the new glyph instance.
 * @param next Pointer to the handle of the previous glyph in the stream.
 * @param index The index of the glyph in the character cache.
 *
 * @return The pointer to the next available instance slot in the pool.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/FyrJc
 */
GlyphInstance* CreateGlyphInstance(GlyphInstance* instance, GlyphInstance** next, s32 index);

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