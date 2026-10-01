#include "common.h"
#include "zukan.h"
#include "zukan_category.h"
#include "main.h"
#include "cdrom.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"
#include "tim.h"
#include "render_context.h"
#include "menu.h"
#include "sdk/libetc.h"
#include "field_sound.h"

/*
 * Encyclopedia overlay UI, entry navigation, resource loading and rendering.
 */

/* Overlay data types. */

/** @brief VRAM locations for a TIM image and its color lookup table. */
typedef struct
{
    s16 x;
    s16 y;
    s16 clut_x;
    s16 clut_y;
} ZukanImageDestination;

/** @brief Offsets of the images and text tables in the UI archive. */
typedef struct
{
    s32 section_count;
    s32 page_image_offset;
    s32 border_image_offset;
    s32 entry_names_offset;
    s32 category_names_offset;
} ZukanArchiveHeader;

/**
 * @brief One visible entry's name index and page resource ID. The high bit of
 *        @c resource_id_and_available marks an unlocked page.
 */
typedef struct
{
    u16 name_index;
    u16 resource_id_and_available;
} ZukanListEntry;

/**
 * @brief One fixed UI sprite's texture selection, UV, palette, dimensions,
 *        and screen position.
 */
typedef struct
{
    u32 source_and_u;
    u32 v_clut_and_size;
    u16 x;
    u16 y;
} ZukanUiSpriteRecord;

/**
 * @brief 2D short position passed to the number/text drawing helpers.
 */
typedef struct
{
    s16 x;
    s16 y;
} ZukanPos;

/** @brief Fade color and the number of interpolation steps left. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 steps_remaining;
} ZukanFadeState;

/**
 * @brief Header of an encyclopedia entry resource. Offsets are relative to the
 *        header; the bytes before @c image_offset are copied to the work buffer
 *        and the page TIM image follows them.
 */
typedef struct
{
    s32 unk0;
    s32 sprites_offset;
    s32 text_offset;
    s32 image_offset;
    s32 related_ids_offset;
} ZukanEntryResourceHeader;

/* Rendering constants. */

#define ZUKAN_FADE_NEUTRAL 0x100
#define ZUKAN_FADE_ADDITIVE_THRESHOLD (ZUKAN_FADE_NEUTRAL + 1)

/* Ordering-table slots used in the in-game RenderContext. NAV_SPRITES holds UI sprites 0-5
 * (the previous/next/return controls tinted during transitions), UI_SPRITES the remaining ones. */
#define ZUKAN_LAYER_NAV_SPRITES 10
#define ZUKAN_LAYER_FADE 11
#define ZUKAN_LAYER_LIST 12
#define ZUKAN_LAYER_DETAIL 13
#define ZUKAN_LAYER_UI_SPRITES 15
#define ZUKAN_NAV_SPRITE_COUNT 6
#define ZUKAN_UI_SPRITE_COUNT 21

/* Scrolling list viewport, relative to the active draw buffer. */
#define ZUKAN_LIST_VIEW_X 0x48
#define ZUKAN_LIST_VIEW_Y 0x36
#define ZUKAN_LIST_VIEW_WIDTH 0xB8
#define ZUKAN_LIST_VIEW_HEIGHT 0x80

#define ZUKAN_VIEW_DETAIL 0
#define ZUKAN_VIEW_LIST 1

#define ZUKAN_TRANSITION_IDLE 0
#define ZUKAN_TRANSITION_NEXT_FADE_OUT 1
#define ZUKAN_TRANSITION_NEXT_FADE_IN 2
#define ZUKAN_TRANSITION_PREVIOUS_FADE_OUT 3
#define ZUKAN_TRANSITION_PREVIOUS_FADE_IN 4
#define ZUKAN_TRANSITION_RETURN_TO_LIST 5
#define ZUKAN_TRANSITION_OPEN_DETAIL 6

/* VRAM homes of the UI page image, the UI border image and the loaded entry image. */
#define ZUKAN_PAGE_IMAGE_X 0x140
#define ZUKAN_PAGE_IMAGE_Y 0
#define ZUKAN_BORDER_IMAGE_X 0x340
#define ZUKAN_BORDER_IMAGE_Y 0x100
#define ZUKAN_ENTRY_IMAGE_X 0x380
#define ZUKAN_ENTRY_IMAGE_Y 0x100
#define ZUKAN_UI_CLUT_Y 0x1F2
#define ZUKAN_ENTRY_CLUT_Y 0x1EE

#define ZUKAN_LIST_ROW_HEIGHT 16
#define ZUKAN_LIST_VISIBLE_ROWS 8
#define ZUKAN_FADE_STEPS 6
#define ZUKAN_TRANSITION_FRAMES 8
#define ZUKAN_SCROLL_STEPS 4
#define ZUKAN_DETAIL_TEXT_LINES 12
#define ZUKAN_ENTRY_AVAILABLE_FLAG 0x8000
#define ZUKAN_RESOURCE_ID_MASK 0x7FFF
#define ZUKAN_ENTRY_RESOURCE_BASE 0xBFC
#define ZUKAN_UI_RESOURCE_ID 0x5E3

