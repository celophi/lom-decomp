#include "common.h"
#include <libetc.h>
#include <libgpu.h>

#include "common/gpu_packet.h"
#include "common/render_context.h"
#include "common/screen_fade.h"
#include "common/tim.h"
#include "internal/zukan.h"
#include "internal/zukan_category.h"
#include "main/audio/akao.h"
#include "main/cd_resources.h"
#include "main/cdrom.h"
#include "main/controller.h"
#include "main/display.h"
#include "main/main.h"
#include "overlays/field/field_input.h"
#include "overlays/field/field_numeric_sprites.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_text.h"
#include "overlays/field/field_ui_text.h"
#include "overlays/menu/menu.h"
#include "overlays/zukan/zukan.h"

/*
 * Encyclopedia overlay UI, entry navigation, resource loading and rendering.
 */

/* Ordering-table slots used in the in-game RenderContext. NAV_SPRITES holds UI sprites 0-5
 * (the previous/next/return controls tinted during transitions), UI_SPRITES the remaining ones. */
#define ZUKAN_LAYER_NAV_SPRITES 10
#define ZUKAN_LAYER_FADE 11
#define ZUKAN_LAYER_LIST 12
#define ZUKAN_LAYER_DETAIL 13
#define ZUKAN_LAYER_UI_SPRITES 15
#define ZUKAN_NAV_SPRITE_COUNT 6
#define ZUKAN_UI_SPRITE_COUNT 21

/* UI sprites tinted during entry transitions. Sprites below ZUKAN_UI_SPRITE_PAGE_CONTROLS
 * are dimmed in the list view, where paging is unavailable. */
#define ZUKAN_UI_SPRITE_PREVIOUS 0
#define ZUKAN_UI_SPRITE_NEXT 3
#define ZUKAN_UI_SPRITE_PAGE_CONTROLS 4
#define ZUKAN_UI_SPRITE_RETURN 5
#define ZUKAN_TINT_DIM GPU_COLOR_WORD(0x30, 0x30, 0x30)
#define ZUKAN_TINT_HIGHLIGHT GPU_COLOR_WORD(0xFF, 0xE0, 0xE0)

/* ZukanUiSpriteRecord image values. */
#define ZUKAN_UI_IMAGE_PAGE 0
#define ZUKAN_UI_IMAGE_BORDER 1

#define ZUKAN_SCROLL_ARROW_COLOR GPU_COLOR_WORD(0x80, 0x80, 0xF0)
#define ZUKAN_SELECTION_COLOR GPU_COLOR_WORD(0xF0, 0x80, 0xF0)
#define ZUKAN_PAGE_NUMBER_Y 189

/* Scrolling list viewport, relative to the active draw buffer. */
#define ZUKAN_LIST_VIEW_X 72
#define ZUKAN_LIST_VIEW_Y 54
#define ZUKAN_LIST_VIEW_WIDTH 184
#define ZUKAN_LIST_VIEW_HEIGHT 128
#define ZUKAN_LIST_NAME_X 102

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
#define ZUKAN_ENTRY_CLUT_Y 0x1EE

/* The field's scene textures fill 256-line pixel columns, grouped into 64-pixel texture pages.
 * From this page count on they reach the border image position. */
#define ZUKAN_FIELD_TEXTURE_COLUMN_BYTES (256 * 2)
#define ZUKAN_FIELD_TEXTURE_PAGE_WIDTH 64
#define ZUKAN_FIELD_TEXTURE_PAGES_AT_BORDER 9

#define ZUKAN_WORK_BUFFER_SIZE 0x8000
#define ZUKAN_LIST_CAPACITY 512
#define ZUKAN_LIST_ROW_HEIGHT 16
#define ZUKAN_LIST_VISIBLE_ROWS 8
#define ZUKAN_TRANSITION_FRAMES 8
#define ZUKAN_SCROLL_STEPS 4
#define ZUKAN_DETAIL_TEXT_LINES 12
#define ZUKAN_ENTRY_RESOURCE_BASE 0xBFC

