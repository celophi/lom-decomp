#include "common.h"
#include "zukan.h"
#include "zukan_category.h"
#include "main.h"
#include "cdrom.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

/*
 * Encyclopedia overlay UI, entry navigation, resource loading and rendering.
 */

/* Overlay data types. */

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ZukanRect;

typedef struct
{
    s16 x;
    s16 y;
    s16 clut_x;
    s16 clut_y;
} ZukanImageDestination;

/**
 * @brief One encyclopedia list entry: glyph index and a discovered flag in the
 *        top bit of @c resource_flags.
 */
typedef struct
{
    u16 glyph_index;
    u16 resource_flags;
} ZukanListEntry;

/**
 * @brief One per-entry glyph record. @c attributes and @c texture pack the sprite fields;
 *        @c x and @c y are the on-screen X/Y used when building primitives.
 */
typedef struct
{
    u32 attributes;
    u32 texture;
    u16 x;
    u16 y;
} ZukanUiSpriteRecord;

/**
 * @brief Flat triangle GPU primitive (code 0x22) used for the scroll arrows.
 */
typedef struct
{
    u_long tag;
    u_char r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
} ZukanPolyF3;

/**
 * @brief Persistent draw state used while rendering the encyclopedia screen.
 */
typedef struct
{
    u8 pad0[0x30];
    s32 list_ordering_table;
    s32 detail_ordering_table;
    u8 pad38[0x4008];
    void* prim_cursor;
    s16 unk4044;
    s16 frame_flag;
} ZukanDrawState;

/**
 * @brief 2D short position passed to the number/text drawing helpers.
 */
typedef struct
{
    s16 x;
    s16 y;
} ZukanPos;

typedef struct
{
    s32 tag;
    s32 color_and_code;
    s16 x0;
    s16 y0;
    s16 x1;
    u16 y1;
} ZukanLinePacket;

typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} ZukanFadePrimitive;

typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 steps_remaining;
} ZukanFadeState;

typedef struct
{
    u8 pad[0xC];
    s32 size;
    s32 data_offset;
} ZukanEntryResourceHeader;

/* Rendering constants. */

#define ZUKAN_GPU_ADDRESS_MASK 0xFFFFFF
#define ZUKAN_GPU_TAG_HIGH_MASK 0xFF000000
#define ZUKAN_FADE_NEUTRAL 0x100
#define ZUKAN_FADE_ADDITIVE_THRESHOLD (ZUKAN_FADE_NEUTRAL + 1)
#define ZUKAN_FADE_ADDITIVE_DRAW_MODE 0x25
#define ZUKAN_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define ZUKAN_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((ZukanFadePrimitive*)((u8*)(primitive) + sizeof(type)))

#define ZUKAN_VIEW_DETAIL 0
#define ZUKAN_VIEW_LIST 1

#define ZUKAN_TRANSITION_IDLE 0
#define ZUKAN_TRANSITION_NEXT_FADE_OUT 1
#define ZUKAN_TRANSITION_NEXT_FADE_IN 2
#define ZUKAN_TRANSITION_PREVIOUS_FADE_OUT 3
#define ZUKAN_TRANSITION_PREVIOUS_FADE_IN 4
#define ZUKAN_TRANSITION_RETURN_TO_LIST 5
#define ZUKAN_TRANSITION_OPEN_DETAIL 6

#define ZUKAN_FADE_STEPS 6
#define ZUKAN_TRANSITION_FRAMES 8
#define ZUKAN_SCROLL_STEPS 4
#define ZUKAN_ENTRY_AVAILABLE_FLAG 0x8000
#define ZUKAN_RESOURCE_ID_MASK 0x7FFF
#define ZUKAN_ENTRY_RESOURCE_BASE 0xBFC
#define ZUKAN_UI_RESOURCE_ID 0x5E3


/* Overlay state. */

extern u8 D_800EC3E0;
extern u8 g_zukan_resource_archive[];
extern u8* g_zukan_resource_buffer;
extern u8* g_zukan_work_buffer;
extern s32 g_zukan_exit_requested;
extern ZukanUiSpriteRecord g_zukan_ui_sprites[];
extern ZukanListEntry g_zukan_list_entries[];
extern ZukanFadeState g_zukan_fade_target;
extern s16 g_zukan_input_blocked;
extern s32 g_zukan_category;
extern s32 g_zukan_entry_count;
extern s32 g_zukan_previous_resource_id;
extern ZukanFadeState g_zukan_fade_current;
extern s32 g_zukan_displayed_entry;
extern s32 g_zukan_transition_frame;
extern s32 g_zukan_view_mode;
extern s32 g_zukan_image_mode;
extern s32 g_zukan_selected_entry;
extern s32 g_zukan_transition_state;
extern s32 g_zukan_scroll_steps;
extern s32 g_zukan_scroll_y;
extern s32 g_zukan_scroll_target_y;
extern s32 g_zukan_next_resource_id;

/*
 * These storage symbols are used both as addresses and byte buffers. Keep the
 * byte-oriented declarations private to this overlay implementation.
 */

/* Shared menu helper. */
void play_menu_sfx(s32 sfx_id, s32 volume);