/* Overlay state. */

extern u8 D_800EC3E0[];

/** @brief String @p index in an archive text section selected by @p field. */
#define ZUKAN_ARCHIVE_TEXT(archive, field, index)                                                                                                              \
    ((u8*)(((ZukanArchiveHeader*)(archive))->field + (*(u16*)((index) * 2 + ((ZukanArchiveHeader*)(archive))->field + (archive)) + (s32)(archive))))

/** @brief Address of the FIELD UI string whose offset pair is @p entry, the @p index-th table entry. */
#define FIELD_UI_TEXT_AT(entry, index) ((entry) - (index) * 2 + (entry)[0] + ((entry)[1] << 8))

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

/* FIELD routines that stay resident while this overlay is loaded. */
void* func_800A88A0(void* packet_cursor, u_long* ordering_table, u8* text, s32 color, s32 x, s32 y, s32 flags);
void* func_800A8B04(u_long* ordering_table, void* packet_cursor, s32 value, s32 color, ZukanPos* position, s32 flags);
void* field_draw_sprite_number(u_long* ordering_table, void* packet_cursor, s32 value, s32 digit_count, ZukanPos* position, s32 flags);
void* field_draw_sprite_glyph(void* packet_cursor, u_long* ordering_table, s32 glyph, ZukanPos* position, s32 flags);

/* Helper routines. */
void zukan_upload_ui_images(s32 work_buffer);
inline s32 zukan_upload_tim(ZukanImageDestination* destinations, TimPrefix* tim);
s32 zukan_handle_input(void);
void zukan_scroll_to_selection(void);
static DR_TPAGE* zukan_emit_draw_mode_5(DR_TPAGE* draw_mode, u_long* ordering_table);
static DR_TPAGE* zukan_emit_draw_mode_1d(DR_TPAGE* draw_mode, u_long* ordering_table);
static DR_TPAGE* zukan_emit_texture_draw_mode(DR_TPAGE* draw_mode, u_long* ordering_table);
void zukan_render_ui(RenderContext* render_ctx);
void* zukan_emit_ui_sprite(SPRT* sprite, u_long* ordering_table, u32 sprite_index, s32 x, s32 y, s32 variant);
void zukan_start_next_entry_transition(void);
void zukan_start_previous_entry_transition(void);
void zukan_update_transition(RenderContext* render_ctx);
void zukan_render_content(RenderContext* render_ctx);
static LINE_F2* zukan_emit_panel_outline(LINE_F2* line, u_long* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color);
void zukan_set_fade_target(s16 red, s16 green, s16 blue, s16 steps);
TILE* zukan_render_fade(TILE* tile, u_long* ordering_table);
void zukan_build_entry_list(s32 category);
void zukan_load_entry(s32 index);
void zukan_load_related_entry(s32 resource_id);
u8* zukan_render_detail_text(u8* packet_cursor, u_long* ordering_table);
void* zukan_render_detail_sprites(SPRT* sprite, u_long* ordering_table);
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
    s32 unused[2];

    g_zukan_category = category;
    g_zukan_work_buffer = (u8*)((work_buffer + 3) & ~3);
    zukan_upload_ui_images((next_buffer = work_buffer + 0x8000, work_buffer));
    field_reset_input_repeat();
    zukan_set_fade_target(ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_STEPS);
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
 */
void zukan_upload_ui_images(s32 work_buffer)
{
    ZukanImageDestination destinations;
    u8* archive = g_zukan_resource_archive;

    destinations.x = ZUKAN_BORDER_IMAGE_X;
    destinations.y = ZUKAN_BORDER_IMAGE_Y;
    destinations.clut_x = 0;
    destinations.clut_y = ZUKAN_UI_CLUT_Y;
    zukan_upload_tim(&destinations, (TimPrefix*)(archive + ((ZukanArchiveHeader*)archive)->border_image_offset));

    destinations.x = ZUKAN_PAGE_IMAGE_X;
    destinations.y = ZUKAN_PAGE_IMAGE_Y;
    destinations.clut_x = 0;
    destinations.clut_y = ZUKAN_UI_CLUT_Y;
    zukan_upload_tim(&destinations, (TimPrefix*)(archive + ((ZukanArchiveHeader*)archive)->page_image_offset));
}

/**
 * @brief Upload a TIM image and its optional CLUT to VRAM.
 * @param destinations VRAM destinations for pixels and CLUT data.
 * @param tim TIM image data.
 * @return The TIM pixel-mode bits from the flags word.
 */