/** @brief Address of the section that @p field locates, as a byte offset from @p header. */
#define ZUKAN_SECTION(header, field) ((u8*)(header) + (header)->field)

/** @brief The related-entry ids of the entry resource @p header. */
#define ZUKAN_RELATED_ENTRIES(header) ((ZukanRelatedEntries*)ZUKAN_SECTION(header, related_ids_offset))

/** @brief The UI archive viewed through its header. */
#define ZUKAN_ARCHIVE ((ZukanArchiveHeader*)g_zukan_resource_archive)

/** @brief String @p index in the UI archive text section selected by @p field. */
#define ZUKAN_ARCHIVE_TEXT(field, index) (ZUKAN_SECTION(ZUKAN_ARCHIVE, field) + ((u16*)ZUKAN_SECTION(ZUKAN_ARCHIVE, field))[index])

/** @brief Offsets of the images and text tables in the UI archive. */
typedef struct
{
    s32 section_count;
    s32 page_image_offset;
    s32 border_image_offset;
    s32 entry_names_offset;
    s32 category_names_offset;
} ZukanArchiveHeader;

/** @brief One visible entry's name index, page resource ID and unlock state. */
typedef struct
{
    u16 name_index;
    u16 resource_id : 15;
    u16 available : 1;
} ZukanListEntry;

/** @brief One fixed UI sprite's source image, UV, palette, dimensions and screen position. */
typedef struct
{
    u32 image : 8;
    u32 u : 24;
    u32 v : 8;
    u32 clut_column : 6; /**< Palette column, in 16-entry units, on row VRAM_CLUT_Y. */
    u32 width : 9;
    u32 height : 9;
    u16 x;
    u16 y;
} ZukanUiSpriteRecord;

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

/** @brief Image resource uploaded by zukan_load_ui_resource: a size word followed by the pixels. */
typedef struct
{
    s32 packed_size; /**< Width in the low half, height in the high half. */
    u_long pixels[1];
} ZukanUiImageResource;

/** @brief Resource ids of the entries linked from a detail page. */
typedef struct
{
    u16 previous;
    u16 next;
} ZukanRelatedEntries;

extern u32 g_field_vram_byte_count;
extern u8 g_zukan_resource_archive[];
extern ZukanEntryResourceHeader* g_zukan_resource_buffer;
extern RenderContext* g_zukan_render_buffers;
extern ZukanEntryResourceHeader* g_zukan_work_buffer;
extern s32 g_zukan_exit_requested;
extern ZukanUiSpriteRecord g_zukan_ui_sprites[ZUKAN_UI_SPRITE_COUNT];
extern ZukanListEntry g_zukan_list_entries[ZUKAN_LIST_CAPACITY];
extern s16 g_zukan_input_blocked;
extern s32 g_zukan_category;
extern s32 g_zukan_entry_count;
extern s32 g_zukan_previous_resource_id;
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

void zukan_upload_ui_images(u8* work_buffer);
inline s32 zukan_upload_tim(TimUploadDestination* destinations, TimPrefix* tim);
s32 zukan_handle_input(void);
void zukan_scroll_to_selection(void);
inline void* zukan_emit_page_image_tpage(void* packet_cursor, u_long* ordering_table);
inline void* zukan_emit_border_image_tpage(void* packet_cursor, u_long* ordering_table);
inline void* zukan_emit_entry_image_tpage(void* packet_cursor, u_long* ordering_table);
void zukan_render_ui(RenderContext* render_ctx);
void* zukan_emit_ui_sprite(SPRT* sprite, u_long* ordering_table, u32 sprite_index, s32 x, s32 y, s32 variant);
void zukan_start_next_entry_transition(void);
void zukan_start_previous_entry_transition(void);
void zukan_update_transition(RenderContext* render_ctx);
void zukan_render_content(RenderContext* render_ctx);
void zukan_build_entry_list(s32 category);
void zukan_load_entry(s32 index);
void zukan_load_related_entry(s32 resource_id);
u8* zukan_render_detail_text(u8* packet_cursor, u_long* ordering_table);
void* zukan_render_detail_sprites(SPRT* sprite, u_long* ordering_table);
void zukan_commit_loaded_entry(void);