/* Helper routines. */
void zukan_upload_ui_images(s32 work_buffer);
s32 zukan_upload_tim(ZukanImageDestination* destinations, u8* tim);
s32 zukan_handle_input(void);
void zukan_scroll_to_selection(void);
static s32 zukan_emit_draw_mode_5(s32 packet_cursor, s32 ordering_table);
static s32 zukan_emit_draw_mode_1d(s32 packet_cursor, s32 ordering_table);
static s32 zukan_emit_texture_draw_mode(s32 packet_cursor, s32 ordering_table);
void zukan_render_ui(u8* frame_context);
s32 zukan_emit_ui_sprite(s32 packet_cursor, s32* ordering_table, u32 sprite_index, s32 x, s32 y, s32 variant);
void zukan_start_next_entry_transition(void);
void zukan_start_previous_entry_transition(void);
void zukan_update_transition(u8* frame_context);
void zukan_render_content(ZukanDrawState* ctx);
static ZukanLinePacket* zukan_emit_panel_outline(ZukanLinePacket* packet, s32* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color);
void zukan_set_fade_target(s16 red, s16 green, s16 blue, s16 steps);
ZukanFadePrimitive* zukan_render_fade(ZukanFadePrimitive* primitive, u_long* ordering_table_tag);
void zukan_build_entry_list(s32 category);
void zukan_load_entry(s32 index);
void zukan_load_related_entry(s32 resource_id);
s32 zukan_render_detail_text(s32 packet_cursor, s32 ordering_table);
void* zukan_render_detail_sprites(SPRT* sprite, s32* ordering_table);
void zukan_commit_loaded_entry(void);

/**
 * @brief Initialize encyclopedia state and return the next free work-buffer address.
 * @param work_buffer Start of the overlay work buffer.
 * @param category Encyclopedia category to display.
 * @return First address after the encyclopedia's reserved work area.
 */
s32 zukan_initialize_state(s32 work_buffer, s32 category)
{
    s32 next_buffer;
    volatile s32 scratch[2];

    g_zukan_category = category;
    g_zukan_work_buffer = (u8*)((work_buffer + 3) & ~3);
    zukan_upload_ui_images((next_buffer = work_buffer + 0x8000, work_buffer));
    func_800AA02C();
    zukan_set_fade_target(0x100, 0x100, 0x100, ZUKAN_FADE_STEPS);
    g_zukan_transition_state = ZUKAN_TRANSITION_IDLE;
    g_zukan_view_mode = ZUKAN_VIEW_LIST;
    g_zukan_selected_entry = 0;
    g_zukan_scroll_target_y = 0;
    g_zukan_scroll_y = 0;
    g_zukan_scroll_steps = 0;
    zukan_build_entry_list(category);
    while (1)
    {
        while (1)
        {
            return next_buffer;
        }
    }
}

/**
 * @brief Upload the two UI image blocks used by the encyclopedia screen.
 * @param work_buffer Overlay work-buffer address retained by the caller.
 * @see decomp.me (100%)
 */
void zukan_upload_ui_images(s32 work_buffer)
{
    ZukanImageDestination destinations;
    u8* archive = g_zukan_resource_archive;

    destinations.x = 0x340;
    destinations.y = 0x100;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    zukan_upload_tim(&destinations, archive + *(s32*)(archive + 8));

    destinations.x = 0x140;
    destinations.y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    zukan_upload_tim(&destinations, archive + *(s32*)(archive + 4));
}

/**
 * @brief Upload a TIM image and its optional CLUT to VRAM.
 * @param destinations VRAM destinations for pixels and CLUT data.
 * @param tim TIM image data.
 * @return The TIM pixel-mode bits from the flags word.
 * @see decomp.me (100%)
 */
s32 zukan_upload_tim(ZukanImageDestination* destinations, u8* tim)
{
    ZukanRect upload_rect;
    s32 flags;
    s32 clut_block_size;
    u16* pixel_dimensions;
    s32 mode;

    flags = *(s32*)(tim + 4);
    clut_block_size = *(s32*)(tim + 8);
    mode = flags & 7;

    if (flags & 8)
    {
        upload_rect.x = destinations->clut_x;
        upload_rect.y = destinations->clut_y;
        upload_rect.w = 0x100;
        upload_rect.h = 1;
        LoadImage(&upload_rect, tim + 0x14);
        pixel_dimensions = (u16*)(clut_block_size - (-(s32)tim) + 0x10);
    }
    else
    {
        pixel_dimensions = (u16*)(tim + 0x10);
    }

    upload_rect.x = destinations->x;
    upload_rect.y = destinations->y;
    upload_rect.w = pixel_dimensions[0];
    upload_rect.h = pixel_dimensions[1];
    LoadImage(&upload_rect, clut_block_size - (-(s32)tim) + 0x14);
    return mode;
}

/**
 * @brief Build the current frame and advance the encyclopedia transition state.
 * @param frame_context Render context for the current frame.
 * @see decomp.me (100%)
 */
void zukan_update_frame(s32 frame_context)
{
    zukan_render_ui(frame_context);
    zukan_update_transition(frame_context);
    zukan_render_content(frame_context);
    g_frame_counter += 1;
    zukan_handle_input();

    if (g_zukan_scroll_steps != 0)
    {
        g_zukan_scroll_y += (g_zukan_scroll_target_y - g_zukan_scroll_y) / g_zukan_scroll_steps;
        g_zukan_scroll_steps -= 1;
        return;
    }
    g_zukan_scroll_y = g_zukan_scroll_target_y;
}

/**
 * @brief Process encyclopedia navigation and view-change input.
 * @return Unspecified; callers ignore the return value.
 */
