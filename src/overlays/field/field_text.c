#include "field_text.h"
#include "akao.h"
#include "cdrom.h"
#include "cd_resources.h"
#include "common.h"
#include "gpu_packet.h"
#include "display.h"
#include "field_animation.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define FIELD_TEXT_STATE_MASK 7
#define FIELD_TEXT_PORTRAIT_SLOT 0x08
#define FIELD_TEXT_PORTRAIT_RIGHT 0x10
#define FIELD_TEXT_PORTRAIT_OUTSIDE 0x20
#define FIELD_TEXT_PORTRAIT_MASK (FIELD_TEXT_PORTRAIT_RIGHT | FIELD_TEXT_PORTRAIT_OUTSIDE)
#define FIELD_TEXT_STYLE_BOLD 0x40
#define FIELD_TEXT_STYLE_PLAIN 0x80
#define FIELD_TEXT_STYLE_MASK (FIELD_TEXT_STYLE_BOLD | FIELD_TEXT_STYLE_PLAIN)
#define FIELD_TEXT_INSTANT 0x800
#define FIELD_TEXT_AUTO_CLOSE 0x1000
#define FIELD_TEXT_REOPEN_PACKED 0x2000
#define FIELD_TEXT_REOPEN_FIXED 0x4000
#define FIELD_TEXT_REOPEN_MASK (FIELD_TEXT_REOPEN_PACKED | FIELD_TEXT_REOPEN_FIXED)
/** @brief Portrait VRAM slot (0 or 1) of a flags word. */
#define FIELD_TEXT_PORTRAIT_INDEX(flags) (((flags) >> 3) & 1)
/** @brief Portrait placement (FieldTextPortraitSide) of a flags word. */
#define FIELD_TEXT_PORTRAIT_SIDE(flags) (((flags) >> 4) & 3)
/** @brief Reopen mode of a flags word: 1 packed, 2 fixed placement. */
#define FIELD_TEXT_REOPEN_MODE(flags) (((flags) >> 13) & 3)
#define FIELD_TEXT_CACHE_WIDTH 256
#define FIELD_TEXT_CACHE_HEIGHT 96
#define FIELD_TEXT_PIXELS_PER_WORD 4
#define FIELD_TEXT_CACHE_ROW_WORDS (FIELD_TEXT_CACHE_WIDTH / FIELD_TEXT_PIXELS_PER_WORD)
#define FIELD_TEXT_CACHE_ROW_BYTES (FIELD_TEXT_CACHE_ROW_WORDS * 2)
/** Glyph staging rows in the scratchpad: 16 glyph pixels plus the 4 carried from the previous glyph. */
#define FIELD_TEXT_STAGING_ROW_WORDS 5
#define FIELD_TEXT_LINE_HEIGHT 12
#define FIELD_TEXT_LINE_SPACING 16
#define FIELD_TEXT_PORTRAIT_SIZE 48
#define FIELD_TEXT_PORTRAIT_MARGIN 56
#define FIELD_TEXT_TRANSITION_FRAMES 4
/** Display time of a timed window, in frames. */
#define FIELD_TEXT_TIMED_FRAMES 50
/** Line width of the immediate-string window; wide enough never to wrap. */
#define FIELD_TEXT_IMMEDIATE_WIDTH 0xFF0

/* Character codes and glyph advances. Codes of 0x80 and above are two-byte characters. */
#define FIELD_TEXT_FIRST_PRINTABLE 0x20
#define FIELD_TEXT_WIDE_SPACE 0x80
/** Pseudo code for FIELD_TEXT_CMD_INDENT at a line start; widened when no portrait sits on the left. */
#define FIELD_TEXT_CODE_INDENT 0xFFFF
#define FIELD_TEXT_SPACE_WIDTH 5
#define FIELD_TEXT_WIDE_WIDTH 12
#define FIELD_TEXT_DOUBLE_BYTE_WIDTH 9
/** Added in 16 bits: commands 0x19-0x1F select the high byte (1-7) of a two-byte code. */
#define FIELD_TEXT_DOUBLE_BYTE_BIAS 0xFFE8
#define FIELD_TEXT_SHORT_DELAY 4
#define FIELD_TEXT_CHOICE_BLINK_FRAMES 4
#define FIELD_TEXT_PROMPT_BLINK_FRAMES 8

/* Buttons in the controller-protocol order of FieldInputState (PAD_BTN_* before the byte swap). */
#define FIELD_TEXT_PAD_L3 0x0002
#define FIELD_TEXT_PAD_UP 0x0010
#define FIELD_TEXT_PAD_DOWN 0x0040
#define FIELD_TEXT_PAD_CROSS 0x4000
#define FIELD_TEXT_SFX_CURSOR 0x7D
#define FIELD_TEXT_SFX_CONFIRM 0x7E
#define FIELD_TEXT_SFX_PAN 0x80

/* VRAM layout: the text cache and the window frame textures below it share the 4bpp page at (960, 256). */
#define FIELD_TEXT_CACHE_VRAM_X 960
#define FIELD_TEXT_CACHE_VRAM_Y 384
#define FIELD_TEXT_FRAME_VRAM_Y 480
#define FIELD_TEXT_TPAGE_VRAM_Y 256
#define FIELD_TEXT_PORTRAIT_VRAM_X 1012
#define FIELD_TEXT_PORTRAIT_VRAM_Y 272
#define FIELD_TEXT_CLUT_X 304
#define FIELD_TEXT_PORTRAIT_CLUT_Y 506
#define FIELD_TEXT_TEXT_CLUT_Y 508
#define FIELD_TEXT_WINDOW_CLUT_Y 509
#define FIELD_TEXT_PROMPT_CLUT_Y 510
#define FIELD_TEXT_ALT_CLUT_Y 511
/* Texture coordinates of the cache, frame and portrait images inside that page. */
#define FIELD_TEXT_CACHE_V (FIELD_TEXT_CACHE_VRAM_Y - FIELD_TEXT_TPAGE_VRAM_Y)
#define FIELD_TEXT_FRAME_V (FIELD_TEXT_FRAME_VRAM_Y - FIELD_TEXT_TPAGE_VRAM_Y)
#define FIELD_TEXT_PORTRAIT_U ((FIELD_TEXT_PORTRAIT_VRAM_X - FIELD_TEXT_CACHE_VRAM_X) * FIELD_TEXT_PIXELS_PER_WORD)

#define FIELD_TEXT_FRAME_TILE_WIDTH 64
#define FIELD_TEXT_FRAME_TILE_HEIGHT 32
#define FIELD_TEXT_BORDER_WIDTH 8
/** @brief P_TAG word that links a packet of type @p type to the packet at @p next. */
#define FIELD_TEXT_TAG(next, type) (((u32)(next) & 0xFFFFFF) | ((sizeof(type) / 4 - 1) << 24))
#define FIELD_TEXT_SPRITE_COLOR (0x65000000 | GPU_TINT_NEUTRAL)
#define FIELD_TEXT_QUAD_COLOR (0x2D000000 | GPU_TINT_NEUTRAL)
#define FIELD_TEXT_CHOICE_COLOR (0x7D000000 | GPU_TINT_NEUTRAL)
#define FIELD_TEXT_SPRITE_SHADOW 0x66000000
#define FIELD_TEXT_QUAD_SHADOW 0x2E000000

/* Fixed RAM blocks shared with the main executable and the field resource loader. */
#define FIELD_TEXT_SYSTEM ((FieldTextSystem*)0x801ED000)
#define FIELD_TEXT_WINDOWS (FIELD_TEXT_SYSTEM->windows)
#define FIELD_TEXT_IMMEDIATE_STATE (&FIELD_TEXT_SYSTEM->windows[1])
#define FIELD_TEXT_PENDING_CONFIG ((FieldTextConfig*)0x801ED408)
#define FIELD_TEXT_INPUT ((FieldInputState*)0x801ED600)
#define FIELD_TEXT_SCRATCH ((u16*)0x1F800000)
/** Transition mesh vertices, built in the same scratchpad area. */
#define FIELD_TEXT_MESH ((FieldTextVertex*)0x1F800000)
/** @brief Number of 64-pixel frame tiles across a window of @p width. */
#define FIELD_TEXT_MESH_COLUMNS(width) (((width) + 0x3F) >> 6)
/** @brief Number of 32-pixel frame tiles down a window of @p height. */
#define FIELD_TEXT_MESH_ROWS(height) (((height) + 0x1F) >> 5)
/** 4bpp text cache (256 by 96 pixels), preloaded with the window textures. */
#define FIELD_TEXT_CACHE ((u16*)0x801DE000)
/** 12-row, 16-pixel-wide glyph bitmaps, starting at the space character. */
#define FIELD_TEXT_FONT ((u16*)0x801E1200)
#define FIELD_TEXT_GLYPH_WIDTHS ((u8*)0x801E26E0)
#define FIELD_TEXT_GLYPH_RUN_OFFSETS ((u16*)0x801E2758)
#define FIELD_TEXT_GLYPH_RUNS ((u8*)0x801E2780)
/**
 * @brief Address of the cache word at @p byte_offset within a cache @p row.
 * @note The offset is added before the row address; a pointer sum would swap the addu operands.
 */
#define FIELD_TEXT_CACHE_WORD(row, byte_offset) ((u16*)((byte_offset) + (s32)(row)))

/** @brief Modes in the low three bits of a runtime window's flags. */
typedef enum
{
    FIELD_TEXT_CLOSED,
    FIELD_TEXT_OPENING,
    FIELD_TEXT_ACTIVE,
    FIELD_TEXT_CLOSING,
    FIELD_TEXT_TIMED,
    FIELD_TEXT_MODE_UNKNOWN_5,
    FIELD_TEXT_IMMEDIATE
} FieldTextMode;

/** @brief Portrait placement, bits 4-5 of a window's flags. */
typedef enum
{
    FIELD_TEXT_SIDE_LEFT,   /**< Inside the window, left of the text. */
    FIELD_TEXT_SIDE_RIGHT,  /**< Inside the window, right of the text. */
    FIELD_TEXT_SIDE_OUTSIDE /**< Beside the window; the frame does not widen. */
} FieldTextPortraitSide;

/** @brief Pending actions completed by the dialogue prompt. */
typedef enum
{
    FIELD_TEXT_FLOW_NONE,
    FIELD_TEXT_FLOW_END,
    FIELD_TEXT_FLOW_CLEAR,
    FIELD_TEXT_FLOW_NEWLINE,
    FIELD_TEXT_FLOW_WAIT,
    FIELD_TEXT_FLOW_CHOICE = 0x10
} FieldTextFlow;

/** @brief Control bytes below the first printable text character. */
typedef enum
{
    FIELD_TEXT_CMD_END = 0,
    FIELD_TEXT_CMD_NEWLINE = 1,
    FIELD_TEXT_CMD_WAIT_NEWLINE = 2,
    FIELD_TEXT_CMD_WAIT_CLEAR = 3,
    FIELD_TEXT_CMD_CLEAR = 4,
    FIELD_TEXT_CMD_WAIT = 5,
    FIELD_TEXT_CMD_FINISH = 6,
    FIELD_TEXT_CMD_CHOICE = 7,
    FIELD_TEXT_CMD_TWO_SPACES = 8,
    FIELD_TEXT_CMD_THREE_SPACES = 9,
    FIELD_TEXT_CMD_FOUR_SPACES = 10,
    FIELD_TEXT_CMD_SPACES = 11,
    FIELD_TEXT_CMD_SHORT_DELAY = 12,
    FIELD_TEXT_CMD_DELAY = 13,
    FIELD_TEXT_CMD_MACRO = 14,
    FIELD_TEXT_CMD_INLINE_TEXT = 15,
    FIELD_TEXT_CMD_COLOR = 16,
    FIELD_TEXT_CMD_DEFAULT_COLOR = 17,
    FIELD_TEXT_CMD_PREFIXED_GLYPH_RUN = 18,
    FIELD_TEXT_CMD_INDENT = 19,
    FIELD_TEXT_CMD_WIDE_CHARACTER = 25,
    FIELD_TEXT_CMD_EXTENDED_GLYPH_RUN = 31,
} FieldTextCommand;

/** @brief A 16-color palette followed by a 48-by-48, 4bpp portrait. */
typedef struct
{
    u16 palette[16];
    u8 pixels[FIELD_TEXT_PORTRAIT_SIZE][FIELD_TEXT_PORTRAIT_SIZE / 2];
} FieldTextPortrait;

/** @brief Byte view of a field text flags word. */
typedef struct
{
    u8 low;
    u8 byte1;
    u8 byte2;
    u8 byte3;
} FieldTextFlagBytes;

/** @brief Text flags accessed as a word or individual control bytes. */
typedef union
{
    u32 word;
    FieldTextFlagBytes b;
} FieldTextFlags;

/**
 * @brief Runtime state for one dialogue window.
 * @note Cache U coordinates count 4bpp pixels; each 256-pixel span wraps by line_height rows.
 *       Screen text lines are spaced 16 pixels apart. Inline macros use a budget of -1.
 */
typedef struct
{
    u8* text_cursor;
    u8* macro_cursor;
    u8* glyph_cursor;
    FieldTextPortrait* portrait;
    FieldTextFlags flags;
    u8 flow_code;
    u8 line_count;
    u8 choice_start_line;
    u8 choice_index;
    u8 choice_count;
    u8 pending_spaces;
    u8 char_delay;
    u8 text_color;
    u8 scroll_timer;
    u8 needs_init;
    u8 prompt_frame;
    u8 prompt_timer;
    u8 inline_text[41];
    u8 last_was_break;
    u16 transition_frame;
    s16 macro_remaining;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u16 line_advance;
    u16 line_height;
    u16 remaining_width;
    u16 cursor_u;
    u16 cursor_v;
    u16 region_start_u;
    u16 region_start_v;
    u16 region_end_u;
    u16 region_end_v;
    u16 dirty_start_u;
    u16 dirty_start_v;
    u16 dirty_end_u;
    u16 dirty_end_v;
    u16 row_carry[14];
    s32 transition_anchor_x;
    u32 unknown_0x90;
    s32 transition_anchor_y;
} FieldTextState;

/** @brief Screen point that an opening or closing window grows from or shrinks to. */
typedef struct
{
    u16 x;
    u16 y;
} FieldTextAnchor;

/** @brief Transition anchor accessed as a word (zero means none) or as coordinates. */
typedef union
{
    u32 word;
    FieldTextAnchor pos;
} FieldTextAnchorWord;

/** @brief Pending configuration copied into a field text-window state. */
struct FieldTextConfig
{
    FieldTextPortrait* portrait;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    FieldTextAnchorWord anchor;
    FieldTextFlags flags;
    u8* text;
};

/** @brief Field text renderer globals and four runtime window slots. */
typedef struct
{
    u8 _pad00[4];
    FieldTextConfig* configs;
    u8* timed_text;
    u8 _pad0C[0x14 - 0xC];
    u32 draw_mode0;
    u32 draw_mode1;
    u16 text_clut;
    u16 text_alt_clut;
    u16 window_clut;
    u16 prompt_clut;
    u16 portrait_clut[2];
    u16 portrait_slots;
    u16 _pad2A[(0x34 - 0x2A) / 2];
    FieldTextState windows[4];
} FieldTextSystem;

/** @brief Four screen-space corners of a quad, in POLY vertex order. */
typedef struct
{
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
    s16 x3;
    s16 y3;
} FieldTextQuad;

/** @brief Packet buffer viewed as SDK primitives or packed GPU words. */
typedef union
{
    DR_TPAGE draw_mode;
    SPRT sprite;
    SPRT_16 sprite16;
    POLY_FT4 quad;
    struct
    {
        u32 tag;
        u32 rgbc;
        u32 xy;
        u32 uv;
        u32 wh;
    } sprite_words;
    struct
    {
        u32 tag;
        u32 rgbc;
        u32 xy0;
        u32 uv0;
        u32 xy1;
        u32 uv1;
        u32 xy2;
        u32 uv2;
        u32 xy3;
        u32 uv3;
    } quad_words;
} FieldTextPacket;

