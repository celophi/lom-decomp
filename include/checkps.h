#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
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

extern s32 g_textCursorX;
extern s32 g_textCursorY;
extern s32 g_textOriginX;

extern s32 D_8005D060;
extern u32 D_80052428;
extern s32 D_80061088;
extern u8 D_8005D088;
extern s32 D_8005D068[4];
extern s32 D_8005D078[3];
extern s32 D_800610A0;
extern s32 D_80061094;
extern s32 D_80061098;
extern u8 D_8005B744[];
extern u8 D_801ED600;
extern s32 D_80061090;
extern s32 D_800610A4;
extern s32 D_800610A8;
extern s32 D_8005CFE8;
extern u16 D_8005D030[];
extern u16 D_8005D018[];
extern u8 D_8004FD00[8];
extern u8 D_8005CFF0[];
extern u32 D_8004FCC4;
extern s8 *D_8005CFC8;
extern s8 *D_8005CFD4;
extern u8 *D_8005CFD0;
extern u8 D_8005CF91[];
extern u8 D_8005CFD8[];
extern u8 *D_8005CFCC;
extern u8 D_8005CF90[];
extern u8 D_8005CF92[];
extern s32 D_8005CFEC;
extern u8 D_8005CF93[];

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
    s16 unk0;
    s16 unk2;
    u32 unk4;
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
s32 func_8001687C(unsigned short);

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