inline s32 zukan_upload_tim(ZukanImageDestination* destinations, TimPrefix* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    TimDimensions* pixel_dimensions;
    s32 mode;

    flags = tim->flags;
    clut_block_size = tim->clut_block.bnum;
    mode = flags & 7;

    if (flags & TIM_FLAG_HAS_CLUT)
    {
        setRECT(&upload_rect, destinations->clut_x, destinations->clut_y, CLUT_ENTRY_COUNT, 1);
        LoadImage(&upload_rect, (u_long*)tim->clut_data);
        pixel_dimensions = &TIM_PIXEL_BLOCK(tim, clut_block_size)->dimensions;
    }
    else
    {
        pixel_dimensions = &tim->clut_block.dimensions;
    }

    setRECT(&upload_rect, destinations->x, destinations->y, pixel_dimensions->width, pixel_dimensions->height);
    LoadImage(&upload_rect, (u_long*)(TIM_PIXEL_BLOCK(tim, clut_block_size) + 1));
    return mode;
}

/**
 * @brief Build the current frame and advance the encyclopedia transition state.
 * @param render_ctx Render context for the current frame.
 */
void zukan_update_frame(RenderContext* render_ctx)
{
    zukan_render_ui(render_ctx);
    zukan_update_transition(render_ctx);
    zukan_render_content(render_ctx);
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
    s32 related_resource_id;

    if (g_zukan_transition_state != ZUKAN_TRANSITION_IDLE)
    {
        return;
    }

    if (g_pad_input & PADstart)
    {
        g_zukan_exit_requested = 1;
        return;
    }

    if (g_zukan_view_mode != ZUKAN_VIEW_DETAIL)
    {
        if (g_zukan_input_blocked != 0)
        {
            return;
        }

        if (g_pad_input & PADRdown)
        {
            g_zukan_exit_requested = 1;
            return;
        }

        selection_moved = 0;
        if (g_pad_input & (PADRright | PADi))
        {
            if (g_zukan_list_entries[g_zukan_selected_entry].resource_id_and_available >> 15)
            {
                field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                zukan_set_fade_target(0, 0, 0, ZUKAN_FADE_STEPS);
                zukan_load_entry(g_zukan_selected_entry);
                g_zukan_transition_state = ZUKAN_TRANSITION_OPEN_DETAIL;
                g_zukan_transition_frame = 0;
            }
            else
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
            }
            return;
        }

        repeat_count = 1;
        if (g_pad_input & PADR1)
        {
            repeat_count = ZUKAN_LIST_VISIBLE_ROWS;
            g_pad_input = PADLdown;
        }
        else if (g_pad_input & PADL1)
        {
            repeat_count = ZUKAN_LIST_VISIBLE_ROWS;
            g_pad_input = PADLup;
        }

        while (repeat_count != 0)
        {
            if (g_pad_input & (PADLdown | PADLright))
            {
                g_zukan_selected_entry++;
                if (g_zukan_selected_entry >= g_zukan_entry_count)
                {
                    g_zukan_selected_entry = 0;
                }
                selection_moved = 1;
            }
            else if (g_pad_input & (PADLleft | PADLup))
            {
                g_zukan_selected_entry--;
                if (g_zukan_selected_entry < 0)
                {
                    g_zukan_selected_entry = g_zukan_entry_count - 1;
                }
                selection_moved = 1;
            }
            if ((g_zukan_selected_entry == g_zukan_entry_count - 1) || (g_zukan_selected_entry == 0))
            {
                repeat_count = 1;
            }
            repeat_count--;
        }

        if (selection_moved != 0)
        {
            field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
            zukan_scroll_to_selection();
        }
        return;
    }

    if (g_pad_input & (PADRdown | PADRright | PADi))
    {
        field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
        zukan_set_fade_target(0, 0, 0, ZUKAN_FADE_STEPS);
        g_zukan_transition_state = ZUKAN_TRANSITION_RETURN_TO_LIST;
        g_zukan_transition_frame = 0;
        return;
    }

    if (g_pad_input & (PADLright | PADR1))
    {
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        zukan_start_next_entry_transition();
        related_resource_id = g_zukan_next_resource_id;
        if ((related_resource_id != 0) && (g_pad_input & PADLright))
        {
            zukan_load_related_entry(related_resource_id);
            return;
        }

        g_zukan_selected_entry++;
        if (g_zukan_selected_entry >= g_zukan_entry_count)
        {
            g_zukan_selected_entry = 0;
        }
        while ((g_zukan_list_entries[g_zukan_selected_entry].resource_id_and_available >> 15) == 0)
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

    if (g_pad_input & (PADLleft | PADL1))
    {
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        zukan_start_previous_entry_transition();
        related_resource_id = g_zukan_previous_resource_id;
        if ((related_resource_id != 0) && (g_pad_input & PADLleft))
        {
            zukan_load_related_entry(related_resource_id);
            return;
        }

        g_zukan_selected_entry--;
        if (g_zukan_selected_entry < 0)
        {
            g_zukan_selected_entry = g_zukan_entry_count - 1;
        }
        while ((g_zukan_list_entries[g_zukan_selected_entry].resource_id_and_available >> 15) == 0)
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
    else if (delta > ZUKAN_LIST_VIEW_HEIGHT - ZUKAN_LIST_ROW_HEIGHT)
    {
        g_zukan_scroll_target_y = selection_y - (ZUKAN_LIST_VIEW_HEIGHT - ZUKAN_LIST_ROW_HEIGHT);
        g_zukan_scroll_steps = ZUKAN_SCROLL_STEPS;
    }
}

/**
 * @brief Append texture-page draw mode 5 to an ordering table.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
static DR_TPAGE* zukan_emit_draw_mode_5(DR_TPAGE* draw_mode, u_long* ordering_table)
{
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, ZUKAN_PAGE_IMAGE_X, ZUKAN_PAGE_IMAGE_Y));
    addPrim(ordering_table, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Append texture-page draw mode 0x1D to an ordering table.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
static DR_TPAGE* zukan_emit_draw_mode_1d(DR_TPAGE* draw_mode, u_long* ordering_table)
{
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, ZUKAN_BORDER_IMAGE_X, ZUKAN_BORDER_IMAGE_Y));
    addPrim(ordering_table, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Append the texture-page draw mode for the loaded encyclopedia image.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
static DR_TPAGE* zukan_emit_texture_draw_mode(DR_TPAGE* draw_mode, u_long* ordering_table)
{
    setDrawTPage(draw_mode, 0, 0, getTPage(g_zukan_image_mode, 0, ZUKAN_ENTRY_IMAGE_X, ZUKAN_ENTRY_IMAGE_Y));
    addPrim(ordering_table, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Emit the fixed encyclopedia UI sprites and fade overlay.
 * @param render_ctx Render context for the current frame.
 */
void zukan_render_ui(RenderContext* render_ctx)
{
    u_long* ordering_table;
    s32 i;
    void* packet_cursor;
    s32 unused[2];

    ordering_table = &render_ctx->ot[ZUKAN_LAYER_NAV_SPRITES];
    packet_cursor = render_ctx->prim_cursor;
    for (i = 0; i < ZUKAN_NAV_SPRITE_COUNT; i++)
    {
        packet_cursor = zukan_emit_ui_sprite(packet_cursor, ordering_table, i, g_zukan_ui_sprites[i].x, g_zukan_ui_sprites[i].y, 0);
    }

    ordering_table = &render_ctx->ot[ZUKAN_LAYER_UI_SPRITES];
    for (i = ZUKAN_NAV_SPRITE_COUNT; i < ZUKAN_UI_SPRITE_COUNT; i++)
    {
        packet_cursor = zukan_emit_ui_sprite(packet_cursor, ordering_table, i, g_zukan_ui_sprites[i].x, g_zukan_ui_sprites[i].y, 1);
    }

    render_ctx->prim_cursor = zukan_render_fade(packet_cursor, &render_ctx->ot[ZUKAN_LAYER_FADE]);
}

/**
 * @brief Emit one fixed UI sprite and its texture-page command.
 * @param sprite Primitive-buffer position receiving the sprite packet.
 * @param ordering_table Ordering table receiving the sprite.
 * @param sprite_index Index of the UI sprite definition.
 * @param x Screen x coordinate.
 * @param y Screen y coordinate.
 * @param variant Sprite-group selector supplied by the caller.
 * @return Primitive-buffer address after the emitted packets.
 */
void* zukan_emit_ui_sprite(SPRT* sprite, u_long* ordering_table, u32 sprite_index, s32 x, s32 y, s32 variant)
{
    ZukanUiSpriteRecord* sprite_record;
    DR_TPAGE* draw_mode;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);

    if (g_zukan_view_mode != ZUKAN_VIEW_DETAIL)
    {
        if (sprite_index < 4)
        {
            SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0x30, 0x30, 0x30));
        }
    }
    else if ((sprite_index == 0) || (sprite_index == 3) || (sprite_index == 5))
    {
        switch (g_zukan_transition_state)
        {
        case ZUKAN_TRANSITION_NEXT_FADE_OUT:
            if (sprite_index == 3)
            {
                SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0xFF, 0xE0, 0xE0));
            }
            break;
        case ZUKAN_TRANSITION_PREVIOUS_FADE_OUT:
            if (sprite_index == 0)
            {
                SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0xFF, 0xE0, 0xE0));
            }
            break;
        case ZUKAN_TRANSITION_RETURN_TO_LIST:
            if (sprite_index == 5)
            {
                SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0xFF, 0xE0, 0xE0));
            }
            break;
        }
    }

    setSprt(sprite);
    setXY0(sprite, x + 8, y);

    sprite_record = &g_zukan_ui_sprites[sprite_index];
    setWH(sprite, (sprite_record->v_clut_and_size >> 14) & 0x1FF, sprite_record->v_clut_and_size >> 23);
    setUV0(sprite, sprite_record->source_and_u >> 8, sprite_record->v_clut_and_size);
    sprite->clut = ((sprite_record->v_clut_and_size >> 8) & 0x3F) | getClut(0, ZUKAN_UI_CLUT_Y);

    addPrim(ordering_table, sprite);

    sprite++;
    draw_mode = (DR_TPAGE*)sprite;
    switch ((u8)sprite_record->source_and_u)
    {
    case 0:
        setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, ZUKAN_PAGE_IMAGE_X, ZUKAN_PAGE_IMAGE_Y));
        addPrim(ordering_table, draw_mode);
        sprite = (SPRT*)(draw_mode + 1);
        break;
    case 1:
        setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, ZUKAN_BORDER_IMAGE_X, ZUKAN_BORDER_IMAGE_Y));
        addPrim(ordering_table, draw_mode);
        sprite = (SPRT*)(draw_mode + 1);
        break;
    }

    return sprite;
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
 * @param render_ctx Render context whose primitive cursor is preserved.
 */