s32 zukan_handle_input(void)
{
    s32 repeat_count;
    s32 selection_moved;
    s32 last_index;
    s32 entry_count;
    s32 forward_input;
    s32 backward_input;
    s32 related_resource_id;

    if (g_zukan_transition_state != 0)
    {
        return;
    }

    if (g_pad_input & 0x800)
    {
        g_zukan_exit_requested = 1;
        return;
    }

    if (g_zukan_view_mode != 0)
    {
        if (g_zukan_input_blocked != 0)
        {
            return;
        }

        if (g_pad_input & 0x40)
        {
            g_zukan_exit_requested = 1;
            return;
        }

        selection_moved = 0;
        if (g_pad_input & 0x220)
        {
            if (g_zukan_list_entries[g_zukan_selected_entry].resource_flags >> 15)
            {
                play_menu_sfx(0x7E, 0x80);
                zukan_set_fade_target(0, 0, 0, ZUKAN_FADE_STEPS);
                zukan_load_entry(g_zukan_selected_entry);
                g_zukan_transition_state = ZUKAN_TRANSITION_OPEN_DETAIL;
                g_zukan_transition_frame = 0;
            }
            else
            {
                play_menu_sfx(0x78, 0x80);
            }
            return;
        }

        repeat_count = 1;
        if (g_pad_input & 8)
        {
            repeat_count = 8;
            g_pad_input = 0x4000;
        }
        else if (g_pad_input & 4)
        {
            repeat_count = 8;
            g_pad_input = 0x1000;
        }

        if (repeat_count != 0)
        {
            forward_input = g_pad_input & 0x6000;
            backward_input = g_pad_input & 0x9000;
            entry_count = g_zukan_entry_count;
            last_index = entry_count - 1;
            do
            {
                if (forward_input)
                {
                    g_zukan_selected_entry++;
                    if (g_zukan_selected_entry >= entry_count)
                    {
                        g_zukan_selected_entry = 0;
                    }
                    selection_moved = 1;
                }
                else if (backward_input)
                {
                    g_zukan_selected_entry--;
                    if (g_zukan_selected_entry < 0)
                    {
                        g_zukan_selected_entry = last_index;
                    }
                    selection_moved = 1;
                }
                if ((g_zukan_selected_entry == last_index) || (g_zukan_selected_entry == 0))
                {
                    repeat_count = 1;
                }
                repeat_count--;
            } while (repeat_count != 0);
        }

        if (selection_moved != 0)
        {
            play_menu_sfx(0x7D, 0x80);
            zukan_scroll_to_selection();
        }
        return;
    }

    if (g_pad_input & 0x260)
    {
        play_menu_sfx(0x7E, 0x80);
        zukan_set_fade_target(0, 0, 0, ZUKAN_FADE_STEPS);
        g_zukan_transition_state = ZUKAN_TRANSITION_RETURN_TO_LIST;
        g_zukan_transition_frame = 0;
        return;
    }

    if (g_pad_input & 0x2008)
    {
        play_menu_sfx(0x7D, 0x80);
        zukan_start_next_entry_transition();
        related_resource_id = g_zukan_next_resource_id;
        if ((related_resource_id != 0) && (g_pad_input & 0x2000))
        {
            zukan_load_related_entry(related_resource_id);
            return;
        }

        g_zukan_selected_entry++;
        if (g_zukan_selected_entry >= g_zukan_entry_count)
        {
            g_zukan_selected_entry = 0;
        }
        while ((g_zukan_list_entries[g_zukan_selected_entry].resource_flags >> 15) == 0)
        {
            g_zukan_selected_entry++;
            if (g_zukan_selected_entry >= g_zukan_entry_count)
            {
                g_zukan_selected_entry = 0;
            }
        }
        zukan_load_entry(g_zukan_selected_entry);
        zukan_scroll_to_selection();
        return;
    }

    if (g_pad_input & 0x8004)
    {
        play_menu_sfx(0x7D, 0x80);
        zukan_start_previous_entry_transition();
        related_resource_id = g_zukan_previous_resource_id;
        if ((related_resource_id != 0) && (g_pad_input & 0x8000))
        {
            zukan_load_related_entry(related_resource_id);
            return;
        }

        g_zukan_selected_entry--;
        if (g_zukan_selected_entry < 0)
        {
            g_zukan_selected_entry = g_zukan_entry_count - 1;
        }
        while ((g_zukan_list_entries[g_zukan_selected_entry].resource_flags >> 15) == 0)
        {
            g_zukan_selected_entry--;
            if (g_zukan_selected_entry < 0)
            {
                g_zukan_selected_entry = g_zukan_entry_count - 1;
            }
        }
        zukan_load_entry(g_zukan_selected_entry);
        zukan_scroll_to_selection();
    }
}

/**
 * @brief Adjust the scroll target so the selected list row remains visible.
 */
void zukan_scroll_to_selection(void)
{
    s32 selection_y = g_zukan_selected_entry << 4;
    s32 delta = selection_y - g_zukan_scroll_y;

    if (delta < 0)
    {
        g_zukan_scroll_target_y = selection_y;
        g_zukan_scroll_steps = ZUKAN_SCROLL_STEPS;
    }
    else if (delta > 0x70)
    {
        g_zukan_scroll_target_y = selection_y - 0x70;
        g_zukan_scroll_steps = ZUKAN_SCROLL_STEPS;
    }
}

/**
 * @brief Append texture-page draw mode 5 to an ordering table.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
static s32 zukan_emit_draw_mode_5(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 5);
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Append texture-page draw mode 0x1D to an ordering table.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
static s32 zukan_emit_draw_mode_1d(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 0x1D);
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Append the texture-page draw mode for the loaded encyclopedia image.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
static s32 zukan_emit_texture_draw_mode(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 0x1E | ((g_zukan_image_mode & 3) << 7));
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Emit the fixed encyclopedia UI sprites and fade overlay.
 * @param frame_context Render context for the current frame.
 */
