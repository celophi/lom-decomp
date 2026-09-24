#include "field_text.h"
#include "cdrom.h"
#include "cd_resources.h"
#include "common.h"
#include "gpu_packet.h"
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
#define FIELD_TEXT_CACHE_WIDTH 256
#define FIELD_TEXT_PIXELS_PER_WORD 4
#define FIELD_TEXT_CACHE_ROW_WORDS (FIELD_TEXT_CACHE_WIDTH / FIELD_TEXT_PIXELS_PER_WORD)
#define FIELD_TEXT_LINE_HEIGHT 12
#define FIELD_TEXT_LINE_SPACING 16
#define FIELD_TEXT_PORTRAIT_SIZE 48
#define FIELD_TEXT_PORTRAIT_MARGIN 56
#define FIELD_TEXT_TRANSITION_FRAMES 4

#define FIELD_TEXT_FRAME_TILE_WIDTH 64
#define FIELD_TEXT_FRAME_TILE_HEIGHT 32
#define FIELD_TEXT_BORDER_WIDTH 8
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

typedef struct
{
    u16 x;
    u16 y;
} FieldTextAnchor;

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

void field_text_typeset(FieldTextState* state, s32 budget);
void field_text_blit_glyph(FieldTextState* state, s32 code, u16 width);
void field_text_clear_cache(FieldTextState* state);
s32 field_text_advance_line(FieldTextState* state);
void field_text_clear_window(FieldTextState* state);
void field_text_apply_config(FieldTextState* state);
void field_text_build_transition_quad(FieldTextState* state, FieldTextQuad* out, s32 frame);
void field_text_build_window_packets(FieldTextState* state, u8** cursor, FieldOrderingTags* ot);
void field_text_build_transition_packets(FieldTextState* state, FieldTextQuad* quad, u8** cursor, FieldOrderingTags* ot);
void field_text_scroll_cache(FieldTextState* state);
void field_text_queue_uploads(FieldTextState* state, u16** cursor);
void field_text_save_config(u16 slot);
void field_text_close(FieldTextState* state, s32 animate);
void field_text_render_window(FieldTextState* state, u8** cursor, FieldOrderingTags* ot);
void field_text_queue_portrait_upload(FieldTextPortrait* image, u8** cursor, s32 slot, s32 mirror);
void field_text_restore_window(u16 slot, s32 placement_mode);

extern u8* g_field_timed_text;
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

    rect.x = 960;
    rect.y = 384;
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
 * @see decomp.me (100%)
 */