/** @brief Ordering-table pair; text packets join the second chain. */
struct FieldOrderingTags
{
    u32 tag0;
    u32 tag1;
};

/** @brief Scratchpad vertex, also copied as a packed GPU coordinate word. */
typedef union
{
    DVECTOR pos;
    u32 word;
} FieldTextVertex;

/** @brief Controller sample used by dialogue navigation. */
typedef struct
{
    u8 device_type;
    u8 analog_direction_bits;
    u16 held_buttons;
    u16 pressed_buttons;
    u16 repeat_buttons;
    DVECTOR right_stick;
    u32 left_stick_axes;
} FieldInputState;

static void field_text_typeset(FieldTextState* state, s32 budget);
static void field_text_blit_glyph(FieldTextState* state, s32 code, u16 width);
static void field_text_clear_cache(FieldTextState* state);
static s32 field_text_advance_line(FieldTextState* state);
static void field_text_clear_window(FieldTextState* state);
static void field_text_apply_config(FieldTextState* state);
static void field_text_build_transition_quad(FieldTextState* state, FieldTextQuad* out, s32 frame);
static void field_text_build_window_packets(FieldTextState* state, u8** cursor, FieldOrderingTags* ot);
static void field_text_build_transition_packets(FieldTextState* state, FieldTextQuad* quad, u8** cursor, FieldOrderingTags* ot);
static void field_text_scroll_cache(FieldTextState* state);
static void field_text_queue_uploads(FieldTextState* state, u8** cursor);
static void field_text_save_config(u16 slot);
static void field_text_close(FieldTextState* state, s32 animate);
static void field_text_render_window(FieldTextState* state, u8** cursor, FieldOrderingTags* ot);
static void field_text_queue_portrait_upload(FieldTextPortrait* image, u8** cursor, s32 slot, s32 mirror);
static void field_text_restore_window(u16 slot, s32 placement_mode);

extern s16 g_field_text_portrait_slots;
extern s32 g_field_text_window0_flags;

/**
 * @brief Upload the text typeset by field_text_build_sprites from the cache to VRAM.
 * @note Whole 64-word cache rows are uploaded in place; a trailing partial row is packed first.
 */
void field_text_upload_immediate_cache(void)
{
    RECT rect;
    u16* cache;
    u16* src;
    u16* dst;
    s32 words;
    s32 rows;
    s32 text_width;
    s32 i;
    s32 j;

    rect.x = FIELD_TEXT_CACHE_VRAM_X;
    rect.y = FIELD_TEXT_CACHE_VRAM_Y;
    text_width = FIELD_TEXT_IMMEDIATE_STATE->width - FIELD_TEXT_IMMEDIATE_STATE->remaining_width;
    words = ((text_width & 3) + text_width + 5) >> 2;
    cache = FIELD_TEXT_CACHE;
    if (words >= FIELD_TEXT_CACHE_ROW_WORDS)
    {
        rect.w = FIELD_TEXT_CACHE_ROW_WORDS;
        rows = words / FIELD_TEXT_CACHE_ROW_WORDS;
        rect.h = rows * FIELD_TEXT_LINE_HEIGHT;
        LoadImage(&rect, (u_long*)FIELD_TEXT_CACHE);
        cache += rect.w * rect.h;
        words -= rows * FIELD_TEXT_CACHE_ROW_WORDS;
        rect.y = rect.y + rect.h;
    }
    if (words > 0)
    {
        /* Pack glyph rows 1..11 behind row 0 so the partial span is contiguous. */
        dst = cache + words;
        src = cache + FIELD_TEXT_CACHE_ROW_WORDS;
        j = FIELD_TEXT_LINE_HEIGHT - 1;
        while (--j != -1)
        {
            i = words;
            while (--i != -1)
            {
                *dst++ = *src++;
            }
            src += FIELD_TEXT_CACHE_ROW_WORDS - words;
        }
        rect.w = words;
        rect.h = FIELD_TEXT_LINE_HEIGHT;
        LoadImage(&rect, (u_long*)cache);
    }
}

/**
 * @brief Decode text commands and draw glyphs until the character budget or a prompt stops the step.
 * @param state Window and nested text cursors to advance.
 * @param budget Character budget; zero draws without a limit.
 */
static void field_text_typeset(FieldTextState* state, s32 budget)
{
    u8* glyph_run;
    u8* cursor;
    u8* look_cursor;
    u8* look_text;
    u8* look_macro;
    u8* look_glyph_run;
    s32 remaining;
    s8 advance;
    s32 new_line;
    s32 first_character;
    s32 look_advance;
    s32 glyph_width;
    u16 code;
    u16 width;
    u16 look_code;
    u16 look_width;
    s16 look_budget;
    u32 character;
    u16 y;
    u32 x;
    s16 aligned_u;
    u8 opcode;
    u8 word_continues;
    FieldTextMacro* macro;
    FieldTextMacro* look_macro_entry;

    remaining = budget;
    width = 0;
    advance = 0;
    first_character = 1;
    y = state->cursor_v;
    x = (state->cursor_u + state->width) - state->remaining_width;
    while (x >= FIELD_TEXT_CACHE_WIDTH)
    {
        x -= FIELD_TEXT_CACHE_WIDTH;
        y += state->line_height;
    }
    aligned_u = x & 0xFFFC;
    state->dirty_end_u = aligned_u;
    state->dirty_start_u = aligned_u;
    state->dirty_end_v = y;
    state->dirty_start_v = y;
    if (state->width == state->remaining_width)
    {
        state->last_was_break = 0;
        new_line = 1;
    }
    else
    {
        new_line = 0;
    }

    while (1)
    {
        glyph_run = state->glyph_cursor;
        if (glyph_run != NULL)
        {
            cursor = glyph_run;
            code = 0;
        }
        else
        {
            cursor = state->macro_cursor;
            if (cursor == NULL)
            {
                cursor = state->text_cursor;
            }
            code = 0;
        }

        do
        {
            if (state->pending_spaces != 0)
            {
                code = ' ';
                width = FIELD_TEXT_SPACE_WIDTH;
                advance = 0;
                state->pending_spaces = state->pending_spaces - 1;
            }
            else
            {
                opcode = *cursor;
                cursor++;
                if ((opcode < FIELD_TEXT_FIRST_PRINTABLE) && (opcode != FIELD_TEXT_CMD_WIDE_CHARACTER))
                {
                    switch (opcode)
                    {
                    case FIELD_TEXT_CMD_END:
                        if (state->glyph_cursor != NULL)
                        {
                            cursor = state->macro_cursor;
                            state->glyph_cursor = NULL;
                            if (cursor == NULL)
                            {
                                cursor = state->text_cursor;
                            }
                            break;
                        }
                        if (state->macro_cursor != NULL)
                        {
                            cursor = state->text_cursor;
                            state->macro_cursor = NULL;
                            break;
                        }
                        if (state->choice_count != 0)
                        {
                            goto set_wide;
                        }
                        state->flow_code = FIELD_TEXT_FLOW_END;
                        goto set_break;
                    case FIELD_TEXT_CMD_FINISH:
                        if (state->glyph_cursor != NULL)
                        {
                            cursor = state->macro_cursor;
                            state->glyph_cursor = NULL;
                            if (cursor == NULL)
                            {
                                cursor = state->text_cursor;
                            }
                            break;
                        }
                        if (state->macro_cursor != NULL)
                        {
                            cursor = state->text_cursor;
                            state->macro_cursor = NULL;
                            break;
                        }
                        state->text_cursor = NULL;
                        if (state->flags.word & FIELD_TEXT_AUTO_CLOSE)
                        {
                            field_text_close(state, 1);
                        }
                        return;
                    case FIELD_TEXT_CMD_NEWLINE:
                        if (field_text_advance_line(state) == 1)
                        {
                            goto store_and_return;
                        }
                        new_line = 1;
                        state->last_was_break = 0;
                        break;
                    case FIELD_TEXT_CMD_WAIT_NEWLINE:
                        state->flow_code = FIELD_TEXT_FLOW_NEWLINE;
                        goto set_break;
                    case FIELD_TEXT_CMD_WAIT_CLEAR:
                        state->flow_code = FIELD_TEXT_FLOW_CLEAR;
                        goto set_break;
                    case FIELD_TEXT_CMD_CLEAR:
                        if (first_character == 0)
                        {
                            return;
                        }
                        field_text_clear_window(state);
                        goto store_and_return;
                    case FIELD_TEXT_CMD_WAIT:
                        state->flow_code = FIELD_TEXT_FLOW_WAIT;
                        goto set_break;
                    case FIELD_TEXT_CMD_CHOICE:
                        if (state->choice_count == 0)
                        {
                            state->choice_start_line = state->line_count;
                        }
                        state->choice_count = state->choice_count + 1;
                        break;
                    case FIELD_TEXT_CMD_TWO_SPACES:
                        state->pending_spaces = 2;
                        break;
                    case FIELD_TEXT_CMD_THREE_SPACES:
                        state->pending_spaces = 3;
                        break;
                    case FIELD_TEXT_CMD_FOUR_SPACES:
                        state->pending_spaces = 4;
                        break;
                    case FIELD_TEXT_CMD_SPACES:
                        state->pending_spaces = *cursor;
                        cursor++;
                        break;
                    case FIELD_TEXT_CMD_SHORT_DELAY:
                        state->char_delay = FIELD_TEXT_SHORT_DELAY;
                        goto store_and_return;
                    case FIELD_TEXT_CMD_DELAY:
                        state->char_delay = *cursor;
                        cursor++;
                        goto store_and_return;
                    case FIELD_TEXT_CMD_MACRO:
                        opcode = *cursor;
                        cursor++;
                        state->text_cursor = cursor;
                        macro = &g_field_text_macros[opcode];
                        state->macro_cursor = macro->text;
                        cursor = state->macro_cursor;
                        state->macro_remaining = macro->character_limit;
                        break;
                    case FIELD_TEXT_CMD_INLINE_TEXT:
                        state->text_cursor = cursor;
                        state->macro_cursor = state->inline_text;
                        cursor = state->inline_text;
                        state->macro_remaining = -1;
                        break;
                    case FIELD_TEXT_CMD_COLOR:
                        state->text_color = *cursor;
                        cursor++;
                        break;
                    case FIELD_TEXT_CMD_DEFAULT_COLOR:
                        state->text_color = 0;
                        break;
                    case FIELD_TEXT_CMD_INDENT:
                        if (new_line != 0)
                        {
                            code = FIELD_TEXT_CODE_INDENT;
                            width = FIELD_TEXT_WIDE_WIDTH;
                            advance = 1;
                        }
                        break;
                    case FIELD_TEXT_CMD_PREFIXED_GLYPH_RUN:
                        opcode = *cursor;
                        cursor++;
                        if (opcode == 0)
                        {
                            code = ' ';
                            width = FIELD_TEXT_SPACE_WIDTH;
                            advance = 2;
                        }
                        /* fallthrough */
                    case FIELD_TEXT_CMD_EXTENDED_GLYPH_RUN:
                        opcode = *cursor + FIELD_TEXT_CMD_EXTENDED_GLYPH_RUN;
                        cursor++;
                        /* fallthrough */
                    default:
                        if (state->macro_cursor != NULL)
                        {
                            state->macro_cursor = cursor;
                        }
                        else
                        {
                            state->text_cursor = cursor;
                        }
                        state->glyph_cursor = FIELD_TEXT_GLYPH_RUNS + FIELD_TEXT_GLYPH_RUN_OFFSETS[opcode];
                        cursor = state->glyph_cursor;
                        break;
                    }
                }
                else
                {
                    if (opcode >= FIELD_TEXT_FIRST_PRINTABLE)
                    {
                        code = opcode;
                        advance = 1;
                    }
                    else
                    {
                        code = *cursor | ((opcode + FIELD_TEXT_DOUBLE_BYTE_BIAS) << 8);
                        cursor++;
                        advance = 2;
                    }

                    character = code;
                    if (character == FIELD_TEXT_WIDE_SPACE)
                    {
                        width = FIELD_TEXT_WIDE_WIDTH;
                    }
                    else if (character >= FIELD_TEXT_WIDE_SPACE)
                    {
                        width = FIELD_TEXT_DOUBLE_BYTE_WIDTH;
                    }
                    else
                    {
                        width = FIELD_TEXT_GLYPH_WIDTHS[character];
                    }
                }
            }

            character = code;

            if ((character == ' ') || (character == FIELD_TEXT_WIDE_SPACE))
            {
                if (state->remaining_width < width)
                {
                    state->last_was_break = 1;
                    code = 0;
                }
            }
        } while (code == 0);
        if (state->remaining_width < width)
        {
            if (field_text_advance_line(state) == 1)
            {
                return;
            }
            new_line = 1;
            state->last_was_break = 0;
        }
        word_continues = state->last_was_break;
        if ((code == ' ') || (code == FIELD_TEXT_WIDE_SPACE) || (code == FIELD_TEXT_CODE_INDENT))
        {
            state->last_was_break = 1;
        }
        else
        {
            look_width = width;
            if (word_continues != 0)
            {
                look_advance = advance;
                look_cursor = cursor;
                look_text = state->text_cursor;
                look_macro = state->macro_cursor;
                look_glyph_run = state->glyph_cursor;
                look_budget = state->macro_remaining;
                do
                {
                    if (look_glyph_run != NULL)
                    {
                        look_glyph_run = look_cursor;
                    }
                    else if (look_macro != NULL)
                    {
                        if (look_budget != -1)
                        {
                            if ((s16)(look_budget -= look_advance) <= 0)
                            {
                                look_cursor = NULL;
                            }
                        }
                        look_macro = look_cursor;
                    }
                    else
                    {
                        look_text = look_cursor;
                    }
                    look_cursor = look_glyph_run;
                    if (look_cursor == NULL)
                    {
                        look_cursor = look_text;
                        if (look_macro != NULL)
                        {
                            look_cursor = look_macro;
                        }
                    }
                    look_code = 0;
                    do
                    {
                        opcode = *look_cursor;
                        look_cursor++;
                        if ((opcode < FIELD_TEXT_FIRST_PRINTABLE) && (opcode != FIELD_TEXT_CMD_WIDE_CHARACTER))
                        {
                            switch (opcode)
                            {
                            case FIELD_TEXT_CMD_SHORT_DELAY:
                            case FIELD_TEXT_CMD_DELAY:
                            case FIELD_TEXT_CMD_COLOR:
                            case FIELD_TEXT_CMD_DEFAULT_COLOR:
                                break;
                            case FIELD_TEXT_CMD_END:
                            case FIELD_TEXT_CMD_FINISH:
                                if (look_glyph_run != NULL)
                                {
                                    look_glyph_run = NULL;
                                    look_cursor = look_text;
                                    if (look_macro != NULL)
                                    {
                                        look_cursor = look_macro;
                                    }
                                    break;
                                }
                                if (look_macro != NULL)
                                {
                                    look_macro = NULL;
                                    look_cursor = look_text;
                                    break;
                                }
                                /* fallthrough */
                            case FIELD_TEXT_CMD_NEWLINE:
                            case FIELD_TEXT_CMD_WAIT_NEWLINE:
                            case FIELD_TEXT_CMD_WAIT_CLEAR:
                            case FIELD_TEXT_CMD_CLEAR:
                            case FIELD_TEXT_CMD_WAIT:
                            case FIELD_TEXT_CMD_CHOICE:
                            case FIELD_TEXT_CMD_TWO_SPACES:
                            case FIELD_TEXT_CMD_THREE_SPACES:
                            case FIELD_TEXT_CMD_FOUR_SPACES:
                            case FIELD_TEXT_CMD_SPACES:
                            case FIELD_TEXT_CMD_INDENT:
                                word_continues = 0;
                                break;
                            case FIELD_TEXT_CMD_MACRO:
                                opcode = *look_cursor;
                                look_text = ++look_cursor;
                                look_macro_entry = &g_field_text_macros[opcode];
                                look_cursor = look_macro_entry->text;
                                look_budget = look_macro_entry->character_limit;
                                look_macro = look_cursor;
                                break;
                            case FIELD_TEXT_CMD_INLINE_TEXT:
                                look_text = look_cursor;
                                look_cursor = state->inline_text;
                                look_macro = look_cursor;
                                look_budget = -1;
                                break;
                            case FIELD_TEXT_CMD_PREFIXED_GLYPH_RUN:
                                opcode = *look_cursor;
                                look_cursor++;
                                if (opcode == 0)
                                {
                                    word_continues = 0;
                                }
                                /* fallthrough */
                            case FIELD_TEXT_CMD_EXTENDED_GLYPH_RUN:
                                opcode = *look_cursor + FIELD_TEXT_CMD_EXTENDED_GLYPH_RUN;
                                look_cursor++;
                                /* fallthrough */
                            default:
                                if (look_macro != NULL)
                                {
                                    look_macro = look_cursor;
                                }
                                else
                                {
                                    look_text = look_cursor;
                                }
                                look_cursor = FIELD_TEXT_GLYPH_RUNS + FIELD_TEXT_GLYPH_RUN_OFFSETS[opcode];
                                look_glyph_run = look_cursor;
                                break;
                            }
                        }
                        else
                        {
                            if (opcode >= FIELD_TEXT_FIRST_PRINTABLE)
                            {
                                look_code = opcode;
                                look_advance = 1;
                            }
                            else
                            {
                                look_code = *look_cursor | ((opcode + FIELD_TEXT_DOUBLE_BYTE_BIAS) << 8);
                                look_cursor++;
                                look_advance = 2;
                            }
                            if (look_code != 0)
                            {
                                if ((look_code == ' ') || (look_code == FIELD_TEXT_WIDE_SPACE) || (look_code == FIELD_TEXT_CODE_INDENT))
                                {
                                    word_continues = 0;
                                }
                                else if (look_code >= FIELD_TEXT_WIDE_SPACE)
                                {
                                    look_width += FIELD_TEXT_DOUBLE_BYTE_WIDTH;
                                }
                                else
                                {
                                    look_width += FIELD_TEXT_GLYPH_WIDTHS[look_code];
                                }
                            }
                        }
                    } while (look_code == 0 && word_continues != 0);
                } while (word_continues != 0);

                if (state->remaining_width < look_width)
                {
                    if (field_text_advance_line(state) == 1)
                    {
                        return;
                    }
                    new_line = 1;
                }
                state->last_was_break = 0;
            }
        }
        if (state->glyph_cursor != NULL)
        {
            state->glyph_cursor = cursor;
        }
        else if (state->macro_cursor != NULL)
        {
            if (state->macro_remaining != -1)
            {
                state->macro_remaining -= advance;
                if (state->macro_remaining <= 0)
                {
                    cursor = NULL;
                }
            }
            state->macro_cursor = cursor;
        }
        else
        {
            state->text_cursor = cursor;
        }
        glyph_width = width;
        if (code == FIELD_TEXT_CODE_INDENT)
        {
            code = ' ';
            width = FIELD_TEXT_WIDE_WIDTH;
            if ((state->portrait == NULL) || (state->flags.word & FIELD_TEXT_PORTRAIT_MASK))
            {
                field_text_blit_glyph(state, ' ', FIELD_TEXT_WIDE_WIDTH);
            }
            glyph_width = width;
        }
        if (glyph_width != 0)
        {
            field_text_blit_glyph(state, code, glyph_width);
        }
        if ((remaining != 0) && !(state->flags.word & FIELD_TEXT_INSTANT) && ((new_line == 0) || (code != ' ')))
        {
            first_character = 0;
            remaining--;
            new_line = 0;
            if (remaining == 0)
            {
                break;
            }
        }
    }
    return;

    /* Stop tails shared by the prompt and delay commands. */
set_wide:
    state->flow_code = FIELD_TEXT_FLOW_CHOICE;
    state->prompt_frame = 0;
    state->prompt_timer = FIELD_TEXT_CHOICE_BLINK_FRAMES;
    state->choice_index = 0;
    goto store_and_return;

set_break:
    state->prompt_frame = 0;
    state->prompt_timer = FIELD_TEXT_PROMPT_BLINK_FRAMES;

store_and_return:
    if (state->glyph_cursor != NULL)
    {
        state->glyph_cursor = cursor;
        return;
    }
    if (state->macro_cursor != NULL)
    {
        state->macro_cursor = cursor;
        return;
    }
    state->text_cursor = cursor;
}