void zukan_render_ui(u8* frame_context)
{
    u8* ordering_table;
    s32 i;
    s32 packet_cursor;
    s32 detail_flag;
    volatile s32 scratch[2];

    ordering_table = frame_context + 0x28;
    i = 0;
    packet_cursor = *(s32*)(frame_context + 0x4040);
    do
    {
        packet_cursor = zukan_emit_ui_sprite(packet_cursor, ordering_table, i, g_zukan_ui_sprites[i].x,
                           g_zukan_ui_sprites[i].y, 0);
        i++;
    } while (i < 6);

    ordering_table = frame_context + 0x3C;
    i = 6;
    detail_flag = 1;
    do
    {
        packet_cursor = zukan_emit_ui_sprite(packet_cursor, ordering_table, i, g_zukan_ui_sprites[i].x,
                           g_zukan_ui_sprites[i].y, detail_flag);
        i++;
    } while (i < 0x15);

    *(s32*)(frame_context + 0x4040) = zukan_render_fade(packet_cursor, frame_context + 0x2C);
}

/**
 * @brief Emit one fixed UI sprite and its texture-page command.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering table receiving the sprite.
 * @param sprite_index Index of the UI sprite definition.
 * @param x Screen x coordinate.
 * @param y Screen y coordinate.
 * @param variant Sprite-group selector supplied by the caller.
 * @return Primitive-buffer address after the emitted packets.
 */
s32 zukan_emit_ui_sprite(s32 packet_cursor, s32* ordering_table, u32 sprite_index, s32 x, s32 y, s32 variant)
{
    ZukanUiSpriteRecord* sprite_record;
    DR_TPAGE* draw_mode;

    SET_BGR0_PACKED(((SPRT*)packet_cursor), GPU_TINT_NEUTRAL);

    if (g_zukan_view_mode != 0)
    {
        if (sprite_index < 4)
        {
            SET_BGR0_PACKED(((SPRT*)packet_cursor), 0x303030);
        }
    }
    else if ((sprite_index == 0) || (sprite_index == 3) || (sprite_index == 5))
    {
        switch (g_zukan_transition_state)
        {
        case 1:
            if (sprite_index == 3)
            {
                SET_BGR0_PACKED(((SPRT*)packet_cursor), 0xE0E0FF);
            }
            break;
        case ZUKAN_TRANSITION_PREVIOUS_FADE_OUT:
            if (sprite_index == 0)
            {
                SET_BGR0_PACKED(((SPRT*)packet_cursor), 0xE0E0FF);
            }
            break;
        case ZUKAN_TRANSITION_RETURN_TO_LIST:
            if (sprite_index == 5)
            {
                SET_BGR0_PACKED(((SPRT*)packet_cursor), 0xE0E0FF);
            }
            break;
        }
    }

    setlen(((SPRT*)packet_cursor), 4);
    ((SPRT*)packet_cursor)->code = 0x64;
    ((SPRT*)packet_cursor)->x0 = x + 8;
    ((SPRT*)packet_cursor)->y0 = y;

    sprite_record = &g_zukan_ui_sprites[sprite_index];
    ((SPRT*)packet_cursor)->w = (sprite_record->texture >> 14) & 0x1FF;
    ((SPRT*)packet_cursor)->h = sprite_record->texture >> 23;
    ((SPRT*)packet_cursor)->u0 = sprite_record->attributes >> 8;
    ((SPRT*)packet_cursor)->v0 = sprite_record->texture;
    ((SPRT*)packet_cursor)->clut = ((sprite_record->texture >> 8) & 0x3F) | 0x7C80;

    addPrim(ordering_table, ((SPRT*)packet_cursor));

    packet_cursor += sizeof(SPRT);
    draw_mode = (DR_TPAGE*)packet_cursor;
    switch ((u8)sprite_record->attributes)
    {
    case 0:
        setDrawTPage(draw_mode, 0, 0, 5);
        addPrim(ordering_table, draw_mode);
        packet_cursor += sizeof(DR_TPAGE);
        break;
    case 1:
        setDrawTPage(draw_mode, 0, 0, 0x1D);
        addPrim(ordering_table, draw_mode);
        packet_cursor += sizeof(DR_TPAGE);
        break;
    }

    return packet_cursor;
}

/**
 * @brief Begin the fade transition to the next encyclopedia entry.
 */
void zukan_start_next_entry_transition(void)
{
    zukan_set_fade_target(0, 0, 0, ZUKAN_FADE_STEPS);
    g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_OUT;
    g_zukan_transition_frame = 0;
}

/**
 * @brief Begin the fade transition to the previous encyclopedia entry.
 */
void zukan_start_previous_entry_transition(void)
{
    zukan_set_fade_target(0, 0, 0, ZUKAN_FADE_STEPS);
    g_zukan_transition_state = ZUKAN_TRANSITION_PREVIOUS_FADE_OUT;
    g_zukan_transition_frame = 0;
}

/**
 * @brief Advance an active list/detail or entry-change transition.
 * @param frame_context Render context whose primitive cursor is preserved.
 */