void field_text_typeset(FieldTextState* state, s32 budget)
{
    u8* glyph_run;
    u8* cursor;
    u8* look_cursor;
    u8* look_text;
    u8* look_macro;
    u8* look_glyph_run;
    s32 remaining;
    signed char advance;
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
                code = 0x20;
                width = 5;
                advance = 0;
                state->pending_spaces = state->pending_spaces - 1;
            }
            else
            {
                opcode = *cursor;
                cursor++;
                if ((opcode < 0x20) && (opcode != FIELD_TEXT_CMD_WIDE_CHARACTER))
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
                        state->char_delay = 4;
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
                            code = 0xFFFF;
                            width = 0xC;
                            advance = 1;
                        }
                        break;
                    case FIELD_TEXT_CMD_PREFIXED_GLYPH_RUN:
                        opcode = *cursor;
                        cursor++;
                        if (opcode == 0)
                        {
                            code = 0x20;
                            width = 5;
                            advance = 2;
                        }
                        /* fallthrough */
                    case FIELD_TEXT_CMD_EXTENDED_GLYPH_RUN:
                        opcode = *cursor + 0x1F;
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
                    if (opcode >= 0x20)
                    {
                        code = opcode;
                        advance = 1;
                    }
                    else
                    {
                        code = *cursor | ((opcode + 0xFFE8) << 8);
                        cursor++;
                        advance = 2;
                    }

                    character = code;
                    if (character == 0x80)
                    {
                        width = 0xC;
                    }
                    else if (character >= 0x80)
                    {
                        width = 9;
                    }
                    else
                    {
                        width = FIELD_TEXT_GLYPH_WIDTHS[character];
                    }
                }
            }

            character = code;

            if ((character == 0x20) || (character == 0x80))
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
        if ((code == 0x20) || (code == 0x80) || (code == 0xFFFF))
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
                        if ((opcode < 0x20) && (opcode != FIELD_TEXT_CMD_WIDE_CHARACTER))
                        {
                            switch (opcode)
                            {
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
                                goto stop_lookahead;
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
                                }
                                else if (look_macro != NULL)
                                {
                                    look_macro = NULL;
                                    look_cursor = look_text;
                                }
                                else
                                {
                                    word_continues = 0;
                                }
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
                                opcode = *look_cursor + 0x1F;
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
                            if (opcode >= 0x20)
                            {
                                look_code = opcode;
                                look_advance = 1;
                            }
                            else
                            {
                                look_code = *look_cursor | ((opcode + 0xFFE8) << 8);
                                look_cursor++;
                                look_advance = 2;
                            }
                            if (look_code != 0)
                            {
                                if ((look_code == 0x20) || (look_code == 0x80) || (look_code == 0xFFFF))
                                {
                                stop_lookahead:
                                    word_continues = 0;
                                }
                                else if (look_code >= 0x80)
                                {
                                    look_width += 9;
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
        if (code == 0xFFFF)
        {
            code = 0x20;
            width = 0xC;
            if ((state->portrait == 0) || (state->flags.word & FIELD_TEXT_PORTRAIT_MASK))
            {
                field_text_blit_glyph(state, 0x20, 0xC);
            }
            glyph_width = width;
        }
        if (glyph_width != 0)
        {
            field_text_blit_glyph(state, code, glyph_width);
        }
        if ((remaining != 0) && !(state->flags.word & FIELD_TEXT_INSTANT) && ((new_line == 0) || (code != 0x20)))
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

set_wide:
    state->flow_code = FIELD_TEXT_FLOW_CHOICE;
    state->prompt_frame = 0;
    state->prompt_timer = 4;
    state->choice_index = 0;
    goto store_and_return;

set_break:
    state->prompt_frame = 0;
    state->prompt_timer = 8;

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
 * @see decomp.me (100%) scratch not yet published
 */
void field_text_blit_glyph(FieldTextState* state, s32 code, u16 width)
{
    u16* scratch;
    u16* carry;
    u16* glyph;
    u8* line;
    u16* dst;
    u16* row_src;
    u16* row_dst;
    u8* px;
    s32 rows;
    s32 shift;
    s32 i;
    s32 j;
    s32 r;
    s32 col;
    s32 words;
    s32 lo_fill;
    s32 hi_fill;
    s32 lo_shadow;
    s32 hi_shadow;
    u32 nibbles;
    u32 left;
    u32 mask;
    u32 fill;
    u32 shade;
    u32 acc;
    u32 cur;
    u32 next;
    u32 avail;
    u32 span;
    s32 nib;

    scratch = FIELD_TEXT_SCRATCH;
    carry = state->row_carry;
    i = state->line_height;
    rows = i;
    shift = state->width - state->remaining_width;
    for (i = rows - 1; i != -1; i--)
    {
        j = 4;
        if (shift != 0)
        {
            *scratch++ = *carry++;
        }
        else
        {
            j = 5;
        }
        for (j = j - 1; j != -1; j--)
        {
            *scratch++ = 0;
        }
    }

    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        lo_fill = 6;
        hi_fill = 0x60;
        lo_shadow = 7;
        hi_shadow = 0x70;
        nibbles = width + 2;
    }
    else
    {
        switch (state->text_color)
        {
        case 0:
            lo_fill = 2;
            hi_fill = 0x20;
            lo_shadow = 3;
            hi_shadow = 0x30;
            break;
        case 1:
            lo_fill = 4;
            hi_fill = 0x40;
            lo_shadow = 5;
            hi_shadow = 0x50;
            break;
        case 2:
            lo_fill = 6;
            hi_fill = 0x60;
            lo_shadow = 7;
            hi_shadow = 0x70;
            break;
        case 3:
            lo_fill = 8;
            hi_fill = 0x80;
            lo_shadow = 9;
            hi_shadow = 0x90;
            break;
        case 4:
            lo_fill = 0xA;
            hi_fill = 0xA0;
            lo_shadow = 0xB;
            hi_shadow = 0xB0;
            break;
        case 5:
            lo_fill = 0xC;
            hi_fill = 0xC0;
            lo_shadow = 0xD;
            hi_shadow = 0xD0;
            break;
        default:
            lo_fill = 0xE;
            hi_fill = 0xE0;
            lo_shadow = 0xF;
            hi_shadow = 0xF0;
            break;
        }
        nibbles = width + 1;
    }

    glyph = FIELD_TEXT_FONT + ((u16)code - 0x20) * FIELD_TEXT_LINE_HEIGHT;
    px = (u8*)FIELD_TEXT_SCRATCH + (((u32)shift & 3) >> 1);
    fill = 0;
    shade = fill;
    acc = fill;
    for (i = rows - 1; i != -1; i--)
    {
        u8* p = px;

        nib = shift & 1;
        mask = 0x8000;
        if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
        {
            if (i != 0)
            {
                next = *glyph;
                cur = (next & 0xFFFF) >> 1;
                next |= (next & 0xFFFF) >> 2;
                acc |= next;
                next |= cur;
                shade |= next;
            }
            else
            {
                next = cur = 0;
            }
            for (j = nibbles - 1; j != -1; j--)
            {
                if (nib == 0)
                {
                    if ((shade & mask) != 0)
                    {
                        *p = lo_shadow | (*p & 0xF0);
                    }
                    nib = 1;
                    if ((fill & mask) != 0)
                    {
                        *p = lo_fill | (*p & 0xF0);
                    }
                }
                else
                {
                    if ((shade & mask) != 0)
                    {
                        *p = hi_shadow | (*p & 0xF);
                    }
                    nib = 0;
                    if ((fill & mask) != 0)
                    {
                        *p = hi_fill | (*p & 0xF);
                    }
                    p++;
                }
                mask >>= 1;
            }
            shade = acc;
            fill = cur;
        }
        else
        {
            cur = *glyph;
            next = cur >> 1;
            acc |= next;
            next |= cur;
            for (j = nibbles - 1; j != -1; j--)
            {
                if (nib == 0)
                {
                    if ((acc & mask) != 0)
                    {
                        *p = lo_shadow | (*p & 0xF0);
                    }
                    nib = 1;
                    if ((cur & mask) != 0)
                    {
                        *p = lo_fill | (*p & 0xF0);
                    }
                }
                else
                {
                    if ((acc & mask) != 0)
                    {
                        *p = hi_shadow | (*p & 0xF);
                    }
                    nib = 0;
                    if ((cur & mask) != 0)
                    {
                        *p = hi_fill | (*p & 0xF);
                    }
                    p++;
                }
                mask >>= 1;
            }
        }
        glyph++;
        acc = next;
        px += 10;
    }

    i = state->cursor_v;
    j = state->cursor_u + shift;
    while (j >= FIELD_TEXT_CACHE_WIDTH)
    {
        j -= FIELD_TEXT_CACHE_WIDTH;
        i += rows;
    }

    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        avail = state->remaining_width + 4;
    }
    else
    {
        avail = state->remaining_width;
    }
    if (avail < nibbles)
    {
        span = avail + (shift & 3);
    }
    else
    {
        span = nibbles + (shift & 3);
    }
    scratch = FIELD_TEXT_SCRATCH;
    left = (span + 3) >> 2;
    if (left != 0)
    {
        do
        {
            line = (u8*)FIELD_TEXT_CACHE + (i << 7);
            col = j >> 2;
            dst = FIELD_TEXT_CACHE_WORD(line, col * 2);
            if ((u32)(col + left) >= 0x41U)
            {
                words = 0x40 - col;
                left -= words;
                j = 0;
                i += rows;
            }
            else
            {
                j += left * 4;
                words = left;
                left = 0;
            }
            row_src = scratch;
            for (r = rows - 1; r != -1; r--)
            {
                u16* s = row_src;

                row_dst = dst;
                for (col = words - 1; col != -1; col--)
                {
                    *row_dst++ = *s++;
                }
                row_src += 5;
                dst += 0x40;
            }
            scratch += words;
        } while (left != 0);
    }

    state->dirty_end_u = j;
    j = (shift & 3) + width;
    scratch = FIELD_TEXT_SCRATCH + (j >> 2);
    carry = state->row_carry;
    state->dirty_end_v = i;
    for (i = rows - 1; i != -1; i--)
    {
        *carry++ = *scratch;
        scratch += 5;
    }
    state->remaining_width = state->remaining_width - width;
}

/**
 * @brief Clear the window cache region and mark it for upload.
 * @param state Window whose cached text is erased.
 */
void field_text_clear_cache(FieldTextState* state)
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
    cache_row = (FIELD_TEXT_CACHE + (cache_u >> 2)) + (cache_v << 6);
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
            cache_row = FIELD_TEXT_CACHE + (cache_v << 6);
            rows_remaining = (state->region_end_v - cache_v) << 6;
            while (--rows_remaining != -1)
            {
                *cache_row++ = 0;
            }
        }
        if (state->region_end_u != 0)
        {
            cache_row = FIELD_TEXT_CACHE + (state->region_end_v << 6);
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
s32 field_text_advance_line(FieldTextState* state)
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
        state->scroll_timer = 0x10;
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
 * @see decomp.me (100%)
 */
void field_text_clear_window(FieldTextState* state)
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
 * @see decomp.me (100%)
 */

void field_text_init(void)
{
    RECT rect;
    FieldTextState* state;
    s32 count;
    u32 draw_mode;
    FieldTextSystem* text_sys = FIELD_TEXT_SYSTEM;

    cdrom_stream(CD_RES_FIELD_WINDOW_TEXTURES, FIELD_TEXT_CACHE);

    setRECT(&rect, 304, 508, 16, 4);
    LoadImage(&rect, (u_long*)FIELD_TEXT_CACHE);

    setRECT(&rect, 960, 480, 64, 32);
    LoadImage(&rect, (u_long*)(FIELD_TEXT_CACHE + 0x40));

    draw_mode = _get_mode(1, 0, getTPage(0, 0, 960, 256));
    text_sys->draw_mode0 = draw_mode;
    text_sys->draw_mode1 = draw_mode;
    text_sys->text_clut = getClut(304, 508);
    text_sys->text_alt_clut = getClut(304, 511);
    text_sys->window_clut = getClut(304, 509);
    text_sys->prompt_clut = getClut(304, 510);
    text_sys->portrait_clut[0] = getClut(304, 506);
    text_sys->portrait_clut[1] = getClut(304, 507);
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
 * @see decomp.me (100%)
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
 * @see decomp.me (100%)
 */

void field_text_reset_scratch(void)
{
    if ((g_field_text_window0_flags & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_TIMED)
    {
        field_text_close(FIELD_TEXT_WINDOWS, 0);
    }
    /* TODO: remove the one-pass scope without changing the initialization registers. */
    do
    {
        FieldTextState* state = FIELD_TEXT_IMMEDIATE_STATE;
        state->dirty_end_u = FIELD_TEXT_CACHE_WIDTH;
        state->region_end_u = FIELD_TEXT_CACHE_WIDTH;
        state->dirty_end_v = 0x60;
        state->region_end_v = 0x60;
        state->line_advance = 0xFF0;
        state->width = 0xFF0;
        state->remaining_width = 0xFF0;
        state->line_height = 0xC;
        state->dirty_start_u = 0;
        state->cursor_u = 0;
        state->region_start_u = 0;
        state->dirty_start_v = 0;
        state->cursor_v = 0;
        state->region_start_v = 0;
        state->line_count = 0;
        state->portrait = 0;
        state->text_cursor = 0;
        state->macro_cursor = 0;
        state->glyph_cursor = 0;
        state->flow_code = FIELD_TEXT_FLOW_NONE;
        state->pending_spaces = 0;
        state->choice_count = 0;
        state->text_color = 0;
        state->scroll_timer = 0;
        state->needs_init = 0;

        state->flags.word = ((((state->flags.word & ~FIELD_TEXT_STATE_MASK) | 6) & ~FIELD_TEXT_STYLE_MASK) | 0x800) & ~FIELD_TEXT_AUTO_CLOSE;
    } while (0);
}

/**
 * @brief Typeset a string and describe its cached spans as sprite primitives.
 * @param prim Output sprite array.
 * @param text Text to typeset.
 * @param text_style Text palette/style selector; only the low 16 bits are used.
 * @return Number of sprite spans written.
 * @see decomp.me (100%)
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
    state->macro_cursor = 0;
    state->glyph_cursor = 0;
    state->pending_spaces = 0;
    state->flow_code = FIELD_TEXT_FLOW_NONE;
    remaining = state->line_height;
    start_x = state->width - state->remaining_width;
    state->text_cursor = text;
    while (--remaining != -1)
    {
        *carry = 0;
        carry += 1;
    }
    field_text_typeset(state, 0);
    tex_u = start_x;
    state->remaining_width = state->remaining_width & 0xFFFC;
    end_x = state->width - state->remaining_width;
    tex_v = 0;
    while (tex_u >= FIELD_TEXT_CACHE_WIDTH)
    {
        tex_u -= FIELD_TEXT_CACHE_WIDTH;
        tex_v += 0xC;
    }
    remaining = ((end_x - start_x) + 3) >> 2;
    if (remaining != 0)
    {
        do
        {
            col = tex_u >> 2;
            prim->v0 = tex_v - 0x80;
            prim->u0 = tex_u;
            if ((col + remaining) >= 0x41)
            {
                cols = 0x40 - col;
                tex_u = 0;
                tex_v += 0xC;
                remaining -= cols;
            }
            else
            {
                cols = remaining;
                remaining = 0;
            }
            setWH(prim, cols * 4, 0xC);
            if (style >= 8)
            {
                prim->clut = getClut(304, 508);
            }
            else
            {
                prim->clut = getClut(304, 511);
            }
            prim += 1;
            count += 1;
        } while (remaining != 0);
    }
    return count;
}

/**
 * @brief Open a text window after the cache region used by earlier active slots.
 * @param slot Window slot index.
 * @see decomp.me (100%)
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
        state->flags.word = (flags & ~FIELD_TEXT_REOPEN_MASK) | 0x2000;
        field_text_save_config(slot);
        return;
    }
    field_text_apply_config(state);
    if (state->portrait != 0)
    {
        if ((system->portrait_slots & 1) == 0)
        {
            state->flags.word &= ~8;
            system->portrait_slots |= 1;
        }
        else
        {
            state->flags.word |= 8;
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
        n -= 0x10;
    }
    state->dirty_end_u = x;
    state->region_end_u = x;
    state->dirty_end_v = y;
    state->region_end_v = y;
}

/**
 * @brief Open a text window in its fixed cache region.
 * @param slot Window slot index.
 * @see decomp.me (100%)
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
        state->flags.word = (flags & ~FIELD_TEXT_REOPEN_MASK) | 0x4000;
        field_text_save_config(slot);
        return;
    }
    field_text_apply_config(state);
    if (state->portrait != 0)
    {
        if (slot == 0)
        {
            state->flags.word &= ~8;
            system->portrait_slots |= 1;
        }
        else
        {
            state->flags.word |= 8;
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
        y = 0x30;
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
        h -= 0x10;
    }
    state->dirty_end_u = x;
    state->region_end_u = x;
    state->dirty_end_v = y;
    state->region_end_v = y;
}

/**
 * @brief Apply the pending text configuration to a runtime window state.
 * @param state Window state to initialize.
 * @see decomp.me (100%)
 */

void field_text_apply_config(FieldTextState* state)
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
    if ((state->portrait != 0) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) != 0x20) && ((s32)config_value < 0x30))
    {
        config_value = 0x30;
    }
    state->remaining_width = width;
    state->width = width;
    state->height = config_value;
    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
    {
        state->line_advance = width + 4;
        state->line_height = 0xD;
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | 2;
    }
    else
    {
        state->line_height = 0xC;
        state->line_advance = width;
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | 1;
    }
    state->text_cursor = 0;
    state->macro_cursor = 0;
    state->glyph_cursor = 0;
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
    state->flags.word &= ~0x800;
    state->flags.word &= ~FIELD_TEXT_AUTO_CLOSE;
    state->flags.word &= ~FIELD_TEXT_REOPEN_MASK;
}