/**
 * @brief Run the encyclopedia screen until the player leaves it.
 * @param render_buffers Storage for the two render contexts, followed by the encyclopedia work area.
 * @param category Encyclopedia category to display.
 */
void zukan_run(RenderContext* render_buffers, s32 category)
{
    RenderContext* draw_buffer;
    RenderContext* next_buffer;
    RenderContext* other_buffer;
    RenderContext* buffers;
    s32 texture_pages;

    g_zukan_render_buffers = render_buffers;
    render_buffers += 2;

    setRECT(&g_zukan_render_buffers[0].clear_rect, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    setRECT(&g_zukan_render_buffers[1].clear_rect, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    SetDefDispEnv(&g_zukan_render_buffers[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_zukan_render_buffers[1].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_zukan_render_buffers[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_zukan_render_buffers[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    buffers = g_zukan_render_buffers;
    buffers[1].draw_env.dtd = 0;
    buffers[0].draw_env.dtd = 0;
    g_zukan_exit_requested = 0;
    g_zukan_render_buffers[0].frame_parity = 0;
    g_zukan_render_buffers[1].frame_parity = 1;

    g_zukan_resource_buffer = (ZukanEntryResourceHeader*)ALIGN4(zukan_initialize_state((u8*)render_buffers, category));

    next_buffer = g_zukan_render_buffers;
    ClearOTagR(next_buffer->ot, RENDER_CONTEXT_OT_ENTRY_COUNT);
    ClearOTagR(g_zukan_render_buffers[1].ot, RENDER_CONTEXT_OT_ENTRY_COUNT);
    VSync(0);
    PutDispEnv(&next_buffer->disp_env);
    update_controllers();

    while (1)
    {
        draw_buffer = next_buffer;
        ClearOTagR(draw_buffer->ot, RENDER_CONTEXT_OT_ENTRY_COUNT);
        draw_buffer->prim_cursor = &draw_buffer->ot[RENDER_CONTEXT_OT_ENTRY_COUNT];
        field_text_reset_scratch();
        field_update_input_repeat();
        zukan_update_frame(draw_buffer);
        field_text_upload_immediate_cache();
        DrawSync(0);
        set_controller_vsync_interval(FRAME_VSYNC_INTERVAL);
        VSync(FRAME_VSYNC_INTERVAL);

        if (g_zukan_exit_requested != 0)
        {
            break;
        }

        ClearImage(&draw_buffer->clear_rect, 0, 0, 0);
        other_buffer = g_zukan_render_buffers;

        if (draw_buffer == g_zukan_render_buffers)
        {
            other_buffer = draw_buffer + 1;
        }

        next_buffer = other_buffer;
        PutDispEnv(&other_buffer->disp_env);
        PutDrawEnv(&next_buffer->draw_env);
        DrawOTag(&draw_buffer->ot[RENDER_CONTEXT_OT_ENTRY_COUNT - 1]);
        update_controllers();
        cdrom_process_state();
    }

    texture_pages = g_field_vram_byte_count / ZUKAN_FIELD_TEXTURE_COLUMN_BYTES;
    texture_pages = (texture_pages + ZUKAN_FIELD_TEXTURE_PAGE_WIDTH - 1) / ZUKAN_FIELD_TEXTURE_PAGE_WIDTH;

    if (texture_pages >= ZUKAN_FIELD_TEXTURE_PAGES_AT_BORDER)
    {
        zukan_load_ui_resource();
    }

    field_reset_input_repeat();
    field_text_reset_windows();
}

/**
 * @brief Initialize encyclopedia state and return the next free work-buffer address.
 * @param work_buffer Start of the overlay work buffer.
 * @param category Encyclopedia category to display.
 * @return First address after the encyclopedia's reserved work area.
 */
u8* zukan_initialize_state(u8* work_buffer, s32 category)
{
    u8* next_buffer;
    s32 unused[2];

    g_zukan_category = category;
    g_zukan_work_buffer = (ZukanEntryResourceHeader*)ALIGN4(work_buffer);
    next_buffer = work_buffer + ZUKAN_WORK_BUFFER_SIZE;
    zukan_upload_ui_images(work_buffer);
    field_reset_input_repeat();
    set_screen_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_STEPS);
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
void zukan_upload_ui_images(u8* work_buffer)
{
    TimUploadDestination destinations;
    ZukanArchiveHeader* archive = ZUKAN_ARCHIVE;

    destinations.pixel_x = ZUKAN_BORDER_IMAGE_X;
    destinations.pixel_y = ZUKAN_BORDER_IMAGE_Y;
    destinations.clut_x = 0;
    destinations.clut_y = VRAM_CLUT_Y;
    zukan_upload_tim(&destinations, (TimPrefix*)ZUKAN_SECTION(archive, border_image_offset));

    destinations.pixel_x = ZUKAN_PAGE_IMAGE_X;
    destinations.pixel_y = ZUKAN_PAGE_IMAGE_Y;
    destinations.clut_x = 0;
    destinations.clut_y = VRAM_CLUT_Y;
    zukan_upload_tim(&destinations, (TimPrefix*)ZUKAN_SECTION(archive, page_image_offset));
}

/**
 * @brief Upload a TIM image and its optional CLUT to VRAM.
 * @param destinations VRAM destinations for pixels and CLUT data.
 * @param tim TIM image data.
 * @return The TIM pixel-mode bits from the flags word.
 */
inline s32 zukan_upload_tim(TimUploadDestination* destinations, TimPrefix* tim)
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

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y, pixel_dimensions->width, pixel_dimensions->height);
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
    g_frame_counter++;
    zukan_handle_input();

    if (g_zukan_scroll_steps != 0)
    {
        g_zukan_scroll_y += (g_zukan_scroll_target_y - g_zukan_scroll_y) / g_zukan_scroll_steps;
        g_zukan_scroll_steps--;
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
            if (g_zukan_list_entries[g_zukan_selected_entry].available)
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                set_screen_fade_target(FADE_BLACK, FADE_BLACK, FADE_BLACK, SCREEN_FADE_STEPS);
                zukan_load_entry(g_zukan_selected_entry);
                g_zukan_transition_state = ZUKAN_TRANSITION_OPEN_DETAIL;
                g_zukan_transition_frame = 0;
            }
            else
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
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

        for (; repeat_count != 0; repeat_count--)
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
        }

        if (selection_moved != 0)
        {
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            zukan_scroll_to_selection();
        }

        return;
    }

    if (g_pad_input & (PADRdown | PADRright | PADi))
    {
        field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
        set_screen_fade_target(FADE_BLACK, FADE_BLACK, FADE_BLACK, SCREEN_FADE_STEPS);
        g_zukan_transition_state = ZUKAN_TRANSITION_RETURN_TO_LIST;
        g_zukan_transition_frame = 0;
        return;
    }

    if (g_pad_input & (PADLright | PADR1))
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
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

        while (!g_zukan_list_entries[g_zukan_selected_entry].available)
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
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
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

        while (!g_zukan_list_entries[g_zukan_selected_entry].available)
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
    s32 row_y = g_zukan_selected_entry * ZUKAN_LIST_ROW_HEIGHT - g_zukan_scroll_y;

    if (row_y < 0)
    {
        g_zukan_scroll_target_y = g_zukan_selected_entry * ZUKAN_LIST_ROW_HEIGHT;
        g_zukan_scroll_steps = ZUKAN_SCROLL_STEPS;
    }
    else if (row_y > ZUKAN_LIST_VIEW_HEIGHT - ZUKAN_LIST_ROW_HEIGHT)
    {
        g_zukan_scroll_target_y = g_zukan_selected_entry * ZUKAN_LIST_ROW_HEIGHT - (ZUKAN_LIST_VIEW_HEIGHT - ZUKAN_LIST_ROW_HEIGHT);
        g_zukan_scroll_steps = ZUKAN_SCROLL_STEPS;
    }
}

/**
 * @brief Append the texture-page command for the UI page image.
 * @param packet_cursor Primitive-buffer position receiving the packet.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
inline void* zukan_emit_page_image_tpage(void* packet_cursor, u_long* ordering_table)
{
    DR_TPAGE* draw_mode = packet_cursor;

    setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, ZUKAN_PAGE_IMAGE_X, ZUKAN_PAGE_IMAGE_Y));
    addPrim(ordering_table, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Append the texture-page command for the UI border image.
 * @param packet_cursor Primitive-buffer position receiving the packet.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
inline void* zukan_emit_border_image_tpage(void* packet_cursor, u_long* ordering_table)
{
    DR_TPAGE* draw_mode = packet_cursor;

    setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, ZUKAN_BORDER_IMAGE_X, ZUKAN_BORDER_IMAGE_Y));
    addPrim(ordering_table, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Append the texture-page command for the loaded entry image.
 * @param packet_cursor Primitive-buffer position receiving the packet.
 * @param ordering_table Ordering-table tag address.
 * @return Primitive-buffer address after the emitted packet.
 */
inline void* zukan_emit_entry_image_tpage(void* packet_cursor, u_long* ordering_table)
{
    DR_TPAGE* draw_mode = packet_cursor;

    setDrawTPage(draw_mode, 0, 0, getTPage(g_zukan_image_mode, GPU_BLEND_HALF, ZUKAN_ENTRY_IMAGE_X, ZUKAN_ENTRY_IMAGE_Y));
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

    render_ctx->prim_cursor = render_screen_fade(packet_cursor, &render_ctx->ot[ZUKAN_LAYER_FADE]);
}

/**
 * @brief Emit one fixed UI sprite and its texture-page command.
 * @param sprite Primitive-buffer position receiving the sprite packet.
 * @param ordering_table Ordering table receiving the sprite.
 * @param sprite_index Index of the UI sprite definition.
 * @param x Screen x coordinate.
 * @param y Screen y coordinate.
 * @param variant Unused; 0 for the navigation sprites and 1 for the others.
 * @return Primitive-buffer address after the emitted packets.
 */
void* zukan_emit_ui_sprite(SPRT* sprite, u_long* ordering_table, u32 sprite_index, s32 x, s32 y, s32 variant)
{
    ZukanUiSpriteRecord* sprite_record;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);

    if (g_zukan_view_mode != ZUKAN_VIEW_DETAIL)
    {
        if (sprite_index < ZUKAN_UI_SPRITE_PAGE_CONTROLS)
        {
            SET_BGR0_PACKED(sprite, ZUKAN_TINT_DIM);
        }
    }
    else if ((sprite_index == ZUKAN_UI_SPRITE_PREVIOUS) || (sprite_index == ZUKAN_UI_SPRITE_NEXT) || (sprite_index == ZUKAN_UI_SPRITE_RETURN))
    {
        switch (g_zukan_transition_state)
        {
            case ZUKAN_TRANSITION_NEXT_FADE_OUT:
                if (sprite_index == ZUKAN_UI_SPRITE_NEXT)
                {
                    SET_BGR0_PACKED(sprite, ZUKAN_TINT_HIGHLIGHT);
                }

                break;
            case ZUKAN_TRANSITION_PREVIOUS_FADE_OUT:
                if (sprite_index == ZUKAN_UI_SPRITE_PREVIOUS)
                {
                    SET_BGR0_PACKED(sprite, ZUKAN_TINT_HIGHLIGHT);
                }

                break;
            case ZUKAN_TRANSITION_RETURN_TO_LIST:
                if (sprite_index == ZUKAN_UI_SPRITE_RETURN)
                {
                    SET_BGR0_PACKED(sprite, ZUKAN_TINT_HIGHLIGHT);
                }

                break;
        }
    }

    setSprt(sprite);
    setXY0(sprite, x + 8, y);

    sprite_record = &g_zukan_ui_sprites[sprite_index];
    setWH(sprite, sprite_record->width, sprite_record->height);
    setUV0(sprite, sprite_record->u, sprite_record->v);
    setClut(sprite, sprite_record->clut_column * 16, VRAM_CLUT_Y);

    addPrim(ordering_table, sprite);

    sprite++;

    switch (sprite_record->image)
    {
        case ZUKAN_UI_IMAGE_PAGE:
            sprite = zukan_emit_page_image_tpage(sprite, ordering_table);
            break;
        case ZUKAN_UI_IMAGE_BORDER:
            sprite = zukan_emit_border_image_tpage(sprite, ordering_table);
            break;
    }

    return sprite;
}

/**
 * @brief Begin the fade transition to the next encyclopedia entry.
 */
void zukan_start_next_entry_transition(void)
{
    set_screen_fade_target(FADE_BLACK, FADE_BLACK, FADE_BLACK, SCREEN_FADE_STEPS);
    g_zukan_transition_state = ZUKAN_TRANSITION_NEXT_FADE_OUT;
    g_zukan_transition_frame = 0;
}

/**
 * @brief Begin the fade transition to the previous encyclopedia entry.
 */
void zukan_start_previous_entry_transition(void)
{
    set_screen_fade_target(FADE_BLACK, FADE_BLACK, FADE_BLACK, SCREEN_FADE_STEPS);
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
                    set_screen_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_STEPS);
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
                    set_screen_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_STEPS);
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
                    set_screen_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_STEPS);
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
                    set_screen_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_STEPS);
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
    Vec2s pos;
    u8* packet_cursor;
    u_long* ordering_table;
    s32 entry_index;
    s32 row_y;
    POLY_F3* tri;
    TILE* tile;
    u8* env_prim;

    packet_cursor = render_ctx->prim_cursor;

    if (g_zukan_view_mode != ZUKAN_VIEW_DETAIL)
    {
        ordering_table = &render_ctx->ot[ZUKAN_LAYER_LIST];

        packet_cursor = field_draw_text((SPRT*)packet_cursor, ordering_table, ZUKAN_ARCHIVE_TEXT(category_names_offset, g_zukan_category), 10, SCREEN_WIDTH / 2,
                                        34, FIELD_TEXT_ALIGN_CENTER);

        /* Arrows above and below the list while more rows are scrolled out of view. */
        if (g_zukan_scroll_y != 0)
        {
            tri = (POLY_F3*)packet_cursor;
            SET_BGR0_PACKED(tri, ZUKAN_SCROLL_ARROW_COLOR);
            setPolyF3(tri);
            setSemiTrans(tri, 1);
            tri->x0 = SCREEN_WIDTH / 2 - 7;
            tri->x1 = SCREEN_WIDTH / 2;
            tri->x2 = SCREEN_WIDTH / 2 + 7;
            tri->y1 = 46;
            tri->y0 = tri->y2 = 53;
            addPrim(ordering_table, tri);
            packet_cursor += sizeof(POLY_F3);
        }

        if (g_zukan_scroll_y + ZUKAN_LIST_VIEW_HEIGHT < g_zukan_entry_count * ZUKAN_LIST_ROW_HEIGHT)
        {
            tri = (POLY_F3*)packet_cursor;
            SET_BGR0_PACKED(tri, ZUKAN_SCROLL_ARROW_COLOR);
            setPolyF3(tri);
            setSemiTrans(tri, 1);
            tri->x0 = SCREEN_WIDTH / 2 - 7;
            tri->x1 = SCREEN_WIDTH / 2;
            tri->x2 = SCREEN_WIDTH / 2 + 7;
            tri->y1 = 187;
            tri->y0 = tri->y2 = 180;
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

            if ((row_y <= -ZUKAN_LIST_ROW_HEIGHT) || (row_y >= ZUKAN_LIST_VIEW_HEIGHT))
            {
                continue;
            }

            pos.x = 0;
            pos.y = row_y;
            packet_cursor = field_draw_number_wide(ordering_table, (SPRT*)packet_cursor, entry_index + 1, 0, &pos, FIELD_TEXT_ALIGN_LEFT);

            if (g_zukan_list_entries[entry_index].available)
            {
                packet_cursor =
                    field_draw_text((SPRT*)packet_cursor, ordering_table, ZUKAN_ARCHIVE_TEXT(entry_names_offset, g_zukan_list_entries[entry_index].name_index),
                                    0, ZUKAN_LIST_NAME_X, row_y, FIELD_TEXT_ALIGN_CENTER);
            }
            else
            {
                packet_cursor = field_draw_text((SPRT*)packet_cursor, ordering_table, FIELD_UI_TEXT_AT(g_field_ui_text_dashes, FIELD_UI_TEXT_DASHES), 0,
                                                ZUKAN_LIST_NAME_X, row_y, FIELD_TEXT_ALIGN_CENTER);
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
        SET_BGR0_PACKED(tile, ZUKAN_SELECTION_COLOR);
        setTile(tile);
        setSemiTrans(tile, 1);
        setXY0(tile, 0, row_y);
        setWH(tile, ZUKAN_LIST_VIEW_WIDTH, ZUKAN_LIST_ROW_HEIGHT - 1);
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

        pos.x = 262;
        pos.y = ZUKAN_PAGE_NUMBER_Y;
        packet_cursor = field_draw_sprite_glyph((SPRT*)packet_cursor, ordering_table, FIELD_DIGIT_GLYPH_SEPARATOR, &pos, 1);

        pos.x = 238;
        pos.y = ZUKAN_PAGE_NUMBER_Y;
        packet_cursor = field_draw_sprite_number(ordering_table, packet_cursor, g_zukan_displayed_entry + 1, 3, &pos, 0);

        pos.x = 270;
        pos.y = ZUKAN_PAGE_NUMBER_Y;
        packet_cursor = field_draw_sprite_number(ordering_table, packet_cursor, g_zukan_entry_count, 3, &pos, 0);
    }

    render_ctx->prim_cursor = packet_cursor;
}

#include "../../common/panel_outline/emit_panel_outline.inc.c"
#include "../../common/screen_fade/set_screen_fade_target.inc.c"
#include "../../common/screen_fade/render_screen_fade.inc.c"

/**
 * @brief Build the visible entry list for an encyclopedia category.
 * @param category Category index to enumerate.
 */
void zukan_build_entry_list(s32 category)
{
    s32 resource_ids[ZUKAN_LIST_CAPACITY];
    s32 name_indices[ZUKAN_LIST_CAPACITY];
    s32 i;

    g_zukan_entry_count = zukan_build_category_entries(category, resource_ids, name_indices);

    for (i = 0; i < g_zukan_entry_count; i++)
    {
        if (resource_ids[i] != 0)
        {
            g_zukan_list_entries[i].available = 1;
            g_zukan_list_entries[i].resource_id = resource_ids[i];
            g_zukan_list_entries[i].name_index = name_indices[i];
        }
        else
        {
            g_zukan_list_entries[i].available = 0;
            g_zukan_list_entries[i].resource_id = 0;
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
    cdrom_queue_read(g_zukan_list_entries[index].resource_id + ZUKAN_ENTRY_RESOURCE_BASE, g_zukan_resource_buffer);
}

/**
 * @brief Request an entry resource referenced by the current detail record.
 * @param resource_id Related resource identifier.
 */
void zukan_load_related_entry(s32 resource_id)
{
    ZukanEntryResourceHeader* resource = g_zukan_resource_buffer;

    cdrom_queue_read((resource_id + ZUKAN_ENTRY_RESOURCE_BASE) & 0xFFFF, resource);
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
    s32 x;

    line_offsets = (u16*)ZUKAN_SECTION(g_zukan_work_buffer, text_offset);
    offsets = line_offsets;
    line_y = 26;

    for (i = 0; i < ZUKAN_DETAIL_TEXT_LINES; i++)
    {
        text = (u8*)line_offsets + *offsets;
        x = 48;

        while (*text == ' ')
        {
            text++;
            x += 12;
        }

        packet_cursor = field_draw_text((SPRT*)packet_cursor, ordering_table, text, 0, x, line_y, FIELD_TEXT_ALIGN_LEFT);
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
    u16* sprite_data = (u16*)ZUKAN_SECTION(g_zukan_work_buffer, sprites_offset);
    s32 count = sprite_data[0] + (sprite_data[1] << 8);

    sprite_data += 2;

    while (count != 0)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        sprite->x0 = *sprite_data++;
        sprite->y0 = *sprite_data++;
        sprite->u0 = *sprite_data++;
        sprite->v0 = *sprite_data++;
        sprite->w = *sprite_data++;
        sprite->h = *sprite_data++;

        if (g_zukan_image_mode != 0)
        {
            setClut(sprite, 0, ZUKAN_ENTRY_CLUT_Y);
        }
        else
        {
            setClut(sprite, *sprite_data * 16, ZUKAN_ENTRY_CLUT_Y);
        }

        sprite_data += 2;
        addPrim(ordering_table, sprite);
        sprite++;
        count--;
    }

    return zukan_emit_entry_image_tpage(sprite, ordering_table);
}

/**
 * @brief Copy the current encyclopedia resource, upload its TIM image, and cache its metadata.
 */
void zukan_commit_loaded_entry(void)
{
    TimUploadDestination destinations;
    ZukanEntryResourceHeader* resource;
    u8* src;
    u8* dst;
    u8* end;

    cdrom_wait_queue_empty();

    resource = g_zukan_resource_buffer;
    dst = (u8*)g_zukan_work_buffer;
    end = ZUKAN_SECTION(resource, image_offset);

    for (src = (u8*)resource; src != end; src++)
    {
        *dst++ = *src;
    }

    destinations.pixel_x = ZUKAN_ENTRY_IMAGE_X;
    destinations.pixel_y = ZUKAN_ENTRY_IMAGE_Y;
    destinations.clut_x = 0;
    destinations.clut_y = ZUKAN_ENTRY_CLUT_Y;
    g_zukan_image_mode = zukan_upload_tim(&destinations, (TimPrefix*)ZUKAN_SECTION(g_zukan_resource_buffer, image_offset));

    g_zukan_previous_resource_id = ZUKAN_RELATED_ENTRIES(g_zukan_resource_buffer)->previous;
    g_zukan_next_resource_id = ZUKAN_RELATED_ENTRIES(g_zukan_resource_buffer)->next;
}

/**
 * @brief Load the encyclopedia UI resource and upload its image to VRAM.
 */
void zukan_load_ui_resource(void)
{
    RECT rect;
    s32 width;
    s32 height;
    ZukanUiImageResource* image = (ZukanUiImageResource*)g_zukan_resource_archive;

    cdrom_queue_read(CD_RES_ZUKAN_UI, image);
    width = image->packed_size;
    height = width >> 16;
    width &= 0xFFFF;
    setRECT(&rect, ZUKAN_BORDER_IMAGE_X, ZUKAN_BORDER_IMAGE_Y, width, height);
    LoadImage(&rect, image->pixels);
}