void zukan_update_transition(u8* frame_context)
{
    s32 saved_packet_cursor;
    volatile s32 scratch[2];
    if (g_zukan_transition_state != 0)
    {
        saved_packet_cursor = *(s32*)(frame_context + 0x4040);
        switch (g_zukan_transition_state)
        {
        case ZUKAN_TRANSITION_NEXT_FADE_OUT: {
            s32* counter = &g_zukan_transition_frame;
            if (++*counter == ZUKAN_TRANSITION_FRAMES)
            {
                zukan_commit_loaded_entry();
                g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_IN;
                *counter = 0;
                g_zukan_displayed_entry = g_zukan_selected_entry;
                zukan_set_fade_target(0x100, 0x100, 0x100, ZUKAN_FADE_STEPS);
            }
            break;
        }
        case ZUKAN_TRANSITION_NEXT_FADE_IN:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                g_zukan_transition_state = ZUKAN_TRANSITION_IDLE;
            }
            break;
        case ZUKAN_TRANSITION_PREVIOUS_FADE_OUT: {
            s32* counter = &g_zukan_transition_frame;
            if (++*counter == ZUKAN_TRANSITION_FRAMES)
            {
                zukan_commit_loaded_entry();
                g_zukan_transition_state = ZUKAN_TRANSITION_PREVIOUS_FADE_IN;
                *counter = 0;
                g_zukan_displayed_entry = g_zukan_selected_entry;
                zukan_set_fade_target(0x100, 0x100, 0x100, ZUKAN_FADE_STEPS);
            }
            break;
        }
        case ZUKAN_TRANSITION_PREVIOUS_FADE_IN:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                g_zukan_transition_state = ZUKAN_TRANSITION_IDLE;
                g_zukan_transition_frame = 0;
            }
            break;
        case ZUKAN_TRANSITION_RETURN_TO_LIST:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_IN;
                g_zukan_transition_frame = 0;
                g_zukan_view_mode = ZUKAN_VIEW_LIST;
                zukan_set_fade_target(0x100, 0x100, 0x100, ZUKAN_FADE_STEPS);
            }
            break;
        case ZUKAN_TRANSITION_OPEN_DETAIL: {
            s32* counter = &g_zukan_transition_frame;
            if (++*counter == ZUKAN_TRANSITION_FRAMES)
            {
                zukan_commit_loaded_entry();
                g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_IN;
                g_zukan_view_mode = 0;
                *counter = 0;
                g_zukan_displayed_entry = g_zukan_selected_entry;
                zukan_set_fade_target(0x100, 0x100, 0x100, ZUKAN_FADE_STEPS);
            }
            break;
        }
        }
        *(s32*)(frame_context + 0x4040) = saved_packet_cursor;
    }
}

/**
 * @brief Draw the encyclopedia entry-list or detail view.
 * @param ctx Encyclopedia draw state whose primitive cursor is advanced in place.
 * @see decomp.me (100%)
 */