/**
 * @brief Update, upload, and render all field text windows for one frame.
 * @param packet_cursor Address of the render-packet cursor.
 * @param ot Ordering-table base address.
 * @param draw_count Current field draw count; 1 selects the render-only path.
 * @see decomp.me (100%)
 */

void field_text_update(u8** packet_cursor, FieldOrderingTags* ot, s32 draw_count)
{
    FieldInputState* input = FIELD_TEXT_INPUT;
    FieldTextState* state = FIELD_TEXT_WINDOWS;
    FieldTextConfig* config;
    u8* src;
    u8* dst;
    s32 i;
    s32 n;
    s32 keys;
    s32 mode;
    s32 tmp;
    u16 idx;

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
                if (state->portrait != 0)
                {
                    field_text_queue_portrait_upload(state->portrait, packet_cursor, (state->flags.word >> 3) & 1,
                                                     (state->flags.word & FIELD_TEXT_PORTRAIT_MASK) != 0x10);
                }
                field_text_clear_window(state);
                field_text_queue_uploads(state, (u16**)packet_cursor);
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
                            if ((keys & 0x10) != 0)
                            {
                                tmp = state->choice_index;
                                if (tmp == 0)
                                {
                                    tmp = state->choice_count;
                                }
                                state->choice_index = tmp - 1;
                                akao_play_sfx(0x7D, 0, 0x80, 0x7F);
                            }
                            if ((keys & 0x40) != 0)
                            {
                                if (state->choice_index < (state->choice_count - 1))
                                {
                                    state->choice_index = state->choice_index + 1;
                                }
                                else
                                {
                                    state->choice_index = 0;
                                }
                                akao_play_sfx(0x7D, 0, 0x80, 0x7F);
                            }
                            if ((input->pressed_buttons & 0x4002) != 0)
                            {
                                state->flow_code = FIELD_TEXT_FLOW_NONE;
                                state->choice_count = 0;
                                state->text_cursor = 0;
                                if ((state->flags.word & FIELD_TEXT_AUTO_CLOSE) != 0)
                                {
                                    if (state->portrait != 0)
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
                                    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
                                    {
                                        state->flags.word = state->flags.word & ~FIELD_TEXT_STATE_MASK;
                                    }
                                    else
                                    {
                                        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | 3;
                                        state->transition_frame = 0;
                                    }
                                }
                                akao_play_sfx(0x7E, 0, 0x80, 0x7F);
                            }
                        }
                        else if (((input->pressed_buttons & 0x4002) != 0) && (state->prompt_frame != 2))
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
                            field_text_queue_uploads(state, (u16**)packet_cursor);
                        }
                    }
                    else if (state->text_cursor != 0)
                    {
                        field_text_typeset(state, 4);
                        field_text_queue_uploads(state, (u16**)packet_cursor);
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
                        state->prompt_timer = 4;
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
                                state->text_cursor = 0;
                                if ((state->flags.word & FIELD_TEXT_AUTO_CLOSE) != 0)
                                {
                                    if (state->portrait != 0)
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
                                    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == FIELD_TEXT_STYLE_BOLD)
                                    {
                                        state->flags.word = state->flags.word & ~FIELD_TEXT_STATE_MASK;
                                    }
                                    else
                                    {
                                        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | 3;
                                        state->transition_frame = 0;
                                    }
                                }
                                break;
                            case FIELD_TEXT_FLOW_CLEAR:
                                field_text_clear_window(state);
                                field_text_queue_uploads(state, (u16**)packet_cursor);
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
                            state->prompt_timer = 8;
                        }
                    }
                }
            }
            if (((state->flags.word & FIELD_TEXT_STATE_MASK) == FIELD_TEXT_CLOSED) && ((state->flags.word & FIELD_TEXT_REOPEN_MASK) != 0))
            {
                idx = 3 - i;
                mode = (state->flags.word >> 13) & 3;
                dst = (u8*)FIELD_TEXT_PENDING_CONFIG;
                n = sizeof(FieldTextConfig) - 1;
                config = &g_field_text_saved_configs[idx];
                src = (u8*)config;
                for (; n != -1; n--)
                {
                    *dst++ = *src++;
                }
                if (mode == 1)
                {
                    field_text_open_packed_window(idx);
                }
                else
                {
                    field_text_open_fixed_window(idx);
                }
                if (config->text != 0)
                {
                    field_text_set_string(idx, config->text, config->flags.b.byte2);
                }
            }
            break;
        case FIELD_TEXT_TIMED:
            if (state->needs_init == 1)
            {
                state->transition_frame = 0x32;
                state->needs_init = 0;
            }
            if (state->text_cursor != 0)
            {
                field_text_clear_window(state);
                field_text_typeset(state, 0);
                state->text_cursor = 0;
                state->flow_code = FIELD_TEXT_FLOW_NONE;
                state->dirty_start_u = state->region_start_u;
                state->dirty_start_v = state->region_start_v;
                state->dirty_end_u = state->region_end_u;
                state->dirty_end_v = state->region_end_v;
                field_text_queue_uploads(state, (u16**)packet_cursor);
            }
            field_text_build_window_packets(state, packet_cursor, ot);
            state->transition_frame = state->transition_frame - 1;
            if (state->transition_frame == 0)
            {
                if (state->portrait != 0)
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
 * @see decomp.me (100%)
 */

void field_text_build_transition_quad(FieldTextState* state, FieldTextQuad* out, s32 frame)
{
    s32 half_w;
    s32 half_h;
    s32 pad_h;
    s32 n;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 x3;
    s32 y3;
    s32 cx;
    s32 cy;
    s32 rem;
    s32 neg_h;

    if ((state->portrait != 0) && (((state->flags.word >> 4) & 3) < 2))
    {
        half_w = state->width + FIELD_TEXT_PORTRAIT_MARGIN;
    }
    else
    {
        half_w = state->width;
    }
    half_w = half_w / 2 + 8;
    half_h = (u32)state->height / 2;
    pad_h = half_h + 8;
    neg_h = -pad_h;
    x2 = -half_w;
    x3 = half_w;
    n = frame + 3;
    if (n > 4)
    {
        n = 4;
    }
    x0 = (x2 * n) / 4;
    x1 = (x3 * n) / 4;
    x2 = (x2 * n) / 4;
    y1 = (x3 * n) / 4;
    x3 = y1;
    y0 = ((neg_h + 2) * frame) / 4 - 2;
    y1 = ((neg_h + 2) * frame) / 4 - 2;
    y2 = ((half_h + 6) * frame) / 4 + 2;
    y3 = ((half_h + 6) * frame) / 4 + 2;
    cx = state->x + half_w;
    cy = state->y + pad_h;
    if (state->flags.b.byte2 != 0)
    {
        rem = 4 - frame;
        cx = (cx * frame + state->transition_anchor_x * rem) / 4;
        cy = (cy * frame + (0xE0 - state->transition_anchor_y) * rem) / 4;
    }
    out->x0 = cx + x0;
    out->x1 = cx + x1;
    out->x2 = cx + x2;
    out->x3 = cx + x3;
    out->y0 = cy + y0;
    out->y1 = cy + y1;
    out->y2 = cy + y2;
    out->y3 = cy + y3;
}

/**
 * @brief Center a portrait vertically and pack its screen Y coordinate.
 * @param y Top of the window.
 * @param h Height of the text area.
 * @return Screen Y coordinate in the upper halfword.
 */
static inline s32 field_text_portrait_y_word(s32 y, s32 h)
{
    h -= 0x30;
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
 * @see decomp.me (100%)
 */

void field_text_build_window_packets(FieldTextState* state, u8** cursor, FieldOrderingTags* ot)
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
    s32 row_height;
    s32 texture_u_origin;
    s32 uv_base;
    s32 cache_u;
    s32 cache_v;
    s32 scroll_pixels;
    s32 available_pixels;
    u32 sprite_color;
    s32 bottom_border_v;
    u32 tag_mask;
    u32 tag_len;
    u32 corner_size;
    u32 border_tile_size;
    s32 top_border_v;

    sprite_color = FIELD_TEXT_SPRITE_COLOR;
    packet_cursor = *cursor;
    prim = (FieldTextPacket*)packet_cursor;
    first = packet_cursor;
    packet_cursor += sizeof(DR_TPAGE);
    prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x01000000;
    prim->sprite_words.rgbc = text_system->draw_mode0;
    texture_u_origin = 0;
    uv_base = 0xE0;
    if ((state->flags.word & FIELD_TEXT_STYLE_MASK) == 0)
    {
        y = state->y;
        rows = 1;
        top_border_v = 0xF000;
        bottom_border_v = 0xF800;
        tag_mask = 0xFFFFFF;
        tag_len = 0x04000000;
        corner_size = 0x80008;
        border_tile_size = 0x80040;
        do
        {
            {
                s32 uv_hi;
                uv_hi = text_system->window_clut << 16;
                if (rows != 0)
                {
                    uv = (uv_hi | top_border_v) | texture_u_origin;
                }
                else
                {
                    uv = (uv_hi | bottom_border_v) | texture_u_origin;
                }
            }
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            xy = state->x | (y << 16);
            prim->sprite_words.uv = uv;
            uv += 8;
            prim->sprite_words.tag = ((u32)packet_cursor & tag_mask) | tag_len;
            prim->sprite_words.rgbc = sprite_color;
            prim->sprite_words.wh = corner_size;
            prim->sprite_words.xy = xy;
            xy += 8;
            if ((state->portrait != 0) && (((state->flags.word >> 4) & 3) < 2))
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
                    prim->sprite_words.tag = ((u32)packet_cursor & tag_mask) | tag_len;
                    prim->sprite_words.rgbc = sprite_color;
                    prim->sprite_words.xy = xy;
                    prim->sprite_words.uv = uv;
                    if (pixels_remaining >= 0x41)
                    {
                        prim->sprite_words.wh = border_tile_size;
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
            prim->sprite_words.tag = ((u32)packet_cursor & tag_mask) | tag_len;
            prim->sprite_words.rgbc = sprite_color;
            prim->sprite_words.xy = xy;
            prim->sprite_words.uv = uv;
            prim->sprite_words.wh = corner_size;
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
                prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
                prim->sprite_words.rgbc = sprite_color;
                prim->sprite_words.uv = uv;
                prim->sprite_words.wh = row_height | 8;
                uv -= 0x40;
                if ((state->portrait != 0) && (((state->flags.word >> 4) & 3) < 2))
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
                        prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
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
                prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
                prim->sprite_words.rgbc = sprite_color;
                prim->sprite_words.xy = xy;
                prim->sprite_words.uv = uv;
                prim->sprite_words.wh = row_height | 8;
            } while (rows > 0);
        }
        texture_u_origin = 0;
    }
    uv_base = 0x80;
    row_height = state->line_height;
    cache_u = state->region_start_u;
    cache_v = state->region_start_v;
    scroll_pixels = state->scroll_timer;
    y = state->y + 8;
    rows = state->height >> 4;
    rows -= 1;
    if (rows != -1)
    {
        s32 neg16;
        do
        {
            if ((state->portrait != 0) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0))
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
                    if ((scroll_pixels != 0) && ((u32)(0x10 - scroll_pixels) >= (u32)row_height))
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
                        prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
                        prim->sprite_words.rgbc = sprite_color;
                        prim->sprite_words.xy = xy;
                        prim->sprite.u0 = texture_u_origin + cache_u;
                        prim->sprite.clut = text_system->text_clut;
                        if (scroll_pixels != 0)
                        {
                            neg16 = 0xFFF0;
                            prim->sprite.v0 = (uv_base + cache_v + 0x10) - scroll_pixels;
                            prim->sprite.h = row_height + (scroll_pixels + neg16);
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
    if (state->portrait != 0)
    {
        prim = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(DR_TPAGE);
        prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x01000000;
        prim->sprite_words.rgbc = text_system->draw_mode1;
        if ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0)
        {
            xy = (state->x + 8) & 0xFFFF;
        }
        else
        {
            s32 px;
            s32 pw;
            px = state->x;
            pw = state->width;
            pw += 0x10;
            px += pw;
            xy = px & 0xFFFF;
        }
        {
            s32 py;
            s32 ph;
            u32 pflags;
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            py = state->y;
            ph = state->height;
            pflags = state->flags.word;
            prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
            prim->sprite_words.rgbc = FIELD_TEXT_SPRITE_SHADOW;
            py = field_text_portrait_y_word(py, ph);
            xy |= py;
            prim->sprite_words.xy = xy + 0x20002;
            uv = ((((((pflags >> 3) & 1) * 0x30) + 0x110) & 0xFF) << 8) | 0xD0;
            prim->sprite_words.uv = (text_system->text_clut << 16) | uv;
            prim->sprite_words.wh = 0x300030;
            prim = (FieldTextPacket*)packet_cursor;
            packet_cursor += sizeof(SPRT);
            prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
            prim->sprite_words.rgbc = sprite_color;
            prim->sprite_words.xy = xy;
            prim->sprite_words.uv = (text_system->portrait_clut[(state->flags.word >> 3) & 1] << 16) | uv;
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
            prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x01000000;
            prim->sprite_words.rgbc = text_system->draw_mode0;
            if (state->flow_code == FIELD_TEXT_FLOW_CHOICE)
            {
                prim = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(SPRT_16);
                prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x03000000;
                prim->sprite_words.rgbc = FIELD_TEXT_CHOICE_COLOR;
                if ((state->portrait != 0) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0))
                {
                    prim->sprite16.x0 = state->x + FIELD_TEXT_PORTRAIT_MARGIN;
                }
                else
                {
                    prim->sprite16.x0 = state->x + 0xE;
                }
                prim->sprite16.y0 = state->y + ((state->choice_start_line + state->choice_index) * 0x10);
                rows = state->prompt_frame;
                if (rows == 3)
                {
                    rows = 1;
                }
                setUV0(&prim->sprite16, (rows << 4) + 0x60, 0xE0);
                prim->sprite16.clut = text_system->prompt_clut;
            }
            else
            {
                prim = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(SPRT);
                prim->sprite_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x04000000;
                prim->sprite_words.rgbc = sprite_color;
                if ((state->portrait != 0) && (((state->flags.word >> 4) & 3) < 2))
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
                {
                    s32 frame;
                    frame = state->prompt_frame;
                    prim->sprite_words.wh = 0x80010;
                    prim->sprite.v0 = (frame * 8) - 0x20;
                }
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

void field_text_build_transition_packets(FieldTextState* state, FieldTextQuad* quad, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextSystem* text_system = (FieldTextSystem*)0x801ED000;
    FieldTextVertex* vertex;
    FieldTextVertex* mesh;
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
    s32 row_v;
    s32 sel;
    s32 texture_u_origin;
    u32 texture_command;
    s32 rows_remaining;
    s32 texture_v_origin;

    base_x = 0;
    base_y = 0;
    dx = 0;
    dy = 0;
    if ((state->portrait != 0) && (((state->flags.word >> 4) & 3) < 2))
    {
        content_width = state->width + FIELD_TEXT_PORTRAIT_MARGIN;
    }
    else
    {
        content_width = state->width;
    }

    /* Build the unwarped mesh in scratchpad RAM. */
    vertex = (FieldTextVertex*)0x1F800000;
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

    rows_remaining = state->height;
    rows_remaining -= 1;
    mesh_y += 8;
    if (rows_remaining != -1)
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
            if (rows_remaining >= 0x21)
            {
                mesh_y += 0x20;
                rows_remaining -= 0x20;
            }
            else
            {
                mesh_y += rows_remaining;
                rows_remaining = 0;
            }
            rows_remaining -= 1;
        } while (rows_remaining != -1);
    }

    rows_remaining = 1;
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
        rows_remaining -= 1;
        mesh_y += 8;
    } while (rows_remaining != -1);

    mesh_y = 8;
    u = state->region_start_u;
    rows_remaining = state->height >> 4;
    rows_remaining -= 1;
    text_vertex_count = 0;
    if (rows_remaining != -1)
    {
        do
        {
            mesh_x = 8;
            if ((state->portrait != 0) && ((state->flags.word & FIELD_TEXT_PORTRAIT_MASK) == 0))
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
            rows_remaining -= 1;
            mesh_y += 0x10;
        } while (rows_remaining != -1);
    }

    rows_remaining = 1;
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
        rows_remaining -= 1;
        mesh_y += 0x30;
    } while (rows_remaining != -1);

    rows_remaining = 1;
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
        rows_remaining -= 1;
        mesh_y += 0x30;
    } while (rows_remaining != -1);

    /* Interpolate the two side edges once per row, then interpolate across each row. */
    vertex = (FieldTextVertex*)0x1F800000;
    previous_y = -1;
    mesh_width = content_width + 0x10;
    rows_remaining = ((((state->height + 0x1F) >> 5) + 3) * (((content_width + 0x3F) >> 6) + 3)) + text_vertex_count + 7;
    mesh_height = state->height + 0x10;
    if (rows_remaining != -1)
    {
        do
        {
            u = vertex->pos.vy;
            if (previous_y != u)
            {
                previous_y = u;
                base_x = (((quad->x2 - quad->x0) * u) / mesh_height) + quad->x0;
                base_y = (((quad->y2 - quad->y0) * u) / mesh_height) + quad->y0;
                dx = ((((quad->x3 - quad->x1) * u) / mesh_height) + quad->x1) - base_x;
                dy = ((((quad->y3 - quad->y1) * u) / mesh_height) + quad->y1) - base_y;
            }
            vertex->pos.vy = ((dy * vertex->pos.vx) / mesh_width) + base_y;
            rows_remaining -= 1;
            vertex->pos.vx = ((dx * vertex->pos.vx) / mesh_width) + base_x;
            vertex += 1;
        } while (rows_remaining != -1);
    }

    /* Draw the top and bottom borders, followed by the tiled window interior. */
    tpage = getTPage(0, 0, 960, 256) << 16;
    texture_u_origin = 0;
    texture_v_origin = 0xE0;
    texture_command = FIELD_TEXT_QUAD_COLOR;
    vertex = (FieldTextVertex*)0x1F800000;
    packet_height = 8;
    rows_remaining = 1;
    first = *cursor;
    packet_cursor = first;
    clut = text_system->window_clut;
    clut <<= 16;
    do
    {
        if (rows_remaining == 0)
        {
            texture_uv = texture_u_origin | 0xF800;
        }
        else
        {
            texture_uv = texture_u_origin | 0xF000;
        }
        poly = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(POLY_FT4);
        sel = (u32)packet_cursor & 0xFFFFFF;
        poly->quad_words.tag = sel | 0x09000000;
        poly->quad_words.uv0 = clut | texture_uv;
        poly->quad_words.uv1 = tpage | (texture_uv + 8);
        poly->quad_words.uv2 = texture_uv + (packet_height << 8);
        poly->quad_words.rgbc = texture_command;
        poly->quad_words.uv3 = texture_uv + ((packet_height << 8) | 8);
        texture_uv += 8;
        poly->quad_words.xy0 = vertex[0].word;
        pixels_remaining = content_width;
        poly->quad_words.xy1 = vertex[1].word;
        bottom_vertices = vertex + ((content_width + 0x3F) >> 6) + 4;
        poly->quad_words.xy2 = bottom_vertices[-1].word;
        poly->quad_words.xy3 = bottom_vertices[0].word;
        vertex += 1;
        if (content_width > 0)
        {
            do
            {
                poly = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(POLY_FT4);
                poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
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
        poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
        poly->quad_words.uv0 = clut | texture_uv;
        poly->quad_words.uv1 = tpage | (texture_uv + 7);
        poly->quad_words.rgbc = texture_command;
        poly->quad_words.uv2 = texture_uv + (packet_height << 8);
        poly->quad_words.uv3 = texture_uv + ((packet_height << 8) | 7);
        poly->quad_words.xy0 = vertex[0].word;
        poly->quad_words.xy1 = vertex[1].word;
        poly->quad_words.xy2 = bottom_vertices[0].word;
        poly->quad_words.xy3 = bottom_vertices[1].word;
        rows_remaining -= 1;
        vertex = (FieldTextVertex*)0x1F800000 + ((((state->height + 0x1F) >> 5) + 1) * (((content_width + 0x3F) >> 6) + 3));
        packet_height = 7;
    } while (rows_remaining != -1);

    {
        FieldTextVertex* bottom_vertices;

        vertex = (FieldTextVertex*)0x1F800000 + 3 + ((content_width + 0x3F) >> 6);
        bottom_vertices = vertex + ((content_width + 0x3F) >> 6) + 3;
        rows_remaining = state->height;
        if (rows_remaining > 0)
        {
            do
            {
                texture_uv = (texture_v_origin << 8) | (texture_u_origin + 0xE0);
                packet_height = 0x1F00;
                if (rows_remaining < 0x20)
                {
                    packet_height = rows_remaining << 8;
                }
                poly = (FieldTextPacket*)packet_cursor;
                packet_cursor += sizeof(POLY_FT4);
                poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
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
                        poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
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
                poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
                poly->quad_words.uv0 = clut | texture_uv;
                poly->quad_words.uv1 = tpage | (texture_uv + 8);
                poly->quad_words.uv2 = texture_uv + packet_height;
                poly->quad_words.rgbc = texture_command;
                poly->quad_words.uv3 = texture_uv + (packet_height | 8);
                poly->quad_words.xy0 = vertex[0].word;
                rows_remaining -= 0x20;
                poly->quad_words.xy1 = vertex[1].word;
                poly->quad_words.xy2 = bottom_vertices[0].word;
                poly->quad_words.xy3 = bottom_vertices[1].word;
                vertex += 2;
                bottom_vertices += 2;
            } while (rows_remaining > 0);
        }
    }

    /* Text vertices are pairs of upper/lower endpoints split at cache page boundaries. */
    texture_u_origin = 0;
    texture_v_origin = 0x80;
    u = state->region_start_u;
    row_v = state->region_start_v;
    text_area_height = state->height;
    rows_remaining = text_area_height >> 4;
    rows_remaining -= 1;
    vertex = (FieldTextVertex*)0x1F800000 + ((((text_area_height + 0x1F) >> 5) + 3) * (((content_width + 0x3F) >> 6) + 3));
    if (rows_remaining != -1)
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
                    poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
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
            rows_remaining -= 1;
            vertex += 2;
        } while (rows_remaining != -1);
    }

    tpage = getTPage(0, 0, 960, 256) << 16;
    /* The final eight mesh vertices hold the portrait shadow and image quads. */
    mesh = (FieldTextVertex*)0x1F800000;
    if (state->portrait != 0)
    {
        poly = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(POLY_FT4);
        frame_rows = state->height;
        frame_rows += 0x1F;
        frame_rows >>= 5;
        frame_rows += 3;
        sel = (state->flags.word >> 3) & 1;
        clut = text_system->text_clut;
        poly->quad_words.tag = ((u32)packet_cursor & 0xFFFFFF) | 0x09000000;
        poly->quad_words.rgbc = FIELD_TEXT_QUAD_SHADOW;
        texture_uv = (((((sel * 0x30) + 0x110) & 0xFF) << 8) | 0xD0);
        poly->quad_words.uv0 = (clut << 16) | texture_uv;
        poly->quad_words.uv2 = texture_uv + 0x3000;
        poly->quad_words.uv1 = (texture_uv + 0x2F) | tpage;
        clut = content_width + 0x3F;
        clut = clut >> 6;
        vertex = mesh + text_vertex_count + (frame_rows * (clut + 3));
        poly->quad_words.uv3 = texture_uv + 0x302F;
        poly->quad_words.xy0 = vertex[0].word;
        poly->quad_words.xy1 = vertex[1].word;
        poly->quad_words.xy2 = vertex[2].word;
        poly->quad_words.xy3 = vertex[3].word;
        vertex += 4;
        poly = (FieldTextPacket*)packet_cursor;
        packet_cursor += sizeof(POLY_FT4);
        sel = (state->flags.word >> 3) & 1;
        clut = text_system->portrait_clut[sel];
        texture_uv = ((((sel * 0x30) + 0x110) & 0xFF) << 8) | 0xD0;
        poly->quad_words.uv2 = texture_uv + 0x3000;
        poly->quad_words.uv1 = (texture_uv + 0x2F) | tpage;
        dy = (u32)packet_cursor & 0xFFFFFF;
        poly->quad_words.tag = dy | 0x09000000;
        poly->quad_words.rgbc = texture_command;
        poly->quad_words.uv3 = texture_uv + 0x302F;
        poly->quad_words.uv0 = (clut << 16) | texture_uv;
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
 * @see decomp.me (100%)
 */

void field_text_scroll_cache(FieldTextState* state)
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
            destination_row = (FIELD_TEXT_CACHE + (destination_u >> 2)) + (destination_v << 6);
            source_row = (FIELD_TEXT_CACHE + (source_u >> 2)) + (source_v << 6);
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
        destination_row = (FIELD_TEXT_CACHE + (next_u >> 2)) + (next_v << 6);
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
 * @see decomp.me (100%)
 */

void field_text_queue_uploads(FieldTextState* state, u16** cursor)
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

    cur = *cursor;
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
    row = (u8*)FIELD_TEXT_CACHE + (y << 7);
    src = FIELD_TEXT_CACHE_WORD(row, column_bytes);
    h = state->line_height;
    req->rect.x = column + 960;
    w = span >> 1;
    req->rect.y = y + 384;
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
            src = FIELD_TEXT_CACHE + (y << 6);
            req->rect.x = 960;
            req->rect.y = y + 384;
            req->rect.w = FIELD_TEXT_CACHE_ROW_WORDS;
            req->rect.h = state->dirty_end_v - y;
            req->data = (u_long*)src;
            field_queue_vram_upload(req);
        }
        req = (FieldImageReq*)cur;
        if (state->dirty_end_u != 0)
        {
            cur += sizeof(FieldImageReq) / sizeof(*cur);
            req->rect.x = 960;
            req->rect.y = state->dirty_end_v + 384;
            req->rect.w = state->dirty_end_u >> 2;
            req->rect.h = h;
            src = FIELD_TEXT_CACHE + (state->dirty_end_v << 6);
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
    *cursor = cur;
}

/**
 * @brief Attach a text string to a window, or defer it while the slot reopens.
 * @param window_index Window slot; only the low 16 bits are used.
 * @param text Text pointer.
 * @param text_options Text options; bit 0 enables automatic close.
 * @see decomp.me (100%)
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
    state->macro_cursor = 0;
    state->glyph_cursor = 0;
    state->pending_spaces = 0;
    state->flow_code = FIELD_TEXT_FLOW_NONE;
    state->flags.word = (state->flags.word & ~FIELD_TEXT_AUTO_CLOSE) | ((options & 1) << 12);
}

/**
 * @brief Save the pending text configuration for a window slot.
 * @param slot Window slot index.
 * @see decomp.me (100%)
 */

void field_text_save_config(u16 slot)
{
    u8* src;
    u8* dst;
    s32 bytes_remaining;
    FieldTextConfig* config;

    src = (u8*)FIELD_TEXT_PENDING_CONFIG;
    bytes_remaining = sizeof(FieldTextConfig) - 1;
    config = &g_field_text_saved_configs[slot];
    config->text = NULL;
    dst = (u8*)config;
    for (; bytes_remaining != -1; bytes_remaining--)
    {
        *dst++ = *src++;
    }
}

/**
 * @brief Close a text window and release its portrait slot.
 * @param state Text-window state.
 * @param animate Non-zero starts the closing animation when supported.
 * @see decomp.me (100%)
 */

void field_text_close(FieldTextState* state, s32 animate)
{
    if (state->portrait != 0)
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
        state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | 3;
        state->transition_frame = 0;
    }
}

