#include "main.h"
#include "field_text.h"
#include "saved_game.h"
#include "common.h"
#include "cdrom.h"
#include "controller.h"
#include "pad.h"
#include "display.h"
#include "gpu_packet.h"
#include "tim.h"
#include "sdk/libetc.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define GOLEM_ORDERING_TABLE_SIZE 16
#define GOLEM_PACKET_BUFFER_SIZE 0x4000
#define GOLEM_PANEL_COUNT 154
#define GOLEM_BLOCK_LIST_ROW_HEIGHT 40
#define GOLEM_SCROLL_FRAMES 4
#define GOLEM_ROTATION_COUNT 4
#define GOLEM_GRID_SIDE 6
#define GOLEM_GRID_CELL_SIZE 16
#define GOLEM_VERTICAL_MARKER 0x50
#define GOLEM_HORIZONTAL_MARKER 0x4E
#define GOLEM_SOUND_MOVE 0x7D
#define GOLEM_SOUND_REJECT 0x78
#define GOLEM_SOUND_PICK_UP 0x7E
#define GOLEM_SOUND_PLACE 0x120
/** @brief The pad context, addressed directly through the saved-game buffer it aliases. */
#define GOLEM_PAD_CTX ((PadContext*)g_saved_game.bytes)
#define GOLEM_LOGIC_BLOCK(index) (GOLEM_PAD_CTX->logic_blocks[(index)].word)
#define GOLEM_PANEL_BEHAVIOR_SHIFT 3
#define GOLEM_PANEL_FLASH_SHIFT 7
#define GOLEM_PANEL_FLASH_MASK (0xF << GOLEM_PANEL_FLASH_SHIFT)
#define GOLEM_PANEL_FLASH_FRAMES 3
#define GOLEM_ACTIVATE_PANEL(word) (((word) & ~GOLEM_PANEL_FLASH_MASK) | (GOLEM_PANEL_FLASH_FRAMES << GOLEM_PANEL_FLASH_SHIFT))
#define GOLEM_SET_PANEL_FLASH(word, frames) (((word) & ~GOLEM_PANEL_FLASH_MASK) | (((frames) & 0xF) << GOLEM_PANEL_FLASH_SHIFT))
#define GOLEM_SHARED_PLUS_TEXT_INDEX 11
#define GOLEM_FADE_NEUTRAL 0x100
#define GOLEM_FADE_ADDITIVE_THRESHOLD (GOLEM_FADE_NEUTRAL + 1)
#define GOLEM_FADE_ADDITIVE_DRAW_MODE 0x25
#define GOLEM_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))
/**
 * @brief Address of string @p index in the archive section @p offset bytes into @p archive.
 * @note Summed as integers, index first, to match the original address arithmetic.
 */
#define GOLEM_ARCHIVE_TEXT(archive, offset, index) ((u8*)((offset) + (*(u16*)((index) * 2 + (offset) + (u32)(archive)) + (u32)(archive))))
/** @brief Address of the FIELD UI string whose offset pair is @p entry, the @p index-th table entry. */
#define FIELD_UI_TEXT_AT(entry, index) ((entry) - (index) * 2 + (entry)[0] + ((entry)[1] << 8))

/** @brief Animated controls within the panel record array. */
typedef enum
{
    GOLEM_PANEL_SCROLL_DOWN = 0,
    GOLEM_PANEL_SCROLL_UP = 1,
    GOLEM_PANEL_ROTATE = 3
} GolemPanelIndex;

/** @brief Visibility and flash behavior encoded in a panel record. */
typedef enum
{
    GOLEM_PANEL_ALWAYS = 0,
    GOLEM_PANEL_GRID_4X4 = 1,
    GOLEM_PANEL_GRID_5X5 = 2,
    GOLEM_PANEL_GRID_6X6 = 3,
    GOLEM_PANEL_IF_SCROLL_UP = 4,
    GOLEM_PANEL_IF_SCROLL_DOWN = 5,
    GOLEM_PANEL_FLASH_WHITE = 6,
    GOLEM_PANEL_ALWAYS_7 = 7
} GolemPanelBehavior;

/** @brief Ordering-table layers, drawn from highest index to lowest. */
typedef enum
{
    GOLEM_LAYER_FADE = 9,
    GOLEM_LAYER_TEXT = 10,
    GOLEM_LAYER_CURSOR = 11,
    GOLEM_LAYER_GRID = 13,
    GOLEM_LAYER_BLOCK_LIST = 14,
    GOLEM_LAYER_PANELS = 15
} GolemRenderLayer;

/** @brief One frame's ordering table, GPU packets, and VRAM environments. */
typedef struct
{
    u_long ordering_table[GOLEM_ORDERING_TABLE_SIZE];
    u8 packets[GOLEM_PACKET_BUFFER_SIZE];
    u8* packet_cursor;
    RECT clear_rect;
    s32 buffer_index;
    DISPENV display_env;
    DRAWENV draw_env;
} GolemRenderContext;

/** @brief Availability and palette data for one logic block. */
typedef struct
{
    u16 is_unavailable;
    u16 clut;
} GolemLogicBlockStatus;

/** @brief Packed high texture-height bits and the panel's horizontal position. */
typedef union
{
    u32 packed;
    struct
    {
        u16 texture_height_high;
        u16 x;
    } fields;
} GolemPanelDimensions;

/** @brief Texture, animation, and screen rectangle for one UI panel. */
typedef struct
{
    /** @brief Blend mode [1:0], semitransparency [2], behavior [6:3],
     *         flash frames [10:7], and texture U [18:11]. */
    u32 attributes;
    /** @brief Texture V [10:3], CLUT X / 16 [16:11], cell width [25:17],
     *         and the low six cell-height bits [31:26]. */
    u32 texture;
    GolemPanelDimensions dimensions;
    u16 y;
    u16 width;
    u16 height;
    u16 reserved;
} GolemPanelRecord;

/** @brief VRAM destinations for a TIM image and its CLUT. */
typedef struct
{
    u16 x;
    u16 y;
    u16 clut_x;
    u16 clut_y;
} GolemImageClutPos;

/** @brief Shared placement information for all rotations of a logic-block icon. */
typedef struct
{
    u8 part_count;
    u8 reserved;
    u8 grid_width;
    u8 grid_height;
    s16 origin_x;
    s16 origin_y;
} GolemIconHeader;

/** @brief Position of the base glyph in one rotated icon. */
typedef struct
{
    s8 x;
    s8 y;
    u16 reserved;
} GolemIconOrigin;

/** @brief One glyph positioned within a rotated icon. */
typedef struct
{
    s8 x;
    s8 y;
    s16 glyph_id;
} GolemIconPart;

/** @brief A base glyph and up to four additional glyphs for one rotation. */
typedef struct
{
    GolemIconOrigin origin;
    GolemIconPart parts[4];
} GolemIconRotation;

/** @brief Placement information and four rotated layouts for a logic-block icon. */
typedef struct
{
    GolemIconHeader header;
    GolemIconRotation rotations[GOLEM_ROTATION_COUNT];
} GolemCompositeIconRow;

/** @brief UV coordinates and dimensions for one glyph. */
typedef struct
{
    u8 u0;
    u8 reserved_01;
    u8 v0;
    u8 reserved_03;
    u16 width;
    u16 height;
} GolemGlyphMetric;

/** @brief Fade color and its remaining interpolation step count. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 steps_remaining;
} GolemFadeState;

/** @brief Byte offsets of the name and description sections from the archive header. */
typedef struct
{
    s32 names_offset;
    s32 descriptions_offset;
} GolemTextSections;

/**
 * @brief Counted archive containing the two golem text sections.
 * @note Section offsets are relative to this header. Each section begins with
 *       u16 string offsets relative to that section, followed by encoded text.
 */
typedef struct
{
    u32 section_count;
    GolemTextSections sections;
} GolemTextArchive;