void zukan_render_content(ZukanDrawState* ctx)
{
    volatile s32 stack_pad[2];
    u8 draw_env[0x60];
    ZukanPos pos;
    u8* packet_cursor;
    s32* ordering_table;
    s32 entry_index;
    s32 row_y;
    u8* archive;
    u8* fallback_glyph;
    u8* fallback_glyph_base;
    ZukanListEntry* list_entry;

    packet_cursor = ctx->prim_cursor;

    if (g_zukan_view_mode != 0)
    {
        ZukanPolyF3 *tri;
        TILE* tile;
        u8* env_prim;

        ordering_table = &ctx->list_ordering_table;

        {
            s32 table_off;
            u16 glyph_off;
            u8* glyph_ptr;
            u8* final_ptr;
            u8* title_base = g_zukan_resource_archive;
            table_off = *(s32*)(title_base + 0x10);
            glyph_off = *(u16*)(g_zukan_category * 2 + table_off + title_base);
            glyph_ptr = title_base + glyph_off;
            final_ptr = (u8*)(table_off + (s32)glyph_ptr);
            packet_cursor = (u8*)func_800A88A0(packet_cursor, ordering_table, final_ptr, 0xA, 0xA0, 0x22, 2);
        }

        if (g_zukan_scroll_y != 0)
        {
            tri = (ZukanPolyF3 *)packet_cursor;
            *(u32*)&tri->r0 = 0xF08080;
            setlen(tri, 4);
            tri->code = 0x22;
            tri->x0 = 0x99;
            tri->x1 = 0xA0;
            tri->x2 = 0xA7;
            tri->y1 = 0x2E;
            tri->y0 = tri->y2 = 0x35;
            addPrim(ordering_table, tri);
            packet_cursor += sizeof(ZukanPolyF3);
        }

        if (g_zukan_scroll_y + 0x80 < g_zukan_entry_count * 0x10)
        {
            tri = (ZukanPolyF3 *)packet_cursor;
            *(u32*)&tri->r0 = 0xF08080;
            setlen(tri, 4);
            tri->code = 0x22;
            tri->x0 = 0x99;
            tri->x1 = 0xA0;
            tri->x2 = 0xA7;
            tri->y1 = 0xBB;
            tri->y0 = tri->y2 = 0xB4;
            addPrim(ordering_table, tri);
            packet_cursor += sizeof(ZukanPolyF3);
        }

        if (ctx->frame_flag != 8)
        {
            SetDefDrawEnv(draw_env, 0, 0xF0, 0x140, 0xE0);
        }
        else
        {
            SetDefDrawEnv(draw_env, 0, 8, 0x140, 0xE0);
        }
        SetDrawEnv(packet_cursor, draw_env);
        addPrim(ordering_table, packet_cursor);
        packet_cursor += 0x40;

        entry_index = 0;
        if (g_zukan_entry_count > 0)
        {
            do
            {
                do
                {
                    archive = &g_zukan_resource_archive;
                } while (0);
            } while (0);
            do
            {
                fallback_glyph = &D_800EC3E0;
                fallback_glyph_base = fallback_glyph - 0x1C;
            } while (0);
            list_entry = g_zukan_list_entries;
            list_entry++;
            list_entry--;
loop_head:
            row_y = (entry_index * 0x10) - g_zukan_scroll_y;
            row_y++;
            row_y--;
            row_y++;
            row_y--;
            if ((u32)(row_y + 0xF) >= 0x8F)
            {
                goto loop_inc_cull;
            }

            pos.x = 0;
            pos.y = row_y;
            packet_cursor = (u8*)func_800A8B04(ordering_table, packet_cursor, entry_index + 1, 0, &pos, 0);
            if (list_entry->resource_flags >> 15)
            {
                goto glyph_true;
            }
            goto glyph_false;

glyph_true:
            {
                s32 table_off;
                s32 glyph_index_offset;
                s32 glyph_record_addr;
                s32 glyph_addr;
                s32 final_addr;
                u16 glyph_off;
                table_off = *(s32*)(archive + 0xC);
                glyph_index_offset = list_entry->glyph_index * 2;
                glyph_index_offset += table_off;
                glyph_record_addr = glyph_index_offset + (s32)archive;
                glyph_off = *(u16*)glyph_record_addr;
                glyph_addr = glyph_off + (s32)archive;
                final_addr = table_off + glyph_addr;
                packet_cursor = (u8*)func_800A88A0(packet_cursor, ordering_table, (u8*)final_addr, 0, 0x66, row_y, 2);
            }
            goto loop_inc_visible;

glyph_false:
            {
                u8 high_byte;
                u8 low_byte;
                s32 shifted_offset;
                s32 based_offset;
                s32 final_offset;
                high_byte = fallback_glyph[1];
                low_byte = D_800EC3E0;
                shifted_offset = high_byte << 8;
                based_offset = shifted_offset + (s32)fallback_glyph_base;
                final_offset = low_byte + based_offset;
                packet_cursor = (u8*)func_800A88A0(packet_cursor, ordering_table, (u8*)final_offset,
                    0, 0x66, row_y, 2);
            }

loop_inc_visible:
            do
            {
                do
                {
                    if (++entry_index < g_zukan_entry_count)
                    {
                        list_entry++;
                        goto loop_head;
                    }
                } while (0);
            } while (0);
            goto loop_done;
loop_inc_cull:
            do
            {
                do
                {
                    if (++entry_index < g_zukan_entry_count)
                    {
                        list_entry++;
                        goto loop_head;
                    }
                } while (0);
            } while (0);
loop_done:
            ;
        }

        row_y = (g_zukan_selected_entry * 0x10) - g_zukan_scroll_y;
        if (row_y < 0)
        {
            row_y = 0;
        }
        else if (row_y >= 0x71)
        {
            row_y = 0x70;
        }

        tile = (TILE*)packet_cursor;
        *(u32*)&tile->r0 = 0xF080F0;
        setlen(tile, 3);
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = row_y;
        tile->w = 0xB8;
        tile->h = 0xF;
        addPrim(ordering_table, tile);
        packet_cursor += sizeof(TILE);

        env_prim = packet_cursor;
        if (ctx->frame_flag != 8)
        {
            SetDefDrawEnv(draw_env, 0x48, 0x126, 0xB8, 0x80);
        }
        else
        {
            SetDefDrawEnv(draw_env, 0x48, 0x3E, 0xB8, 0x80);
        }
        SetDrawEnv(env_prim, draw_env);
        addPrim(ordering_table, env_prim);
        packet_cursor = env_prim + 0x40;
    }
    else
    {
        ordering_table = &ctx->detail_ordering_table;
        packet_cursor = (u8*)zukan_render_detail_text(packet_cursor, ordering_table);
        packet_cursor = (u8*)zukan_render_detail_sprites(packet_cursor, ordering_table);

        pos.x = 0x106;
        pos.y = 0xBD;
        packet_cursor = (u8*)func_800AD524(packet_cursor, ordering_table, 0xB, &pos, 1);

        pos.x = 0xEE;
        pos.y = 0xBD;
        packet_cursor = (u8*)func_800AD208(ordering_table, packet_cursor, g_zukan_displayed_entry + 1, 3, &pos, 0);

        pos.x = 0x10E;
        pos.y = 0xBD;
        packet_cursor = (u8*)func_800AD208(ordering_table, packet_cursor, g_zukan_entry_count, 3, &pos, 0);
    }

    ctx->prim_cursor = packet_cursor;
}

/**
 * @brief Emit a rectangular four-line panel outline.
 * @param packet Next line packet in the primitive buffer.
 * @param ordering_table Ordering table receiving the lines.
 * @param x Left edge.
 * @param y Top edge.
 * @param width Panel width.
 * @param height Panel height.
 * @param color Packed line color.
 * @return First packet after the four outline lines.
 * @see GOLEM golem_emit_panel_outline (100%)
 */
static ZukanLinePacket* zukan_emit_panel_outline(ZukanLinePacket* packet, s32* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    s32 tag_mask;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y;
    tag_mask = ZUKAN_GPU_TAG_HIGH_MASK;
    packet->tag = (packet->tag & ZUKAN_GPU_TAG_HIGH_MASK) | (*ordering_table & ZUKAN_GPU_ADDRESS_MASK);
    *ordering_table = (*ordering_table & tag_mask) | ((s32)packet & ZUKAN_GPU_ADDRESS_MASK);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x + width;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x + width;
    tag_mask = y + height;
    packet->y0 = tag_mask;
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    return packet + 1;
}