void zukan_update_transition(RenderContext* render_ctx)
{
    void* saved_packet_cursor;
    s32 unused[2];

    if (g_zukan_transition_state != ZUKAN_TRANSITION_IDLE)
    {
        saved_packet_cursor = render_ctx->prim_cursor;
        switch (g_zukan_transition_state)
        {
        case ZUKAN_TRANSITION_NEXT_FADE_OUT:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                zukan_commit_loaded_entry();
                g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_IN;
                g_zukan_transition_frame = 0;
                g_zukan_displayed_entry = g_zukan_selected_entry;
                zukan_set_fade_target(ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_STEPS);
            }
            break;
        case ZUKAN_TRANSITION_NEXT_FADE_IN:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                g_zukan_transition_state = ZUKAN_TRANSITION_IDLE;
            }
            break;
        case ZUKAN_TRANSITION_PREVIOUS_FADE_OUT:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                zukan_commit_loaded_entry();
                g_zukan_transition_state = ZUKAN_TRANSITION_PREVIOUS_FADE_IN;
                g_zukan_transition_frame = 0;
                g_zukan_displayed_entry = g_zukan_selected_entry;
                zukan_set_fade_target(ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_STEPS);
            }
            break;
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
                zukan_set_fade_target(ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_STEPS);
            }
            break;
        case ZUKAN_TRANSITION_OPEN_DETAIL:
            if (++g_zukan_transition_frame == ZUKAN_TRANSITION_FRAMES)
            {
                zukan_commit_loaded_entry();
                g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_IN;
                g_zukan_view_mode = ZUKAN_VIEW_DETAIL;
                g_zukan_transition_frame = 0;
                g_zukan_displayed_entry = g_zukan_selected_entry;
                zukan_set_fade_target(ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_NEUTRAL, ZUKAN_FADE_STEPS);
            }
            break;
        }
        render_ctx->prim_cursor = saved_packet_cursor;
    }
}