/**
 * @brief Expand a font glyph into the 4bpp text cache with a drop shadow.
 * @param state Destination window and text style.
 * @param code Character code; the font starts at the space character.
 * @param width Glyph advance in pixels.
 */
static void field_text_blit_glyph(FieldTextState* state, s32 code, u16 width)
{
    u16* staging;
    u16* carry;
    u16* glyph_row;
    u8* cache_line;
    u16* cache_dst;
    u16* row_src;
    u16* row_dst;
    u8* staging_row;
    s32 rows;
    s32 line_x;
    s32 i;
    s32 j;
    s32 r;
    s32 col;
    s32 words;
    s32 fill_low;
    s32 fill_high;
    s32 shadow_low;
    s32 shadow_high;
    u32 pixel_count;
    u32 words_left;
    u32 bit;
    u32 fill;
    u32 shade;
    u32 acc;
    u32 cur;
    u32 next;
    u32 room;
    u32 pixel_span;
    s32 high_nibble;

    staging = FIELD_TEXT_SCRATCH;
    carry = state->row_carry;
    rows = state->line_height;
    i = rows;
    line_x = state->width - state->remaining_width;
    while (--i != -1)
    {
        j = FIELD_TEXT_STAGING_ROW_WORDS - 1;
        if (line_x != 0)
        {
            *staging++ = *carry++;
        }
        else
        {
            j = FIELD_TEXT_STAGING_ROW_WORDS;
        }
        while (--j != -1)
        {
            *staging++ = 0;
        }
    }

    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        fill_low = 6;
        fill_high = 0x60;
        shadow_low = 7;
        shadow_high = 0x70;
        pixel_count = width + 2;
    }
    else
    {
        switch (state->text_color)
        {
        case 0:
            fill_low = 2;
            fill_high = 0x20;
            shadow_low = 3;
            shadow_high = 0x30;
            break;
        case 1:
            fill_low = 4;
            fill_high = 0x40;
            shadow_low = 5;
            shadow_high = 0x50;
            break;
        case 2:
            fill_low = 6;
            fill_high = 0x60;
            shadow_low = 7;
            shadow_high = 0x70;
            break;
        case 3:
            fill_low = 8;
            fill_high = 0x80;
            shadow_low = 9;
            shadow_high = 0x90;
            break;
        case 4:
            fill_low = 0xA;
            fill_high = 0xA0;
            shadow_low = 0xB;
            shadow_high = 0xB0;
            break;
        case 5:
            fill_low = 0xC;
            fill_high = 0xC0;
            shadow_low = 0xD;
            shadow_high = 0xD0;
            break;
        default:
            fill_low = 0xE;
            fill_high = 0xE0;
            shadow_low = 0xF;
            shadow_high = 0xF0;
            break;
        }
        pixel_count = width + 1;
    }

    glyph_row = FIELD_TEXT_FONT + ((u16)code - ' ') * FIELD_TEXT_LINE_HEIGHT;
    staging_row = (u8*)FIELD_TEXT_SCRATCH + (((u32)line_x & 3) >> 1);
    fill = 0;
    shade = 0;
    acc = 0;
    for (i = rows - 1; i != -1; i--)
    {
        u8* pixels = staging_row;

        high_nibble = line_x & 1;
        bit = 0x8000;
        if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
        {
            if (i != 0)
            {
                next = *glyph_row;
                cur = (next & 0xFFFF) >> 1;
                next |= (next & 0xFFFF) >> 2;
                acc |= next;
                next |= cur;
                shade |= next;
            }
            else
            {
                cur = 0;
                next = 0;
            }
            for (j = pixel_count - 1; j != -1; j--)
            {
                if (high_nibble == 0)
                {
                    if ((shade & bit) != 0)
                    {
                        *pixels = shadow_low | (*pixels & 0xF0);
                    }
                    high_nibble = 1;
                    if ((fill & bit) != 0)
                    {
                        *pixels = fill_low | (*pixels & 0xF0);
                    }
                }
                else
                {
                    if ((shade & bit) != 0)
                    {
                        *pixels = shadow_high | (*pixels & 0xF);
                    }
                    high_nibble = 0;
                    if ((fill & bit) != 0)
                    {
                        *pixels = fill_high | (*pixels & 0xF);
                    }
                    pixels++;
                }
                bit >>= 1;
            }
            shade = acc;
            fill = cur;
        }
        else
        {
            cur = *glyph_row;
            next = cur >> 1;
            acc |= next;
            next |= cur;
            for (j = pixel_count - 1; j != -1; j--)
            {
                if (high_nibble == 0)
                {
                    if ((acc & bit) != 0)
                    {
                        *pixels = shadow_low | (*pixels & 0xF0);
                    }
                    high_nibble = 1;
                    if ((cur & bit) != 0)
                    {
                        *pixels = fill_low | (*pixels & 0xF0);
                    }
                }
                else
                {
                    if ((acc & bit) != 0)
                    {
                        *pixels = shadow_high | (*pixels & 0xF);
                    }
                    high_nibble = 0;
                    if ((cur & bit) != 0)
                    {
                        *pixels = fill_high | (*pixels & 0xF);
                    }
                    pixels++;
                }
                bit >>= 1;
            }
        }
        glyph_row++;
        acc = next;
        staging_row += FIELD_TEXT_STAGING_ROW_WORDS * sizeof(u16);
    }

    i = state->cursor_v;
    j = state->cursor_u + line_x;
    while (j >= FIELD_TEXT_CACHE_WIDTH)
    {
        j -= FIELD_TEXT_CACHE_WIDTH;
        i += rows;
    }

    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        room = state->remaining_width + 4;
    }
    else
    {
        room = state->remaining_width;
    }
    if (room < pixel_count)
    {
        pixel_span = room + (line_x & 3);
    }
    else
    {
        pixel_span = pixel_count + (line_x & 3);
    }
    staging = FIELD_TEXT_SCRATCH;
    words_left = (pixel_span + 3) >> 2;
    if (words_left != 0)
    {
        do
        {
            cache_line = (u8*)FIELD_TEXT_CACHE + i * FIELD_TEXT_CACHE_ROW_BYTES;
            col = j >> 2;
            cache_dst = FIELD_TEXT_CACHE_WORD(cache_line, col * 2);
            if ((u32)(col + words_left) > FIELD_TEXT_CACHE_ROW_WORDS)
            {
                words = FIELD_TEXT_CACHE_ROW_WORDS - col;
                words_left -= words;
                j = 0;
                i += rows;
            }
            else
            {
                j += words_left * 4;
                words = words_left;
                words_left = 0;
            }
            row_src = staging;
            for (r = rows - 1; r != -1; r--)
            {
                u16* src = row_src;

                row_dst = cache_dst;
                for (col = words - 1; col != -1; col--)
                {
                    *row_dst++ = *src++;
                }
                row_src += FIELD_TEXT_STAGING_ROW_WORDS;
                cache_dst += FIELD_TEXT_CACHE_ROW_WORDS;
            }
            staging += words;
        } while (words_left != 0);
    }

    state->dirty_end_u = j;
    j = (line_x & 3) + width;
    staging = FIELD_TEXT_SCRATCH + (j >> 2);
    carry = state->row_carry;
    state->dirty_end_v = i;
    for (i = rows - 1; i != -1; i--)
    {
        *carry++ = *staging;
        staging += FIELD_TEXT_STAGING_ROW_WORDS;
    }
    state->remaining_width = state->remaining_width - width;
}

/**
 * @brief Clear the window cache region and mark it for upload.
 * @param state Window whose cached text is erased.
 */
static void field_text_clear_cache(FieldTextState* state)
{
    u16* cache_row;
    u16* pixel_word;
    s32 pixel_span;
    s32 byte_span;
    s32 cache_u;
    s32 cache_v;
    s32 glyph_rows;
    s32 rows_remaining;
    s32 words_remaining;

    cache_v = state->region_start_v;
    cache_u = state->region_start_u;
    if (cache_v == state->region_end_v)
    {
        pixel_span = state->region_end_u - cache_u;
        byte_span = pixel_span >> 1;
    }
    else
    {
        pixel_span = FIELD_TEXT_CACHE_WIDTH - cache_u;
        byte_span = pixel_span >> 1;
    }
    glyph_rows = state->line_height;
    cache_row = (FIELD_TEXT_CACHE + (cache_u >> 2)) + (cache_v * FIELD_TEXT_CACHE_ROW_WORDS);
    for (rows_remaining = glyph_rows - 1; rows_remaining != -1; rows_remaining--)
    {
        pixel_word = cache_row;
        words_remaining = byte_span >> 1;
        while (--words_remaining != -1)
        {
            *pixel_word++ = 0;
        }
        cache_row += FIELD_TEXT_CACHE_ROW_WORDS;
    }

    if (cache_v != state->region_end_v)
    {
        cache_v += glyph_rows;
        if (cache_v != state->region_end_v)
        {
            cache_row = FIELD_TEXT_CACHE + (cache_v * FIELD_TEXT_CACHE_ROW_WORDS);
            rows_remaining = (state->region_end_v - cache_v) * FIELD_TEXT_CACHE_ROW_WORDS;
            while (--rows_remaining != -1)
            {
                *cache_row++ = 0;
            }
        }
        if (state->region_end_u != 0)
        {
            cache_row = FIELD_TEXT_CACHE + (state->region_end_v * FIELD_TEXT_CACHE_ROW_WORDS);
            for (rows_remaining = glyph_rows - 1; rows_remaining != -1; rows_remaining--)
            {
                pixel_word = cache_row;
                words_remaining = state->region_end_u >> 2;
                while (--words_remaining != -1)
                {
                    *pixel_word++ = 0;
                }
                cache_row += FIELD_TEXT_CACHE_ROW_WORDS;
            }
        }
    }
    state->dirty_start_u = state->region_start_u;
    state->dirty_start_v = state->region_start_v;
    state->dirty_end_u = state->region_end_u;
    state->dirty_end_v = state->region_end_v;
}

/**
 * @brief Advance to the next text line or start scrolling when the window is full.
 * @param state Window cursor to advance.
 * @return 1 when the window is full; 0 when the next line is available.
 */
static s32 field_text_advance_line(FieldTextState* state)
{
    u16 x;
    u16 y;

    y = state->cursor_v;
    x = state->cursor_u + state->line_advance;
    while (x >= FIELD_TEXT_CACHE_WIDTH)
    {
        x -= FIELD_TEXT_CACHE_WIDTH;
        y += state->line_height;
    }
    if ((y == state->region_end_v) && (x == state->region_end_u))
    {
        state->scroll_timer = FIELD_TEXT_LINE_SPACING;
        return 1;
    }
    state->cursor_u = x;
    state->cursor_v = y;
    state->remaining_width = state->width;
    state->line_count = state->line_count + 1;
    return 0;
}

/**
 * @brief Clear the window and return its cursor to the first line.
 * @param state Window to clear.
 */
static void field_text_clear_window(FieldTextState* state)
{
    u16 start_u = state->region_start_u;
    u16 start_v = state->region_start_v;
    u16 width = state->width;

    state->line_count = 0;
    state->cursor_u = start_u;
    state->cursor_v = start_v;
    state->remaining_width = width;
    field_text_clear_cache(state);
}

/**
 * @brief Initialize field text textures, CLUT state, and window slots.
 */