extern s32 D_80122C00;
/** @brief Two-byte little-endian offset of "+" in the shared text directory. */
extern u8 D_800EC3DA[];
extern GolemRenderContext* g_golem_render_buffers;
extern s32 g_golem_exit_requested;
/** @brief Word-aligned scratch memory after the render contexts. */
extern u8* g_golem_work_buffer;
extern GolemFadeState g_golem_fade_target;
extern s32 g_golem_scroll_steps;
extern s32 g_golem_logic_block_count;
extern s32 g_golem_grid_size_class;
extern s32 g_golem_scroll_y;
extern GolemLogicBlockStatus g_golem_block_status[];
extern s32 g_golem_is_placing_block;
extern s32 g_golem_scroll_target_y;
extern GolemFadeState g_golem_fade_current;
extern s32 g_golem_block_x;
extern s32 g_golem_block_y;
extern s32 g_golem_cursor_steps;
extern s32 g_golem_cursor_x;
extern s32 g_golem_cursor_y;
extern s32 g_golem_active_logic_type;
extern s32 g_golem_cursor_target_x;
extern s32 g_golem_cursor_target_y;
extern s32 D_8014C26C;
extern s32 g_golem_saved_logic_type_slot;
/** @brief Frames remaining in the auxiliary scalar interpolation. */
extern s32 g_golem_interpolation_steps;
/** @brief Interpolated scalar; its purpose is not yet identified. */
extern s32 g_golem_interpolation_value;
extern s32 g_golem_block_rotation;
/** @brief Target for the auxiliary scalar interpolation. */
extern s32 g_golem_interpolation_target;
extern s32 g_golem_selected_block;
extern s32 g_golem_restore_slot_on_cancel;
/** @brief 4bpp editor atlas with sixteen 16-color palettes. */
extern TimPrefix g_golem_ui_image;
/** @brief Name and description text for the logic blocks. */
extern GolemTextArchive g_golem_text_archive;
extern GolemGlyphMetric g_golem_glyph_metrics[];
extern GolemPanelRecord g_golem_panel_records[GOLEM_PANEL_COUNT];
extern GolemCompositeIconRow g_golem_composite_icon_rows[];

/* FIELD overlay functions called from the editor; FIELD has no header that declares them. */
void func_800A3938(s32 sound_id, s32 pan);
void* func_800A88A0(void* sprite_cursor, u_long* ordering_table, u8* text, s32 text_color, s32 x, s32 y, s32 flags);
void func_800A8B90(u8* dest, s32 value, s32 style);
u32 func_800CB758(void);
void func_800CB918(s32 block_index, s32 rotation, s32 x, s32 y);
s32 func_800CBA9C(s32 block_index, s32 rotation, s32 x, s32 y);
s32 func_800CBC0C(s32 block_index, s32 rotation, s32 x, s32 y);
s32 func_800CBD70(GolemLogicBlockStatus* block_status);
void func_800CBE64(s32 block_index);
void func_800CBEC4(s32* markers);
void golem_run(GolemRenderContext* render_buffers, s32 restore_slot_on_cancel);
u8* golem_initialize_state(u8* work_buffer, s32 restore_slot_on_cancel);
void golem_upload_ui_image(void);
void golem_upload_image_archive(GolemImageClutPos* destinations, TimPrefix* tim);
void golem_update_frame(GolemRenderContext* render_context);
void golem_handle_input(void);
void golem_reset_block_position(void);
u8* golem_draw_grid_markers(u8* packet_cursor, u_long* ordering_table);
u8* golem_emit_grid_marker(u8* packet_cursor, u_long* ordering_table, s32 x, s32 y, s32 glyph);
void golem_reset_cursor_motion(void);
u8* golem_finish_grid_marker_run(u8* packet_cursor, u_long* ordering_table);
u8* golem_draw_cursor(u8* packet_cursor, GolemRenderContext* render_context);
void golem_render(GolemRenderContext* render_context);
u8* golem_draw_block_list(u8* packet_buffer, GolemRenderContext* render_context);
u8* golem_draw_logic_grid(u8* packet_cursor, GolemRenderContext* render_context);
u8* golem_draw_panel(u8* packet_cursor, u_long* ordering_table, s32 panel_index, s32 x, s32 y, s32 width, s32 height);
u8* golem_draw_composite_icon(u8* packet_cursor, u_long* ordering_table, s32 block_index, s32 rotation, s32 x, s32 y, s32 clut, s32 use_origin, s32 style);
u8* golem_emit_glyph(u8* packet_cursor, u_long* ordering_table, s32 glyph_id, s32 x, s32 y, s32 clut, s32 style);
LINE_F2* golem_emit_panel_outline(LINE_F2* packet, u_long* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color);
void golem_set_fade_target(s16 red, s16 green, s16 blue, s16 steps);
u8* golem_render_fade(u8* packet_cursor, u_long* ordering_table_tag);
void golem_append_encoded_string(u8* dest, u8* src);
s32 golem_encoded_string_length(u8* text);
void golem_copy_encoded_string(u8* dst, u8* src);

/**
 * @brief Run the golem logic-grid editor until the user exits.
 * @param render_buffers Storage for the two render contexts and packet heap.
 * @param restore_slot_on_cancel Restore the prior logic-type slot on cancel.
 * @see decomp.me (100%)
 */