/**
 * @brief Draw the encyclopedia entry-list or detail view.
 * @param render_ctx Render context whose primitive cursor is advanced in place.
 */
void zukan_render_content(RenderContext* render_ctx)
{
    s32 unused[2];
    DRAWENV draw_env;
    ZukanPos pos;
    u8* packet_cursor;
    u_long* ordering_table;
    s32 entry_index;
    s32 row_y;

    packet_cursor = render_ctx->prim_cursor;

    if (g_zukan_view_mode != ZUKAN_VIEW_DETAIL)
    {
        POLY_F3* tri;
        TILE* tile;
        u8* env_prim;

        ordering_table = &render_ctx->ot[ZUKAN_LAYER_LIST];

        {
            u8* archive = g_zukan_resource_archive;
            packet_cursor =
                func_800A88A0(packet_cursor, ordering_table, ZUKAN_ARCHIVE_TEXT(archive, category_names_offset, g_zukan_category), 0xA, 0xA0, 0x22, 2);
        }

        if (g_zukan_scroll_y != 0)
        {
            tri = (POLY_F3*)packet_cursor;
            SET_BGR0_PACKED(tri, GPU_COLOR_WORD(0x80, 0x80, 0xF0));
            setPolyF3(tri);
            setSemiTrans(tri, 1);
            tri->x0 = 0x99;
            tri->x1 = 0xA0;
            tri->x2 = 0xA7;
            tri->y1 = 0x2E;
            tri->y0 = tri->y2 = 0x35;
            addPrim(ordering_table, tri);
            packet_cursor += sizeof(POLY_F3);
        }

        if (g_zukan_scroll_y + ZUKAN_LIST_VIEW_HEIGHT < g_zukan_entry_count * ZUKAN_LIST_ROW_HEIGHT)
        {
            tri = (POLY_F3*)packet_cursor;
            SET_BGR0_PACKED(tri, GPU_COLOR_WORD(0x80, 0x80, 0xF0));
            setPolyF3(tri);
            setSemiTrans(tri, 1);
            tri->x0 = 0x99;
            tri->x1 = 0xA0;
            tri->x2 = 0xA7;
            tri->y1 = 0xBB;
            tri->y0 = tri->y2 = 0xB4;
            addPrim(ordering_table, tri);
            packet_cursor += sizeof(POLY_F3);
        }

        if (render_ctx->clear_rect.y != VRAM_BACK_DRAW_Y)
        {
            SetDefDrawEnv(&draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
        }
        else
        {
            SetDefDrawEnv(&draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
        }
        SetDrawEnv((DR_ENV*)packet_cursor, &draw_env);
        addPrim(ordering_table, packet_cursor);
        packet_cursor += sizeof(DR_ENV);

        for (entry_index = 0; entry_index < g_zukan_entry_count; entry_index++)
        {
            row_y = (entry_index * ZUKAN_LIST_ROW_HEIGHT) - g_zukan_scroll_y;
            if ((row_y < -15) || (row_y >= ZUKAN_LIST_VIEW_HEIGHT))
            {
                continue;
            }

            pos.x = 0;
            pos.y = row_y;
            packet_cursor = func_800A8B04(ordering_table, packet_cursor, entry_index + 1, 0, &pos, 0);
            if (g_zukan_list_entries[entry_index].resource_id_and_available >> 15)
            {
                u8* archive = g_zukan_resource_archive;
                packet_cursor = func_800A88A0(packet_cursor, ordering_table,
                                              ZUKAN_ARCHIVE_TEXT(archive, entry_names_offset, g_zukan_list_entries[entry_index].name_index), 0, 0x66, row_y, 2);
            }
            else
            {
                packet_cursor = func_800A88A0(packet_cursor, ordering_table, FIELD_UI_TEXT_AT(D_800EC3E0, 14), 0, 0x66, row_y, 2);
            }
        }

        row_y = (g_zukan_selected_entry * ZUKAN_LIST_ROW_HEIGHT) - g_zukan_scroll_y;
        if (row_y < 0)
        {
            row_y = 0;
        }
        else if (row_y > ZUKAN_LIST_VIEW_HEIGHT - ZUKAN_LIST_ROW_HEIGHT)
        {
            row_y = ZUKAN_LIST_VIEW_HEIGHT - ZUKAN_LIST_ROW_HEIGHT;
        }

        tile = (TILE*)packet_cursor;
        SET_BGR0_PACKED(tile, GPU_COLOR_WORD(0xF0, 0x80, 0xF0));
        setTile(tile);
        setSemiTrans(tile, 1);
        setXY0(tile, 0, row_y);
        setWH(tile, 0xB8, 0xF);
        addPrim(ordering_table, tile);
        packet_cursor += sizeof(TILE);

        env_prim = packet_cursor;
        if (render_ctx->clear_rect.y != VRAM_BACK_DRAW_Y)
        {
            SetDefDrawEnv(&draw_env, ZUKAN_LIST_VIEW_X, SCREEN_HEIGHT + ZUKAN_LIST_VIEW_Y, ZUKAN_LIST_VIEW_WIDTH, ZUKAN_LIST_VIEW_HEIGHT);
        }
        else
        {
            SetDefDrawEnv(&draw_env, ZUKAN_LIST_VIEW_X, VRAM_BACK_DRAW_Y + ZUKAN_LIST_VIEW_Y, ZUKAN_LIST_VIEW_WIDTH, ZUKAN_LIST_VIEW_HEIGHT);
        }
        SetDrawEnv((DR_ENV*)env_prim, &draw_env);
        addPrim(ordering_table, env_prim);
        packet_cursor = env_prim + sizeof(DR_ENV);
    }
    else
    {
        ordering_table = &render_ctx->ot[ZUKAN_LAYER_DETAIL];
        packet_cursor = zukan_render_detail_text(packet_cursor, ordering_table);
        packet_cursor = zukan_render_detail_sprites((SPRT*)packet_cursor, ordering_table);

        pos.x = 0x106;
        pos.y = 0xBD;
        packet_cursor = field_draw_sprite_glyph(packet_cursor, ordering_table, 0xB, &pos, 1);

        pos.x = 0xEE;
        pos.y = 0xBD;
        packet_cursor = field_draw_sprite_number(ordering_table, packet_cursor, g_zukan_displayed_entry + 1, 3, &pos, 0);

        pos.x = 0x10E;
        pos.y = 0xBD;
        packet_cursor = field_draw_sprite_number(ordering_table, packet_cursor, g_zukan_entry_count, 3, &pos, 0);
    }

    render_ctx->prim_cursor = packet_cursor;
}

/**
 * @brief Emit a rectangular four-line panel outline.
 * @param line Next line packet in the primitive buffer.
 * @param ordering_table Ordering table receiving the lines.
 * @param x Left edge.
 * @param y Top edge.
 * @param width Panel width.
 * @param height Panel height.
 * @param color Packed line color.
 * @return First packet after the four outline lines.
 * @see GOLEM golem_emit_panel_outline
 */
static LINE_F2* zukan_emit_panel_outline(LINE_F2* line, u_long* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x, y, x + width, y);
    addPrim(ordering_table, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x + width, y, x + width, y + height);
    addPrim(ordering_table, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x + width, y + height, x, y + height);
    addPrim(ordering_table, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY2(line, x, y, x, y + height);
    addPrim(ordering_table, line);
    return line + 1;
}

/**
 * @brief Set the target color and duration for the screen fade.
 * @param red Target red component.
 * @param green Target green component.
 * @param blue Target blue component.
 * @param steps Number of fade steps.
 * @see GOLEM golem_set_fade_target
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
 * @param tile Next free position in the packet buffer.
 * @param ordering_table Ordering-table tag receiving the fade.
 * @return First primitive after the emitted fade packets.
 * @see GOLEM golem_render_fade
 */
TILE* zukan_render_fade(TILE* tile, u_long* ordering_table)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;
    DR_TPAGE* tpage;

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
            setRGB0(tile, g_zukan_fade_current.red - 1, g_zukan_fade_current.green - 1, g_zukan_fade_current.blue - 1);
        }
        else
        {
            if (g_zukan_fade_current.red == ZUKAN_FADE_NEUTRAL)
            {
                tile->r0 = 0;
            }
            else
            {
                tile->r0 = ~g_zukan_fade_current.red;
            }
            if (g_zukan_fade_current.green == ZUKAN_FADE_NEUTRAL)
            {
                tile->g0 = 0;
            }
            else
            {
                tile->g0 = ~g_zukan_fade_current.green;
            }
            if (g_zukan_fade_current.blue == ZUKAN_FADE_NEUTRAL)
            {
                tile->b0 = 0;
            }
            else
            {
                tile->b0 = ~g_zukan_fade_current.blue;
            }
        }

        setTile(tile);
        setSemiTrans(tile, 1);
        tile->w = SCREEN_WIDTH;
        draw_mode = getTPage(0, 1, ZUKAN_PAGE_IMAGE_X, ZUKAN_PAGE_IMAGE_Y);
        SET_YX0(tile, 0, 0);
        tile->h = SCREEN_HEIGHT;
        addPrim(ordering_table, tile);

        tile++;
        tpage = (DR_TPAGE*)tile;
        if (g_zukan_fade_current.red < ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = getTPage(0, 2, ZUKAN_PAGE_IMAGE_X, ZUKAN_PAGE_IMAGE_Y);
        }
        setDrawTPage(tpage, 0, 0, draw_mode);
        addPrim(ordering_table, tpage);

        tile = (TILE*)(tpage + 1);
    }
    return tile;
}