void field_text_init(void)
{
    RECT rect;
    FieldTextState* state;
    s32 count;
    u32 draw_mode;
    FieldTextSystem* text_sys = FIELD_TEXT_SYSTEM;

    cdrom_stream(CD_RES_FIELD_WINDOW_TEXTURES, FIELD_TEXT_CACHE);

    /* The file starts with the four text CLUTs, followed by the window frame textures. */
    setRECT(&rect, FIELD_TEXT_CLUT_X, FIELD_TEXT_TEXT_CLUT_Y, 16, 4);
    LoadImage(&rect, (u_long*)FIELD_TEXT_CACHE);

    setRECT(&rect, FIELD_TEXT_CACHE_VRAM_X, FIELD_TEXT_FRAME_VRAM_Y, FIELD_TEXT_FRAME_TILE_WIDTH, FIELD_TEXT_FRAME_TILE_HEIGHT);
    LoadImage(&rect, (u_long*)(FIELD_TEXT_CACHE + 16 * 4));

    draw_mode = _get_mode(1, 0, getTPage(0, 0, FIELD_TEXT_CACHE_VRAM_X, FIELD_TEXT_TPAGE_VRAM_Y));
    text_sys->draw_mode0 = draw_mode;
    text_sys->draw_mode1 = draw_mode;
    text_sys->text_clut = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_TEXT_CLUT_Y);
    text_sys->text_alt_clut = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_ALT_CLUT_Y);
    text_sys->window_clut = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_WINDOW_CLUT_Y);
    text_sys->prompt_clut = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_PROMPT_CLUT_Y);
    text_sys->portrait_clut[0] = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_PORTRAIT_CLUT_Y);
    text_sys->portrait_clut[1] = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_PORTRAIT_CLUT_Y + 1);
    text_sys->portrait_slots = 0;

    state = FIELD_TEXT_WINDOWS;
    for (count = 3; count != -1; count--)
    {
        state->flags.word &= ~FIELD_TEXT_STATE_MASK;
        state++;
    }

    DrawSync(0);
}

/**
 * @brief Deactivate all field text windows and release portrait slots.
 */
void field_text_reset_windows(void)
{
    FieldTextState* state;
    s32 count;

    g_field_text_portrait_slots = 0;
    state = FIELD_TEXT_WINDOWS;
    for (count = 3; count != -1; count--)
    {
        state->flags.word &= ~FIELD_TEXT_STATE_MASK;
        state++;
    }
}

/**
 * @brief Reset the scratch state used for immediate string rendering.
 */