void golem_run(GolemRenderContext* render_buffers, s32 restore_slot_on_cancel)
{
    GolemRenderContext* draw_buffer;
    GolemRenderContext* next_buffer;
    GolemRenderContext* other_buffer;
    GolemRenderContext* buffers;

    g_golem_render_buffers = render_buffers;
    render_buffers += 2;

    setRECT(&g_golem_render_buffers[0].clear_rect, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    setRECT(&g_golem_render_buffers[1].clear_rect, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    SetDefDispEnv(&g_golem_render_buffers[0].display_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_golem_render_buffers[1].display_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_golem_render_buffers[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_golem_render_buffers[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    buffers = g_golem_render_buffers;
    buffers[1].draw_env.dtd = 0;
    buffers[0].draw_env.dtd = 0;
    g_golem_exit_requested = 0;
    g_golem_render_buffers[0].buffer_index = 0;
    g_golem_render_buffers[1].buffer_index = 1;

    g_golem_work_buffer = (u8*)(((u32)golem_initialize_state((u8*)render_buffers, restore_slot_on_cancel) + 3) & ~3);

    next_buffer = g_golem_render_buffers;
    ClearOTagR(next_buffer->ordering_table, GOLEM_ORDERING_TABLE_SIZE);
    ClearOTagR(g_golem_render_buffers[1].ordering_table, GOLEM_ORDERING_TABLE_SIZE);
    VSync(0);
    PutDispEnv(&next_buffer->display_env);
    update_controllers();

    for (;;)
    {
        draw_buffer = next_buffer;
        ClearOTagR(draw_buffer->ordering_table, GOLEM_ORDERING_TABLE_SIZE);
        draw_buffer->packet_cursor = draw_buffer->packets;
        field_text_reset_scratch();
        field_update_input_repeat();
        golem_update_frame(draw_buffer);
        field_text_upload_immediate_cache();
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);

        if (g_golem_exit_requested != 0)
        {
            break;
        }

        ClearImage(&draw_buffer->clear_rect, 0, 0, 0);
        other_buffer = g_golem_render_buffers;
        if (draw_buffer == g_golem_render_buffers)
        {
            other_buffer = draw_buffer + 1;
        }
        next_buffer = other_buffer;
        PutDispEnv(&other_buffer->display_env);
        PutDrawEnv(&next_buffer->draw_env);
        DrawOTag(&draw_buffer->ordering_table[GOLEM_LAYER_PANELS]);
        update_controllers();
        cdrom_process_state();
    }

    field_reset_input_repeat();
    field_text_reset_windows();
}

/**
 * @brief Initialize logic-grid state and upload the editor resources.
 * @param work_buffer First byte after the double-buffered render contexts.
 * @param restore_slot_on_cancel Restore the prior logic-type slot on cancel.
 * @return The unchanged work-buffer pointer.
 * @see decomp.me (100%)
 */
u8* golem_initialize_state(u8* work_buffer, s32 restore_slot_on_cancel)
{
    s32 slot_index;
    s32 logic_type;
    s32 selected_logic_type;
    PadContext* menu_data;
    s32 stack_pad[2]; /* never used, but the original frame size needs it */

    g_golem_restore_slot_on_cancel = restore_slot_on_cancel;
    if (restore_slot_on_cancel != 0)
    {
        menu_data = GOLEM_PAD_CTX;
        selected_logic_type = menu_data->large_history_index;
        g_golem_saved_logic_type_slot = D_80122C00;
        D_80122C00 = 0;
        for (slot_index = 0; slot_index < LARGE_HISTORY_RECORD_COUNT; slot_index++)
        {
            if (*(menu_data->large_history_order + slot_index) == selected_logic_type)
            {
                D_80122C00 = slot_index;
            }
        }
    }

    g_golem_grid_size_class = func_800CB758() - 4;
    logic_type = GOLEM_PAD_CTX->large_history_order[D_80122C00];
    g_golem_block_rotation = 0;
    g_golem_block_y = 0;
    g_golem_block_x = 0;
    g_golem_is_placing_block = 0;
    g_golem_cursor_target_x = 0xB8;
    g_golem_cursor_x = 0xB8;
    g_golem_cursor_target_y = 0x50;
    g_golem_cursor_y = 0x50;
    g_golem_active_logic_type = logic_type;
    golem_reset_cursor_motion();
    g_golem_logic_block_count = func_800CBD70(g_golem_block_status);
    g_golem_scroll_steps = 0;
    g_golem_scroll_y = 0;
    g_golem_scroll_target_y = 0;
    g_golem_selected_block = 0;
    golem_upload_ui_image();
    field_reset_input_repeat();
    golem_set_fade_target(0x100, 0x100, 0x100, 6);
    D_8014C26C = 0;
    g_golem_interpolation_target = 0;
    g_golem_interpolation_value = 0;
    g_golem_interpolation_steps = 0;
    return work_buffer;
}

/**
 * @brief Upload the golem editor image and CLUT to their VRAM locations.
 * @see decomp.me (100%)
 */
void golem_upload_ui_image(void)
{
    GolemImageClutPos destinations;

    destinations.x = SCREEN_WIDTH;
    destinations.y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = VRAM_CLUT_Y;
    golem_upload_image_archive(&destinations, &g_golem_ui_image);
}

/**
 * @brief Upload the editor TIM image and flatten its palettes into one VRAM row.
 * @param destinations VRAM destinations for the image and CLUT blocks.
 * @param tim TIM resource containing 256 palette entries.
 * @note Requires a CLUT block; CLUT-less TIM files are not supported.
 * @see decomp.me (100%)
 */
void golem_upload_image_archive(GolemImageClutPos* destinations, TimPrefix* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    TimDimensions* pixel_dimensions;

    flags = tim->flags;
    clut_block_size = tim->clut_block.bnum;

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
}

/**
 * @brief Render and update one editor frame.
 * @param render_context Frame ordering table and packet storage.
 * @see decomp.me (100%)
 */
void golem_update_frame(GolemRenderContext* render_context)
{
    golem_render(render_context);
    g_frame_counter += 1;
    golem_handle_input();
    if (g_golem_interpolation_steps != 0)
    {
        g_golem_interpolation_value += (g_golem_interpolation_target - g_golem_interpolation_value) / g_golem_interpolation_steps;
        g_golem_interpolation_steps -= 1;
        return;
    }
    g_golem_interpolation_value = g_golem_interpolation_target;
}

/**
 * @brief Handle logic-block selection, placement, rotation, and cancellation.
 * @see decomp.me (100%)
 */
void golem_handle_input(void)
{
    s32 saved_x, saved_y, repeat_count, block_index, scroll_steps, input, cancel_pressed;
    if ((g_pad_input & PAD_BTN_START) && (g_golem_is_placing_block == 0))
    {
        func_800A3938(GOLEM_SOUND_MOVE, 0x80);
        g_golem_exit_requested = 1;
        if (g_golem_restore_slot_on_cancel != 0)
        {
            D_80122C00 = g_golem_saved_logic_type_slot;
        }
        return;
    }
    scroll_steps = g_golem_scroll_steps;
    if (scroll_steps != 0)
    {
        g_golem_scroll_steps = scroll_steps - 1;
        g_golem_scroll_y += (g_golem_scroll_target_y - g_golem_scroll_y) / scroll_steps;
        if (g_golem_scroll_steps != 0)
        {
            return;
        }
        g_golem_selected_block = g_golem_scroll_y / GOLEM_BLOCK_LIST_ROW_HEIGHT;
    }
    else
    {
        g_golem_scroll_y = g_golem_scroll_target_y;
        g_golem_selected_block = g_golem_scroll_target_y / GOLEM_BLOCK_LIST_ROW_HEIGHT;
    }
    if (g_golem_scroll_steps != 0)
    {
        return;
    }
    if (g_golem_is_placing_block != 0)
    {
        input = g_pad_input;
        if (input & (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT))
        {
            saved_x = g_golem_block_x;
            saved_y = g_golem_block_y;
            if (input & PAD_BTN_UP)
            {
                g_golem_block_y = saved_y - 1;
            }
            else if (input & PAD_BTN_DOWN)
            {
                g_golem_block_y = saved_y + 1;
            }
            else if (input & PAD_BTN_LEFT)
            {
                g_golem_block_x = saved_x - 1;
            }
            else if (input & PAD_BTN_RIGHT)
            {
                g_golem_block_x = saved_x + 1;
            }
            if (func_800CBC0C(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y) == 0)
            {
                g_golem_block_x = saved_x;
                g_golem_block_y = saved_y;
                func_800A3938(GOLEM_SOUND_REJECT, 0x80);
            }
            else
            {
                func_800A3938(GOLEM_SOUND_MOVE, 0x80);
            }
            return;
        }
        if (input & (PAD_BTN_SQUARE | PAD_BTN_TRIANGLE))
        {
            if (input & PAD_BTN_SQUARE)
            {
                if (g_golem_block_rotation >= GOLEM_ROTATION_COUNT - 1)
                {
                    g_golem_block_rotation = 0;
                }
                else
                {
                    g_golem_block_rotation++;
                }
            }
            else
            {
                if (g_golem_block_rotation == 0)
                {
                    g_golem_block_rotation = GOLEM_ROTATION_COUNT - 1;
                }
                else
                {
                    g_golem_block_rotation--;
                }
            }
            if (func_800CBC0C(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y) == 0)
            {
                golem_reset_block_position();
            }
            func_800A3938(GOLEM_SOUND_MOVE, 0x80);
            g_golem_panel_records[GOLEM_PANEL_ROTATE].attributes = GOLEM_ACTIVATE_PANEL(g_golem_panel_records[GOLEM_PANEL_ROTATE].attributes);
            return;
        }
        if (input & (PAD_BTN_START | PAD_BTN_L3 | PAD_BTN_CROSS))
        {
            if (func_800CBA9C(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y) != 0)
            {
                g_golem_is_placing_block = 0;
                golem_reset_cursor_motion();
                func_800A3938(GOLEM_SOUND_PLACE, 0x80);
                func_800CB918(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y);
            }
            else
            {
                func_800A3938(GOLEM_SOUND_REJECT, 0x80);
            }
            return;
        }
        if (input & PAD_BTN_CIRCLE)
        {
            g_golem_is_placing_block = 0;
            golem_reset_cursor_motion();
            func_800A3938(GOLEM_SOUND_REJECT, 0x80);
            GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
        }
        return;
    }
    if (g_golem_logic_block_count != 0)
    {
        repeat_count = 1;
        input = g_pad_input;
        if (input & PAD_BTN_L1)
        {
            g_pad_input = PAD_BTN_UP;
            repeat_count = 3;
        }
        else if (input & PAD_BTN_R1)
        {
            g_pad_input = PAD_BTN_DOWN;
            repeat_count = 3;
        }
        input = g_pad_input;
        if (input & (PAD_BTN_UP | PAD_BTN_DOWN))
        {
            func_800A3938(GOLEM_SOUND_MOVE, 0x80);
            while (repeat_count != 0)
            {
                if (g_pad_input & PAD_BTN_UP)
                {
                    if (g_golem_scroll_target_y != 0)
                    {
                        g_golem_scroll_steps = GOLEM_SCROLL_FRAMES;
                        g_golem_scroll_target_y -= GOLEM_BLOCK_LIST_ROW_HEIGHT;
                        g_golem_panel_records[GOLEM_PANEL_SCROLL_UP].attributes = GOLEM_ACTIVATE_PANEL(g_golem_panel_records[GOLEM_PANEL_SCROLL_UP].attributes);
                    }
                }
                else if ((g_pad_input & PAD_BTN_DOWN) && ((g_golem_scroll_target_y / GOLEM_BLOCK_LIST_ROW_HEIGHT) != (g_golem_logic_block_count - 1)))
                {
                    g_golem_scroll_steps = GOLEM_SCROLL_FRAMES;
                    g_golem_scroll_target_y += GOLEM_BLOCK_LIST_ROW_HEIGHT;
                    g_golem_panel_records[GOLEM_PANEL_SCROLL_DOWN].attributes = GOLEM_ACTIVATE_PANEL(g_golem_panel_records[GOLEM_PANEL_SCROLL_DOWN].attributes);
                }
                repeat_count--;
            }
            return;
        }
        if (input & PAD_BTN_R2)
        {
            block_index = g_golem_selected_block;
            repeat_count = 0;
            while (repeat_count < g_golem_logic_block_count)
            {
                block_index++;
                if (block_index == g_golem_logic_block_count)
                {
                    block_index = 0;
                }
                repeat_count++;
                if (GOLEM_PAD_CTX->logic_blocks[block_index].f.logic_type == g_golem_active_logic_type)
                {
                    break;
                }
            }
            g_golem_scroll_target_y = block_index * GOLEM_BLOCK_LIST_ROW_HEIGHT;
            g_golem_scroll_steps = GOLEM_SCROLL_FRAMES;
            g_golem_panel_records[GOLEM_PANEL_SCROLL_DOWN].attributes = GOLEM_ACTIVATE_PANEL(g_golem_panel_records[GOLEM_PANEL_SCROLL_DOWN].attributes);
            func_800A3938(GOLEM_SOUND_MOVE, 0x80);
            return;
        }
        if (input & PAD_BTN_L2)
        {
            block_index = g_golem_selected_block;
            repeat_count = 0;
            while (repeat_count < g_golem_logic_block_count)
            {
                block_index--;
                if (block_index < 0)
                {
                    block_index = g_golem_logic_block_count - 1;
                }
                if (GOLEM_PAD_CTX->logic_blocks[block_index].f.logic_type == g_golem_active_logic_type)
                {
                    break;
                }
                repeat_count++;
            }
            g_golem_scroll_target_y = block_index * GOLEM_BLOCK_LIST_ROW_HEIGHT;
            g_golem_scroll_steps = GOLEM_SCROLL_FRAMES;
            g_golem_panel_records[GOLEM_PANEL_SCROLL_UP].attributes = GOLEM_ACTIVATE_PANEL(g_golem_panel_records[GOLEM_PANEL_SCROLL_UP].attributes);
            func_800A3938(GOLEM_SOUND_MOVE, 0x80);
            return;
        }
        if (input & (PAD_BTN_L3 | PAD_BTN_CROSS))
        {
            if (g_golem_block_status[g_golem_selected_block].is_unavailable == 0)
            {
                g_golem_is_placing_block = 1;
                if (GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.logic_type == g_golem_active_logic_type)
                {
                    g_golem_block_x = GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.grid_x;
                    g_golem_block_y = GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.grid_y;
                    g_golem_block_rotation = GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.rotation;
                }
                else
                {
                    golem_reset_block_position();
                    g_golem_block_rotation = 0;
                }
                func_800CBE64(g_golem_selected_block);
                func_800A3938(GOLEM_SOUND_PICK_UP, 0x80);
            }
            return;
        }
        cancel_pressed = input & PAD_BTN_CIRCLE;
    }
    else
    {
        cancel_pressed = g_pad_input & PAD_BTN_CIRCLE;
    }
    if (cancel_pressed != 0)
    {
        func_800A3938(GOLEM_SOUND_MOVE, 0x80);
        g_golem_exit_requested = 1;
        if (g_golem_restore_slot_on_cancel != 0)
        {
            D_80122C00 = g_golem_saved_logic_type_slot;
        }
    }
}

/**
 * @brief Reset the active block to its layout-defined grid origin.
 * @see decomp.me (100%)
 */
void golem_reset_block_position(void)
{
    if (g_golem_grid_size_class == 0)
    {
        g_golem_block_x = g_golem_composite_icon_rows[GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.shape].header.grid_width - 1;
        g_golem_block_y = g_golem_composite_icon_rows[GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.shape].header.grid_height - 1;
        return;
    }
    if (g_golem_grid_size_class < 0)
    {
        return;
    }
    if (g_golem_grid_size_class < 3)
    {
        g_golem_block_x = g_golem_composite_icon_rows[GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.shape].header.grid_width;
        g_golem_block_y = g_golem_composite_icon_rows[GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.shape].header.grid_height;
    }
}

/**
 * @brief Emit the logic-grid divider markers and their texture-page packet.
 * @param packet_cursor Packet cursor, threaded through the marker emitters.
 * @param ordering_table Ordering-table tag for the marker packets.
 * @return Packet cursor after the marker run.
 * @see decomp.me (100.00%)
 * @see working/func_80140DEC/
 */
u8* golem_draw_grid_markers(u8* packet_cursor, u_long* ordering_table)
{
    /* Vertical edges (6 rows of 5), then horizontal edges (5 rows of 6). */
    s32 markers[2 * GOLEM_GRID_SIDE * (GOLEM_GRID_SIDE - 1)];
    s32 marker_index;
    s32 row;
    s32 column;
    s32 vertical_marker;
    s32 horizontal_marker;

    func_800CBEC4(markers);

    marker_index = 0;
    for (row = 0; row < GOLEM_GRID_SIDE; row++)
    {
        for (column = 0; column < GOLEM_GRID_SIDE - 1; column++, marker_index++)
        {
            vertical_marker = markers[marker_index];
            if (vertical_marker != 0)
            {
                packet_cursor = golem_emit_grid_marker(packet_cursor, ordering_table, column * GOLEM_GRID_CELL_SIZE + 12, row * GOLEM_GRID_CELL_SIZE + 4,
                                                       vertical_marker == GOLEM_VERTICAL_MARKER ? 2 : 3);
            }
        }
    }

    for (row = 0; row < GOLEM_GRID_SIDE - 1; row++)
    {
        for (column = 0; column < GOLEM_GRID_SIDE; column++, marker_index++)
        {
            horizontal_marker = markers[marker_index];
            if (horizontal_marker != 0)
            {
                packet_cursor = golem_emit_grid_marker(packet_cursor, ordering_table, column * GOLEM_GRID_CELL_SIZE + 4, row * GOLEM_GRID_CELL_SIZE + 12,
                                                       horizontal_marker != GOLEM_HORIZONTAL_MARKER);
            }
        }
    }

    packet_cursor = golem_finish_grid_marker_run(packet_cursor, ordering_table);
    return packet_cursor;
}

/**
 * @brief Emit one 8x8 logic-grid marker sprite.
 * @param packet_cursor Next free GPU packet.
 * @param ordering_table Ordering-table tag to receive the sprite.
 * @param x Sprite x coordinate.
 * @param y Sprite y coordinate.
 * @param glyph Marker glyph index.
 * @return Packet cursor past the sprite.
 * @see decomp.me (100%)
 */
u8* golem_emit_grid_marker(u8* packet_cursor, u_long* ordering_table, s32 x, s32 y, s32 glyph)
{
    SPRT* sprite = (SPRT*)packet_cursor;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setWH(sprite, 8, 8);
    setUV0(sprite, glyph * 8 - 0x70, 0x58);
    setXY0(sprite, x, y);
    sprite->clut = getClut(112, 498);
    addPrim(ordering_table, sprite);
    return packet_cursor + sizeof(SPRT);
}

/**
 * @brief Return the animated cursor to its resting target.
 * @see decomp.me (100%)
 */
void golem_reset_cursor_motion(void)
{
    g_golem_cursor_target_x = 0xB8;
    g_golem_cursor_target_y = 0x50;
    g_golem_cursor_steps = 8;
}

/**
 * @brief Append the draw-mode packet that closes a grid-marker run.
 * @param packet_cursor Next free GPU packet.
 * @param ordering_table Ordering-table tag to receive the packet.
 * @return Packet cursor past the draw-mode packet.
 * @see decomp.me (100%)
 */
u8* golem_finish_grid_marker_run(u8* packet_cursor, u_long* ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, getTPage(0, 1, 320, 0));
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Draw and advance the animated logic-grid cursor.
 * @param packet_cursor Next free GPU packet.
 * @param render_context Render context containing the cursor ordering-table tag.
 * @return Packet cursor past the sprite and draw-mode packet.
 * @see decomp.me (100%)
 */
u8* golem_draw_cursor(u8* packet_cursor, GolemRenderContext* render_context)
{
    SPRT* sprite = (SPRT*)packet_cursor;
    DR_TPAGE* draw_mode;
    u_long* ordering_table;
    u16 cursor_x;
    u16 cursor_y;
    s32 x_step;
    s32 y_step;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    cursor_x = g_golem_cursor_x;
    cursor_y = g_golem_cursor_y;
    setWH(sprite, 0x10, 0x10);
    setUV0(sprite, 0xB0, 0xF0);
    sprite->clut = getClut(112, 498);
    setXY0(sprite, cursor_x + 8, cursor_y);
    addPrim(&render_context->ordering_table[GOLEM_LAYER_CURSOR], sprite);

    packet_cursor += sizeof(SPRT);
    ordering_table = &render_context->ordering_table[GOLEM_LAYER_CURSOR];

    if (g_golem_cursor_steps != 0)
    {
        x_step = (g_golem_cursor_target_x - g_golem_cursor_x) / g_golem_cursor_steps;
        y_step = (g_golem_cursor_target_y - g_golem_cursor_y) / g_golem_cursor_steps;
        g_golem_cursor_steps--;
        g_golem_cursor_x += x_step;
        g_golem_cursor_y += y_step;
    }
    else
    {
        g_golem_cursor_x = g_golem_cursor_target_x;
        g_golem_cursor_y = g_golem_cursor_target_y;
    }

    draw_mode = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 1, 320, 0));
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Emit the golem panel grid, the cursor, and the selected item's name
 *        and detail text into the render context's packet buffer.
 * @param render_context Render context with the packet cursor and ordering table.
 * @see decomp.me (100.00%)
 */
void golem_render(GolemRenderContext* render_context)
{
    s32 stack_pad[2]; /* never used, but the original frame size needs it */
    u8 name_buffer[0x100];
    u8* name_text;
    u8 number_text[0x100];
    GolemPanelRecord* panel_record;
    GolemTextArchive* archive;
    GolemTextSections* sections;
    u8* packet_cursor;
    s32 panel_index;
    u_long* panel_ordering_table;
    s32 descriptions_offset;
    s32 description_index;
    s32 names_offset;
    s32 name_index;

    panel_ordering_table = &render_context->ordering_table[GOLEM_LAYER_PANELS];
    packet_cursor = render_context->packet_cursor;
    panel_index = 0;
    do
    {
        panel_record = &g_golem_panel_records[panel_index];
        packet_cursor = golem_draw_panel(packet_cursor, panel_ordering_table, panel_index, panel_record->dimensions.fields.x, panel_record->y,
                                         panel_record->width, panel_record->height);
        panel_index++;
    } while (panel_index < GOLEM_PANEL_COUNT);

    packet_cursor = golem_draw_block_list(packet_cursor, render_context);
    packet_cursor = golem_draw_logic_grid(packet_cursor, render_context);
    packet_cursor = golem_draw_cursor(packet_cursor, render_context);
    panel_ordering_table = &render_context->ordering_table[GOLEM_LAYER_TEXT];

    if (g_golem_logic_block_count != 0)
    {
        name_text = name_buffer;
        sections = &g_golem_text_archive.sections;
        names_offset = sections->names_offset;
        /* The id is bits 7:2 of the block word, read through its low byte. */
        name_index = (u8)GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 2;
        archive = &g_golem_text_archive;
        golem_copy_encoded_string(name_text, GOLEM_ARCHIVE_TEXT(archive, names_offset, name_index));
        if (GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.quantity)
        {
            golem_append_encoded_string(name_text, FIELD_UI_TEXT_AT(D_800EC3DA, GOLEM_SHARED_PLUS_TEXT_INDEX));
            func_800A8B90(number_text, GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.quantity, 1);
            golem_append_encoded_string(name_text, number_text);
        }
        packet_cursor = func_800A88A0(packet_cursor, panel_ordering_table, name_text, 0, 0xA0, 0xA0, 2);
        descriptions_offset = archive->sections.descriptions_offset;
        description_index = (u8)GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 2;
        packet_cursor =
            func_800A88A0(packet_cursor, panel_ordering_table, GOLEM_ARCHIVE_TEXT(archive, descriptions_offset, description_index), 0, 0xA0, 0xB0, 2);
    }

    render_context->packet_cursor = golem_render_fade(packet_cursor, &render_context->ordering_table[GOLEM_LAYER_FADE]);
}

/**
 * @brief Draw the visible logic-block list and highlight the selected block.
 * @param packet_buffer Next free GPU packet.
 * @param render_context Frame buffer and block-list ordering table.
 * @return Packet cursor after the icons and viewport commands.
 * @see decomp.me (100.00%)
 * @see working/func_80141478/code.c
 */
u8* golem_draw_block_list(u8* packet_buffer, GolemRenderContext* render_context)
{
    DRAWENV draw_env;
    u_long* ordering_table;
    u8* next_packet;
    s32 block_y;
    s32 block_index;
    s32 draw_y;
    s32 scroll_above;
    u8* packet_cursor;

    packet_cursor = packet_buffer;
    ordering_table = &render_context->ordering_table[GOLEM_LAYER_BLOCK_LIST];
    SetDrawEnv((DR_ENV*)packet_cursor, &g_golem_render_buffers[render_context->buffer_index ^ 1].draw_env);
    next_packet = packet_cursor + sizeof(DR_ENV);
    addPrim(ordering_table, packet_cursor);

    for (block_index = 0; block_index < g_golem_logic_block_count; block_index++)
    {
        scroll_above = g_golem_scroll_y - GOLEM_BLOCK_LIST_ROW_HEIGHT;
        block_y = block_index * GOLEM_BLOCK_LIST_ROW_HEIGHT - scroll_above;
        if (block_y > -GOLEM_BLOCK_LIST_ROW_HEIGHT)
        {
            if (block_y < 3 * GOLEM_BLOCK_LIST_ROW_HEIGHT)
            {
                if (g_golem_selected_block == block_index && g_golem_block_status[block_index].is_unavailable == 0)
                {
                    next_packet = golem_draw_composite_icon(next_packet, ordering_table, block_index, 0, 6, block_y + 4, g_golem_block_status[block_index].clut,
                                                            1, g_golem_is_placing_block ? 0x80 : 2);
                }
                else
                {
                    next_packet = golem_draw_composite_icon(next_packet, ordering_table, block_index, 0, 6,
                                                            block_index * GOLEM_BLOCK_LIST_ROW_HEIGHT - g_golem_scroll_y + GOLEM_BLOCK_LIST_ROW_HEIGHT + 4,
                                                            g_golem_block_status[block_index].clut, 1, 0);
                }
            }
        }
    }

    packet_cursor = next_packet;
    draw_y = 0x1E;
    if (render_context->buffer_index != 0)
    {
        draw_y = 0x106;
    }
    SetDefDrawEnv(&draw_env, 0xCA, draw_y, 0x4C, 0x74);
    SetDrawEnv((DR_ENV*)packet_cursor, &draw_env);
    addPrim(ordering_table, packet_cursor);
    return packet_cursor + sizeof(DR_ENV);
}

/**
 * @brief Draw placed blocks, the active placement preview, and grid dividers.
 * @param packet_cursor Next free GPU packet.
 * @param render_context Frame buffer and grid ordering table.
 * @return Packet cursor after the icons, dividers, and viewport commands.
 * @see decomp.me (100.00%)
 * @see working/func_801416C8/code.c
 */
u8* golem_draw_logic_grid(u8* packet_cursor, GolemRenderContext* render_context)
{
    DRAWENV draw_env;
    u8* cursor;
    u_long* ordering_table;
    u8* next_packet;
    s32 block_index;
    LogicBlock logic_block;
    s32 cursor_target_x;
    s32 cursor_target_y;
    s32 draw_y;

    cursor = packet_cursor;
    ordering_table = &render_context->ordering_table[GOLEM_LAYER_GRID];
    SetDrawEnv((DR_ENV*)cursor, &g_golem_render_buffers[render_context->buffer_index ^ 1].draw_env);
    addPrim(ordering_table, cursor);
    cursor += sizeof(DR_ENV);
    next_packet = cursor;

    if (g_golem_is_placing_block != 0)
    {
        next_packet = golem_draw_composite_icon(cursor, ordering_table, g_golem_selected_block, g_golem_block_rotation, g_golem_block_x * GOLEM_GRID_CELL_SIZE,
                                                g_golem_block_y * GOLEM_GRID_CELL_SIZE, g_golem_block_status[g_golem_selected_block].clut, 0, 3);
        cursor_target_x =
            g_golem_composite_icon_rows[GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.shape].rotations[g_golem_block_rotation].origin.x * 8 +
            g_golem_block_x * GOLEM_GRID_CELL_SIZE - g_golem_grid_size_class * 8 + 0x3C;
        cursor_target_y =
            g_golem_composite_icon_rows[GOLEM_PAD_CTX->logic_blocks[g_golem_selected_block].f.shape].rotations[g_golem_block_rotation].origin.y * 8 +
            g_golem_block_y * GOLEM_GRID_CELL_SIZE - g_golem_grid_size_class * 8 + 0x3C;
        if ((cursor_target_x != g_golem_cursor_x || cursor_target_y != g_golem_cursor_y) && g_golem_cursor_steps == 0)
        {
            g_golem_cursor_target_x = cursor_target_x;
            g_golem_cursor_target_y = cursor_target_y;
            g_golem_cursor_steps = 4;
        }
    }

    next_packet = golem_draw_grid_markers(next_packet, ordering_table);

    for (block_index = 0; block_index < g_golem_logic_block_count; block_index++)
    {
        logic_block = GOLEM_PAD_CTX->logic_blocks[block_index];
        if (logic_block.f.logic_type == g_golem_active_logic_type)
        {
            next_packet =
                golem_draw_composite_icon(next_packet, ordering_table, block_index, logic_block.f.rotation, logic_block.f.grid_x * GOLEM_GRID_CELL_SIZE,
                                          GOLEM_PAD_CTX->logic_blocks[block_index].f.grid_y * GOLEM_GRID_CELL_SIZE, g_golem_block_status[block_index].clut, 0,
                                          block_index == g_golem_selected_block ? (g_golem_is_placing_block ? 0x80 : 2) : 0);
        }
    }

    cursor = next_packet;

    switch (g_golem_grid_size_class)
    {
    case 0:
        draw_y = 0x38;
        if (render_context->buffer_index != 0)
        {
            draw_y = 0x120;
        }
        SetDefDrawEnv(&draw_env, 0x50, draw_y, 0x40, 0x40);
        break;
    case 1:
        draw_y = 0x30;
        if (render_context->buffer_index != 0)
        {
            draw_y = 0x118;
        }
        SetDefDrawEnv(&draw_env, 0x48, draw_y, 0x50, 0x50);
        break;
    case 2:
        draw_y = 0x28;
        if (render_context->buffer_index != 0)
        {
            draw_y = 0x110;
        }
        SetDefDrawEnv(&draw_env, 0x40, draw_y, 0x60, 0x60);
        break;
    }

    SetDrawEnv((DR_ENV*)cursor, &draw_env);
    addPrim(ordering_table, cursor);
    return cursor + sizeof(DR_ENV);
}

/**
 * @brief Draw one UI panel record as a grid of textured sprites plus a
 *        trailing draw-mode packet, gated by the record's state field.
 * @param packet_cursor Running packet cursor.
 * @param ordering_table Ordering-table tag for the panel packets.
 * @param panel_index Index of the panel record.
 * @param x    Screen x of the panel origin.
 * @param y    Screen y of the panel origin.
 * @param width Total panel width in pixels.
 * @param height Total panel height in pixels.
 * @return Packet cursor after the panel packets.
 * @see decomp.me (100.00%)
 * @see working/func_80141AD0_golem/
 */
u8* golem_draw_panel(u8* packet_cursor, u_long* ordering_table, s32 panel_index, s32 x, s32 y, s32 width, s32 height)
{
    SPRT* sprite;
    s32 flash_frames;
    s32 tint;
    s32 y_offset;
    s32 x_offset;
    s32 row_height;
    s32 cell_height;
    s32 segment_width;
    s32 available_width;
    s32 remaining_height;
    s32 packet_code;
    s32 bottom_texture_height;
    DR_TPAGE* draw_mode;
    u8 unused[16]; /* never used, but the original stack frame reserves it */

    tint = GPU_TINT_NEUTRAL;

    switch ((g_golem_panel_records[panel_index].attributes >> GOLEM_PANEL_BEHAVIOR_SHIFT) & 0xF)
    {
    case GOLEM_PANEL_ALWAYS:
    case GOLEM_PANEL_ALWAYS_7:
        break;
    case GOLEM_PANEL_GRID_4X4:
    case GOLEM_PANEL_GRID_5X5:
    case GOLEM_PANEL_GRID_6X6:
        if ((((g_golem_panel_records[panel_index].attributes >> GOLEM_PANEL_BEHAVIOR_SHIFT) & 0xF) - GOLEM_PANEL_GRID_4X4) != g_golem_grid_size_class)
        {
            return packet_cursor;
        }
        break;
    case GOLEM_PANEL_IF_SCROLL_UP:
        if (g_golem_scroll_y == 0)
        {
            return packet_cursor;
        }
        flash_frames = (g_golem_panel_records[panel_index].attributes >> GOLEM_PANEL_FLASH_SHIFT) & 0xF;
        if (flash_frames != 0)
        {
            tint = GPU_COLOR_WORD(0xC0, 0, 0);
            g_golem_panel_records[panel_index].attributes = GOLEM_SET_PANEL_FLASH(g_golem_panel_records[panel_index].attributes, flash_frames - 1);
        }
        break;
    case GOLEM_PANEL_IF_SCROLL_DOWN:
        if (g_golem_scroll_y / GOLEM_BLOCK_LIST_ROW_HEIGHT >= g_golem_logic_block_count - 1)
        {
            return packet_cursor;
        }
        flash_frames = (g_golem_panel_records[panel_index].attributes >> GOLEM_PANEL_FLASH_SHIFT) & 0xF;
        if (flash_frames != 0)
        {
            tint = GPU_COLOR_WORD(0xC0, 0, 0);
            g_golem_panel_records[panel_index].attributes = GOLEM_SET_PANEL_FLASH(g_golem_panel_records[panel_index].attributes, flash_frames - 1);
        }
        break;
    case GOLEM_PANEL_FLASH_WHITE:
        flash_frames = (g_golem_panel_records[panel_index].attributes >> GOLEM_PANEL_FLASH_SHIFT) & 0xF;
        if (flash_frames != 0)
        {
            tint = GPU_COLOR_WORD(0xC0, 0xC0, 0xC0);
            g_golem_panel_records[panel_index].attributes = GOLEM_SET_PANEL_FLASH(g_golem_panel_records[panel_index].attributes, flash_frames - 1);
        }
        break;
    }

    ordering_table += 1;
    ordering_table -= 1;
    y_offset = 0;
    while (y_offset < height)
    {
        remaining_height = height - y_offset;
        row_height = remaining_height;
        segment_width = g_golem_panel_records[panel_index].texture >> 26;
        cell_height = ((g_golem_panel_records[panel_index].dimensions.packed & 7) << 6) | segment_width;
        packet_code = 0x64;
        x_offset = 0;
        if (cell_height < row_height)
        {
            row_height = cell_height;
        }
        while (x_offset < width)
        {
            sprite = (SPRT*)packet_cursor;
            available_width = width - x_offset;
            segment_width = (g_golem_panel_records[panel_index].texture >> 17) & 0x1FF;
            if (segment_width >= available_width)
            {
                segment_width = available_width;
            }
            SET_BGR0_PACKED(sprite, tint);
            setlen(sprite, 4);
            

            /* need to figure out how to remove this hack */ 
            do
            {
                do
                {
                    setcode(sprite, packet_code);
                } while (0);
            } while (0);
            if ((g_golem_panel_records[panel_index].attributes >> 2) & 1)
            {
                setcode(sprite, 0x66); /* setSemiTrans(sprite, 1); */ 
            }


            setXY0(sprite, (x + 8) + x_offset, y + y_offset);
            setWH(sprite, segment_width, row_height);
            setUV0(sprite, g_golem_panel_records[panel_index].attributes >> 11, g_golem_panel_records[panel_index].texture >> 3);
            sprite->clut = ((g_golem_panel_records[panel_index].texture >> 11) & 0x3F) | getClut(0, VRAM_CLUT_Y);
            addPrim(ordering_table, sprite);
            x_offset += (g_golem_panel_records[panel_index].texture >> 17) & 0x1FF;
            packet_cursor += sizeof(SPRT);
        }
        bottom_texture_height = g_golem_panel_records[panel_index].texture >> 26;
        cell_height = ((g_golem_panel_records[panel_index].dimensions.packed & 7) << 6) | bottom_texture_height;
        y_offset += cell_height;
        remaining_height = height - y_offset;
    }

    draw_mode = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_mode, 0, 0, ((g_golem_panel_records[panel_index].attributes & 3) << 5) | 5);
    addPrim(ordering_table, draw_mode);
    packet_cursor += sizeof(DR_TPAGE);
    return packet_cursor;
}

/**
 * @brief Draw a rotated logic-block icon from its base glyph and positioned parts.
 * @param packet_cursor Running packet cursor.
 * @param ordering_table Ordering-table tag for the icon packets.
 * @param block_index Logic-block index selecting the packed record.
 * @param rotation Index of the icon's four rotated layouts.
 * @param x          Screen x of the cell.
 * @param y          Screen y of the cell.
 * @param clut       CLUT selector passed to the part glyphs.
 * @param use_origin When 1, offset x/y by the layout row's origin fields.
 * @param style      Style flags forwarded to golem_emit_glyph for each part.
 * @return Packet cursor past the trailing draw-mode packet.
 * @see decomp.me (100.00%)
 * @see working/func_80141EB4_golem/
 */
u8* golem_draw_composite_icon(u8* packet_cursor, u_long* ordering_table, s32 block_index, s32 rotation, s32 x, s32 y, s32 clut, s32 use_origin, s32 style)
{
    LogicBlock logic_block;
    s32 layout_index;
    DR_TPAGE* draw_mode;
    s32 part_index;

    logic_block = GOLEM_PAD_CTX->logic_blocks[block_index];
    layout_index = logic_block.f.shape;
    if (use_origin == 1)
    {
        x += g_golem_composite_icon_rows[layout_index].header.origin_x * 8;
        y += g_golem_composite_icon_rows[layout_index].header.origin_y * 8;
    }
    packet_cursor = golem_emit_glyph(packet_cursor, ordering_table, logic_block.f.id + 0x13,
                                     (g_golem_composite_icon_rows[layout_index].rotations[rotation].origin.x * 8) + x,
                                     (g_golem_composite_icon_rows[layout_index].rotations[rotation].origin.y * 8) + y, 9, 0);
    for (part_index = 0; part_index < g_golem_composite_icon_rows[layout_index].header.part_count; part_index++)
    {
        packet_cursor =
            golem_emit_glyph(packet_cursor, ordering_table, g_golem_composite_icon_rows[layout_index].rotations[rotation].parts[part_index].glyph_id,
                             g_golem_composite_icon_rows[layout_index].rotations[rotation].parts[part_index].x * GOLEM_GRID_CELL_SIZE + x,
                             g_golem_composite_icon_rows[layout_index].rotations[rotation].parts[part_index].y * GOLEM_GRID_CELL_SIZE + y, clut, style);
    }
    draw_mode = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_mode, 0, 0, 0x25);
    addPrim(ordering_table, draw_mode);
    packet_cursor += sizeof(DR_TPAGE);
    return packet_cursor;
}

/**
 * @brief Emit one glyph: an optional colored backing TILE (chosen by the low
 *        style bits) followed by a textured SPRT.
 * @param packet_cursor Running packet cursor.
 * @param ordering_table Ordering-table tag for the glyph packets.
 * @param glyph_id Entry index into the glyph-metrics table.
 * @param x     Screen x of the glyph.
 * @param y     Screen y of the glyph.
 * @param clut CLUT selector; 0xF and 9 also gate brightness overrides.
 * @param style Bit 7 dims the sprite; low bits 1-3 select a backing color.
 * @return Packet cursor past the glyph packets.
 * @see decomp.me (100%)
 */
u8* golem_emit_glyph(u8* packet_cursor, u_long* ordering_table, s32 glyph_id, s32 x, s32 y, s32 clut, s32 style)
{
    TILE* tile;
    SPRT* sprite;
    GolemGlyphMetric* tile_metric;
    GolemGlyphMetric* sprite_metric;
    GolemGlyphMetric* tile_metrics;
    GolemGlyphMetric* sprite_metrics;

    if ((style & 0x7F) != 0)
    {
        tile = (TILE*)packet_cursor;
        switch (style & 0x7F)
        {
        case 1:
            SET_BGR0_PACKED(packet_cursor, GPU_COLOR_WORD(0x80, 0, 0));
            break;
        case 2:
            SET_BGR0_PACKED(packet_cursor, GPU_COLOR_WORD(0x80, 0, 0x80));
            break;
        case 3:
            SET_BGR0_PACKED(packet_cursor, GPU_COLOR_WORD(0, 0x80, 0));
            break;
        }
        setlen(tile, 3);
        setcode(tile, 0x62);
        tile_metrics = g_golem_glyph_metrics;
        tile_metric = &tile_metrics[glyph_id];
        setXY0(tile, x, y);
        setWH(tile, tile_metric->width, tile_metric->height);
        addPrim(ordering_table, tile);
        packet_cursor = (u8*)(tile + 1);
    }

    sprite = (SPRT*)packet_cursor;
    SET_BGR0_PACKED(sprite, 0x606060);
    setSprt(sprite);
    if (style & 0x80)
    {
        setRGB0(sprite, 0x38, 0x38, 0x38);
        setSemiTrans(sprite, 1);
    }
    if (clut == 0xF)
    {
        setRGB0(sprite, 0x40, 0x40, 0x40);
        setSemiTrans(sprite, 1);
    }
    if (clut != 9)
    {
        setSemiTrans(sprite, 1);
    }
    sprite_metrics = g_golem_glyph_metrics;
    sprite_metric = &sprite_metrics[glyph_id];
    setXY0(sprite, x, y);
    setWH(sprite, sprite_metric->width, sprite_metric->height);
    setUV0(sprite, sprite_metric->u0, sprite_metric->v0);
    sprite->clut = (clut & 0x3F) | getClut(0, VRAM_CLUT_Y);
    addPrim(ordering_table, sprite);
    return packet_cursor + sizeof(SPRT);
}

/**
 * @brief Emit a rectangle outline as four LINE_F2 packets (top, right,
 *        bottom, left) linked into the ordering table.
 * @param packet Running packet cursor; one 0x10-byte packet per edge.
 * @param ordering_table Ordering-table tag for the line packets.
 * @param x     Left edge x.
 * @param y     Top edge y.
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param color Packed BGR line color.
 * @return Packet cursor after four line packets.
 * @see decomp.me (100%)
 */
LINE_F2* golem_emit_panel_outline(LINE_F2* packet, u_long* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    SET_BGR0_PACKED(packet, color);
    setLineF2(packet);
    setXY0(packet, x, y);
    packet->x1 = x + width;
    packet->y1 = y;
    addPrim(ordering_table, packet);
    packet++;

    SET_BGR0_PACKED(packet, color);
    setLineF2(packet);
    setXY0(packet, x + width, y);
    packet->x1 = x + width;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    SET_BGR0_PACKED(packet, color);
    setLineF2(packet);
    setXY0(packet, x + width, y + height);
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    SET_BGR0_PACKED(packet, color);
    setLineF2(packet);
    setXY0(packet, x, y);
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    return packet + 1;
}

/**
 * @brief Set the screen-fade target color and step count.
 * @param red Target red component (0x100 = neutral).
 * @param green Target green component.
 * @param blue Target blue component.
 * @param steps Number of frames to reach the target.
 * @see decomp.me (100%)
 */
void golem_set_fade_target(s16 red, s16 green, s16 blue, s16 steps)
{
    g_golem_fade_target.red = red;
    g_golem_fade_target.green = green;
    g_golem_fade_target.blue = blue;
    g_golem_fade_target.steps_remaining = steps;
}

/**
 * @brief Step the current fade color toward the target, then emit a
 *        full-screen semi-transparent TILE plus its draw-mode packet unless
 *        the fade sits at neutral (0x100/0x100/0x100).
 * @param packet_cursor Next free byte in the GPU packet buffer.
 * @param ordering_table_tag  Ordering-table tag the packets are linked into.
 * @return The advanced packet cursor.
 * @see decomp.me (100%)
 */
u8* golem_render_fade(u8* packet_cursor, u_long* ordering_table_tag)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;
    DR_TPAGE* draw_mode_packet;

    if (g_golem_fade_target.steps_remaining != 0)
    {
        red_step = (g_golem_fade_target.red - g_golem_fade_current.red) / g_golem_fade_target.steps_remaining;
        green_step = (g_golem_fade_target.green - g_golem_fade_current.green) / g_golem_fade_target.steps_remaining;
        blue_step = (g_golem_fade_target.blue - g_golem_fade_current.blue) / g_golem_fade_target.steps_remaining;
        g_golem_fade_target.steps_remaining = g_golem_fade_target.steps_remaining - 1;
        g_golem_fade_current.red = g_golem_fade_current.red + red_step;
        g_golem_fade_current.green = g_golem_fade_current.green + green_step;
        g_golem_fade_current.blue = g_golem_fade_current.blue + blue_step;
    }
    else
    {
        g_golem_fade_current.red = g_golem_fade_target.red;
        g_golem_fade_current.green = g_golem_fade_target.green;
        g_golem_fade_current.blue = g_golem_fade_target.blue;
    }
    if ((g_golem_fade_current.red != GOLEM_FADE_NEUTRAL) || (g_golem_fade_current.green != g_golem_fade_current.red) ||
        (g_golem_fade_current.blue != g_golem_fade_current.green))
    {
        if (g_golem_fade_current.red >= GOLEM_FADE_ADDITIVE_THRESHOLD)
        {
            setRGB0((TILE*)packet_cursor, g_golem_fade_current.red - 1, g_golem_fade_current.green - 1, g_golem_fade_current.blue - 1);
        }
        else
        {
            if (g_golem_fade_current.red == GOLEM_FADE_NEUTRAL)
            {
                ((TILE*)packet_cursor)->r0 = 0;
            }
            else
            {
                ((TILE*)packet_cursor)->r0 = ~g_golem_fade_current.red;
            }
            if (g_golem_fade_current.green == GOLEM_FADE_NEUTRAL)
            {
                ((TILE*)packet_cursor)->g0 = 0;
            }
            else
            {
                ((TILE*)packet_cursor)->g0 = ~g_golem_fade_current.green;
            }
            if (g_golem_fade_current.blue == GOLEM_FADE_NEUTRAL)
            {
                ((TILE*)packet_cursor)->b0 = 0;
            }
            else
            {
                ((TILE*)packet_cursor)->b0 = ~g_golem_fade_current.blue;
            }
        }

        setTile((TILE*)packet_cursor);
        setSemiTrans((TILE*)packet_cursor, 1);
        ((TILE*)packet_cursor)->w = SCREEN_WIDTH;
        draw_mode = GOLEM_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0((TILE*)packet_cursor, 0, 0);
        ((TILE*)packet_cursor)->h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, (TILE*)packet_cursor);

        packet_cursor += sizeof(TILE);
        if (g_golem_fade_current.red < GOLEM_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = GOLEM_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        draw_mode_packet = (DR_TPAGE*)packet_cursor;
        setDrawTPage(draw_mode_packet, 0, 0, draw_mode);
        addPrim(ordering_table_tag, draw_mode_packet);

        packet_cursor += sizeof(DR_TPAGE);
    }
    return packet_cursor;
}