/**
 * @brief Build the visible entry list for an encyclopedia category.
 * @param category Category index to enumerate.
 */
void zukan_build_entry_list(s32 category)
{
    s32 resource_ids[0x200];
    s32 name_indices[0x200];
    s32 i;

    g_zukan_entry_count = zukan_build_category_entries(category, resource_ids, name_indices);
    for (i = 0; i < g_zukan_entry_count; i++)
    {
        if (resource_ids[i] != 0)
        {
            g_zukan_list_entries[i].resource_id_and_available |= ZUKAN_ENTRY_AVAILABLE_FLAG;
            g_zukan_list_entries[i].resource_id_and_available =
                (g_zukan_list_entries[i].resource_id_and_available & ZUKAN_ENTRY_AVAILABLE_FLAG) | ((u16)resource_ids[i] & ZUKAN_RESOURCE_ID_MASK);
            g_zukan_list_entries[i].name_index = name_indices[i];
        }
        else
        {
            g_zukan_list_entries[i].resource_id_and_available = 0;
            g_zukan_list_entries[i].name_index = 0;
        }
    }
}

/**
 * @brief Request the resource for a visible encyclopedia entry.
 * @param index Visible entry index.
 */
void zukan_load_entry(s32 index)
{
    cdrom_queue_read((g_zukan_list_entries[index].resource_id_and_available & ZUKAN_RESOURCE_ID_MASK) + ZUKAN_ENTRY_RESOURCE_BASE, g_zukan_resource_buffer);
}