void field_text_reset_scratch(void)
{
    if ((g_field_text_window0_flags & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_TIMED)
    {
        field_text_close(FIELD_TEXT_WINDOWS, 0);
    }
    /* Without the loop scope the state pointer and the flag masks swap registers. */
    do
    {
        FieldTextState* state = FIELD_TEXT_IMMEDIATE_STATE;
        state->dirty_end_u = FIELD_TEXT_CACHE_WIDTH;
        state->region_end_u = FIELD_TEXT_CACHE_WIDTH;
        state->dirty_end_v = FIELD_TEXT_CACHE_HEIGHT;
        state->region_end_v = FIELD_TEXT_CACHE_HEIGHT;
        state->line_advance = FIELD_TEXT_IMMEDIATE_WIDTH;
        state->width = FIELD_TEXT_IMMEDIATE_WIDTH;
        state->remaining_width = FIELD_TEXT_IMMEDIATE_WIDTH;
        state->line_height = FIELD_TEXT_LINE_HEIGHT;
        state->dirty_start_u = 0;
        state->cursor_u = 0;
        state->region_start_u = 0;
        state->dirty_start_v = 0;
        state->cursor_v = 0;
        state->region_start_v = 0;
        state->line_count = 0;
        state->portrait = NULL;
        state->text_cursor = NULL;
        state->macro_cursor = NULL;
        state->glyph_cursor = NULL;
        state->flow_code = FIELD_TEXT_FLOW_NONE;
        state->pending_spaces = 0;
        state->choice_count = 0;
        state->text_color = 0;
        state->scroll_timer = 0;
        state->needs_init = 0;

        state->flags.word = ((((state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_IMMEDIATE) & ~FIELD_TEXT_STYLE_MASK) | FIELD_TEXT_INSTANT) &
                            ~FIELD_TEXT_AUTO_CLOSE;
    } while (0);
}

/**
 * @brief Typeset a string and describe its cached spans as sprite primitives.
 * @param prim Output sprite array.
 * @param text Text to typeset.
 * @param text_style Text palette/style selector; only the low 16 bits are used.
 * @return Number of sprite spans written.
 */
s32 field_text_build_sprites(SPRT* prim, u8* text, s32 text_style)
{
    u16 style = text_style;
    s32 count = 0;
    FieldTextState* state = FIELD_TEXT_IMMEDIATE_STATE;
    u16* carry;
    s32 remaining;
    s32 start_x;
    s32 end_x;
    s32 tex_u;
    s32 tex_v;
    s32 col;
    s32 cols;

    state->last_was_break = 1;
    state->text_color = style & 7;
    carry = state->row_carry;
    state->macro_cursor = NULL;
    state->glyph_cursor = NULL;
    state->pending_spaces = 0;
    state->flow_code = FIELD_TEXT_FLOW_NONE;
    remaining = state->line_height;
    start_x = state->width - state->remaining_width;
    state->text_cursor = text;
    while (--remaining != -1)
    {
        *carry++ = 0;
    }
    field_text_typeset(state, 0);
    tex_u = start_x;
    state->remaining_width = state->remaining_width & 0xFFFC;
    end_x = state->width - state->remaining_width;
    tex_v = 0;
    while (tex_u >= FIELD_TEXT_CACHE_WIDTH)
    {
        tex_u -= FIELD_TEXT_CACHE_WIDTH;
        tex_v += FIELD_TEXT_LINE_HEIGHT;
    }
    remaining = ((end_x - start_x) + 3) >> 2;
    if (remaining != 0)
    {
        do
        {
            col = tex_u >> 2;
            /* In 8 bits this is tex_v + FIELD_TEXT_CACHE_V. */
            prim->v0 = tex_v - 0x80;
            prim->u0 = tex_u;
            if ((col + remaining) > FIELD_TEXT_CACHE_ROW_WORDS)
            {
                cols = FIELD_TEXT_CACHE_ROW_WORDS - col;
                tex_u = 0;
                tex_v += FIELD_TEXT_LINE_HEIGHT;
                remaining -= cols;
            }
            else
            {
                cols = remaining;
                remaining = 0;
            }
            setWH(prim, cols * FIELD_TEXT_PIXELS_PER_WORD, FIELD_TEXT_LINE_HEIGHT);
            if (style >= 8)
            {
                prim->clut = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_TEXT_CLUT_Y);
            }
            else
            {
                prim->clut = getClut(FIELD_TEXT_CLUT_X, FIELD_TEXT_ALT_CLUT_Y);
            }
            prim++;
            count++;
        } while (remaining != 0);
    }
    return count;
}

/**
 * @brief Open a text window after the cache region used by earlier active slots.
 * @param slot Window slot index; only the low 16 bits are used.
 * @note Old-style definition: callers pass a word and the body works on a u16.
 */
void field_text_open_packed_window(slot) u16 slot;
{
    FieldTextSystem* system = FIELD_TEXT_SYSTEM;
    FieldTextState* state;
    FieldTextState* prev;
    u32 flags;
    s32 n;
    s32 w;
    s32 x;
    s32 y;
    u16 wrap;

    if ((g_field_text_window0_flags & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_TIMED)
    {
        field_text_close(FIELD_TEXT_WINDOWS, 0);
    }
    state = &system->windows[slot];
    if ((state->flags.word & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_ACTIVE)
    {
        field_text_close(state, 0);
    }
    flags = state->flags.word;
    if ((flags & FIELD_TEXT_STATE_MASK) != 0)
    {
        state->flags.word = (flags & ~FIELD_TEXT_REOPEN_MASK) | FIELD_TEXT_REOPEN_PACKED;
        field_text_save_config(slot);
        return;
    }
    field_text_apply_config(state);
    if (state->portrait != NULL)
    {
        if ((system->portrait_slots & 1) == 0)
        {
            state->flags.word &= ~FIELD_TEXT_PORTRAIT_SLOT;
            system->portrait_slots |= 1;
        }
        else
        {
            state->flags.word |= FIELD_TEXT_PORTRAIT_SLOT;
            system->portrait_slots |= 2;
        }
    }
    x = 0;
    y = 0;
    n = slot;
    prev = &system->windows[0];
    while (--n != -1)
    {
        if ((prev->flags.word & FIELD_TEXT_STATE_MASK) != 0)
        {
            x = prev->region_end_u;
            y = prev->region_end_v;
        }
        prev += 1;
    }
    n = state->height;
    state->dirty_start_u = x;
    state->cursor_u = x;
    state->region_start_u = x;
    state->dirty_start_v = y;
    state->cursor_v = y;
    state->region_start_v = y;
    while (n > 0)
    {
        w = state->line_advance;
        while (w > 0)
        {
            wrap = FIELD_TEXT_CACHE_WIDTH - x;
            if (w >= wrap)
            {
                w -= wrap;
                x = 0;
                y += state->line_height;
            }
            else
            {
                x += w;
                w = 0;
            }
        }
        n -= FIELD_TEXT_LINE_SPACING;
    }
    state->dirty_end_u = x;
    state->region_end_u = x;
    state->dirty_end_v = y;
    state->region_end_v = y;
}

/**
 * @brief Open a text window in its fixed cache region.
 * @param slot Window slot index; only the low 16 bits are used.
 * @note Old-style definition: callers pass a word and the body works on a u16.
 */
void field_text_open_fixed_window(slot) u16 slot;
{
    FieldTextSystem* system = FIELD_TEXT_SYSTEM;
    FieldTextState* state;
    u32 flags;
    s32 h;
    s32 w;
    s32 x;
    s32 y;
    u16 wrap;

    if ((g_field_text_window0_flags & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_TIMED)
    {
        field_text_close(FIELD_TEXT_WINDOWS, 0);
    }
    state = &system->windows[slot];
    if ((state->flags.word & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_ACTIVE)
    {
        field_text_close(state, 0);
    }
    flags = state->flags.word;
    if ((flags & FIELD_TEXT_STATE_MASK) != 0)
    {
        state->flags.word = (flags & ~FIELD_TEXT_REOPEN_MASK) | FIELD_TEXT_REOPEN_FIXED;
        field_text_save_config(slot);
        return;
    }
    field_text_apply_config(state);
    if (state->portrait != NULL)
    {
        if (slot == 0)
        {
            state->flags.word &= ~FIELD_TEXT_PORTRAIT_SLOT;
            system->portrait_slots |= 1;
        }
        else
        {
            state->flags.word |= FIELD_TEXT_PORTRAIT_SLOT;
            system->portrait_slots |= 2;
        }
    }
    if (slot == 0)
    {
        x = 0;
        y = 0;
    }
    else
    {
        x = 0;
        y = FIELD_TEXT_PORTRAIT_SIZE;
    }
    h = state->height;
    state->dirty_start_u = x;
    state->cursor_u = x;
    state->region_start_u = x;
    state->dirty_start_v = y;
    state->cursor_v = y;
    state->region_start_v = y;
    while (h > 0)
    {
        w = state->line_advance;
        while (w > 0)
        {
            wrap = FIELD_TEXT_CACHE_WIDTH - x;
            if (w >= wrap)
            {
                w -= wrap;
                x = 0;
                y += state->line_height;
            }
            else
            {
                x += w;
                w = 0;
            }
        }
        h -= FIELD_TEXT_LINE_SPACING;
    }
    state->dirty_end_u = x;
    state->region_end_u = x;
    state->dirty_end_v = y;
    state->region_end_v = y;
}

/**
 * @brief Apply the pending text configuration to a runtime window state.
 * @param state Window state to initialize.
 */
static void field_text_apply_config(FieldTextState* state)
{
    FieldTextConfig* config = FIELD_TEXT_PENDING_CONFIG;
    u32 flags;
    u32 state_flags;
    u32 config_value;
    s32 width;

    state->portrait = config->portrait;
    state->x = config->x;
    state->y = config->y;
    state->transition_anchor_x = config->anchor.pos.x;
    state->unknown_0x90 = 0;
    state->transition_anchor_y = config->anchor.pos.y;
    state->flags.b.byte3 = (u8)config->flags.word;
    flags = (state->flags.word & ~FIELD_TEXT_STYLE_MASK) | ((config->flags.word >> 2) & 0xC0);
    state->flags.word = flags;
    state_flags = flags & ~0x700;
    state_flags |= (config->flags.word >> 4) & 0x700;
    state->flags.word = state_flags;
    config_value = config->flags.word;
    if ((config_value & 0xC00) == 0xC00)
    {
        state->flags.word = state_flags & ~FIELD_TEXT_PORTRAIT_MASK;
    }
    else
    {
        state->flags.word = (state_flags & ~FIELD_TEXT_PORTRAIT_MASK) | ((config_value >> 6) & 0x30);
    }
    config_value = config->height;
    width = config->width;
    if ((state->portrait != NULL) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) != FIELD_TEXT_PORTRAIT_OUTSIDE) && ((s32)config_value < FIELD_TEXT_PORTRAIT_SIZE))
    {
        config_value = FIELD_TEXT_PORTRAIT_SIZE;
    }
    state->remaining_width = width;
    state->width = width;
    state->height = config_value;
    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        state->line_advance = width + 4;
        state->line_height = FIELD_TEXT_LINE_HEIGHT + 1;
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_ACTIVE;
    }
    else
    {
        state->line_height = FIELD_TEXT_LINE_HEIGHT;
        state->line_advance = width;
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_OPENING;
    }
    state->text_cursor = NULL;
    state->macro_cursor = NULL;
    state->glyph_cursor = NULL;
    state->last_was_break = 1;
    if ((config->anchor.word == 0) && ((config->flags.word & 0x70FF) == 0))
    {
        state->flags.b.byte2 = 0;
    }
    else
    {
        state->flags.b.byte2 = 1;
    }
    state->needs_init = 1;
    state->prompt_timer = 1;
    state->line_count = 0;
    state->char_delay = 0;
    state->text_color = 0;
    state->scroll_timer = 0;
    state->pending_spaces = 0;
    state->flow_code = FIELD_TEXT_FLOW_NONE;
    state->prompt_frame = 0;
    state->transition_frame = 0;
    state->choice_count = 0;
    state->flags.word &= ~FIELD_TEXT_INSTANT;
    state->flags.word &= ~FIELD_TEXT_AUTO_CLOSE;
    state->flags.word &= ~FIELD_TEXT_REOPEN_MASK;
}

/**
 * @brief Release the portrait VRAM slot held by a window.
 * @param state Text-window state.
 */
static inline void field_text_release_portrait(FieldTextState* state)
{
    if (state->portrait != NULL)
    {
        if ((state->flags.word & FIELD_TEXT_PORTRAIT_SLOT) == 0)
        {
            g_field_text_portrait_slots &= 0xFFFE;
        }
        else
        {
            g_field_text_portrait_slots &= 0xFFFD;
        }
    }
}

/**
 * @brief Copy a window configuration byte by byte.
 * @param dst_config Destination configuration.
 * @param src_config Source configuration.
 */
static inline void field_text_copy_config(FieldTextConfig* dst_config, FieldTextConfig* src_config)
{
    u8* dst = (u8*)dst_config;
    u8* src = (u8*)src_config;
    s32 bytes_remaining;

    for (bytes_remaining = sizeof(FieldTextConfig) - 1; bytes_remaining != -1; bytes_remaining--)
    {
        *dst++ = *src++;
    }
}

/**
 * @brief Load a saved window configuration and reopen the slot with it.
 * @param slot Window slot index.
 * @param placement_mode 1 uses packed placement; other values use fixed placement.
 */
static inline void field_text_reopen_window(u16 slot, s32 placement_mode)
{
    FieldTextConfig* config;

    config = &g_field_text_saved_configs[slot];
    field_text_copy_config(FIELD_TEXT_PENDING_CONFIG, config);
    if (placement_mode == 1)
    {
        field_text_open_packed_window(slot);
    }
    else
    {
        field_text_open_fixed_window(slot);
    }
    if (config->text != NULL)
    {
        field_text_set_string(slot, config->text, config->flags.b.byte2);
    }
}

/**
 * @brief Release a window's portrait slot and start closing it.
 * @param state Text-window state; bold windows close at once, others animate.
 */
static inline void field_text_start_closing(FieldTextState* state)
{
    field_text_release_portrait(state);
    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        state->flags.word = state->flags.word & ~FIELD_TEXT_STATE_MASK;
    }
    else
    {
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_CLOSING;
        state->transition_frame = 0;
    }
}

/**
 * @brief Update, upload, and render all field text windows for one frame.
 * @param packet_cursor Address of the render-packet cursor.
 * @param ot Ordering-table base address.
 * @param draw_count Current field draw count; 1 selects the render-only path.
 */
void field_text_update(u8** packet_cursor, FieldOrderingTags* ot, s32 draw_count)
{
    FieldInputState* input = FIELD_TEXT_INPUT;
    FieldTextState* state = FIELD_TEXT_WINDOWS;
    s32 i;
    s32 keys;
    s32 choice;

    i = 3;
    do
    {
        switch ((u8)state->flags.word & FIELD_TEXT_STATE_MASK)
        {
        case FIELD_TEXT_OPENING:
        case FIELD_TEXT_ACTIVE:
        case FIELD_TEXT_CLOSING:
            if (state->needs_init == 1)
            {
                if (state->portrait != NULL)
                {
                    field_text_queue_portrait_upload(state->portrait, packet_cursor, FIELD_TEXT_PORTRAIT_INDEX(state->flags.word),
                                                     (state->flags.word & FIELD_TEXT_PORTRAIT_MASK) != FIELD_TEXT_PORTRAIT_RIGHT);
                }
                field_text_clear_window(state);
                field_text_queue_uploads(state, packet_cursor);
                state->needs_init = 0;
            }
            else if (draw_count == 1)
            {
                field_text_render_window(state, packet_cursor, ot);
                break;
            }
            else
            {
                if (state->flow_code != FIELD_TEXT_FLOW_NONE)
                {
                    if (input->device_type < 3)
                    {
                        if (state->flow_code == FIELD_TEXT_FLOW_CHOICE)
                        {
                            switch (input->device_type)
                            {
                            case 1:
                            case 2:
                                if (input->left_stick_axes != 0)
                                {
                                    keys = input->analog_direction_bits;
                                }
                                else
                                {
                                    keys = input->repeat_buttons;
                                }
                                break;
                            case 0:
                                keys = input->repeat_buttons;
                                break;
                            default:
                                keys = 0;
                                break;
                            }
                            if ((keys & FIELD_TEXT_PAD_UP) != 0)
                            {
                                choice = state->choice_index;
                                if (choice == 0)
                                {
                                    choice = state->choice_count;
                                }
                                state->choice_index = choice - 1;
                                akao_play_sfx(FIELD_TEXT_SFX_CURSOR, 0, FIELD_TEXT_SFX_PAN, AKAO_VOLUME_MAX);
                            }
                            if ((keys & FIELD_TEXT_PAD_DOWN) != 0)
                            {
                                if (state->choice_index < (state->choice_count - 1))
                                {
                                    state->choice_index = state->choice_index + 1;
                                }
                                else
                                {
                                    state->choice_index = 0;
                                }
                                akao_play_sfx(FIELD_TEXT_SFX_CURSOR, 0, FIELD_TEXT_SFX_PAN, AKAO_VOLUME_MAX);
                            }
                            if ((input->pressed_buttons & (FIELD_TEXT_PAD_CROSS | FIELD_TEXT_PAD_L3)) != 0)
                            {
                                state->flow_code = FIELD_TEXT_FLOW_NONE;
                                state->choice_count = 0;
                                state->text_cursor = NULL;
                                if ((state->flags.word & FIELD_TEXT_AUTO_CLOSE) != 0)
                                {
                                    field_text_start_closing(state);
                                }
                                akao_play_sfx(FIELD_TEXT_SFX_CONFIRM, 0, FIELD_TEXT_SFX_PAN, AKAO_VOLUME_MAX);
                            }
                        }
                        else if (((input->pressed_buttons & (FIELD_TEXT_PAD_CROSS | FIELD_TEXT_PAD_L3)) != 0) && (state->prompt_frame != 2))
                        {
                            state->prompt_frame = 2;
                            state->prompt_timer = 3;
                        }
                    }
                }
                else if ((state->flags.word & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_ACTIVE)
                {
                    if (state->char_delay != 0)
                    {
                        state->char_delay = state->char_delay - 1;
                    }
                    else if (state->scroll_timer != 0)
                    {
                        state->scroll_timer = state->scroll_timer - 4;
                        if (state->scroll_timer == 0)
                        {
                            field_text_scroll_cache(state);
                            field_text_queue_uploads(state, packet_cursor);
                        }
                    }
                    else if (state->text_cursor != NULL)
                    {
                        field_text_typeset(state, 4);
                        field_text_queue_uploads(state, packet_cursor);
                    }
                }
            }
            field_text_render_window(state, packet_cursor, ot);
            if (state->flow_code != FIELD_TEXT_FLOW_NONE)
            {
                if (state->flow_code == FIELD_TEXT_FLOW_CHOICE)
                {
                    state->prompt_timer = state->prompt_timer - 1;
                    if (state->prompt_timer == 0)
                    {
                        state->prompt_frame = state->prompt_frame + 1;
                        if (state->prompt_frame == 4)
                        {
                            state->prompt_frame = 0;
                        }
                        state->prompt_timer = FIELD_TEXT_CHOICE_BLINK_FRAMES;
                    }
                }
                else
                {
                    state->prompt_timer = state->prompt_timer - 1;
                    if (state->prompt_timer == 0)
                    {
                        if (state->prompt_frame == 2)
                        {
                            switch (state->flow_code)
                            {
                            case FIELD_TEXT_FLOW_END:
                                state->text_cursor = NULL;
                                if ((state->flags.word & FIELD_TEXT_AUTO_CLOSE) != 0)
                                {
                                    field_text_start_closing(state);
                                }
                                break;
                            case FIELD_TEXT_FLOW_CLEAR:
                                field_text_clear_window(state);
                                field_text_queue_uploads(state, packet_cursor);
                                break;
                            case FIELD_TEXT_FLOW_NEWLINE:
                                field_text_advance_line(state);
                                break;
                            }
                            state->flow_code = FIELD_TEXT_FLOW_NONE;
                        }
                        else
                        {
                            state->prompt_frame = 1 - state->prompt_frame;
                            state->prompt_timer = FIELD_TEXT_PROMPT_BLINK_FRAMES;
                        }
                    }
                }
            }
            if (((state->flags.word & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_CLOSED) && ((state->flags.word & FIELD_TEXT_REOPEN_MASK) != 0))
            {
                field_text_reopen_window(3 - i, FIELD_TEXT_REOPEN_MODE(state->flags.word));
            }
            break;
        case FIELD_TEXT_TIMED:
            if (state->needs_init == 1)
            {
                state->transition_frame = FIELD_TEXT_TIMED_FRAMES;
                state->needs_init = 0;
            }
            if (state->text_cursor != NULL)
            {
                field_text_clear_window(state);
                field_text_typeset(state, 0);
                state->text_cursor = NULL;
                state->flow_code = FIELD_TEXT_FLOW_NONE;
                state->dirty_start_u = state->region_start_u;
                state->dirty_start_v = state->region_start_v;
                state->dirty_end_u = state->region_end_u;
                state->dirty_end_v = state->region_end_v;
                field_text_queue_uploads(state, packet_cursor);
            }
            field_text_build_window_packets(state, packet_cursor, ot);
            state->transition_frame = state->transition_frame - 1;
            if (state->transition_frame == 0)
            {
                field_text_release_portrait(state);
                state->flags.word = state->flags.word & ~FIELD_TEXT_STATE_MASK;
            }
            break;
        case FIELD_TEXT_MODE_UNKNOWN_5:
        case FIELD_TEXT_IMMEDIATE:
            break;
        default:
            break;
        }
        state += 1;
    } while (--i != -1);
}

/**
 * @brief Build the opening/closing transition quad for a text window.
 * @param state Text-window state.
 * @param out Output screen-space quad.
 * @param frame Transition frame in the range 0..4.
 */
static void field_text_build_transition_quad(FieldTextState* state, FieldTextQuad* out, s32 frame)
{
    s32 half_w;
    s32 half_h;
    s32 pad_h;
    s32 scale;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 x3;
    s32 y3;
    s32 center_x;
    s32 center_y;
    s32 frames_left;

    if ((state->portrait != NULL) && (FIELD_TEXT_PORTRAIT_SIDE(state->flags.word) < FIELD_TEXT_SIDE_OUTSIDE))
    {
        half_w = state->width + FIELD_TEXT_PORTRAIT_MARGIN;
    }
    else
    {
        half_w = state->width;
    }
    half_w = half_w / 2 + FIELD_TEXT_BORDER_WIDTH;
    half_h = (u32)state->height / 2;
    pad_h = half_h + FIELD_TEXT_BORDER_WIDTH;

    /* Start from the full-size edges and scale each corner toward the center. */
    x2 = -half_w;
    x3 = half_w;
    y1 = -pad_h;
    scale = frame + 3;
    if (scale > FIELD_TEXT_TRANSITION_FRAMES)
    {
        scale = FIELD_TEXT_TRANSITION_FRAMES;
    }
    x0 = (x2 * scale) / FIELD_TEXT_TRANSITION_FRAMES;
    x1 = (x3 * scale) / FIELD_TEXT_TRANSITION_FRAMES;
    x2 = (x2 * scale) / FIELD_TEXT_TRANSITION_FRAMES;
    x3 = (x3 * scale) / FIELD_TEXT_TRANSITION_FRAMES;
    y0 = ((y1 + 2) * frame) / FIELD_TEXT_TRANSITION_FRAMES - 2;
    y1 = ((y1 + 2) * frame) / FIELD_TEXT_TRANSITION_FRAMES - 2;
    y2 = ((half_h + 6) * frame) / FIELD_TEXT_TRANSITION_FRAMES + 2;
    y3 = ((half_h + 6) * frame) / FIELD_TEXT_TRANSITION_FRAMES + 2;

    center_x = state->x + half_w;
    center_y = state->y + pad_h;
    if (state->flags.b.byte2 != 0)
    {
        /* Move the center between the window and its anchor; anchor Y counts up from the bottom. */
        frames_left = FIELD_TEXT_TRANSITION_FRAMES - frame;
        center_x = (center_x * frame + state->transition_anchor_x * frames_left) / FIELD_TEXT_TRANSITION_FRAMES;
        center_y = (center_y * frame + (VRAM_DRAW_HEIGHT - state->transition_anchor_y) * frames_left) / FIELD_TEXT_TRANSITION_FRAMES;
    }
    out->x0 = center_x + x0;
    out->x1 = center_x + x1;
    out->x2 = center_x + x2;
    out->x3 = center_x + x3;
    out->y0 = center_y + y0;
    out->y1 = center_y + y1;
    out->y2 = center_y + y2;
    out->y3 = center_y + y3;
}

/**
 * @brief Center a portrait vertically and pack its screen Y coordinate.
 * @param y Top of the window.
 * @param h Height of the text area.
 * @return Screen Y coordinate in the upper halfword.
 */
static inline s32 field_text_portrait_y_word(s32 y, s32 h)
{
    h -= FIELD_TEXT_PORTRAIT_SIZE;
    h >>= 1;
    h += 8;
    y += h;
    return y << 16;
}

/**
 * @brief Build a flat field text-window packet chain and splice it into the OT.
 * @param state Text-window state.
 * @param cursor In/out render-packet cursor.
 * @param ot Ordering-table slot.
 */
static void field_text_build_window_packets(FieldTextState* state, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextSystem* text_system = FIELD_TEXT_SYSTEM;
    FieldTextPacket* prim;
    u8* first;
    u8* packet_cursor;
    s32 y;
    s32 uv;
    s32 xy;
    s32 pixels_remaining;
    s32 rows;
    u32 row_height;
    s32 texture_u_origin;
    s32 uv_base;
    s32 cache_u;
    s32 cache_v;
    s32 scroll_pixels;
    s32 available_pixels;
    u32 sprite_color;
    s32 bottom_border_v;
    s32 top_border_v;

    sprite_color = FIELD_TEXT_SPRITE_COLOR;
    packet_cursor = *cursor;
    prim = (FieldTextPacket*)packet_cursor;
    first = packet_cursor;
    packet_cursor += sizeof(DR_TPAGE);
    prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, DR_TPAGE);
    prim->sprite_words.rgbc = text_system->draw_mode0;
    texture_u_origin = 0;
    uv_base = FIELD_TEXT_FRAME_V;
    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == 0)
    {
        y = state->y;
        rows = 1;
        top_border_v = 0xF000;
        bottom_border_v = 0xF800;
        do
        {
            uv = (text_system->window_clut << 16) | (rows != 0 ? top_border_v : bottom_border_v) | texture_u_origin;
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            xy = state->x | (y << 16);
            prim->sprite_words.uv = uv;
            uv += 8;
            prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
            prim->sprite_words.rgbc = sprite_color;
            prim->sprite_words.wh = 0x80008;
            prim->sprite_words.xy = xy;
            xy += 8;
            if ((state->portrait != NULL) && (FIELD_TEXT_PORTRAIT_SIDE(state->flags.word) < FIELD_TEXT_SIDE_OUTSIDE))
            {
                pixels_remaining = state->width + FIELD_TEXT_PORTRAIT_MARGIN;
            }
            else
            {
                pixels_remaining = state->width;
            }
            if (pixels_remaining > 0)
            {
                do
                {
                    prim = (FieldTextPacket*)packet_cursor;
                    packet_cursor += sizeof(SPRT);
                    prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
                    prim->sprite_words.rgbc = sprite_color;
                    prim->sprite_words.xy = xy;
                    prim->sprite_words.uv = uv;
                    if (pixels_remaining >= 0x41)
                    {
                        prim->sprite_words.wh = 0x80040;
                        xy += 0x40;
                        pixels_remaining -= 0x40;
                    }
                    else
                    {
                        prim->sprite_words.wh = pixels_remaining | 0x80000;
                        xy += pixels_remaining;
                        pixels_remaining = 0;
                    }
                } while (pixels_remaining > 0);
            }
            uv += 0x40;
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            rows -= 1;
            prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
            prim->sprite_words.rgbc = sprite_color;
            prim->sprite_words.xy = xy;
            prim->sprite_words.uv = uv;
            prim->sprite_words.wh = 0x80008;
            y += state->height + 8;
        } while (rows != -1);
        rows = state->height;
        y = state->y + 8;
        if (rows > 0)
        {
            do
            {
                uv = (text_system->window_clut << 16) | (uv_base << 8) | (texture_u_origin + 0xE0);
                xy = state->x | (y << 16);
                row_height = 0x200000;
                if (rows < 0x21)
                {
                    row_height = rows << 16;
                }
                prim = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(SPRT);
                prim->sprite_words.xy = xy;
                xy += 8;
                prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
                prim->sprite_words.rgbc = sprite_color;
                prim->sprite_words.uv = uv;
                prim->sprite_words.wh = row_height | 8;
                uv -= 0x40;
                if ((state->portrait != NULL) && (FIELD_TEXT_PORTRAIT_SIDE(state->flags.word) < FIELD_TEXT_SIDE_OUTSIDE))
                {
                    pixels_remaining = state->width + FIELD_TEXT_PORTRAIT_MARGIN;
                }
                else
                {
                    pixels_remaining = state->width;
                }
                if (pixels_remaining > 0)
                {
                    do
                    {
                        prim = (FieldTextPacket*)packet_cursor;
                        packet_cursor += sizeof(SPRT);
                        prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
                        prim->sprite_words.rgbc = sprite_color;
                        prim->sprite_words.xy = xy;
                        prim->sprite_words.uv = uv;
                        if (pixels_remaining >= 0x41)
                        {
                            prim->sprite_words.wh = row_height | 0x40;
                            xy += 0x40;
                            pixels_remaining -= 0x40;
                        }
                        else
                        {
                            prim->sprite_words.wh = pixels_remaining | row_height;
                            xy += pixels_remaining;
                            pixels_remaining = 0;
                        }
                    } while (pixels_remaining > 0);
                }
                uv += 0x48;
                prim = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(SPRT);
                y += 0x20;
                rows -= 0x20;
                prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
                prim->sprite_words.rgbc = sprite_color;
                prim->sprite_words.xy = xy;
                prim->sprite_words.uv = uv;
                prim->sprite_words.wh = row_height | 8;
            } while (rows > 0);
        }
        texture_u_origin = 0;
    }
    uv_base = FIELD_TEXT_CACHE_V;
    row_height = state->line_height;
    cache_u = state->region_start_u;
    cache_v = state->region_start_v;
    scroll_pixels = state->scroll_timer;
    y = state->y + 8;
    rows = state->height >> 4;
    rows -= 1;
    if (rows != -1)
    {
        do
        {
            if ((state->portrait != NULL) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0))
            {
                xy = ((state->x + 0x40) & 0xFFFF) | (y << 16);
            }
            else
            {
                xy = ((state->x + 8) & 0xFFFF) | (y << 16);
            }
            pixels_remaining = state->line_advance;
            if (pixels_remaining > 0)
            {
                do
                {
                    if ((scroll_pixels != 0) && ((FIELD_TEXT_LINE_SPACING - scroll_pixels) >= row_height))
                    {
                        available_pixels = FIELD_TEXT_CACHE_WIDTH - cache_u;
                        cache_u += pixels_remaining;
                        if ((u32)pixels_remaining >= (u32)available_pixels)
                        {
                            xy += available_pixels;
                            pixels_remaining -= available_pixels;
                            cache_v += row_height;
                            cache_u = 0;
                        }
                        else
                        {
                            pixels_remaining = 0;
                        }
                    }
                    else
                    {
                        prim = (FieldTextPacket*)packet_cursor;
                        packet_cursor += sizeof(SPRT);
                        prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
                        prim->sprite_words.rgbc = sprite_color;
                        prim->sprite_words.xy = xy;
                        prim->sprite.u0 = texture_u_origin + cache_u;
                        prim->sprite.clut = text_system->text_clut;
                        if (scroll_pixels != 0)
                        {
                            prim->sprite.v0 = (uv_base + cache_v + FIELD_TEXT_LINE_SPACING) - scroll_pixels;
                            prim->sprite.h = row_height + (scroll_pixels - FIELD_TEXT_LINE_SPACING);
                        }
                        else
                        {
                            prim->sprite.v0 = uv_base + cache_v;
                            prim->sprite.h = row_height;
                        }
                        available_pixels = FIELD_TEXT_CACHE_WIDTH - cache_u;
                        cache_u += pixels_remaining;
                        if ((u32)pixels_remaining >= (u32)available_pixels)
                        {
                            prim->sprite.w = available_pixels;
                            xy += available_pixels;
                            pixels_remaining -= available_pixels;
                            cache_v += row_height;
                            cache_u = 0;
                        }
                        else
                        {
                            prim->sprite.w = pixels_remaining;
                            pixels_remaining = 0;
                        }
                    }
                } while (pixels_remaining > 0);
            }
            if (scroll_pixels != 0)
            {
                y += scroll_pixels;
                scroll_pixels = 0;
            }
            else
            {
                y += 0x10;
            }
            rows -= 1;
        } while (rows != -1);
    }
    if (state->portrait != NULL)
    {
        prim = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(DR_TPAGE);
        prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, DR_TPAGE);
        prim->sprite_words.rgbc = text_system->draw_mode1;
        if ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0)
        {
            xy = (state->x + 8) & 0xFFFF;
        }
        else
        {
            s32 frame_width;

            frame_width = state->width + 2 * FIELD_TEXT_BORDER_WIDTH;
            xy = (state->x + frame_width) & 0xFFFF;
        }
        {
            s32 window_y;
            s32 window_height;
            u32 flags;

            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            window_y = state->y;
            window_height = state->height;
            flags = state->flags.word;
            prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
            prim->sprite_words.rgbc = FIELD_TEXT_SPRITE_SHADOW;
            xy |= field_text_portrait_y_word(window_y, window_height);
            prim->sprite_words.xy = xy + 0x20002;
            uv = ((((FIELD_TEXT_PORTRAIT_INDEX(flags) * FIELD_TEXT_PORTRAIT_SIZE) + FIELD_TEXT_PORTRAIT_VRAM_Y) & 0xFF) << 8) | FIELD_TEXT_PORTRAIT_U;
            prim->sprite_words.uv = (text_system->text_clut << 16) | uv;
            prim->sprite_words.wh = 0x300030;
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
            prim->sprite_words.rgbc = sprite_color;
            prim->sprite_words.xy = xy;
            prim->sprite_words.uv = (text_system->portrait_clut[FIELD_TEXT_PORTRAIT_INDEX(state->flags.word)] << 16) | uv;
            prim->sprite_words.wh = 0x300030;
        }
    }
    if (state->flow_code != FIELD_TEXT_FLOW_NONE)
    {
        if (((state->flags.word & FIELD_TEXT_STYLE_MASK) == 0) ||
            (((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD) && (state->flow_code == FIELD_TEXT_FLOW_CHOICE)))
        {
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(DR_TPAGE);
            prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, DR_TPAGE);
            prim->sprite_words.rgbc = text_system->draw_mode0;
            if (state->flow_code == FIELD_TEXT_FLOW_CHOICE)
            {
                prim = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(SPRT_16);
                prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT_16);
                prim->sprite_words.rgbc = FIELD_TEXT_CHOICE_COLOR;
                if ((state->portrait != NULL) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0))
                {
                    prim->sprite16.x0 = state->x + FIELD_TEXT_PORTRAIT_MARGIN;
                }
                else
                {
                    prim->sprite16.x0 = state->x + 0xE;
                }
                prim->sprite16.y0 = state->y + ((state->choice_start_line + state->choice_index) * FIELD_TEXT_LINE_SPACING);
                rows = state->prompt_frame;
                if (rows == 3)
                {
                    rows = 1;
                }
                setUV0(&prim->sprite16, (rows << 4) + 0x60, FIELD_TEXT_FRAME_V);
                prim->sprite16.clut = text_system->prompt_clut;
            }
            else
            {
                prim = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(SPRT);
                prim->sprite_words.tag = FIELD_TEXT_TAG(packet_cursor, SPRT);
                prim->sprite_words.rgbc = sprite_color;
                if ((state->portrait != NULL) && (FIELD_TEXT_PORTRAIT_SIDE(state->flags.word) < FIELD_TEXT_SIDE_OUTSIDE))
                {
                    prim->sprite.x0 = state->x + ((state->width + FIELD_TEXT_PORTRAIT_MARGIN) >> 1);
                }
                else
                {
                    prim->sprite.x0 = state->x + (state->width >> 1);
                }
                prim->sprite.y0 = state->y + state->height + 6;
                prim->sprite.clut = text_system->prompt_clut;
                prim->sprite.u0 = 0xF0;
                prim->sprite.v0 = (state->prompt_frame * 8) - 0x20;
                prim->sprite_words.wh = 0x80010;
            }
        }
    }
    addPrims(&ot->tag1, first, prim);
    *cursor = packet_cursor;
}