/**
 * @brief Set the target color and duration for the screen fade.
 * @param red Target red component.
 * @param green Target green component.
 * @param blue Target blue component.
 * @param steps Number of fade steps.
 * @see GOLEM golem_set_fade_target (100%)
 */
void zukan_set_fade_target(s16 red, s16 green, s16 blue, s16 steps)
{
    g_zukan_fade_target.red = red;
    g_zukan_fade_target.green = green;
    g_zukan_fade_target.blue = blue;
    g_zukan_fade_target.steps_remaining = steps;
}

/**
 * @brief Advance the fade color and emit the full-screen fade primitive.
 * @param primitive Next fade primitive in the packet buffer.
 * @param ordering_table_tag Ordering-table tag receiving the fade.
 * @return First primitive after the emitted fade packets.
 * @see GOLEM golem_render_fade (100%)
 */
ZukanFadePrimitive* zukan_render_fade(ZukanFadePrimitive* primitive, u_long* ordering_table_tag)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (g_zukan_fade_target.steps_remaining != 0)
    {
        red_step = (g_zukan_fade_target.red - g_zukan_fade_current.red) / g_zukan_fade_target.steps_remaining;
        green_step = (g_zukan_fade_target.green - g_zukan_fade_current.green) / g_zukan_fade_target.steps_remaining;
        blue_step = (g_zukan_fade_target.blue - g_zukan_fade_current.blue) / g_zukan_fade_target.steps_remaining;
        g_zukan_fade_target.steps_remaining = g_zukan_fade_target.steps_remaining - 1;
        g_zukan_fade_current.red = g_zukan_fade_current.red + red_step;
        g_zukan_fade_current.green = g_zukan_fade_current.green + green_step;
        g_zukan_fade_current.blue = g_zukan_fade_current.blue + blue_step;
    }
    else
    {
        g_zukan_fade_current.red = g_zukan_fade_target.red;
        g_zukan_fade_current.green = g_zukan_fade_target.green;
        g_zukan_fade_current.blue = g_zukan_fade_target.blue;
    }

    if ((g_zukan_fade_current.red != ZUKAN_FADE_NEUTRAL) || (g_zukan_fade_current.green != g_zukan_fade_current.red) ||
        (g_zukan_fade_current.blue != g_zukan_fade_current.green))
    {
        if (g_zukan_fade_current.red >= ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = g_zukan_fade_current.red - 1;
            primitive->tile.g0 = g_zukan_fade_current.green - 1;
            primitive->tile.b0 = g_zukan_fade_current.blue - 1;
        }
        else
        {
            if (g_zukan_fade_current.red == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_zukan_fade_current.red;
            }
            if (g_zukan_fade_current.green == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_zukan_fade_current.green;
            }
            if (g_zukan_fade_current.blue == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_zukan_fade_current.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        primitive->tile.w = SCREEN_WIDTH;
        draw_mode = ZUKAN_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0(&primitive->tile, 0, 0);
        primitive->tile.h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, &primitive->tile);

        primitive = ZUKAN_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_zukan_fade_current.red < ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = ZUKAN_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = ZUKAN_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    return primitive;
}

/**
 * @brief Build the visible entry list for an encyclopedia category.
 * @param category Category index to enumerate.
 */
void zukan_build_entry_list(s32 category)
{
    u32 resource_ids[0x200];
    u16 glyph_indices[0x400];
    s32 count;
    s32 i;
    s32 entry_count;

    count = zukan_build_category_entries(category, resource_ids, glyph_indices);
    g_zukan_entry_count = count;
    i = 0;
    if (count > 0)
    {
        entry_count = count;
        do
        {
            if (resource_ids[i] != 0)
            {
                g_zukan_list_entries[i].resource_flags |= ZUKAN_ENTRY_AVAILABLE_FLAG;
                g_zukan_list_entries[i].resource_flags =
                    (g_zukan_list_entries[i].resource_flags & ZUKAN_ENTRY_AVAILABLE_FLAG) | ((u16)resource_ids[i] & ZUKAN_RESOURCE_ID_MASK);
                g_zukan_list_entries[i].glyph_index = glyph_indices[i * 2];
            }
            else
            {
                g_zukan_list_entries[i].resource_flags = 0;
                g_zukan_list_entries[i].glyph_index = 0;
            }
            i++;
        } while (i < entry_count);
    }
}

/**
 * @brief Request the resource for a visible encyclopedia entry.
 * @param index Visible entry index.
 */
void zukan_load_entry(s32 index)
{
    cdrom_queue_read((g_zukan_list_entries[index].resource_flags & ZUKAN_RESOURCE_ID_MASK) + ZUKAN_ENTRY_RESOURCE_BASE, g_zukan_resource_buffer);
}

/**
 * @brief Request an entry resource referenced by the current detail record.
 * @param resource_id Related resource identifier.
 */
void zukan_load_related_entry(s32 resource_id)
{
    s32 resource_buffer = g_zukan_resource_buffer;

    cdrom_queue_read((resource_id + ZUKAN_ENTRY_RESOURCE_BASE) & 0xFFFF, resource_buffer);
}

/**
 * @brief Draw the text lines for the active encyclopedia detail record.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering table receiving text primitives.
 * @return Primitive-buffer address after the text primitives.
 */
s32 zukan_render_detail_text(s32 packet_cursor, s32 ordering_table)
{
    s32 i;
    s32 line_y;
    u16* offsets;
    u16* line_offsets;
    u8* text;
    volatile u8 text_buffer[0x100];

    line_offsets = (u16*)(g_zukan_work_buffer + *(s32*)(g_zukan_work_buffer + 8));
    offsets = line_offsets;
    line_y = 0x1A;
    i = 0;
    do
    {
        s32 width;

        text = (u8*)line_offsets + *offsets;
        width = 0x30;
        if (*text == 0x20)
        {
            u8 inner_space = 0x20;

            do
            {
                text++;
                width += 0xC;
            } while (*text == inner_space);
        }
        packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, 0, width, line_y, 0);
        offsets++;
        i++;
        line_y += 0xD;
    } while (i < 0xC);
    return packet_cursor;
}

