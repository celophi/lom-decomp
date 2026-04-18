#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/memory.h"
#include "psyq/strings.h"

/**
 * The width of the visible screen area, excluding overscan. 
 * This is used for setting up display environments and calculating text layout.
 */
#define SCREEN_WIDTH  320

/**
 * The visible height of the screen, excluding the overscan area. 
 * This is used for setting up display environments and calculating text layout.
 */
#define SCREEN_HEIGHT 240

/**
 * Vertical start of the back buffer's draw region.
 */
#define VRAM_BACK_DRAW_Y   8

/**
 * Vertical start of the back buffer's display region. 
 */
#define VRAM_BACK_DISP_Y   232

/**
 * Height of the draw region for each buffer.
 */
#define VRAM_DRAW_HEIGHT   224

/**
 * The maximum number of glyph entries in the character cache
 */
#define MAX_GLYPH_ENTRIES 256

/**
 * Flag indicating that a glyph is currently cached and valid in the character cache.
 */
#define GLYPH_CACHED_FLAG 0x10000

/**
 * Button bit masks for the game's internal controller state (g_lastInputState / g_debouncedInput).
 *
 * These differ from the raw PSX hardware bit positions. The SCD driver stores
 * the two controller bytes in reversed order, so UpdateControllerInput byte-swaps
 * buttonData first. The face button bits (4-7) are then remapped so the hardware
 * order (Triangle, Circle, Cross, Square) becomes (Square, Cross, Circle, Triangle).
 *
 * Set bit = button pressed.
 */
typedef enum {
    SQUARE   = 0x0010,
    CROSS    = 0x0020,
    CIRCLE   = 0x0040,
    TRIANGLE = 0x0080,
    UP       = 0x1000,
    RIGHT    = 0x2000,
    DOWN     = 0x4000,
    LEFT     = 0x8000
} PadButton;

typedef struct
{
    u8 deviceState; // 0x00 - status / mode flag
    u8 _pad1;
    u16 buttonData; // 0x02 - raw 16-bit input (pre-remap)

    u8 _pad2[0x28]; // 0x04–0x2B - unused here

    s16 axisX; // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY; // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

/**
 * Represents a target or current RGB colour state used by the screen fade system.
 * The "steps" field is used to track the number of frames remaining in a fade transition,
 */
typedef struct {
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeColor;

typedef struct
{
    u8 unk0;
    u8 unk1;
} D_8005CFE0_t;

extern D_8005CFE0_t g_statusFlag;

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
extern s32 g_checkPSState;
extern u16 D_8005D030[];
extern u16 D_8005D018[];

/**
 * Pointer to CD-ROM status register and related I/O ports for communicating with the PS1's CD drive.
 * CD Index/Status Register (Bit0-1 R/W, Bit2-7 Read Only)
 */
extern s8* g_cdStatusRegister = (s8*)0x1F801800;

/**
 * Pointer to CD-ROM response register, used for reading data returned by the CD drive after issuing commands.
 * CD Response Fifo (R) (usually with Index1)
 */
extern u8* g_cdResponseRegister = (s8*)0x1F801801;

/**
 * Pointer to CD-ROM data register, used for sending command parameters to the CD drive after writing a command to the status register.
 * CD Data Fifo - 8bit/16bit (R) (usually with Index0..1)
 */
extern u8* g_cdDataRegister = (u8*)0x1F801802;

/**
 * Pointer to CD-ROM IRQ register, used for handling CD drive interrupts.
 * CD IRQ Register (R/W)
 */
extern s8* g_cdIrqRegister = (s8*)0x1F801803;



/**
 * Descriptor for a single CD-ROM command: opcode, parameter count, response byte count,
 * and the IRQ accumulation threshold that must be reached before reading the response FIFO.
 */
typedef struct
{
    u8 opcode;
    u8 paramCount;
    u8 respCount;
    u8 irqThresh;
} CdCmdEntry;

extern CdCmdEntry g_cdCmdTable[];
extern u8 g_CmdBuf[3];

/**
 * Running accumulator of CD-ROM INT codes received since the last command was dispatched.
 * Incremented by the INT code value on each stable poll; reset to zero once the irqThresh
 * for the current command is reached and the response FIFO is consumed.
 */
extern s32 g_cdIrqAccum;
extern u8 g_clockMode;
extern u8 g_RTCTimeBCD[2];

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

typedef struct {
    union {
        struct {
            s16 x;
            s16 y;
        } coord;
        s32 packed;
    } pos;
    u32 wh;
} GlyphDrawState;

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

/**
 * A double-buffer slot containing the display/draw environments and the clear rect.
 */
typedef struct
{
    DISPENV disp;       // +0x00
    DRAWENV draw;       // +0x14
    RECT    clearRect;  // +0x70
} DisplayBuffer;

/**
 * Top-level render state for the CheckPS overlay.
 * Contains two double-buffer frames, each with an OTag and a DisplayBuffer.
 */
typedef struct
{
    u8      _pad0[0x40];            // +0x0000
    u_long  oTagFront[0x1000];      // +0x0040
    DisplayBuffer front;            // +0x4040
    u8      _pad1[0x7C54];          // +0x40B8
    u_long  oTagBack[0x1000];       // +0xBD0C
    DisplayBuffer back;             // +0xFD0C
} CheckPSState;

void func_80050080(void);
void InitCheckPSDisplay(CheckPSState* state);
void func_8004FD68(int param_1);
void DrawString(u32 str, GlyphDrawState* drawState, s32 color);
void DrawGlyph(GlyphDrawState* drawState, u8* bitmap, s32 color);
void DrawSymmetricTestPattern(void);
s32 PollCdResponse(s32 arg0);
void SendCdCommand(int arg0);
void ExitCheckPS(void);

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