/**
 * @brief Build a warped text-window packet chain for an opening/closing quad.
 * @param state Text-window state.
 * @param quad Transition quad.
 * @param cursor In/out render-packet cursor.
 * @param ot Ordering-table slot.
 * @note The frame, cached text spans, and portrait share a scratchpad mesh.
 *       Its vertices are mapped into the transition quad with integer bilinear interpolation.
 */
static void field_text_build_transition_packets(FieldTextState* state, FieldTextQuad* quad, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextSystem* text_system = FIELD_TEXT_SYSTEM;
    FieldTextVertex* vertex;
    FieldTextVertex* lower_row;
    FieldTextVertex* bottom_vertices;
    FieldTextPacket* poly;
    u8* first;
    s32 content_width;
    s32 pixels_remaining;
    s32 mesh_x;
    s32 mesh_y;
    s32 packet_height;
    s32 chunk;
    s32 edge;
    s32 glyph_u;
    s32 glyph_v;
    s32 frame_rows;
    u16 text_area_height;
    s32 text_vertex_count;
    s32 u;
    s32 avail;
    s32 mesh_width;
    u32 tpage;
    s32 mesh_height;
    s32 clut;
    s32 base_x;
    u8* packet_cursor;
    s32 base_y;
    s32 texture_uv;
    s32 dx;
    s32 dy;
    s32 previous_y;
    s32 vertex_y;
    s32 row_v;
    s32 sel;
    s32 texture_u_origin;
    u32 texture_command;
    s32 remaining;
    s32 texture_v_origin;

    base_x = 0;
    base_y = 0;
    dx = 0;
    dy = 0;
    if ((state->portrait != NULL) && (FIELD_TEXT_PORTRAIT_SIDE(state->flags.word) < FIELD_TEXT_SIDE_OUTSIDE))
    {
        content_width = state->width + FIELD_TEXT_PORTRAIT_MARGIN;
    }
    else
    {
        content_width = state->width;
    }

    /* Build the unwarped mesh in scratchpad RAM. */
    vertex = FIELD_TEXT_MESH;
    mesh_y = 0;
    vertex->pos.vx = 0;
    vertex->pos.vy = mesh_y;
    vertex += 1;
    mesh_x = 8;
    pixels_remaining = content_width;
    if (content_width > 0)
    {
        do
        {
            vertex->pos.vx = mesh_x;
            vertex->pos.vy = mesh_y;
            vertex += 1;
            if (pixels_remaining >= 0x41)
            {
                mesh_x += 0x40;
                pixels_remaining -= 0x40;
            }
            else
            {
                mesh_x += pixels_remaining;
                pixels_remaining = 0;
            }
        } while (pixels_remaining > 0);
    }
    vertex->pos.vx = mesh_x;
    vertex->pos.vy = mesh_y;
    vertex[1].pos.vx = mesh_x + 8;
    vertex[1].pos.vy = mesh_y;
    vertex += 2;

    remaining = state->height;
    remaining -= 1;
    mesh_y += 8;
    if (remaining != -1)
    {
        do
        {
            vertex->pos.vx = 0;
            vertex->pos.vy = mesh_y;
            vertex += 1;
            mesh_x = 8;
            pixels_remaining = content_width;
            if (content_width > 0)
            {
                do
                {
                    vertex->pos.vx = mesh_x;
                    vertex->pos.vy = mesh_y;
                    vertex += 1;
                    if (pixels_remaining >= 0x41)
                    {
                        mesh_x += 0x40;
                        pixels_remaining -= 0x40;
                    }
                    else
                    {
                        mesh_x += pixels_remaining;
                        pixels_remaining = 0;
                    }
                } while (pixels_remaining > 0);
            }
            vertex->pos.vx = mesh_x;
            vertex->pos.vy = mesh_y;
            vertex[1].pos.vx = mesh_x + 8;
            vertex[1].pos.vy = mesh_y;
            vertex += 2;
            if (remaining >= 0x21)
            {
                mesh_y += 0x20;
                remaining -= 0x20;
            }
            else
            {
                mesh_y += remaining;
                remaining = 0;
            }
            remaining -= 1;
        } while (remaining != -1);
    }

    remaining = 1;
    do
    {
        vertex->pos.vx = 0;
        vertex->pos.vy = mesh_y;
        vertex += 1;
        mesh_x = 8;
        pixels_remaining = content_width;
        if (content_width > 0)
        {
            do
            {
                vertex->pos.vx = mesh_x;
                vertex->pos.vy = mesh_y;
                vertex += 1;
                if (pixels_remaining >= 0x41)
                {
                    mesh_x += 0x40;
                    pixels_remaining -= 0x40;
                }
                else
                {
                    mesh_x += pixels_remaining;
                    pixels_remaining = 0;
                }
            } while (pixels_remaining > 0);
        }
        vertex->pos.vx = mesh_x;
        vertex->pos.vy = mesh_y;
        vertex[1].pos.vx = mesh_x + 8;
        vertex[1].pos.vy = mesh_y;
        vertex += 2;
        remaining -= 1;
        mesh_y += 8;
    } while (remaining != -1);

    mesh_y = 8;
    u = state->region_start_u;
    remaining = state->height >> 4;
    remaining -= 1;
    text_vertex_count = 0;
    if (remaining != -1)
    {
        do
        {
            mesh_x = 8;
            if ((state->portrait != NULL) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0))
            {
                mesh_x = 0x40;
            }
            pixels_remaining = state->line_advance;
            if (pixels_remaining > 0)
            {
                do
                {
                    vertex->pos.vx = mesh_x;
                    vertex->pos.vy = mesh_y;
                    vertex[1].pos.vx = mesh_x;
                    vertex[1].pos.vy = state->line_height + mesh_y;
                    vertex += 2;
                    text_vertex_count += 2;
                    avail = FIELD_TEXT_CACHE_WIDTH - u;
                    if (pixels_remaining >= avail)
                    {
                        mesh_x += avail;
                        pixels_remaining -= avail;
                        u = 0;
                    }
                    else
                    {
                        mesh_x += pixels_remaining;
                        u += pixels_remaining;
                        pixels_remaining = 0;
                    }
                } while (pixels_remaining > 0);
            }
            vertex->pos.vx = mesh_x;
            vertex->pos.vy = mesh_y;
            vertex[1].pos.vx = mesh_x;
            vertex[1].pos.vy = state->line_height + mesh_y;
            vertex += 2;
            text_vertex_count += 2;
            remaining -= 1;
            mesh_y += 0x10;
        } while (remaining != -1);
    }

    remaining = 1;
    mesh_y = ((state->height - 0x30) >> 1) + 0xA;
    do
    {
        if ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0)
        {
            mesh_x = 0xA;
        }
        else
        {
            mesh_x = state->width + 0x12;
        }
        for (pixels_remaining = 1; pixels_remaining != -1; pixels_remaining--)
        {
            vertex->pos.vx = mesh_x;
            vertex->pos.vy = mesh_y;
            vertex += 1;
            mesh_x += 0x30;
        }
        remaining -= 1;
        mesh_y += 0x30;
    } while (remaining != -1);

    remaining = 1;
    mesh_y = ((state->height - 0x30) >> 1) + 8;
    do
    {
        if ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0)
        {
            mesh_x = 8;
        }
        else
        {
            mesh_x = state->width + 0x10;
        }
        for (pixels_remaining = 1; pixels_remaining != -1; pixels_remaining--)
        {
            vertex->pos.vx = mesh_x;
            vertex->pos.vy = mesh_y;
            vertex += 1;
            mesh_x += 0x30;
        }
        remaining -= 1;
        mesh_y += 0x30;
    } while (remaining != -1);

    /* Interpolate the two side edges once per row, then interpolate across each row. */
    vertex = FIELD_TEXT_MESH;
    previous_y = -1;
    mesh_width = content_width + 0x10;
    remaining = ((FIELD_TEXT_MESH_ROWS(state->height) + 3) * (FIELD_TEXT_MESH_COLUMNS(content_width) + 3)) + text_vertex_count + 7;
    mesh_height = state->height + 0x10;
    if (remaining != -1)
    {
        do
        {
            vertex_y = vertex->pos.vy;
            if (previous_y != vertex_y)
            {
                previous_y = vertex_y;
                base_x = (((quad->x2 - quad->x0) * vertex_y) / mesh_height) + quad->x0;
                base_y = (((quad->y2 - quad->y0) * vertex_y) / mesh_height) + quad->y0;
                dx = ((((quad->x3 - quad->x1) * vertex_y) / mesh_height) + quad->x1) - base_x;
                dy = ((((quad->y3 - quad->y1) * vertex_y) / mesh_height) + quad->y1) - base_y;
            }
            vertex->pos.vy = ((dy * vertex->pos.vx) / mesh_width) + base_y;
            vertex->pos.vx = ((dx * vertex->pos.vx) / mesh_width) + base_x;
            vertex += 1;
        } while (--remaining != -1);
    }

    /* Draw the top and bottom borders, followed by the tiled window interior. */
    texture_command = FIELD_TEXT_QUAD_COLOR;
    texture_u_origin = 0;
    texture_v_origin = FIELD_TEXT_FRAME_V;
    tpage = getTPage(0, 0, FIELD_TEXT_CACHE_VRAM_X, FIELD_TEXT_TPAGE_VRAM_Y) << 16;
    vertex = FIELD_TEXT_MESH;
    packet_height = 8;
    remaining = 1;
    first = *cursor;
    packet_cursor = first;
    clut = text_system->window_clut << 16;
    do
    {
        texture_uv = texture_u_origin | 0xF000;
        if (remaining == 0)
        {
            texture_uv = texture_u_origin | 0xF800;
        }
        poly = (FieldTextPacket*)packet_cursor;
        /* The row below starts one mesh row (columns + 3 vertices) further on. */
        lower_row = vertex + FIELD_TEXT_MESH_COLUMNS(content_width) + 4;
        packet_cursor += sizeof(POLY_FT4);
        poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
        poly->quad_words.uv0 = clut | texture_uv;
        poly->quad_words.uv1 = tpage | (texture_uv + 8);
        poly->quad_words.uv2 = texture_uv + (packet_height << 8);
        poly->quad_words.rgbc = texture_command;
        poly->quad_words.uv3 = texture_uv + ((packet_height << 8) | 8);
        texture_uv += 8;
        poly->quad_words.xy0 = vertex[0].word;
        poly->quad_words.xy1 = vertex[1].word;
        poly->quad_words.xy2 = vertex[FIELD_TEXT_MESH_COLUMNS(content_width) + 3].word;
        poly->quad_words.xy3 = vertex[FIELD_TEXT_MESH_COLUMNS(content_width) + 4].word;
        bottom_vertices = lower_row;
        pixels_remaining = content_width;
        vertex += 1;
        if (content_width > 0)
        {
            do
            {
                poly = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(POLY_FT4);
                poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
                poly->quad_words.rgbc = texture_command;
                if (pixels_remaining >= 0x41)
                {
                    chunk = 0x3F;
                    pixels_remaining -= 0x40;
                }
                else
                {
                    chunk = pixels_remaining - 1;
                    pixels_remaining = 0;
                }
                poly->quad_words.uv1 = tpage | (texture_uv + chunk);
                poly->quad_words.uv0 = clut | texture_uv;
                poly->quad_words.uv2 = texture_uv + (packet_height << 8);
                poly->quad_words.uv3 = texture_uv + ((packet_height << 8) | chunk);
                poly->quad_words.xy0 = vertex[0].word;
                poly->quad_words.xy1 = vertex[1].word;
                vertex += 1;
                poly->quad_words.xy2 = bottom_vertices[0].word;
                poly->quad_words.xy3 = bottom_vertices[1].word;
                bottom_vertices += 1;
            } while (pixels_remaining > 0);
        }
        texture_uv += 0x40;
        poly = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(POLY_FT4);
        poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
        poly->quad_words.uv0 = clut | texture_uv;
        poly->quad_words.uv1 = tpage | (texture_uv + 7);
        poly->quad_words.rgbc = texture_command;
        poly->quad_words.uv2 = texture_uv + (packet_height << 8);
        poly->quad_words.uv3 = texture_uv + ((packet_height << 8) | 7);
        poly->quad_words.xy0 = vertex[0].word;
        poly->quad_words.xy1 = vertex[1].word;
        poly->quad_words.xy2 = bottom_vertices[0].word;
        poly->quad_words.xy3 = bottom_vertices[1].word;
        remaining -= 1;
        vertex = FIELD_TEXT_MESH + ((FIELD_TEXT_MESH_ROWS(state->height) + 1) * (FIELD_TEXT_MESH_COLUMNS(content_width) + 3));
        packet_height = 7;
    } while (remaining != -1);

    {
        FieldTextVertex* bottom_vertices;

        vertex = FIELD_TEXT_MESH + 3 + FIELD_TEXT_MESH_COLUMNS(content_width);
        bottom_vertices = vertex + FIELD_TEXT_MESH_COLUMNS(content_width) + 3;
        remaining = state->height;
        if (remaining > 0)
        {
            do
            {
                texture_uv = (texture_v_origin << 8) | (texture_u_origin + 0xE0);
                packet_height = 0x1F00;
                if (remaining < 0x20)
                {
                    packet_height = remaining << 8;
                }
                poly = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(POLY_FT4);
                poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
                poly->quad_words.uv0 = clut | texture_uv;
                poly->quad_words.uv1 = tpage | (texture_uv + 8);
                poly->quad_words.uv2 = texture_uv + packet_height;
                poly->quad_words.rgbc = texture_command;
                poly->quad_words.uv3 = texture_uv + (packet_height | 8);
                texture_uv = texture_uv - 0x40;
                poly->quad_words.xy0 = vertex[0].word;
                pixels_remaining = content_width;
                poly->quad_words.xy1 = vertex[1].word;
                poly->quad_words.xy2 = bottom_vertices[0].word;
                poly->quad_words.xy3 = bottom_vertices[1].word;
                vertex += 1;
                bottom_vertices += 1;
                if (content_width > 0)
                {
                    do
                    {
                        poly = (FieldTextPacket*)packet_cursor;
                        packet_cursor += sizeof(POLY_FT4);
                        poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
                        poly->quad_words.rgbc = texture_command;
                        if (pixels_remaining >= 0x41)
                        {
                            chunk = 0x40;
                            pixels_remaining -= 0x40;
                        }
                        else
                        {
                            chunk = pixels_remaining;
                            pixels_remaining = 0;
                        }
                        poly->quad_words.uv1 = tpage | (texture_uv + chunk);
                        poly->quad_words.uv0 = clut | texture_uv;
                        poly->quad_words.uv2 = texture_uv + packet_height;
                        poly->quad_words.uv3 = texture_uv + (packet_height | chunk);
                        poly->quad_words.xy0 = vertex[0].word;
                        poly->quad_words.xy1 = vertex[1].word;
                        poly->quad_words.xy2 = bottom_vertices[0].word;
                        poly->quad_words.xy3 = bottom_vertices[1].word;
                        vertex += 1;
                        bottom_vertices += 1;
                    } while (pixels_remaining > 0);
                }
                texture_uv += 0x48;
                poly = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(POLY_FT4);
                poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
                poly->quad_words.uv0 = clut | texture_uv;
                poly->quad_words.uv1 = tpage | (texture_uv + 8);
                poly->quad_words.uv2 = texture_uv + packet_height;
                poly->quad_words.rgbc = texture_command;
                poly->quad_words.uv3 = texture_uv + (packet_height | 8);
                poly->quad_words.xy0 = vertex[0].word;
                remaining -= 0x20;
                poly->quad_words.xy1 = vertex[1].word;
                poly->quad_words.xy2 = bottom_vertices[0].word;
                poly->quad_words.xy3 = bottom_vertices[1].word;
                vertex += 2;
                bottom_vertices += 2;
            } while (remaining > 0);
        }
    }

    /* Text vertices are pairs of upper/lower endpoints split at cache page boundaries. */
    texture_u_origin = 0;
    texture_v_origin = FIELD_TEXT_CACHE_V;
    u = state->region_start_u;
    row_v = state->region_start_v;
    text_area_height = state->height;
    remaining = text_area_height >> 4;
    remaining -= 1;
    vertex = FIELD_TEXT_MESH + ((FIELD_TEXT_MESH_ROWS(text_area_height) + 3) * (FIELD_TEXT_MESH_COLUMNS(content_width) + 3));
    if (remaining != -1)
    {
        do
        {
            pixels_remaining = state->line_advance;
            if (pixels_remaining > 0)
            {
                do
                {
                    glyph_v = texture_v_origin + row_v;
                    glyph_u = texture_u_origin + u;
                    glyph_v <<= 8;
                    texture_uv = glyph_v | glyph_u;
                    poly = (FieldTextPacket*)packet_cursor;
                    packet_cursor += sizeof(POLY_FT4);
                    poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
                    poly->quad_words.rgbc = texture_command;
                    poly->quad_words.uv0 = (text_system->text_clut << 16) | texture_uv;
                    poly->quad_words.uv1 = tpage | texture_uv;
                    poly->quad_words.uv2 = texture_uv + (state->line_height << 8);
                    poly->quad_words.uv3 = texture_uv + (state->line_height << 8);
                    poly->quad_words.xy0 = vertex[0].word;
                    avail = FIELD_TEXT_CACHE_WIDTH - u;
                    poly->quad_words.xy1 = vertex[2].word;
                    poly->quad_words.xy2 = vertex[1].word;
                    poly->quad_words.xy3 = vertex[3].word;
                    vertex += 2;
                    if (pixels_remaining >= avail)
                    {
                        pixels_remaining -= avail;
                        edge = (glyph_u + avail) - 1;
                        poly->quad.u3 = edge;
                        poly->quad.u1 = edge;
                        u = 0;
                        row_v += state->line_height;
                    }
                    else
                    {
                        u += pixels_remaining;
                        edge = glyph_u + pixels_remaining;
                        pixels_remaining = 0;
                        poly->quad.u3 = edge;
                        poly->quad.u1 = edge;
                    }
                } while (pixels_remaining > 0);
            }
            remaining -= 1;
            vertex += 2;
        } while (remaining != -1);
    }

    /* The final eight mesh vertices hold the portrait shadow and image quads. */
    if (state->portrait != NULL)
    {
        tpage = getTPage(0, 0, FIELD_TEXT_CACHE_VRAM_X, FIELD_TEXT_TPAGE_VRAM_Y) << 16;
        poly = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(POLY_FT4);
        frame_rows = FIELD_TEXT_MESH_ROWS(state->height) + 3;
        sel = FIELD_TEXT_PORTRAIT_INDEX(state->flags.word);
        clut = text_system->text_clut << 16;
        poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
        poly->quad_words.rgbc = FIELD_TEXT_QUAD_SHADOW;
        vertex = (frame_rows * (FIELD_TEXT_MESH_COLUMNS(content_width) + 3)) + FIELD_TEXT_MESH + text_vertex_count;
        texture_uv = ((((sel * FIELD_TEXT_PORTRAIT_SIZE) + FIELD_TEXT_PORTRAIT_VRAM_Y) & 0xFF) << 8) | FIELD_TEXT_PORTRAIT_U;
        poly->quad_words.uv0 = clut | texture_uv;
        poly->quad_words.uv2 = texture_uv + 0x3000;
        poly->quad_words.uv1 = (texture_uv + 0x2F) | tpage;
        poly->quad_words.uv3 = texture_uv + 0x302F;
        poly->quad_words.xy0 = vertex[0].word;
        poly->quad_words.xy1 = vertex[1].word;
        poly->quad_words.xy2 = vertex[2].word;
        poly->quad_words.xy3 = vertex[3].word;
        vertex += 4;
        poly = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(POLY_FT4);
        clut = text_system->portrait_clut[FIELD_TEXT_PORTRAIT_INDEX(state->flags.word)] << 16;
        texture_uv = ((((FIELD_TEXT_PORTRAIT_INDEX(state->flags.word) * FIELD_TEXT_PORTRAIT_SIZE) + FIELD_TEXT_PORTRAIT_VRAM_Y) & 0xFF) << 8) | FIELD_TEXT_PORTRAIT_U;
        poly->quad_words.tag = FIELD_TEXT_TAG(packet_cursor, POLY_FT4);
        poly->quad_words.rgbc = texture_command;
        poly->quad_words.uv2 = texture_uv + 0x3000;
        poly->quad_words.uv1 = (texture_uv + 0x2F) | tpage;
        poly->quad_words.uv3 = texture_uv + 0x302F;
        poly->quad_words.uv0 = clut | texture_uv;
        poly->quad_words.xy0 = vertex[0].word;
        poly->quad_words.xy1 = vertex[1].word;
        poly->quad_words.xy2 = vertex[2].word;
        poly->quad_words.xy3 = vertex[3].word;
    }
    addPrims(&ot->tag1, first, poly);
    *cursor = packet_cursor;
}