/**
 * @brief Draw the sprite records embedded in the active detail resource.
 * @param sprite Current primitive-buffer position interpreted as a sprite packet.
 * @param ordering_table Ordering table receiving the sprite packets.
 * @return Primitive-buffer address after the emitted sprites and draw mode.
 */
void* zukan_render_detail_sprites(SPRT* sprite, s32* ordering_table)
{
    s32 command;
    u8* resource_data = g_zukan_work_buffer;
    u8* sprite_data = resource_data + *(s32*)(resource_data + 4);
    s32 count = *(u16*)sprite_data + (*(u16*)(sprite_data + 2) << 8);

    sprite_data += 4;
    if (count != 0)
    {
        do
        {
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setlen(sprite, 4);
            command = 0x64;
            sprite->code = command;
            sprite->x0 = *(u16*)sprite_data; sprite_data += 2;
            sprite->y0 = *(u16*)sprite_data; sprite_data += 2;
            sprite->u0 = *sprite_data; sprite_data += 2;
            sprite->v0 = *sprite_data; sprite_data += 2;
            sprite->w = *(u16*)sprite_data; sprite_data += 2;
            sprite->h = *(u16*)sprite_data; sprite_data += 2;
            if (g_zukan_image_mode != 0)
            {
                sprite->clut = 0x7B80;
            }
            else
            {
                sprite->clut = (*(u16*)sprite_data & 0x3F) | 0x7B80;
            }
            sprite_data += 4;
            addPrim(ordering_table, sprite);
            sprite++;
            command = count - 1;
            count = command;
        } while (count != 0);
    }

    {
        DR_TPAGE* mode = (DR_TPAGE*)sprite;
        setDrawTPage(mode, 0, 0, getTPage(g_zukan_image_mode, 0, 0x380, 0x100));
        addPrim(ordering_table, mode);
        return mode + 1;
    }
}

/**
 * @brief Copy the current encyclopedia resource, upload its TIM image, and cache its metadata.
 */
void zukan_commit_loaded_entry(void)
{
    ZukanImageDestination destinations;
    ZukanRect upload_rect;
    ZukanImageDestination* layout;
    ZukanRect* rect;
    ZukanEntryResourceHeader* resource_header;
    u8* src;
    u8* dst;
    u8* end;
    u8* tim;
    s32 flags;
    s32 clut_block_size;
    u16* pixel_dimensions;
    s32 mode;

    cdrom_wait_queue_empty();

    resource_header = (ZukanEntryResourceHeader*)g_zukan_resource_buffer;
    dst = g_zukan_work_buffer;
    end = (u8*)resource_header + resource_header->size;
    src = (u8*)resource_header;
    if (src != end)
    {
        do
        {
            *dst++ = *src++;
        } while (src != end);
    }

    destinations.x = 0x380;
    destinations.y = 0x100;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1EE;

    rect = &upload_rect;
    layout = &destinations;
    tim = g_zukan_resource_buffer + ((ZukanEntryResourceHeader*)g_zukan_resource_buffer)->size;
    flags = *(s32*)(tim + 4);
    clut_block_size = *(s32*)(tim + 8);
    mode = flags & 7;

    if (flags & 8)
    {
        upload_rect.x = layout->clut_x;
        upload_rect.y = layout->clut_y;
        rect->w = 0x100;
        rect->h = 1;
        LoadImage(rect, tim + 0x14);
        pixel_dimensions = (u16*)(clut_block_size + (s32)tim + 0x10);
    }
    else
    {
        pixel_dimensions = (u16*)(tim + 0x10);
    }

    upload_rect.x = layout->x;
    upload_rect.y = layout->y;
    upload_rect.w = pixel_dimensions[0];
    upload_rect.h = pixel_dimensions[1];
    LoadImage(&upload_rect, clut_block_size + (s32)tim + 0x14);

    {
        ZukanEntryResourceHeader* resource = (ZukanEntryResourceHeader*)g_zukan_resource_buffer;
        g_zukan_image_mode = mode;
        g_zukan_previous_resource_id = *(u16*)((u8*)resource + resource->data_offset);
        g_zukan_next_resource_id = *(u16*)((u8*)resource + resource->data_offset + 2);
    }
}

/**
 * @brief Load the encyclopedia UI resource and upload its image to VRAM.
 */
void zukan_load_ui_resource(void)
{
    ZukanRect rect;
    s32 packed_dimensions;
    s32 dimension;
    s16 width;
    u8* archive = g_zukan_resource_archive;

    cdrom_queue_read(ZUKAN_UI_RESOURCE_ID, archive);

    packed_dimensions = *(volatile s32*)archive;
    width = packed_dimensions;
    dimension = width;

    *(volatile s16*)&rect.x = 0x340;
    rect.y = 0x100;
    rect.w = dimension;

    dimension = packed_dimensions >> 16;
    rect.h = dimension;

    LoadImage(&rect, archive + 4);
}
