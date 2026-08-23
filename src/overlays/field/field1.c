#include "common.h"

/** @brief Byte view of a field text flags word. */
typedef struct
{
    u8 low;                 // 0x10 state/style byte
    u8 byte1;               // 0x11
    u8 byte2;               // 0x12 transition/text option byte
    u8 byte3;               // 0x13 copied config byte
} FieldTextFlagBytes;

typedef union
{
    u32 word;
    FieldTextFlagBytes b;
} FieldTextFlags;

/** @brief Runtime state for one field dialogue/text window. */
typedef struct
{
    u8* text_cursor;        // 0x00 script cursor
    u8* macro_cursor;       // 0x04 macro-expansion cursor
    u8* glyph_cursor;       // 0x08 nested glyph-run cursor
    u8* portrait;           // 0x0C portrait image, or NULL
    FieldTextFlags flags;   // 0x10
    u8 flow_code;           // 0x14 pending page/prompt action
    u8 line_count;          // 0x15 emitted text-line count
    u8 choice_start_line;   // 0x16 first line of a choice list
    u8 choice_index;        // 0x17 selected choice
    u8 choice_count;        // 0x18 number of choices / choice nesting depth
    u8 pending_spaces;      // 0x19 spaces still to emit
    u8 char_delay;          // 0x1A typewriter delay
    u8 text_color;          // 0x1B text palette/style index
    u8 scroll_timer;        // 0x1C scroll countdown after filling the window
    u8 needs_init;          // 0x1D one-time window/portrait initialization
    u8 prompt_frame;        // 0x1E prompt animation frame
    u8 prompt_timer;        // 0x1F prompt animation timer
    u8 inline_text[0x49 - 0x20]; // 0x20 inline expansion buffer
    u8 last_was_break;      // 0x49 last glyph was a word-break opportunity
    u16 transition_frame;   // 0x4A open/close animation step
    s16 macro_remaining;    // 0x4C active macro character budget; -1 = unlimited
    u16 x;                  // 0x4E window x
    u16 y;                  // 0x50 window y
    u16 width;              // 0x52 text/window width
    u16 height;             // 0x54 text/window height
    u16 line_advance;       // 0x56 staging-buffer advance to the next text row
    u16 line_height;        // 0x58 staging-buffer row height
    u16 remaining_width;    // 0x5A remaining width on the current line
    u16 cursor_u;           // 0x5C staging-buffer text cursor u
    u16 cursor_v;           // 0x5E staging-buffer text cursor v
    u16 region_start_u;     // 0x60 live text-region start u
    u16 region_start_v;     // 0x62 live text-region start v
    u16 region_end_u;       // 0x64 live text-region end u
    u16 region_end_v;       // 0x66 live text-region end v
    u16 dirty_start_u;      // 0x68 dirty upload start u
    u16 dirty_start_v;      // 0x6A dirty upload start v
    u16 dirty_end_u;        // 0x6C dirty upload end u
    u16 dirty_end_v;        // 0x6E dirty upload end v
    u16 row_carry[14];      // 0x70 wrapped glyph-row carry data
    s32 transition_anchor_x;// 0x8C opening-animation anchor x
    u32 reserved90;         // 0x90
    s32 transition_anchor_y;// 0x94 opening-animation y measured from screen bottom
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
typedef struct
{
    u8* portrait;               // 0x00
    u16 x;                      // 0x04
    u16 y;                      // 0x06
    u16 width;                  // 0x08
    u16 height;                 // 0x0A
    FieldTextAnchorWord anchor; // 0x0C
    FieldTextFlags flags;       // 0x10
    u8* text;                   // 0x14 deferred text pointer
} FieldTextConfig;

/** @brief Field text renderer globals and four runtime window slots. */
typedef struct
{
    u8 _pad00[4];
    FieldTextConfig* configs;   // 0x04
    u8 _pad08[0x14 - 8];
    u32 draw_mode0;             // 0x14
    u32 draw_mode1;             // 0x18
    u16 text_clut;              // 0x1C
    u16 text_alt_clut;          // 0x1E
    u16 window_clut;            // 0x20
    u16 prompt_clut;            // 0x22
    u16 portrait_clut0;         // 0x24
    u16 portrait_clut1;         // 0x26
    u16 portrait_slots;         // 0x28 bitmask of occupied portrait VRAM slots
    u16 _pad2A[(0x34 - 0x2A) / 2];
    FieldTextState windows[4];  // 0x34
} FieldTextSystem;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/**
 * @brief One queued VRAM upload.
 * @note Mirrors FieldImageReq in field_scene_internal.h; the destination
 *       rectangle sits inline at 0x04 so its address can be taken directly.
 */
typedef struct FieldImageReq FieldImageReq;
struct FieldImageReq
{
    FieldImageReq* next;    // 0x00
    RECT rect;              // 0x04 destination rectangle in VRAM
    u_long* data;           // 0x0C source pixel data
};

/** @brief libgpu free-size sprite primitive (20 bytes). */
typedef struct
{
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    s16 x0;
    s16 y0;
    u8 u0;
    u8 v0;
    u16 clut;
    s16 w;
    s16 h;
} SPRT;

/** @brief Four screen-space corners of a quad, in POLY vertex order. */
typedef struct
{
    s16 x0;                 // 0x00
    s16 y0;                 // 0x02
    s16 x1;                 // 0x04
    s16 y1;                 // 0x06
    s16 x2;                 // 0x08
    s16 y2;                 // 0x0A
    s16 x3;                 // 0x0C
    s16 y3;                 // 0x0E
} Quad;

/** @brief Two-word GPU primitive (mode / tpage), packet length 1. */
typedef struct
{
    u32 tag;                // 0x00
    u32 code;               // 0x04
} PrimMode;

/** @brief 20-byte sprite primitive addressed a word at a time, packet length 4. */
typedef struct
{
    u32 tag;                // 0x00
    u32 rgbc;               // 0x04
    u32 xy;                 // 0x08
    u32 uv;                 // 0x0C
    u32 wh;                 // 0x10
} PrimSprt;

/** @brief 20-byte sprite primitive with the uv/size fields addressed singly. */
typedef struct
{
    u32 tag;                // 0x00
    u32 rgbc;               // 0x04
    u32 xy;                 // 0x08
    u8  u0;                 // 0x0C
    u8  v0;                 // 0x0D
    u16 clut;               // 0x0E
    s16 w;                  // 0x10
    s16 h;                  // 0x12
} PrimGlyph;

/** @brief 16-byte fixed-size sprite primitive, packet length 3. */
typedef struct
{
    u32 tag;                // 0x00
    u32 rgbc;               // 0x04
    s16 x0;                 // 0x08
    s16 y0;                 // 0x0A
    u8  u0;                 // 0x0C
    u8  v0;                 // 0x0D
    u16 clut;               // 0x0E
} PrimSprt16;

/** @brief 20-byte sprite primitive with separate position/uv/size fields. */
typedef struct
{
    u32 tag;                // 0x00
    u32 rgbc;               // 0x04
    s16 x0;                 // 0x08
    s16 y0;                 // 0x0A
    u8  u0;                 // 0x0C
    u8  v0;                 // 0x0D
    u16 clut;               // 0x0E
    u32 wh;                 // 0x10
} PrimIcon;

/** @brief Ordering-table slot a built packet chain is spliced into. */
/** @brief Pair of ordering-table tags; field text chains into the second tag. */
typedef struct
{
    u32 tag0;               // 0x00
    u32 tag1;               // 0x04
} FieldOrderingTags;

/** @brief One scratchpad mesh vertex. */
typedef struct
{
    s16 x;                  // 0x00
    s16 y;                  // 0x02
} Vec2s;

/** @brief 40-byte textured quad primitive, packet length 9. */
typedef struct
{
    u32 tag;                // 0x00
    u32 rgbc;               // 0x04
    u32 xy0;                // 0x08
    u32 uv0;                // 0x0C
    u32 xy1;                // 0x10
    u32 uv1;                // 0x14
    u32 xy2;                // 0x18
    u32 uv2;                // 0x1C
    u32 xy3;                // 0x20
    u32 uv3;                // 0x24
} PrimQuad;

typedef struct
{
    u8 mode;                 // 0x00 input-repeat mode
    u8 repeat_dir;           // 0x01 repeated direction
    u8 _pad2[2];             // 0x02
    u16 pressed;             // 0x04 newly pressed buttons
    u16 repeat;              // 0x06 repeat/held buttons
    u8 _pad8[4];             // 0x08
    u32 repeat_active;       // 0x0C
} FieldInputState;

void func_800640B4(FieldTextState* state);
void func_800632E0(FieldTextState* state, s32 budget);
s32 func_80064210(FieldTextState* state);
void field_queue_vram_upload(FieldImageReq* req);

void field_text_apply_config(FieldTextState* state);
void field_text_build_window_packets(FieldTextState* state, u8** cursor, FieldOrderingTags* ot);
void func_8006700C(FieldTextState* state, s32 animate);
void field_text_queue_portrait_upload(u8* image, u8** cursor, s32 slot, s32 mirror);
void field_text_queue_uploads(FieldTextState* state, u16** cursor);
void field_text_render_window(FieldTextState* state, u8** cursor, FieldOrderingTags* ot);
void field_text_save_config(u16 slot);
void field_text_scroll_cache(FieldTextState* state);
void field_text_set_string(u16 slot, u8* text, u8 options);

extern FieldTextConfig* D_801ED004;

extern u8* g_field_timed_text;
extern s16 g_field_text_portrait_slots;
extern s32 g_field_text_window0_flags;

/**
 * @brief Reset a text window staging cursor to the start of its live region.
 * @param state Text-window state.
 * @see decomp.me (100%)
 */

void func_8006429C(FieldTextState* state)
{
    u16 start_u = (u16)state->region_start_u;
    u16 start_v = (u16)state->region_start_v;
    u16 width = state->width;

    state->line_count = 0;
    state->cursor_u = start_u;
    state->cursor_v = start_v;
    state->remaining_width = width;
    func_800640B4(state);
}

/**
 * @brief Initialize field text textures, CLUT state, and window slots.
 * @see decomp.me (100%)
 */

void func_800642D4(void)
{
    RECT rect;
    s32 slot;
    s32* flags_ptr;
    u32 draw_mode;
    s32 mask;
    s32 limit;
    FieldTextSystem* text_sys = (FieldTextSystem*)0x801ED000;

    cdrom_stream(0xB1, 0x801DE000);

    rect.x = 0x130;
    rect.y = 0x1FC;
    rect.w = 0x10;
    rect.h = 4;
    LoadImage(&rect, (u32*)0x801DE000);

    rect.x = 0x3C0;
    rect.y = 0x1E0;
    rect.w = 0x40;
    rect.h = 0x20;
    LoadImage(&rect, (u32*)0x801DE080);

    draw_mode = 0xE100041F;
    slot = 3;
    mask = -8;
    limit = -1;
    flags_ptr = (s32*)0x801ED044;

    text_sys->draw_mode0 = draw_mode;
    text_sys->draw_mode1 = draw_mode;
    text_sys->text_clut = 0x7F13;
    text_sys->text_alt_clut = 0x7FD3;
    text_sys->window_clut = 0x7F53;
    text_sys->prompt_clut = 0x7F93;
    text_sys->portrait_clut0 = 0x7E93;
    text_sys->portrait_clut1 = 0x7ED3;
    text_sys->portrait_slots = 0;

    while (slot != limit)
    {
        *flags_ptr &= mask;
        slot -= 1;
        flags_ptr = (s32*)((u8*)flags_ptr + 0x98);
    }

    DrawSync(0);
}

/**
 * @brief Deactivate all field text windows and release portrait slots.
 * @see decomp.me (100%)
 */

void field_text_reset_windows(void)
{
    s32 slot;
    s32* flags_ptr;
    s32 mask;
    s32 limit;
    u32 flags;

    g_field_text_portrait_slots = 0;
    slot = 3;
    mask = -8;
    limit = -1;
    flags_ptr = (s32*)0x801ED044;

    do
    {
        flags = *flags_ptr;
        slot -= 1;
        flags &= mask;
        *flags_ptr = flags;
        flags_ptr = (s32*)((u8*)flags_ptr + 0x98);
    } while (slot != limit);
}

/**
 * @brief Reset the scratch state used for immediate string rendering.
 * @see decomp.me (100%)
 */

void field_text_reset_scratch(void)
{
    if ((g_field_text_window0_flags & 7) == 4)
    {
        func_8006700C((void*)0x801ED034, 0);
    }
    do
    {

        FieldTextState* st = (FieldTextState*)0x801ED0CC;
        st->dirty_end_u = 0x100;
        st->region_end_u = 0x100;
        st->dirty_end_v = 0x60;
        st->region_end_v = 0x60;
        st->line_advance = 0xFF0;
        st->width = 0xFF0;
        st->remaining_width = 0xFF0;
        st->line_height = 0xC;
        st->dirty_start_u = 0;
        st->cursor_u = 0;
        st->region_start_u = 0;
        st->dirty_start_v = 0;
        st->cursor_v = 0;
        st->region_start_v = 0;
        st->line_count = 0;
        st->portrait = 0;
        st->text_cursor = 0;
        st->macro_cursor = 0;
        st->glyph_cursor = 0;
        st->flow_code = 0;
        st->pending_spaces = 0;
        st->choice_count = 0;
        st->text_color = 0;
        st->scroll_timer = 0;
        st->needs_init = 0;

        st->flags.word = ((((st->flags.word & ~7) | 6) & ~0xC0) | 0x800) & ~0x1000;
    } while (0);
}

/**
 * @brief Typeset a string and describe its cached spans as sprite primitives.
 * @param prim Output sprite array.
 * @param text Text to typeset.
 * @param style Text palette/style selector.
 * @return Number of sprite spans written.
 * @see decomp.me (100%)
 */

s32 field_text_build_sprites(SPRT* prim, u8* text, u16 style)
{
    s32 count = 0;
    FieldTextState* st = (FieldTextState*)0x801ED0CC;
    u16* carry;
    s32 remaining;
    s32 start_x;
    s32 end_x;
    s32 tex_u;
    s32 tex_v;
    s32 col;
    s32 cols;

    st->last_was_break = 1;
    st->text_color = style & 7;
    carry = (u16*)0x801ED13C;
    st->macro_cursor = 0;
    st->glyph_cursor = 0;
    st->pending_spaces = 0;
    st->flow_code = 0;
    remaining = st->line_height;
    start_x = st->width - st->remaining_width;
    st->text_cursor = text;
    while (--remaining != -1)
    {
        *carry = 0;
        carry += 1;
    }
    func_800632E0(st, 0);
    tex_u = start_x;
    st->remaining_width = st->remaining_width & 0xFFFC;
    end_x = st->width - st->remaining_width;
    tex_v = 0;
    while (tex_u >= 0x100)
    {
        tex_u -= 0x100;
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
            prim->w = cols * 4;
            prim->h = 0xC;
            if (style >= 8)
            {
                prim->clut = 0x7F13;
            }
            else
            {
                prim->clut = 0x7FD3;
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

void field_text_open_packed_window(u16 slot)
{
    FieldTextSystem* system = (FieldTextSystem*)0x801ED000;
    FieldTextState* st;
    FieldTextState* prev;
    u32 flags;
    s32 n;
    s32 w;
    s32 x;
    s32 y;
    u16 wrap;

    if ((g_field_text_window0_flags & 7) == 4)
    {
        func_8006700C((void*)0x801ED034, 0);
    }
    st = &system->windows[slot];
    if ((st->flags.word & 7) == 2)
    {
        func_8006700C(st, 0);
    }
    flags = st->flags.word;
    if ((flags & 7) != 0)
    {
        st->flags.word = (flags & ~0x6000) | 0x2000;
        field_text_save_config(slot);
        return;
    }
    field_text_apply_config(st);
    if (st->portrait != 0)
    {
        if ((system->portrait_slots & 1) == 0)
        {
            st->flags.word &= ~8;
            system->portrait_slots |= 1;
        }
        else
        {
            st->flags.word |= 8;
            system->portrait_slots |= 2;
        }
    }
    x = 0;
    y = 0;
    n = slot;
    prev = &system->windows[0];
    while (--n != -1)
    {
        if ((prev->flags.word & 7) != 0)
        {
            x = prev->region_end_u;
            y = prev->region_end_v;
        }
        prev += 1;
    }
    n = st->height;
    st->dirty_start_u = x;
    st->cursor_u = x;
    st->region_start_u = x;
    st->dirty_start_v = y;
    st->cursor_v = y;
    st->region_start_v = y;
    while (n > 0)
    {
        w = st->line_advance;
        while (w > 0)
        {
            wrap = 0x100 - x;
            if (w >= wrap)
            {
                w -= wrap;
                x = 0;
                y += st->line_height;
            }
            else
            {
                x += w;
                w = 0;
            }
        }
        n -= 0x10;
    }
    st->dirty_end_u = x;
    st->region_end_u = x;
    st->dirty_end_v = y;
    st->region_end_v = y;
}

/**
 * @brief Open a text window in its fixed cache region.
 * @param slot Window slot index.
 * @see decomp.me (100%)
 */

void field_text_open_fixed_window(u16 slot)
{
    FieldTextSystem* system = (FieldTextSystem*)0x801ED000;
    FieldTextState* st;
    u32 flags;
    s32 h;
    s32 w;
    s32 x;
    s32 y;
    u16 wrap;

    if ((g_field_text_window0_flags & 7) == 4)
    {
        func_8006700C((void*)0x801ED034, 0);
    }
    st = &system->windows[slot];
    if ((st->flags.word & 7) == 2)
    {
        func_8006700C(st, 0);
    }
    flags = st->flags.word;
    if ((flags & 7) != 0)
    {
        st->flags.word = (flags & ~0x6000) | 0x4000;
        field_text_save_config(slot);
        return;
    }
    field_text_apply_config(st);
    if (st->portrait != 0)
    {
        if (slot == 0)
        {
            st->flags.word &= ~8;
            system->portrait_slots |= 1;
        }
        else
        {
            st->flags.word |= 8;
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
    h = st->height;
    st->dirty_start_u = x;
    st->cursor_u = x;
    st->region_start_u = x;
    st->dirty_start_v = y;
    st->cursor_v = y;
    st->region_start_v = y;
    while (h > 0)
    {
        w = st->line_advance;
        while (w > 0)
        {
            wrap = 0x100 - x;
            if (w >= wrap)
            {
                w -= wrap;
                x = 0;
                y += st->line_height;
            }
            else
            {
                x += w;
                w = 0;
            }
        }
        h -= 0x10;
    }
    st->dirty_end_u = x;
    st->region_end_u = x;
    st->dirty_end_v = y;
    st->region_end_v = y;
}

/**
 * @brief Apply the pending text configuration to a runtime window state.
 * @param state Window state to initialize.
 * @see decomp.me (100%)
 */

void field_text_apply_config(FieldTextState* state)
{
    FieldTextConfig* cfg = (FieldTextConfig*)0x801ED408;
    u32 flags;
    u32 state_flags;
    u32 config_flags;
    s32 width;

    state->portrait = cfg->portrait;
    state->x = cfg->x;
    state->y = cfg->y;
    state->transition_anchor_x = cfg->anchor.pos.x;
    state->reserved90 = 0;
    state->transition_anchor_y = cfg->anchor.pos.y;
    state->flags.b.byte3 = (u8)cfg->flags.word;
    flags = (state->flags.word & ~0xC0) | ((cfg->flags.word >> 2) & 0xC0);
    state->flags.word = flags;
    state_flags = flags & ~0x700;
    state_flags |= (cfg->flags.word >> 4) & 0x700;
    state->flags.word = state_flags;
    config_flags = cfg->flags.word;
    if ((config_flags & 0xC00) == 0xC00)
    {
        state->flags.word = state_flags & ~0x30;
    }
    else
    {
        state->flags.word = (state_flags & ~0x30) | ((config_flags >> 6) & 0x30);
    }
    config_flags = cfg->height;
    width = cfg->width;
    if ((state->portrait != 0) && ((state->flags.word & 0x30) != 0x20) && ((s32)config_flags < 0x30))
    {
        config_flags = 0x30;
    }
    state->remaining_width = width;
    state->width = width;
    state->height = config_flags;
    if ((state->flags.word & 0xC0) == 0x40)
    {
        state->line_advance = width + 4;
        state->line_height = 0xD;
        state->flags.word = (state->flags.word & ~7) | 2;
    }
    else
    {
        state->line_height = 0xC;
        state->line_advance = width;
        state->flags.word = (state->flags.word & ~7) | 1;
    }
    state->text_cursor = 0;
    state->macro_cursor = 0;
    state->glyph_cursor = 0;
    state->last_was_break = 1;
    if ((cfg->anchor.word == 0) && ((cfg->flags.word & 0x70FF) == 0))
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
    state->flow_code = 0;
    state->prompt_frame = 0;
    state->transition_frame = 0;
    state->choice_count = 0;
    state->flags.word &= ~0x800;
    state->flags.word &= ~0x1000;
    state->flags.word &= ~0x6000;
}

/**
 * @brief Update, upload, and render all field text windows for one frame.
 * @param packet_cursor Address of the render-packet cursor.
 * @param ot Ordering-table base address.
 * @param draw_count Current field draw count; 1 selects the render-only path.
 * @see decomp.me (100%)
 */

void func_80064C28(u8** packet_cursor, FieldOrderingTags* ot, s32 draw_count)
{
    FieldInputState* input = (FieldInputState*)0x801ED600;
    FieldTextState* st = (FieldTextState*)0x801ED034;
    FieldTextConfig* rec;
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
        switch ((u8)st->flags.word & 7)
        {
        case 1:
        case 2:
        case 3:
            if (st->needs_init == 1)
            {
                if (st->portrait != 0)
                {
                    field_text_queue_portrait_upload(st->portrait, packet_cursor, (st->flags.word >> 3) & 1, (st->flags.word & 0x30) != 0x10);
                }
                func_8006429C(st);
                field_text_queue_uploads(st, (u16**)packet_cursor);
                st->needs_init = 0;
            }
            else if (draw_count == 1)
            {
                field_text_render_window(st, packet_cursor, ot);
                break;
            }
            else
            {
                if (st->flow_code != 0)
                {
                    if (input->mode < 3)
                    {
                        if (st->flow_code == 0x10)
                        {
                            switch (input->mode)
                            {
                            case 1:
                            case 2:
                                if (input->repeat_active != 0)
                                {
                                    keys = input->repeat_dir;
                                }
                                else
                                {
                                    keys = input->repeat;
                                }
                                break;
                            case 0:
                                keys = input->repeat;
                                break;
                            default:
                                keys = 0;
                                break;
                            }
                            if ((keys & 0x10) != 0)
                            {
                                tmp = st->choice_index;
                                if (tmp == 0)
                                {
                                    tmp = st->choice_count;
                                }
                                st->choice_index = tmp - 1;
                                akao_play_sfx(0x7D, 0, 0x80, 0x7F);
                            }
                            if ((keys & 0x40) != 0)
                            {
                                if (st->choice_index < (st->choice_count - 1))
                                {
                                    st->choice_index = st->choice_index + 1;
                                }
                                else
                                {
                                    st->choice_index = 0;
                                }
                                akao_play_sfx(0x7D, 0, 0x80, 0x7F);
                            }
                            if ((input->pressed & 0x4002) != 0)
                            {
                                st->flow_code = 0;
                                st->choice_count = 0;
                                st->text_cursor = 0;
                                if ((st->flags.word & 0x1000) != 0)
                                {
                                    if (st->portrait != 0)
                                    {
                                        if ((st->flags.word & 8) == 0)
                                        {
                                            g_field_text_portrait_slots &= 0xFFFE;
                                        }
                                        else
                                        {
                                            g_field_text_portrait_slots &= 0xFFFD;
                                        }
                                    }
                                    if ((st->flags.word & 0xC0) == 0x40)
                                    {
                                        st->flags.word = st->flags.word & ~7;
                                    }
                                    else
                                    {
                                        st->flags.word = (st->flags.word & ~7) | 3;
                                        st->transition_frame = 0;
                                    }
                                }
                                akao_play_sfx(0x7E, 0, 0x80, 0x7F);
                            }
                        }
                        else if (((input->pressed & 0x4002) != 0) && (st->prompt_frame != 2))
                        {
                            st->prompt_frame = 2;
                            st->prompt_timer = 3;
                        }
                    }
                }
                else if ((st->flags.word & 7) == 2)
                {
                    if (st->char_delay != 0)
                    {
                        st->char_delay = st->char_delay - 1;
                    }
                    else if (st->scroll_timer != 0)
                    {
                        st->scroll_timer = st->scroll_timer - 4;
                        if (st->scroll_timer == 0)
                        {
                            field_text_scroll_cache(st);
                            field_text_queue_uploads(st, (u16**)packet_cursor);
                        }
                    }
                    else if (st->text_cursor != 0)
                    {
                        func_800632E0(st, 4);
                        field_text_queue_uploads(st, (u16**)packet_cursor);
                    }
                }
            }
            field_text_render_window(st, packet_cursor, ot);
            if (st->flow_code != 0)
            {
                if (st->flow_code == 0x10)
                {
                    st->prompt_timer = st->prompt_timer - 1;
                    if (st->prompt_timer == 0)
                    {
                        st->prompt_frame = st->prompt_frame + 1;
                        if (st->prompt_frame == 4)
                        {
                            st->prompt_frame = 0;
                        }
                        st->prompt_timer = 4;
                    }
                }
                else
                {
                    st->prompt_timer = st->prompt_timer - 1;
                    if (st->prompt_timer == 0)
                    {
                        if (st->prompt_frame == 2)
                        {
                            switch (st->flow_code)
                            {
                            case 1:
                                st->text_cursor = 0;
                                if ((st->flags.word & 0x1000) != 0)
                                {
                                    if (st->portrait != 0)
                                    {
                                        if ((st->flags.word & 8) == 0)
                                        {
                                            g_field_text_portrait_slots &= 0xFFFE;
                                        }
                                        else
                                        {
                                            g_field_text_portrait_slots &= 0xFFFD;
                                        }
                                    }
                                    if ((st->flags.word & 0xC0) == 0x40)
                                    {
                                        st->flags.word = st->flags.word & ~7;
                                    }
                                    else
                                    {
                                        st->flags.word = (st->flags.word & ~7) | 3;
                                        st->transition_frame = 0;
                                    }
                                }
                                st->flow_code = 0;
                                break;
                            case 2:
                                func_8006429C(st);
                                field_text_queue_uploads(st, (u16**)packet_cursor);
                                st->flow_code = 0;
                                {
                                    /* Matching: prevents GCC 2.8.0 cross-jumping this tail. */
                                    union { struct { } e; } crossjump = {};
                                    (void)crossjump;
                                }
                                break;
                            case 3:
                                func_80064210(st);
                                st->flow_code = 0;
                                break;
                            default:
                                st->flow_code = 0;
                                break;
                            }
                        }
                        else
                        {
                            st->prompt_frame = 1 - st->prompt_frame;
                            st->prompt_timer = 8;
                        }
                    }
                }
            }
            if (((st->flags.word & 7) == 0) && ((st->flags.word & 0x6000) != 0))
            {
                idx = 3 - i;
                mode = (st->flags.word >> 13) & 3;
                dst = (u8*)0x801ED408;
                n = 0x17;
                rec = &D_801ED004[idx];
                src = (u8*)rec;
                do
                {
                    *dst = *src;
                    src += 1;
                    n -= 1;
                    dst += 1;
                } while (n != -1);
                if (mode == 1)
                {
                    field_text_open_packed_window(idx);
                }
                else
                {
                    field_text_open_fixed_window(idx);
                }
                if (rec->text != 0)
                {
                    field_text_set_string(idx, rec->text, rec->flags.b.byte2);
                }
            }
            break;
        case 4:
            if (st->needs_init == 1)
            {
                st->transition_frame = 0x32;
                st->needs_init = 0;
            }
            if (st->text_cursor != 0)
            {
                func_8006429C(st);
                func_800632E0(st, 0);
                st->text_cursor = 0;
                st->flow_code = 0;
                st->dirty_start_u = st->region_start_u;
                st->dirty_start_v = st->region_start_v;
                st->dirty_end_u = st->region_end_u;
                st->dirty_end_v = st->region_end_v;
                field_text_queue_uploads(st, (u16**)packet_cursor);
            }
            field_text_build_window_packets(st, packet_cursor, ot);
            st->transition_frame = st->transition_frame - 1;
            if (st->transition_frame == 0)
            {
                if (st->portrait != 0)
                {
                    if ((st->flags.word & 8) == 0)
                    {
                        g_field_text_portrait_slots &= 0xFFFE;
                    }
                    else
                    {
                        g_field_text_portrait_slots &= 0xFFFD;
                    }
                }
                st->flags.word = st->flags.word & ~7;
            }
            break;
        case 5:
        case 6:
            break;
        default:
            break;
        }
        st = (FieldTextState*)((u8*)st + 0x98);
    } while (--i != -1);
}

/**
 * @brief Build the opening/closing transition quad for a text window.
 * @param state Text-window state.
 * @param out Output screen-space quad.
 * @param frame Transition frame in the range 0..4.
 * @see decomp.me (100%)
 */

void field_text_build_transition_quad(FieldTextState* state, Quad* out, s32 frame)
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
        half_w = state->width + 0x38;
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
 * @brief Build a flat field text-window packet chain and splice it into the OT.
 * @param state Text-window state.
 * @param cursor In/out render-packet cursor.
 * @param ot Ordering-table slot.
 * @note WIP - not yet byte-matching. Currently 97.57%.
 * @see decomp.me (97.57%)
 */

void field_text_build_window_packets(FieldTextState* st, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextSystem* hw = (FieldTextSystem*)0x801ED000;
    PrimSprt* prim;
    u8* first;
    u8* cur;
    s32 row;
    s32 y;
    s32 uv;
    s32 portrait_uv;
    s32 xy;
    s32 w;
    s32 rows;
    s32 size;
    s32 u_org;
    s32 uv_base;
    s32 col;
    s32 row_v;
    s32 skip;
    s32 avail;
    s32 over;
    s32 x;
    s32 glyph_x;
    s32 icon_v;
    u32 rgbc;
    s32 uv_bottom;
    u32 tag_mask;
    u32 tag_len;
    u32 wh8;
    u32 wh40;
    s32 clip;

    rgbc = 0x65808080;
    cur = *cursor;
    prim = (PrimSprt*)cur;
    first = cur;
    cur += 8;
    prim->tag = ((u32)cur & 0xFFFFFF) | 0x01000000;
    prim->rgbc = hw->draw_mode0;
    u_org = 0;
    uv_base = 0xE0;
    if ((st->flags.word & 0xC0) == 0)
    {
        y = st->y;
        do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
row = 1;
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
        clip = 0xF000;
        uv_bottom = 0xF800;
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
tag_mask = 0xFFFFFF;
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
        do {
tag_len = 0x04000000;
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
        do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
wh8 = 0x80008;
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
        wh40 = 0x80040;
        over = 0x80000;
        do
        {
            {
                s32 uv_hi;
                uv_hi = hw->window_clut << 16;
                if (row != 0)
                {
                    uv_hi |= clip;
                }
                else
                {
                    uv_hi |= uv_bottom;
                }
                uv = uv_hi | u_org;
            }
            prim = (PrimSprt*)cur;
            cur += 0x14;
            prim->uv = uv;
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            do {
            uv += 8;
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            } while (0);
            prim->tag = ((u32)cur & tag_mask) | tag_len;
            prim->rgbc = rgbc;
            prim->wh = wh8;
            xy = st->x | (y << 16);
            prim->xy = xy;
            xy += 8;
            if ((st->portrait != 0) && (((st->flags.word >> 4) & 3) < 2))
            {
                w = st->width + 0x38;
            }
            else
            {
                w = st->width;
            }
            if (w > 0)
            {
                do
                {
                    prim = (PrimSprt*)cur;
                    cur += 0x14;
                    prim->tag = ((u32)cur & tag_mask) | tag_len;
                    prim->rgbc = rgbc;
                    prim->xy = xy;
                    prim->uv = uv;
                    if (w >= 0x41)
                    {
                        prim->wh = wh40;
                        xy += 0x40;
                        w -= 0x40;
                    }
                    else
                    {
                        prim->wh = w | over;
                        xy += w;
                        w = 0;
                    }
                } while (w > 0);
            }
            uv += 0x40;
            prim = (PrimSprt*)cur;
            cur += 0x14;
            row -= 1;
            prim->tag = ((u32)cur & tag_mask) | tag_len;
            prim->rgbc = rgbc;
            prim->xy = xy;
            prim->uv = uv;
            prim->wh = wh8;
            y = y + 8 + st->height;
        } while (row != -1);
        rows = st->height;
        y = st->y + 8;
        if (rows > 0)
        {
            do
            {
                uv = (hw->window_clut << 16) | (uv_base << 8) | (u_org + 0xE0);
                xy = st->x | (y << 16);
                size = 0x200000;
                if (rows < 0x21)
                {
                    size = rows << 16;
                }
                prim = (PrimSprt*)cur;
                cur += 0x14;
                prim->xy = xy;
                xy += 8;
                prim->tag = ((u32)cur & 0xFFFFFF) | 0x04000000;
                prim->rgbc = rgbc;
                prim->uv = uv;
                prim->wh = size | 8;
                uv -= 0x40;
                if ((st->portrait != 0) && (((st->flags.word >> 4) & 3) < 2))
                {
                    w = st->width + 0x38;
                }
                else
                {
                    w = st->width;
                }
                if (w > 0)
                {
                    do
                    {
                        prim = (PrimSprt*)cur;
                        cur += 0x14;
                        prim->tag = ((u32)cur & 0xFFFFFF) | 0x04000000;
                        prim->rgbc = rgbc;
                        prim->xy = xy;
                        prim->uv = uv;
                        if (w >= 0x41)
                        {
                            prim->wh = size | 0x40;
                            xy += 0x40;
                            w -= 0x40;
                        }
                        else
                        {
                            prim->wh = w | size;
                            xy += w;
                            w = 0;
                        }
                    } while (w > 0);
                }
                uv += 0x48;
                prim = (PrimSprt*)cur;
                cur += 0x14;
                y += 0x20;
                rows -= 0x20;
                prim->tag = ((u32)cur & 0xFFFFFF) | 0x04000000;
                prim->rgbc = rgbc;
                prim->xy = xy;
                prim->uv = uv;
                prim->wh = size | 8;
            } while (rows > 0);
        }
        do {
        do {
        do {
        do {
        u_org = 0;
        } while (0);
        } while (0);
        } while (0);
        } while (0);
    }
    uv_base = 0x80;
    size = st->line_height;
    col = st->region_start_u;
    row_v = st->region_start_v;
    skip = st->scroll_timer;
    y = st->y + 8;
    rows = st->height >> 4;
    rows -= 1;
    if (rows != -1)
    {
        s32 neg16;
        s32 one16;
do {
do {
one16 = 0x10;
} while (0);
} while (0);
        do
        {
            if ((st->portrait != 0) && ((st->flags.word & 0x30) == 0))
            {
                xy = ((st->x + 0x40) & 0xFFFF) | (y << 16);
            }
            else
            {
                xy = ((st->x + 8) & 0xFFFF) | (y << 16);
            }
            w = st->line_advance;
            if (w > 0)
            {
                over = (u32)(one16 - skip) < (u32)size;
                neg16 = 0xFFF0;
                clip = size + (skip + neg16);
                do
                {
                    if ((skip != 0) && (over == 0))
                    {
                        avail = 0x100 - col;
                        col += w;
                        if ((u32)w < (u32)avail)
                        {
                            w = 0;
                        }
                        else
                        {
                            xy += avail;
                            w -= avail;
                            row_v += size;
                            col = 0;
                        }
                    }
                    else
                    {
                        prim = (PrimSprt*)cur;
                        cur += 0x14;
                        ((PrimGlyph*)prim)->tag = (u32)cur;
                        ((PrimGlyph*)prim)->tag |= 0x04000000;
                        ((PrimGlyph*)prim)->tag &= 0x04FFFFFF;
                        do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
do {
((PrimGlyph*)prim)->rgbc = rgbc;
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
                        ((PrimGlyph*)prim)->xy = xy;
                        ((PrimGlyph*)prim)->u0 = u_org + col;
                        ((PrimGlyph*)prim)->clut = hw->text_clut;
                        if (skip != 0)
                        {
                            ((PrimGlyph*)prim)->v0 = (uv_base + row_v + 0x10) - skip;
                            do {
do {
do {
do {
do {
do {
((PrimGlyph*)prim)->h = clip;
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
} while (0);
                        }
                        else
                        {
                            ((PrimGlyph*)prim)->v0 = uv_base + row_v;
                            ((PrimGlyph*)prim)->h = size;
                        }
                        avail = 0x100 - col;
                        col += w;
                        if ((u32)w >= (u32)avail)
                        {
                            ((PrimGlyph*)prim)->w = avail;
                            xy += avail;
                            w -= avail;
                            row_v += size;
                            col = 0;
                        }
                        else
                        {
                            ((PrimGlyph*)prim)->w = w;
                            w = 0;
                        }
                    }
                } while (w > 0);
            }
            if (skip != 0)
            {
                y += skip;
                skip = 0;
            }
            else
            {
                y += 0x10;
            }
            do {
do {
do {
do {
rows -= 1;
} while (0);
} while (0);
} while (0);
} while (0);

        } while (rows != -1);
    }
    if (st->portrait != 0)
    {
        prim = (PrimSprt*)cur;
        cur += 8;
        prim->tag = ((u32)cur & 0xFFFFFF) | 0x01000000;
        prim->rgbc = hw->draw_mode1;
        if ((st->flags.word & 0x30) == 0)
        {
            xy = (st->x + 8) & 0xFFFF;
        }
        else
        {
            s32 px;
            s32 pw;
            px = st->x;
            pw = st->width;
            pw += 0x10;
            px += pw;
            xy = px & 0xFFFF;
        }
        {
            s32 py;
            s32 ph;
            u32 pflags;
            prim = (PrimSprt*)cur;
            cur += 0x14;
            py = st->y;
            ph = st->height;
            pflags = st->flags.word;
            prim->tag = ((u32)cur & 0xFFFFFF) | 0x04000000;
            prim->rgbc = 0x66000000;
            ph -= 0x30;
            ph >>= 1;
            ph += 8;
            py += ph;
            py <<= 16;
            xy |= py;
            prim->xy = xy + 0x20002;
            portrait_uv = ((((((pflags >> 3) & 1) * 0x30) + 0x110) & 0xFF) << 8) | 0xD0;
        prim->uv = (hw->text_clut << 16) | portrait_uv;
        prim->wh = 0x300030;
        prim = (PrimSprt*)cur;
        cur += 0x14;
        prim->tag = ((u32)cur & 0xFFFFFF) | 0x04000000;
        prim->rgbc = rgbc;
        prim->xy = xy;
        prim->uv = ((&hw->portrait_clut0)[(st->flags.word >> 3) & 1] << 16) | portrait_uv;
        prim->wh = 0x300030;
        }
    }
    if (st->flow_code != 0)
    {
        if (((st->flags.word & 0xC0) == 0) ||
            (((st->flags.word & 0xC0) == 0x40) && (st->flow_code == 0x10)))
        {
            prim = (PrimSprt*)cur;
            cur += 8;
            prim->tag = ((u32)cur & 0xFFFFFF) | 0x01000000;
            prim->rgbc = hw->draw_mode0;
            if (st->flow_code == 0x10)
            {
                prim = (PrimSprt*)cur;
                cur += 0x10;
                ((PrimSprt16*)prim)->tag = ((u32)cur & 0xFFFFFF) | 0x03000000;
                ((PrimSprt16*)prim)->rgbc = 0x7D808080;
                if ((st->portrait != 0) && ((st->flags.word & 0x30) == 0))
                {
                    ((PrimSprt16*)prim)->x0 = st->x + 0x38;
                }
                else
                {
                    ((PrimSprt16*)prim)->x0 = st->x + 0xE;
                }
                ((PrimSprt16*)prim)->y0 = st->y + ((st->choice_start_line + st->choice_index) * 0x10);
                portrait_uv = st->prompt_frame;
                if (portrait_uv == 3)
                {
                    portrait_uv = 1;
                }
                ((PrimSprt16*)prim)->u0 = (portrait_uv << 4) + 0x60;
                ((PrimSprt16*)prim)->v0 = 0xE0;
                ((PrimSprt16*)prim)->clut = hw->prompt_clut;
            }
            else
            {
                prim = (PrimSprt*)cur;
                cur += 0x14;
                ((PrimIcon*)prim)->tag = ((u32)cur & 0xFFFFFF) | 0x04000000;
                ((PrimIcon*)prim)->rgbc = rgbc;
                if ((st->portrait != 0) && (((st->flags.word >> 4) & 3) < 2))
                {
                    ((PrimIcon*)prim)->x0 = st->x + ((st->width + 0x38) >> 1);
                }
                else
                {
                    ((PrimIcon*)prim)->x0 = st->x + (st->width >> 1);
                }
                ((PrimIcon*)prim)->y0 = st->y + st->height + 6;
                ((PrimIcon*)prim)->clut = hw->prompt_clut;
                ((PrimIcon*)prim)->u0 = 0xF0;
                {
                    s32 frame;
                    frame = st->prompt_frame;
                    ((PrimIcon*)prim)->wh = 0x80010;
                    ((PrimIcon*)prim)->v0 = (frame * 8) - 0x20;
                }
            }
        }
    }
    prim->tag = (prim->tag & 0xFF000000) | (ot->tag1 & 0xFFFFFF);
    ot->tag1 = (ot->tag1 & 0xFF000000) | ((u32)first & 0xFFFFFF);
    *cursor = cur;
}

/**
 * @brief Build a warped text-window packet chain for an opening/closing quad.
 * @param state Text-window state.
 * @param quad Transition quad.
 * @param cursor In/out render-packet cursor.
 * @param ot Ordering-table slot.
 * @note WIP - not yet byte-matching. Currently 91.45%.
 * @see decomp.me (91.45%)
 */

void field_text_build_transition_packets(FieldTextState* st, Quad* quad, u8** cursor, FieldOrderingTags* ot)
{
    FieldTextSystem* hw = (FieldTextSystem*)0x801ED000;
    Vec2s* p;
    Vec2s* build;
    Vec2s* mesh;
    u32* vp;
    u32* hvp2;
    u32* hvp;
    u32* vvp;
    u32* gvp;
    u32* pvp;
    PrimQuad* poly;
    u8* first;
    s32 w;
    s32 span;
    s32 v;
    s32 y;
    s32 text_y;
    s32 n;
    s32 inner;
    s32 chunk;
    s32 edge;
    s32 prows;
    s32 mesh_rows;
    s32 count;
    s32 u;
    s32 avail;
    s32 stride;
    s32 den_x;
    s32 den_y;
    s32 base_x;
    s32 base_y;
    s32 dx;
    s32 dy;
    s32 prev;
    s32 yv;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 uv;
    s32 vvval;
    s32 clut;
    s32 col;
    s32 row_v;
    s32 sel;
    s32 u_org;
    u32 rgbc;
    s32 hspan;
    s32 vspan;
    s32 hy;
    s32 hn;
    s32 vy;
    s32 gspan;
    s32 grows;
    s32 vbase;
    s32 tpage;

    base_x = 0;
    base_y = 0;
    dx = 0;
    dy = 0;
    if ((st->portrait != 0) && (((st->flags.word >> 4) & 3) < 2))
    {
        w = st->width + 0x38;
    }
    else
    {
        w = st->width;
    }

    /* Build the unwarped mesh in scratchpad RAM. */
    build = (Vec2s*)0x1F800000;
    y = 0;
    build->x = 0;
    build->y = y;
    build += 1;
    v = 8;
    span = w;
    if (w > 0)
    {
        do
        {
            build->x = v;
            build->y = y;
            build += 1;
            if (span >= 0x41)
            {
                v += 0x40;
                span -= 0x40;
            }
            else
            {
                v += span;
                span = 0;
            }
        } while (span > 0);
    }
    build->x = v;
    build->y = y;
    build[1].x = v + 8;
    build[1].y = y;
    build += 2;

    mesh_rows = st->height;
    mesh_rows -= 1;
    y += 8;
    if (mesh_rows != -1)
    {
        do
        {
            build->x = 0;
            build->y = y;
            build += 1;
            v = 8;
            span = w;
            if (w > 0)
            {
                do
                {
                    build->x = v;
                    build->y = y;
                    build += 1;
                    if (span >= 0x41)
                    {
                        v += 0x40;
                        span -= 0x40;
                    }
                    else
                    {
                        v += span;
                        span = 0;
                    }
                } while (span > 0);
            }
            build->x = v;
            build->y = y;
            build[1].x = v + 8;
            build[1].y = y;
            build += 2;
            if (mesh_rows >= 0x21)
            {
                y += 0x20;
                mesh_rows -= 0x20;
            }
            else
            {
                y += mesh_rows;
                mesh_rows = 0;
            }
            mesh_rows -= 1;
        } while (mesh_rows != -1);
    }

    n = 1;
    do
    {
        build->x = 0;
        build->y = y;
        build += 1;
        v = 8;
        span = w;
        if (w > 0)
        {
            do
            {
                build->x = v;
                build->y = y;
                build += 1;
                if (span >= 0x41)
                {
                    v += 0x40;
                    span -= 0x40;
                }
                else
                {
                    v += span;
                    span = 0;
                }
            } while (span > 0);
        }
        build->x = v;
        build->y = y;
        build[1].x = v + 8;
        build[1].y = y;
        build += 2;
        n -= 1;
        y += 8;
    } while (n != -1);

    text_y = 8;
    u = st->region_start_u;
    grows = st->height >> 4;
    grows -= 1;
    count = 0;
    if (grows != -1)
    {
        do
        {
            v = 8;
            if ((st->portrait != 0) && ((st->flags.word & 0x30) == 0))
            {
                v = 0x40;
            }
            span = st->line_advance;
            if (span > 0)
            {
                do
                {
                    build->x = v;
                    build->y = text_y;
                    build[1].x = v;
                    avail = 0x100 - u;
                    build[1].y = st->line_height + text_y;
                    build += 2;
                    count += 2;
                    if (span >= avail)
                    {
                        v += avail;
                        span -= avail;
                        u = 0;
                    }
                    else
                    {
                        v += span;
                        u += span;
                        span = 0;
                    }
                } while (span > 0);
            }
            build->x = v;
            build->y = text_y;
            build[1].x = v;
            grows -= 1;
            build[1].y = st->line_height + text_y;
            build += 2;
            count += 2;
            text_y += 0x10;
        } while (grows != -1);
    }

    n = 1;
    y = ((s32)(st->height - 0x30) >> 1) + 0xA;
    do
    {
        if ((st->flags.word & 0x30) == 0)
        {
            v = 0xA;
        }
        else
        {
            v = st->width + 0x12;
        }
        inner = 1;
        do
        {
            build->x = v;
            build->y = y;
            build += 1;
            inner -= 1;
            v += 0x30;
        } while (inner != -1);
        n -= 1;
        y += 0x30;
    } while (n != -1);

    n = 1;
    y = ((s32)(st->height - 0x30) >> 1) + 8;
    do
    {
        if ((st->flags.word & 0x30) == 0)
        {
            v = 8;
        }
        else
        {
            v = st->width + 0x10;
        }
        inner = 1;
        do
        {
            build->x = v;
            build->y = y;
            build += 1;
            inner -= 1;
            v += 0x30;
        } while (inner != -1);
        n -= 1;
        y += 0x30;
    } while (n != -1);

    /* Warp every mesh vertex into the quad. */
    mesh = (Vec2s*)0x1F800000;
    prev = -1;
    den_x = w + 0x10;
    grows = ((((st->height + 0x1F) >> 5) + 3) * (((w + 0x3F) >> 6) + 3)) + count + 7;
    den_y = st->height + 0x10;
    if (grows != -1)
    {
        p = mesh;
        do
        {
            yv = p->y;
            if (prev != yv)
            {
                do
                {
                    do
                    {
                        prev = yv;
                        base_x = (((quad->x2 - quad->x0) * yv) / den_y) + quad->x0;
                        base_y = (((quad->y2 - quad->y0) * yv) / den_y) + quad->y0;
                        dx = ((((quad->x3 - quad->x1) * yv) / den_y) + quad->x1) - base_x;
                        dy = ((((quad->y3 - quad->y1) * yv) / den_y) + quad->y1) - base_y;
                    } while (0);
                } while (0);
            }
            p->y = ((dy * p->x) / den_x) + base_y;
            grows -= 1;
            p->x = ((dx * p->x) / den_x) + base_x;
            p += 1;
        } while (grows != -1);
    }

    /* Emit the packet chain. */
    tpage = 0x1F0000;
    u_org = 0;
    vbase = 0xE0;
    rgbc = 0x2D808080;
    vp = (u32*)0x1F800000;
    hy = 8;
    hn = 1;
    stride = ((w + 0x3F) >> 6) + 3;
    first = *cursor;
    base_x = (s32)first;
    dy = 0x09000000;
    clut = hw->window_clut << 16;
    do
    {
        if (hn == 0)
        {
            uv = u_org | 0xF800;
        }
        else
        {
            uv = u_org | 0xF000;
        }
        poly = (PrimQuad*)base_x;
        base_x += 0x28;
        poly->tag = ((u32)base_x & 0xFFFFFF) | dy;
        poly->uv0 = clut | uv;
        poly->uv1 = tpage | (uv + 8);
        poly->uv2 = uv + (hy << 8);
        poly->rgbc = rgbc;
        poly->uv3 = uv + ((hy << 8) | 8);
        poly->xy0 = vp[0];
        hspan = w;
        poly->xy1 = vp[1];
        uv += 8;
        hvp2 = vp + ((w + 0x3F) >> 6) + 4;
        poly->xy2 = hvp2[-1];
        poly->xy3 = hvp2[0];
        if (w > 0)
        {
            do
            {
                poly = (PrimQuad*)base_x;
                base_x += 0x28;
                poly->tag = ((u32)base_x & 0xFFFFFF) | dy;
                poly->rgbc = rgbc;
                if (hspan >= 0x41)
                {
                    chunk = 0x3F;
                    hspan -= 0x40;
                }
                else
                {
                    chunk = hspan - 1;
                    hspan = 0;
                }
                poly->uv1 = tpage | (uv + chunk);
                poly->uv0 = clut | uv;
                poly->uv2 = uv + (hy << 8);
                poly->uv3 = uv + ((hy << 8) | chunk);
                poly->xy0 = vp[0];
                poly->xy1 = vp[1];
                vp += 1;
                poly->xy2 = hvp2[0];
                poly->xy3 = hvp2[1];
                hvp2 += 1;
            } while (hspan > 0);
        }
        uv += 0x40;
        poly = (PrimQuad*)base_x;
        base_x += 0x28;
        poly->tag = ((u32)base_x & 0xFFFFFF) | dy;
        poly->uv0 = clut | uv;
        poly->uv1 = tpage | (uv + 7);
        poly->rgbc = rgbc;
        poly->uv2 = uv + (hy << 8);
        poly->uv3 = uv + ((hy << 8) | 7);
        poly->xy0 = vp[0];
        poly->xy1 = vp[1];
        poly->xy2 = hvp2[0];
        poly->xy3 = hvp2[1];
        hn -= 1;
        vp = (u32*)0x1F800000 + ((((st->height + 0x1F) >> 5) + 1) * stride);
        hy = 7;
    } while (hn != -1);

    {
        u32* vp2;

        vp = (u32*)0x1F80000C + ((w + 0x3F) >> 6);
        vp2 = vp + stride;
        grows = st->height;
        if (grows > 0)
        {
            do
            {
                uv = (vbase << 8) | (u_org + 0xE0);
                vy = 0x1F00;
                if (grows < 0x20)
                {
                    vy = grows << 8;
                }
                poly = (PrimQuad*)base_x;
                base_x += 0x28;
                poly->tag = ((u32)base_x & 0xFFFFFF) | 0x09000000;
                poly->uv0 = clut | uv;
                poly->uv1 = tpage | (uv + 8);
                poly->uv2 = uv + vy;
                poly->rgbc = rgbc;
                poly->uv3 = uv + (vy | 8);
                vvval = uv - 0x40;
                poly->xy0 = vp[0];
                vspan = w;
                poly->xy1 = vp[1];
                poly->xy2 = vp2[0];
                poly->xy3 = vp2[1];
                vp += 1;
                vp2 += 1;
                if (w > 0)
                {
                    do
                    {
                        poly = (PrimQuad*)base_x;
                        base_x += 0x28;
                        poly->tag = ((u32)base_x & 0xFFFFFF) | 0x09000000;
                        poly->rgbc = rgbc;
                        if (vspan >= 0x41)
                        {
                            chunk = 0x40;
                            vspan -= 0x40;
                        }
                        else
                        {
                            chunk = vspan;
                            vspan = 0;
                        }
                        poly->uv1 = tpage | (vvval + chunk);
                        poly->uv0 = clut | vvval;
                        poly->uv2 = vvval + vy;
                        poly->uv3 = vvval + (vy | chunk);
                        poly->xy0 = vp[0];
                        poly->xy1 = vp[1];
                        poly->xy2 = vp2[0];
                        poly->xy3 = vp2[1];
                        vp += 1;
                        vp2 += 1;
                    } while (vspan > 0);
                }
                vvval += 0x48;
                poly = (PrimQuad*)base_x;
                base_x += 0x28;
                poly->tag = ((u32)base_x & 0xFFFFFF) | 0x09000000;
                poly->uv0 = clut | vvval;
                poly->uv1 = tpage | (vvval + 8);
                poly->uv2 = vvval + vy;
                poly->rgbc = rgbc;
                poly->uv3 = vvval + (vy | 8);
                poly->xy0 = vp[0];
                grows -= 0x20;
                poly->xy1 = vp[1];
                poly->xy2 = vp2[0];
                poly->xy3 = vp2[1];
                vp += 2;
                vp2 += 2;
            } while (grows > 0);
        }
    }

    u_org = 0;
    vbase = 0x80;
    col = st->region_start_u;
    row_v = st->region_start_v;
    grows = st->height >> 4;
    grows -= 1;
    vp = (u32*)0x1F800008 + ((((st->height + 0x1F) >> 5) + 3) * (((w + 0x3F) >> 6) + 3));
    if (grows != -1)
    {
        do
        {
            gspan = st->line_advance;
            v = row_v + vbase;
            if (gspan > 0)
            {
                do
                {
                    uv = (v << 8) | (u_org + col);
                    poly = (PrimQuad*)base_x;
                    base_x += 0x28;
                    poly->tag = ((u32)base_x & 0xFFFFFF) | 0x09000000;
                    poly->rgbc = rgbc;
                    poly->uv1 = tpage | uv;
                    poly->uv0 = (hw->text_clut << 16) | uv;
                    poly->uv2 = uv + (st->line_height << 8);
                    poly->uv3 = uv + (st->line_height << 8);
                    poly->xy0 = vp[0];
                    avail = 0x100 - col;
                    poly->xy1 = vp[1];
                    poly->xy2 = vp[2];
                    poly->xy3 = vp[3];
                    vp += 2;
                    if (gspan >= avail)
                    {
                        gspan -= avail;
                        edge = (col + avail) - 1;
                        poly->uv3 = edge;
                        poly->uv1 = edge;
                        col = 0;
                        row_v += st->line_height;
                    }
                    else
                    {
                        col += gspan;
                        edge = col;
                        gspan = 0;
                        poly->uv3 = edge;
                        poly->uv1 = edge;
                    }
                    v = row_v + vbase;
                } while (gspan > 0);
            }
            grows -= 1;
        } while (grows != -1);
    }

    if (st->portrait != 0)
    {
        poly = (PrimQuad*)base_x;
        base_x += 0x28;
        prows = st->height;
        prows += 0x1F;
        prows >>= 5;
        prows += 3;
        sel = st->flags.word;
        clut = hw->text_clut;
        poly->tag = ((u32)base_x & 0xFFFFFF) | 0x09000000;
        poly->rgbc = 0x2E000000;
        sel >>= 3;
        sel &= 1;
        uv = (((((sel * 0x30) + 0x110) & 0xFF) << 8) | 0xD0);
        poly->uv0 = (clut << 16) | uv;
        poly->uv2 = uv + 0x3000;
        poly->uv1 = (uv + 0x2F) | tpage;
        pvp = (u32*)0x1F800000 + count + (prows * (((w + 0x3F) >> 6) + 3));
        poly->uv3 = uv + 0x302F;
        poly->xy0 = pvp[0];
        poly->xy1 = pvp[1];
        poly->xy2 = pvp[2];
        poly->xy3 = pvp[3];
        pvp += 4;
        poly = (PrimQuad*)base_x;
        base_x += 0x28;
        sel = (st->flags.word >> 3) & 1;
        clut = (&hw->portrait_clut0)[sel];
        uv = ((((sel * 0x30) + 0x110) & 0xFF) << 8) | 0xD0;
        poly->uv2 = uv + 0x3000;
        poly->uv1 = (uv + 0x2F) | tpage;
        poly->tag = ((u32)base_x & 0xFFFFFF) | 0x09000000;
        poly->rgbc = rgbc;
        poly->uv3 = uv + 0x302F;
        poly->uv0 = (clut << 16) | uv;
        poly->xy0 = pvp[0];
        poly->xy1 = pvp[1];
        poly->xy2 = pvp[2];
        poly->xy3 = pvp[3];
    }
    poly->tag = (poly->tag & 0xFF000000) | (ot->tag1 & 0xFFFFFF);
    ot->tag1 = (ot->tag1 & 0xFF000000) | ((u32)first & 0xFFFFFF);
    *cursor = (u8*)base_x;
}

/**
 * @brief Scroll the text cache up one row and clear the vacated row.
 * @param state Text-window state.
 * @see decomp.me (100%)
 */

void field_text_scroll_cache(FieldTextState* st)
{
    u16* dst;
    u16* src;
    u16* d;
    u16* s;
    s32 u;
    s32 v;
    s32 du;
    s32 dv;
    s32 su;
    s32 sv;
    s32 left;
    s32 rows;
    s32 avail;
    s32 span;
    s32 cap;
    s32 lines;
    s32 count;
    u16 pix;

    u = st->region_start_u;
    v = st->region_start_v;
    rows = st->height - 0x10;
    if (rows > 0)
    {
        do
        {
            du = u;
            left = st->line_advance;
            dv = v;
            if (left > 0)
            {
                span = 0x100 - u;
                do
                {
                    u += left;
                    if (left >= span)
                    {
                        left -= span;
                        u = 0;
                        v += st->line_height;
                    }
                    else
                    {
                        left = 0;
                    }
                    span = 0x100 - u;
                } while (left > 0);
            }
            su = u;
            left = st->line_advance;
            sv = v;
            if (left > 0)
            {
                do
                {
                    dst = ((u16*)0x801DE000 + (du >> 2)) + (dv << 6);
                    src = ((u16*)0x801DE000 + (su >> 2)) + (sv << 6);
                    span = 0x100 - su;
                    cap = 0x100 - du;
                    if (cap < span)
                    {
                        span = cap;
                    }
                    if (left < span)
                    {
                        span = left;
                    }
                    du += span;
                    left -= span;
                    if (du >= 0x100)
                    {
                        do
                        {
                            du -= 0x100;
                            dv += st->line_height;
                        } while (du >= 0x100);
                    }
                    su += span;
                    if (su >= 0x100)
                    {
                        do
                        {
                            su -= 0x100;
                            sv += st->line_height;
                        } while (su >= 0x100);
                    }
                    lines = st->line_height;
                    lines -= 1;
                    if (lines != -1)
                    {
                        do
                        {
                            d = dst;
                            cap = span >> 2;
                            cap -= 1;
                            s = src;
                            if (cap != -1)
                            {
                                s32 end = -1;
                                do
                                {
                                    pix = *s;
                                    s += 1;
                                    cap -= 1;
                                    *d = pix;
                                    d += 1;
                                } while (cap != end);
                            }
                            dst += 0x40;
                            lines -= 1;
                            src += 0x40;
                        } while (lines != -1);
                    }
                } while (left > 0);
            }
            rows -= 0x10;
        } while (rows > 0);
    }
    left = st->line_advance;
    if (left > 0)
    {
        do
        {
            dst = ((u16*)0x801DE000 + (u >> 2)) + (v << 6);
            span = 0x100 - u;
            if (left < span)
            {
                span = left;
            }
            u += span;
            left -= span;
            if (u >= 0x100)
            {
                do
                {
                    u -= 0x100;
                    v += st->line_height;
                } while (u >= 0x100);
            }
            lines = st->line_height;
            lines -= 1;
            if (lines != -1)
            {
                do
                {
                    cap = span >> 2;
                    cap -= 1;
                    d = dst;
                    if (cap != -1)
                    {
                        s32 end = -1;
                        do
                        {
                            *d = 0;
                            cap -= 1;
                            d += 1;
                        } while (cap != end);
                    }
                    lines -= 1;
                    dst += 0x40;
                } while (lines != -1);
            }
        } while (left > 0);
    }
    st->dirty_start_u = st->region_start_u;
    st->dirty_start_v = st->region_start_v;
    st->dirty_end_u = st->region_end_u;
    st->dirty_end_v = st->region_end_v;
    st->remaining_width = st->width;
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
    s32 rows;
    s32 count;
    s32 span;
    s32 w;
    s32 x;
    s32 y;
    s32 h;
    s32 addr;
    s32 xbytes;
    s32 yaddr;

    cur = *cursor;
    req = (FieldImageReq*)cur;
    y = state->dirty_start_v;
    x = state->dirty_start_u;
    cur += 8;
    if (y == state->dirty_end_v)
    {
        span = (state->dirty_end_u - x) >> 1;
    }
    else
    {
        span = (0x100 - x) >> 1;
    }
    dst = (u16*)0x801DE000;
    addr = x >> 2;
    xbytes = addr << 1;
    yaddr = (y << 7) + 0x801DE000;
    src = (u16*)(xbytes + yaddr);
    h = state->line_height;
    req->rect.x = addr + 0x3C0;
    w = span >> 1;
    req->rect.y = y + 0x180;
    req->rect.w = w;
    req->rect.h = h;
    if (span == 0x80)
    {
        req->data = (u_long*)src;
    }
    else
    {
        dst = cur;
        cur += ((w * h) + 1) & ~1;
        req->data = (u_long*)dst;
        rows = h;
        rows -= 1;
        if (h != 0)
        {
            do
            {
                count = span >> 1;
                do { do { s = src; } while (0); } while (0);
                while (--count != -1)
                {
                    *dst = *s;
                    s += 1;
                    dst += 1;
                }
                rows -= 1;
                src += 0x40;
            } while (rows != -1);
        }
    }
    field_queue_vram_upload(req);
    if (y != state->dirty_end_v)
    {
        y += h;
        req = (FieldImageReq*)cur;
        if (y != state->dirty_end_v)
        {
            cur += 8;
            src = (u16*)((y << 7) + 0x801DE000);
            req->rect.x = 0x3C0;
            req->rect.y = y + 0x180;
            req->rect.w = 0x40;
            req->rect.h = state->dirty_end_v - y;
            req->data = (u_long*)src;
            field_queue_vram_upload(req);
        }
        req = (FieldImageReq*)cur;
        if (state->dirty_end_u != 0)
        {
            cur += 8;
            req->rect.x = 0x3C0;
            req->rect.y = state->dirty_end_v + 0x180;
            req->rect.w = state->dirty_end_u >> 2;
            req->rect.h = h;
            src = (u16*)((state->dirty_end_v << 7) + 0x801DE000);
            dst = cur;
            cur += ((((state->dirty_end_u >> 2) * h) + 1) & ~1);
            rows = h;
            rows -= 1;
            req->data = (u_long*)dst;
            if (h != 0)
            {
                do
                {
                    count = state->dirty_end_u >> 2;
                    do { s = src; } while (0);
                    while (--count != -1)
                    {
                        *dst = *s;
                        s += 1;
                        dst += 1;
                    }
                    rows -= 1;
                    src += 0x40;
                } while (rows != -1);
            }
            field_queue_vram_upload(req);
        }
    }
    *cursor = cur;
}


/**
 * @brief Attach a text string to a window, or defer it while the slot reopens.
 * @param slot Window slot index.
 * @param text Text pointer.
 * @param options Text options; bit 0 enables automatic close.
 * @see decomp.me (100%)
 */

void field_text_set_string(u16 slot, u8* text, u8 options)
{
    FieldTextSystem* hw = (FieldTextSystem*)0x801ED000;
    FieldTextState* st = &hw->windows[slot];
    FieldTextConfig* rec;

    if ((st->flags.word & 0x6000) != 0)
    {
        rec = &hw->configs[slot];
        rec->text = text;
        rec->flags.b.byte2 = options;
        return;
    }
    st->last_was_break = 1;
    st->text_cursor = (u8*)text;
    st->macro_cursor = 0;
    st->glyph_cursor = 0;
    st->pending_spaces = 0;
    st->flow_code = 0;
    st->flags.word = (st->flags.word & ~0x1000) | ((options & 1) << 12);
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
    s32 n;
    FieldTextConfig* rec;

    src = (u8*)0x801ED408;
    n = 0x17;
    rec = &D_801ED004[slot];
    rec->text = 0;
    dst = (u8*)rec;
    do
    {
        *dst = *src;
        src += 1;
        n -= 1;
        dst += 1;
    } while (n != -1);
}

/**
 * @brief Close a text window and release its portrait slot.
 * @param state Text-window state.
 * @param animate Non-zero starts the closing animation when supported.
 * @see decomp.me (100%)
 */

void func_8006700C(FieldTextState* state, s32 animate)
{
    if (state->portrait != 0)
    {
        if ((state->flags.word & 8) == 0)
        {
            ((FieldTextSystem*)0x801ED000)->portrait_slots &= 0xFFFE;
        }
        else
        {
            ((FieldTextSystem*)0x801ED000)->portrait_slots &= 0xFFFD;
        }
    }
    if ((animate == 0) || ((state->flags.word & 0xC0) == 0x40))
    {
        state->flags.word = state->flags.word & ~7;
    }
    else
    {
        state->flags.word = (state->flags.word & ~7) | 3;
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
    Quad quad;

    switch (state->flags.b.low & 7)
    {
    case 1:
        field_text_build_transition_quad(state, &quad, state->transition_frame);
        field_text_build_transition_packets(state, &quad, cursor, ot);
        state->transition_frame = state->transition_frame + 1;
        if (state->transition_frame == 4)
        {
            state->flags.word = (state->flags.word & ~7) | 2;
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
            state->flags.word = state->flags.word & ~7;
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

void field_text_queue_portrait_upload(u8* image, u8** cursor, s32 slot, s32 mirror)
{
    FieldImageReq* req;
    u8* cur;
    u8* src;
    u8* dst;
    u8* d;
    s32 rows;
    s32 n;
    u32 b;

    cur = *cursor;
    req = (FieldImageReq*)cur;
    cur += 0x20;
    req->rect.x = 0x130;
    req->rect.y = slot + 0x1FA;
    req->rect.w = 0x10;
    req->rect.h = 1;
    req->data = (u_long*)image;
    field_queue_vram_upload(req);
    req += 1;
    req->rect.x = 0x3F4;
    req->rect.y = (slot * 48) + 0x110;
    req->rect.w = 0xC;
    req->rect.h = 0x30;
    if (mirror != 0)
    {
        src = image + 0x20;
        dst = cur;
        cur += 0x480;
        req->data = (u_long*)dst;
        dst += 0x17;
        rows = 0x2F;
        do
        {
            d = dst;
            n = 0x17;
            do
            {
                b = *src;
                src += 1;
                n -= 1;
                *d = ((b & 0xF) << 4) | (b >> 4);
                d -= 1;
            } while (n != -1);
            rows -= 1;
            dst += 0x18;
        } while (rows != -1);
    }
    else
    {
        req->data = (u_long*)(image + 0x20);
    }
    field_queue_vram_upload(req);
    *cursor = cur;
}

/**
 * @brief Start timed text-window mode and precompute its cache extent.
 * @param text Text to display.
 * @see decomp.me (100%)
 */

void field_text_start_timed_window(u8* text)
{
    FieldTextState* st = (FieldTextState*)0x801ED034;
    s32 u;
    s32 v;
    s32 rows;
    s32 span;
    u16 avail;

    field_text_apply_config((FieldTextState*)0x801ED034);
    u = 0;
    rows = st->height;
    v = u;
    /* Alias-qualified stores preserve the original GCC memory scheduling. */
    *(u8**)&st->text_cursor = text;
    g_field_timed_text = text;
    st->dirty_start_u = 0;
    st->cursor_u = 0;
    st->region_start_u = 0;
    st->dirty_start_v = 0;
    st->cursor_v = 0;
    st->region_start_v = 0;
    st->flags.word = ((*(u32*)&st->flags.word & ~7) | 0x804) & ~0x1000;
    if (rows > 0)
    {
        do
        {
            span = st->line_advance;
            if (span > 0)
            {
                do
                {
                    avail = 0x100 - u;
                    if (span >= avail)
                    {
                        u += span;
                        span -= avail;
                        u = 0;
                        v += st->line_height;
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
    st->dirty_end_u = u;
    st->region_end_u = u;
    st->dirty_end_v = v;
    st->region_end_v = v;
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
    s32 n;
    FieldTextConfig* rec;

    dst = (u8*)0x801ED408;
    n = 0x17;
    rec = &D_801ED004[slot];
    src = (u8*)rec;
    do
    {
        *dst = *src;
        src += 1;
        n -= 1;
        dst += 1;
    } while (n != -1);
    if (placement_mode == 1)
    {
        field_text_open_packed_window(slot);
    }
    else
    {
        field_text_open_fixed_window(slot);
    }
    if (rec->text != 0)
    {
        field_text_set_string(slot, rec->text, rec->flags.b.byte2);
    }
}