/**
 * @brief Scroll the text cache up one row and clear the vacated row.
 * @param state Text-window state.
 */
static void field_text_scroll_cache(FieldTextState* state)
{
    u16* destination_row;
    u16* source_row;
    u16* destination;
    u16* source;
    s32 next_u;
    s32 next_v;
    s32 destination_u;
    s32 destination_v;
    s32 source_u;
    s32 source_v;
    s32 pixels_remaining;
    s32 scroll_rows;
    s32 pixel_span;
    s32 copy_count;
    s32 glyph_rows;

    next_u = state->region_start_u;
    next_v = state->region_start_v;
    for (scroll_rows = state->height - FIELD_TEXT_LINE_SPACING; scroll_rows > 0; scroll_rows -= FIELD_TEXT_LINE_SPACING)
    {
        /* Find the start of the next text line. */
        destination_u = next_u;
        pixels_remaining = state->line_advance;
        destination_v = next_v;
        while (pixels_remaining > 0)
        {
            pixel_span = FIELD_TEXT_CACHE_WIDTH - next_u;
            next_u += pixels_remaining;
            if (pixels_remaining >= pixel_span)
            {
                pixels_remaining -= pixel_span;
                next_u = 0;
                next_v += state->line_height;
            }
            else
            {
                pixels_remaining = 0;
            }
        }

        /* Copy that line over the previous one. */
        source_u = next_u;
        pixels_remaining = state->line_advance;
        source_v = next_v;
        while (pixels_remaining > 0)
        {
            destination_row = (FIELD_TEXT_CACHE + (destination_u >> 2)) + (destination_v * FIELD_TEXT_CACHE_ROW_WORDS);
            source_row = (FIELD_TEXT_CACHE + (source_u >> 2)) + (source_v * FIELD_TEXT_CACHE_ROW_WORDS);
            pixel_span = FIELD_TEXT_CACHE_WIDTH - source_u;
            copy_count = FIELD_TEXT_CACHE_WIDTH - destination_u;
            if (copy_count < pixel_span)
            {
                pixel_span = copy_count;
            }
            if (pixels_remaining < pixel_span)
            {
                pixel_span = pixels_remaining;
            }
            destination_u += pixel_span;
            pixels_remaining -= pixel_span;
            while (destination_u >= FIELD_TEXT_CACHE_WIDTH)
            {
                destination_u -= FIELD_TEXT_CACHE_WIDTH;
                destination_v += state->line_height;
            }
            source_u += pixel_span;
            while (source_u >= FIELD_TEXT_CACHE_WIDTH)
            {
                source_u -= FIELD_TEXT_CACHE_WIDTH;
                source_v += state->line_height;
            }
            glyph_rows = state->line_height;
            while (--glyph_rows != -1)
            {
                destination = destination_row;
                copy_count = pixel_span >> 2;
                source = source_row;
                while (--copy_count != -1)
                {
                    *destination++ = *source++;
                }
                destination_row += FIELD_TEXT_CACHE_ROW_WORDS;
                source_row += FIELD_TEXT_CACHE_ROW_WORDS;
            }
        }
    }

    /* Clear the last line. */
    pixels_remaining = state->line_advance;
    while (pixels_remaining > 0)
    {
        destination_row = (FIELD_TEXT_CACHE + (next_u >> 2)) + (next_v * FIELD_TEXT_CACHE_ROW_WORDS);
        pixel_span = FIELD_TEXT_CACHE_WIDTH - next_u;
        if (pixels_remaining < pixel_span)
        {
            pixel_span = pixels_remaining;
        }
        next_u += pixel_span;
        pixels_remaining -= pixel_span;
        while (next_u >= FIELD_TEXT_CACHE_WIDTH)
        {
            next_u -= FIELD_TEXT_CACHE_WIDTH;
            next_v += state->line_height;
        }
        glyph_rows = state->line_height;
        while (--glyph_rows != -1)
        {
            copy_count = pixel_span >> 2;
            destination = destination_row;
            while (--copy_count != -1)
            {
                *destination++ = 0;
            }
            destination_row += FIELD_TEXT_CACHE_ROW_WORDS;
        }
    }
    state->dirty_start_u = state->region_start_u;
    state->dirty_start_v = state->region_start_v;
    state->dirty_end_u = state->region_end_u;
    state->dirty_end_v = state->region_end_v;
    state->remaining_width = state->width;
}