/**
 * @brief Render a text window according to its opening, active, or closing state.
 * @param state Text-window state.
 * @param cursor In/out render-packet cursor.
 * @param ot Ordering-table slot.
 * @see decomp.me (100%)
 */

void field_text_render_window(FieldTextState* state, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextQuad quad;

    switch (state->flags.b.low & FIELD_TEXT_STATE_MASK)
    {
    case 1:
        field_text_build_transition_quad(state, &quad, state->transition_frame);
        field_text_build_transition_packets(state, &quad, cursor, ot);
        state->transition_frame = state->transition_frame + 1;
        if (state->transition_frame == 4)
        {
            state->flags.word = (state->flags.word & ~FIELD_TEXT_STATE_MASK) | 2;
        }
        break;
    case 2:
        field_text_build_window_packets(state, cursor, ot);
        break;
    case 3:
        state->transition_frame = state->transition_frame + 1;
        field_text_build_transition_quad(state, &quad, 4 - state->transition_frame);
        field_text_build_transition_packets(state, &quad, cursor, ot);
        if (state->transition_frame == 4)
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
 * @see decomp.me (100%)
 */

void field_text_queue_portrait_upload(FieldTextPortrait* image, u8** cursor, s32 slot, s32 mirror)
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
    req->rect.x = 304;
    req->rect.y = slot + 506;
    req->rect.w = 16;
    req->rect.h = 1;
    req->data = (u_long*)image->palette;
    field_queue_vram_upload(req);
    req += 1;
    req->rect.x = 1012;
    req->rect.y = (slot * FIELD_TEXT_PORTRAIT_SIZE) + 272;
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
 * @see decomp.me (100%)
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
            rows -= 0x10;
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
 * @see decomp.me (100%)
 */

void field_text_restore_window(u16 slot, s32 placement_mode)
{
    u8* dst;
    u8* src;
    s32 bytes_remaining;
    FieldTextConfig* config;

    dst = (u8*)FIELD_TEXT_PENDING_CONFIG;
    bytes_remaining = sizeof(FieldTextConfig) - 1;
    config = &g_field_text_saved_configs[slot];
    src = (u8*)config;
    for (; bytes_remaining != -1; bytes_remaining--)
    {
        *dst++ = *src++;
    }
    if (placement_mode == 1)
    {
        field_text_open_packed_window(slot);
    }
    else
    {
        field_text_open_fixed_window(slot);
    }
    if (config->text != 0)
    {
        field_text_set_string(slot, config->text, config->flags.b.byte2);
    }
}

/**
 * @brief Set the screen position of a text window.
 * @param slot Window slot; only the low 16 bits are used.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @see decomp.me (100%) TODO
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
 * @see decomp.me (100%) TODO
 */
void field_text_close_window(s32 slot)
{
    FieldTextState* state = &FIELD_TEXT_SYSTEM->windows[slot & 0xFFFF];
    field_text_close(state, 1);
}

/**
 * @brief Read the progress of an active dialogue window.
 * @param slot Window slot; only the low 16 bits are used.
 * @return -1 outside dialogue mode, 2 at a prompt, 1 while text remains, or 0 when finished.
 * @see decomp.me (100%) TODO
 */
s32 field_text_get_status(s32 slot)
{
    FieldTextState* state = &FIELD_TEXT_SYSTEM->windows[slot & 0xFFFF];

    if (((state->flags.word & FIELD_TEXT_STATE_MASK) != 0) && ((state->flags.b.low & FIELD_TEXT_STATE_MASK) < 4))
    {
        if (state->flow_code != FIELD_TEXT_FLOW_NONE)
        {
            return 2;
        }
        return state->text_cursor != 0;
    }
    return -1;
}

/**
 * @brief Read the last selected choice in a text window.
 * @param slot Window slot; only the low 16 bits are used.
 * @return Zero-based choice index.
 * @see decomp.me (100%) TODO
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