/**
 * @brief Append the encoded string @p src to the end of @p dest.
 * @param dest Destination encoded-text buffer (null-terminated).
 * @param src  Source encoded-text buffer (null-terminated).
 * @see decomp.me (100%)
 */
void golem_append_encoded_string(u8* dest, u8* src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = golem_encoded_string_length(dest);
    src_len = golem_encoded_string_length(src);
    for (i = 0; i < src_len; i++)
    {
        dest[dst_len + i] = src[i];
    }
    dest[dst_len + i] = 0;
}

/**
 * @brief Measure an encoded string's length in bytes: lead bytes 0x19..0x1F
 *        start a two-byte character, everything else is one byte.
 * @param text Null-terminated encoded-text buffer.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 golem_encoded_string_length(u8* text)
{
    u8* cursor;
    s32 byte_count;

    cursor = text;
    byte_count = 0;
    while (*cursor != 0)
    {
        if (IS_DBCS_LEAD_BYTE(*cursor))
        {
            cursor += 2;
            byte_count += 2;
        }
        else
        {
            cursor += 1;
            byte_count += 1;
        }
    }
    return byte_count;
}

/**
 * @brief Copy the encoded string @p src to @p dst, honoring two-byte
 *        characters, and null-terminate the destination.
 * @param dst Destination buffer.
 * @param src Source encoded-text buffer (null-terminated).
 * @see decomp.me (100%)
 */
void golem_copy_encoded_string(u8* dst, u8* src)
{
    const u8* scan_cursor;
    s32 byte_count;
    s32 i;

    scan_cursor = src;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    for (i = 0; i < byte_count; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}