/**
 * @brief Queue dirty text-cache rows for VRAM upload.
 * @param state Text-window state.
 * @param cursor In/out packet cursor used for upload requests and staging data.
 */
static void field_text_queue_uploads(FieldTextState* state, u8** cursor)
{
    FieldImageReq* req;
    u16* cur;
    u16* dst;
    u16* src;
    u16* s;
    u8* row;
    s32 rows;
    s32 count;
    s32 span;
    s32 w;
    s32 x;
    s32 y;
    s32 h;
    s32 column;
    s32 column_bytes;

    cur = (u16*)*cursor;
    req = (FieldImageReq*)cur;
    y = state->dirty_start_v;
    x = state->dirty_start_u;
    cur += sizeof(FieldImageReq) / sizeof(*cur);
    if (y == state->dirty_end_v)
    {
        span = (state->dirty_end_u - x) >> 1;
    }
    else
    {
        span = (FIELD_TEXT_CACHE_WIDTH - x) >> 1;
    }
    column = x >> 2;
    column_bytes = column << 1;
    row = (u8*)FIELD_TEXT_CACHE + y * FIELD_TEXT_CACHE_ROW_BYTES;
    src = FIELD_TEXT_CACHE_WORD(row, column_bytes);
    h = state->line_height;
    req->rect.x = column + FIELD_TEXT_CACHE_VRAM_X;
    w = span >> 1;
    req->rect.y = y + FIELD_TEXT_CACHE_VRAM_Y;
    req->rect.w = w;
    req->rect.h = h;
    if (span == FIELD_TEXT_CACHE_WIDTH / 2)
    {
        req->data = (u_long*)src;
    }
    else
    {
        dst = cur;
        cur += ((w * h) + 1) & ~1;
        req->data = (u_long*)dst;
        rows = h;
        while (--rows != -1)
        {
            count = span >> 1;
            s = src;
            while (--count != -1)
            {
                *dst++ = *s++;
            }
            src += FIELD_TEXT_CACHE_ROW_WORDS;
        }
    }
    field_queue_vram_upload(req);
    if (y != state->dirty_end_v)
    {
        y += h;
        req = (FieldImageReq*)cur;
        if (y != state->dirty_end_v)
        {
            cur += sizeof(FieldImageReq) / sizeof(*cur);
            src = FIELD_TEXT_CACHE + (y * FIELD_TEXT_CACHE_ROW_WORDS);
            req->rect.x = FIELD_TEXT_CACHE_VRAM_X;
            req->rect.y = y + FIELD_TEXT_CACHE_VRAM_Y;
            req->rect.w = FIELD_TEXT_CACHE_ROW_WORDS;
            req->rect.h = state->dirty_end_v - y;
            req->data = (u_long*)src;
            field_queue_vram_upload(req);
        }
        req = (FieldImageReq*)cur;
        if (state->dirty_end_u != 0)
        {
            cur += sizeof(FieldImageReq) / sizeof(*cur);
            req->rect.x = FIELD_TEXT_CACHE_VRAM_X;
            req->rect.y = state->dirty_end_v + FIELD_TEXT_CACHE_VRAM_Y;
            req->rect.w = state->dirty_end_u >> 2;
            req->rect.h = h;
            src = FIELD_TEXT_CACHE + (state->dirty_end_v * FIELD_TEXT_CACHE_ROW_WORDS);
            dst = cur;
            cur += (((state->dirty_end_u >> 2) * h) + 1) & ~1;
            rows = h;
            req->data = (u_long*)dst;
            while (--rows != -1)
            {
                count = state->dirty_end_u >> 2;
                s = src;
                while (--count != -1)
                {
                    *dst++ = *s++;
                }
                src += FIELD_TEXT_CACHE_ROW_WORDS;
            }
            field_queue_vram_upload(req);
        }
    }
    *cursor = (u8*)cur;
}

/**
 * @brief Attach a text string to a window, or defer it while the slot reopens.
 * @param window_index Window slot; only the low 16 bits are used.
 * @param text Text pointer.
 * @param text_options Text options; bit 0 enables automatic close.
 */
void field_text_set_string(s32 window_index, u8* text, s32 text_options)
{
    u16 slot = window_index;
    u8 options = text_options;
    FieldTextSystem* text_system = FIELD_TEXT_SYSTEM;
    FieldTextState* state = &text_system->windows[slot];
    FieldTextConfig* config;

    if ((state->flags.word & FIELD_TEXT_REOPEN_MASK) != 0)
    {
        config = &text_system->configs[slot];
        config->text = text;
        config->flags.b.byte2 = options;
        return;
    }
    state->last_was_break = 1;
    state->text_cursor = text;
    state->macro_cursor = NULL;
    state->glyph_cursor = NULL;
    state->pending_spaces = 0;
    state->flow_code = FIELD_TEXT_FLOW_NONE;
    state->flags.word = (state->flags.word & ~FIELD_TEXT_AUTO_CLOSE) | ((options & 1) << 12);
}

/**
 * @brief Save the pending text configuration for a window slot.
 * @param slot Window slot index.
 */
static void field_text_save_config(u16 slot)
{
    FieldTextConfig* config;

    config = &g_field_text_saved_configs[slot];
    config->text = NULL;
    field_text_copy_config(config, FIELD_TEXT_PENDING_CONFIG);
}

/**
 * @brief Close a text window and release its portrait slot.
 * @param state Text-window state.
 * @param animate Non-zero starts the closing animation when supported.
 */
static void field_text_close(FieldTextState* state, s32 animate)
{
    if (state->portrait != NULL)
    {
        if ((state->flags.word & FIELD_TEXT_PORTRAIT_SLOT) == 0)
        {
            FIELD_TEXT_SYSTEM->portrait_slots &= 0xFFFE;
        }
        else
        {
            FIELD_TEXT_SYSTEM->portrait_slots &= 0xFFFD;
        }
    }
    if ((animate == 0) || ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD))
    {
        state->flags.word = state->flags.word & ~FIELD_TEXT_STATE_MASK;
    }
    else
    {
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_CLOSING;
        state->transition_frame = 0;
    }
}

/**
 * @brief Render a text window according to its opening, active, or closing state.
 * @param state Text-window state.
 * @param cursor In/out render-packet cursor.
 * @param ot Ordering-table slot.
 */
static void field_text_render_window(FieldTextState* state, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextQuad quad;

    switch (state->flags.b.low & FIELD_TEXT_STATE_MASK)
    {
    case FIELD_TEXT_OPENING:
        field_text_build_transition_quad(state, &quad, state->transition_frame);
        field_text_build_transition_packets(state, &quad, cursor, ot);
        state->transition_frame = state->transition_frame + 1;
        if (state->transition_frame == FIELD_TEXT_TRANSITION_FRAMES)
        {
            state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_ACTIVE;
        }
        break;
    case FIELD_TEXT_ACTIVE:
        field_text_build_window_packets(state, cursor, ot);
        break;
    case FIELD_TEXT_CLOSING:
        state->transition_frame = state->transition_frame + 1;
        field_text_build_transition_quad(state, &quad, FIELD_TEXT_TRANSITION_FRAMES - state->transition_frame);
        field_text_build_transition_packets(state, &quad, cursor, ot);
        if (state->transition_frame == FIELD_TEXT_TRANSITION_FRAMES)
        {
            state->flags.word = state->flags.word & ~FIELD_TEXT_STATE_MASK;
        }
        break;
    default:
        break;
    }
}

/**
 * @brief Queue palette and image uploads for a field dialogue portrait.
 * @param image Portrait data.
 * @param cursor In/out packet cursor.
 * @param slot Portrait VRAM slot.
 * @param mirror Non-zero mirrors the portrait horizontally before upload.
 */
static void field_text_queue_portrait_upload(FieldTextPortrait* image, u8** cursor, s32 slot, s32 mirror)
{
    FieldImageReq* req;
    u8* packet_cursor;
    u8* src;
    u8* dst;
    u8* row_end;
    s32 rows;
    s32 columns_remaining;
    u32 pixels;

    packet_cursor = *cursor;
    req = (FieldImageReq*)packet_cursor;
    packet_cursor += 2 * sizeof(FieldImageReq);
    req->rect.x = FIELD_TEXT_CLUT_X;
    req->rect.y = slot + FIELD_TEXT_PORTRAIT_CLUT_Y;
    req->rect.w = 16;
    req->rect.h = 1;
    req->data = (u_long*)image->palette;
    field_queue_vram_upload(req);
    req += 1;
    req->rect.x = FIELD_TEXT_PORTRAIT_VRAM_X;
    req->rect.y = (slot * FIELD_TEXT_PORTRAIT_SIZE) + FIELD_TEXT_PORTRAIT_VRAM_Y;
    req->rect.w = FIELD_TEXT_PORTRAIT_SIZE / 4;
    req->rect.h = FIELD_TEXT_PORTRAIT_SIZE;
    /* Reverse the byte order and the two pixels within each byte. */
    if (mirror != 0)
    {
        src = image->pixels[0];
        dst = packet_cursor;
        packet_cursor += sizeof(image->pixels);
        req->data = (u_long*)dst;
        dst += FIELD_TEXT_PORTRAIT_SIZE / 2 - 1;
        rows = FIELD_TEXT_PORTRAIT_SIZE - 1;
        do
        {
            row_end = dst;
            columns_remaining = FIELD_TEXT_PORTRAIT_SIZE / 2 - 1;
            do
            {
                pixels = *src;
                src += 1;
                columns_remaining -= 1;
                *row_end = ((pixels & 0xF) << 4) | (pixels >> 4);
                row_end -= 1;
            } while (columns_remaining != -1);
            rows -= 1;
            dst += FIELD_TEXT_PORTRAIT_SIZE / 2;
        } while (rows != -1);
    }
    else
    {
        req->data = (u_long*)image->pixels;
    }
    field_queue_vram_upload(req);
    *cursor = packet_cursor;
}

/**
 * @brief Start timed text-window mode and precompute its cache extent.
 * @param text Text to display.
 */
void field_text_start_timed_window(u8* text)
{
    FieldTextState* state = FIELD_TEXT_WINDOWS;
    s32 u;
    s32 v;
    s32 rows;
    s32 span;
    u16 avail;

    field_text_apply_config(state);
    u = 0;
    rows = state->height;
    v = u;
    state->text_cursor = text;
    FIELD_TEXT_SYSTEM->timed_text = text;
    state->dirty_start_u = 0;
    state->cursor_u = 0;
    state->region_start_u = 0;
    state->dirty_start_v = 0;
    state->cursor_v = 0;
    state->region_start_v = 0;
    state->flags.word = ((state->flags.word & ~FIELD_TEXT_STATE_MASK) | FIELD_TEXT_TIMED | FIELD_TEXT_INSTANT) & ~FIELD_TEXT_AUTO_CLOSE;
    if (rows > 0)
    {
        do
        {
            span = state->line_advance;
            if (span > 0)
            {
                do
                {
                    avail = FIELD_TEXT_CACHE_WIDTH - u;
                    if (span >= avail)
                    {
                        u += span;
                        span -= avail;
                        u = 0;
                        v += state->line_height;
                    }
                    else
                    {
                        u += span;
                        span = 0;
                    }
                } while (span > 0);
            }
            rows -= FIELD_TEXT_LINE_SPACING;
        } while (rows > 0);
    }
    state->dirty_end_u = u;
    state->region_end_u = u;
    state->dirty_end_v = v;
    state->region_end_v = v;
}

/**
 * @brief Restore a saved window configuration and reopen the slot.
 * @param slot Window slot index.
 * @param placement_mode 1 uses packed placement; other values use fixed placement.
 */
static void field_text_restore_window(u16 slot, s32 placement_mode)
{
    field_text_reopen_window(slot, placement_mode);
}

/**
 * @brief Set the screen position of a text window.
 * @param slot Window slot; only the low 16 bits are used.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 */
void field_text_set_position(s32 slot, s16 x, s16 y)
{
    FieldTextState* state = &FIELD_TEXT_SYSTEM->windows[slot & 0xFFFF];
    state->x = x;
    state->y = y;
}

/**
 * @brief Start closing a text window.
 * @param slot Window slot; only the low 16 bits are used.
 */
void field_text_close_window(s32 slot)
{
    FieldTextState* state = &FIELD_TEXT_SYSTEM->windows[slot & 0xFFFF];
    field_text_close(state, 1);
}

/**
 * @brief Read the progress of an active dialogue window.
 * @param slot Window slot; only the low 16 bits are used.
 * @return A FIELD_TEXT_STATUS_* value: CLOSED outside dialogue mode, PROMPT, BUSY while text remains, or DONE.
 */
s32 field_text_get_status(s32 slot)
{
    FieldTextState* state = &FIELD_TEXT_SYSTEM->windows[slot & 0xFFFF];

    if (((state->flags.word & FIELD_TEXT_STATE_MASK) != FIELD_TEXT_CLOSED) && ((state->flags.b.low & FIELD_TEXT_STATE_MASK) < FIELD_TEXT_TIMED))
    {
        if (state->flow_code != FIELD_TEXT_FLOW_NONE)
        {
            return FIELD_TEXT_STATUS_PROMPT;
        }
        return state->text_cursor != NULL;
    }
    return FIELD_TEXT_STATUS_CLOSED;
}

/**
 * @brief Read the last selected choice in a text window.
 * @param slot Window slot; only the low 16 bits are used.
 * @return Zero-based choice index.
 */
s32 field_text_get_choice(s32 slot)
{
    return FIELD_TEXT_SYSTEM->windows[slot & 0xFFFF].choice_index;
}

/**
 * @brief Format an unsigned value into a window's inline text buffer.
 * @param window_index Window slot; only the low 16 bits are used.
 * @param value Value to format as decimal text.
 * @param digits Number of columns; leading zeroes become spaces. Must be positive.
 */
void field_text_format_number(s32 window_index, u32 value, u8 digits)
{
    u8* text;
    u32 place_value;
    u32 digit;
    s32 leading_zero;
    u32 space;

    leading_zero = 1;
    text = FIELD_TEXT_SYSTEM->windows[window_index & 0xFFFF].inline_text;
    place_value = 1;
    while (--digits != 0)
    {
        place_value = place_value * 10;
    }
    space = ' ';
    if (place_value != 1)
    {
        do
        {
            digit = value / place_value;
            if ((digit == 0) && (leading_zero != 0))
            {
                *text = space;
                text += 1;
            }
            else
            {
                if (digit >= 10)
                {
                    digit = 9;
                }
                *text = digit + '0';
                text += 1;
                leading_zero = 0;
            }
            value = value % place_value;
            place_value = place_value / 10;
        } while (place_value != 1);
    }
    *text = value + '0';
    text[1] = 0;
}