/**
 * @brief Request an entry resource referenced by the current detail record.
 * @param resource_id Related resource identifier.
 */
void zukan_load_related_entry(s32 resource_id)
{
    u8* resource_buffer = g_zukan_resource_buffer;

    cdrom_queue_read((resource_id + ZUKAN_ENTRY_RESOURCE_BASE) & 0xFFFF, resource_buffer);
}

/**
 * @brief Draw the text lines for the active encyclopedia detail record.
 * @param packet_cursor Current primitive-buffer address.
 * @param ordering_table Ordering table receiving text primitives.
 * @return Primitive-buffer address after the text primitives.
 */
u8* zukan_render_detail_text(u8* packet_cursor, u_long* ordering_table)
{
    s32 i;
    s32 line_y;
    u16* offsets;
    u16* line_offsets;
    u8* text;
    u8 unused[0x100];

    line_offsets = (u16*)(g_zukan_work_buffer + ((ZukanEntryResourceHeader*)g_zukan_work_buffer)->text_offset);
    offsets = line_offsets;
    line_y = 26;
    for (i = 0; i < ZUKAN_DETAIL_TEXT_LINES; i++)
    {
        s32 x;

        text = (u8*)line_offsets + *offsets;
        x = 48;
        while (*text == ' ')
        {
            text++;
            x += 12;
        }
        packet_cursor = func_800A88A0(packet_cursor, ordering_table, text, 0, x, line_y, 0);
        offsets++;
        line_y += 13;
    }
    return packet_cursor;
}

/**
 * @brief Draw the sprite records embedded in the active detail resource.
 * @param sprite Current primitive-buffer position interpreted as a sprite packet.
 * @param ordering_table Ordering table receiving the sprite packets.
 * @return Primitive-buffer address after the emitted sprites and draw mode.
 */
void* zukan_render_detail_sprites(SPRT* sprite, u_long* ordering_table)
{
    s32 command;
    u8* resource_data = g_zukan_work_buffer;
    u8* sprite_data = resource_data + ((ZukanEntryResourceHeader*)resource_data)->sprites_offset;
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
            sprite->x0 = *(u16*)sprite_data;
            sprite_data += 2;
            sprite->y0 = *(u16*)sprite_data;
            sprite_data += 2;
            sprite->u0 = *sprite_data;
            sprite_data += 2;
            sprite->v0 = *sprite_data;
            sprite_data += 2;
            sprite->w = *(u16*)sprite_data;
            sprite_data += 2;
            sprite->h = *(u16*)sprite_data;
            sprite_data += 2;
            if (g_zukan_image_mode != 0)
            {
                sprite->clut = getClut(0, ZUKAN_ENTRY_CLUT_Y);
            }
            else
            {
                sprite->clut = (*(u16*)sprite_data & 0x3F) | getClut(0, ZUKAN_ENTRY_CLUT_Y);
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
        setDrawTPage(mode, 0, 0, getTPage(g_zukan_image_mode, 0, ZUKAN_ENTRY_IMAGE_X, ZUKAN_ENTRY_IMAGE_Y));
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
    ZukanEntryResourceHeader* resource;
    u8* src;
    u8* dst;
    u8* end;

    cdrom_wait_queue_empty();

    resource = (ZukanEntryResourceHeader*)g_zukan_resource_buffer;
    dst = g_zukan_work_buffer;
    end = (u8*)resource + resource->image_offset;
    src = (u8*)resource;
    while (src != end)
    {
        *dst++ = *src++;
    }

    destinations.x = ZUKAN_ENTRY_IMAGE_X;
    destinations.y = ZUKAN_ENTRY_IMAGE_Y;
    destinations.clut_x = 0;
    destinations.clut_y = ZUKAN_ENTRY_CLUT_Y;
    g_zukan_image_mode =
        zukan_upload_tim(&destinations, (TimPrefix*)(g_zukan_resource_buffer + ((ZukanEntryResourceHeader*)g_zukan_resource_buffer)->image_offset));

    {
        ZukanEntryResourceHeader* loaded = (ZukanEntryResourceHeader*)g_zukan_resource_buffer;

        g_zukan_previous_resource_id = *(u16*)((u8*)loaded + loaded->related_ids_offset);
        g_zukan_next_resource_id = *(u16*)((u8*)loaded + loaded->related_ids_offset + 2);
    }
}

/**
 * @brief Load the encyclopedia UI resource and upload its image to VRAM.
 */
void zukan_load_ui_resource(void)
{
    RECT rect;
    s32 packed_dimensions;
    s32 dimension;
    s16 width;
    u8* archive = g_zukan_resource_archive;

    cdrom_queue_read(ZUKAN_UI_RESOURCE_ID, archive);

    packed_dimensions = *(volatile s32*)archive;
    width = packed_dimensions;
    dimension = width;

    *(volatile s16*)&rect.x = ZUKAN_BORDER_IMAGE_X;
    rect.y = ZUKAN_BORDER_IMAGE_Y;
    rect.w = dimension;

    dimension = packed_dimensions >> 16;
    rect.h = dimension;

    LoadImage(&rect, (u_long*)(archive + 4));
}
