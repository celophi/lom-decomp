#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @brief Golem logic-block records within the shared menu-layout buffer.
 */
typedef struct
{
    u8 pad_0000[0x29D6];
    u8 logic_block_count;
    s8 selected_logic_type;
    u8 logic_types[3];
    u8 pad_29DB;
    u32 logic_blocks[40];
} GolemMenuData;

/** @brief Availability and palette data for one logic block. */
typedef struct
{
    u16 is_unavailable;
    u16 clut;
} GolemLogicBlockStatus;

/** @brief Panel animation words for the down, up, and rotate controls. */
typedef struct
{
    u32 scroll_down;
    u8 pad_04[0x10];
    u32 scroll_up;
    u8 pad_18[0x24];
    u32 rotate;
} GolemPanelAnimations;

/** @brief One 0x14-byte panel record. */
typedef struct
{
    u8 pad_00[0xA];
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u16 pad_12;
} GolemPanelRecord;

/** @brief VRAM destinations for a TIM image and its CLUT. */
typedef struct
{
    u16 x;
    u16 y;
    u16 clut_x;
    u16 clut_y;
} GolemImageClutPos;

/** @brief Signed position fields in one rotated composite-icon view. */
typedef struct
{
    u8 pad_00[8];
    s8 base_x;
    s8 base_y;
    u8 pad_0A[10];
} GolemIconVariantPosition;

/** @brief Bit-packed texture data for one 0x14-byte panel record. */
typedef struct
{
    u32 attributes;
    u32 texture;
    u32 dimensions;
    u8 pad_0C[8];
} GolemPanelTexture;

/** @brief Header view of one 88-byte composite-icon row. */
typedef struct
{
    u8 part_count;
    u8 reserved_01;
    u8 grid_width;
    u8 grid_height;
    s16 origin_x;
    s16 origin_y;
    s8 base_x;
    s8 base_y;
    u8 reserved_0A[78];
} GolemCompositeIconRow;

/** @brief Positioned glyph view at row + variant*0x14 + part*4. */
typedef struct
{
    u8 pad_00[0xC];
    s8 x;
    s8 y;
    s16 glyph_id;
} GolemCompositeIconPartView;

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

/** @brief LINE_F2-compatible packet used to emit a panel outline. */
typedef struct
{
    s32 tag;
    s32 color_and_code;
    s16 x0;
    s16 y0;
    s16 x1;
    u16 y1;
} GolemLinePacket;

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} GolemFadePrimitive;

/** @brief Fade color and its remaining interpolation step count. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 steps_remaining;
} GolemFadeState;

#define GOLEM_LOGIC_BLOCK(index) (((GolemMenuData*)g_menuLayoutBuffer)->logic_blocks[(index)])
#define GOLEM_PANEL_TEXTURE(index) (((GolemPanelTexture*)&g_golem_panel_records)[(index)])
#define GOLEM_ACTIVATE_PANEL(word) (((word) & ~0x780) | 0x180)
#define GOLEM_GPU_ADDRESS_MASK 0xFFFFFF
#define GOLEM_GPU_TAG_HIGH_MASK 0xFF000000
#define GOLEM_FADE_NEUTRAL 0x100
#define GOLEM_FADE_ADDITIVE_THRESHOLD (GOLEM_FADE_NEUTRAL + 1)
#define GOLEM_FADE_ADDITIVE_DRAW_MODE 0x25
#define GOLEM_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define GOLEM_NEXT_FADE_PRIMITIVE(primitive, type) ((GolemFadePrimitive*)((u8*)(primitive) + sizeof(type)))
#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))

extern u8 g_menuLayoutBuffer[];
extern s32 g_pad_input;
extern s32 g_frame_counter;
extern s32 D_80122C00;
extern u8 D_800EC3DA[];
extern u8* g_golem_render_buffers;
extern s32 g_golem_exit_requested;
extern s32 g_golem_work_buffer;
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
extern s32 D_8014C274;
extern s32 D_8014C278;
extern s32 g_golem_block_rotation;
extern s32 D_8014C280;
extern s32 g_golem_selected_block;
extern s32 g_golem_restore_slot_on_cancel;
extern u8 g_golem_ui_image[];
extern s32 g_golem_text_archive_offset;
extern GolemGlyphMetric g_golem_glyph_metrics[];
extern GolemPanelAnimations g_golem_panel_records;
extern GolemCompositeIconRow g_golem_composite_icon_rows[];

void func_800A3938();
void func_800CB918();
s32 func_800CBA9C();
s32 func_800CBC0C();
void func_800CBE64();
void golem_reset_block_position();
void golem_handle_input(void);
void golem_render();
void golem_upload_image_archive(GolemImageClutPos*, u8*);
s32 golem_emit_glyph();

/**
 * @brief Run the golem logic-grid editor until the user exits.
 * @param render_buffers Storage for the two render contexts and packet heap.
 * @param restore_slot_on_cancel Restore the prior logic-type slot on cancel.
 * @return None.
 * @see decomp.me (100%)
 */
void golem_run(u8* render_buffers, s32 restore_slot_on_cancel)
{
    u8* draw_buffer;
    u8* next_buffer;
    u8* other_buffer;
    u8* draw_env_buffers;

    g_golem_render_buffers = render_buffers;
    render_buffers += 0x8180;

    *(s16*)(g_golem_render_buffers + 0x4044) = 0;
    *(s16*)(g_golem_render_buffers + 0x4046) = 8;
    *(s16*)(g_golem_render_buffers + 0x4048) = 0x140;
    *(s16*)(g_golem_render_buffers + 0x404A) = 0xE0;
    *(s16*)(g_golem_render_buffers + 0x8104) = 0;
    *(s16*)(g_golem_render_buffers + 0x8106) = 0xF0;
    *(s16*)(g_golem_render_buffers + 0x8108) = 0x140;
    *(s16*)(g_golem_render_buffers + 0x810A) = 0xE0;

    SetDefDispEnv(g_golem_render_buffers + 0x4050, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(g_golem_render_buffers + 0x8110, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(g_golem_render_buffers + 0x4064, 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(g_golem_render_buffers + 0x8124, 0, 8, 0x140, 0xE0);

    draw_env_buffers = g_golem_render_buffers;
    draw_env_buffers[0x813A] = 0;
    draw_env_buffers[0x407A] = 0;
    g_golem_exit_requested = 0;
    *(s32*)(g_golem_render_buffers + 0x404C) = 0;
    *(s32*)(g_golem_render_buffers + 0x810C) = 1;

    g_golem_work_buffer = (golem_initialize_state(render_buffers, restore_slot_on_cancel) + 3) & ~3;

    next_buffer = g_golem_render_buffers;
    ClearOTagR(next_buffer, 0x10);
    ClearOTagR(g_golem_render_buffers + 0x40C0, 0x10);
    func_8002054C(0);
    func_80019FB8(next_buffer + 0x4050);
    func_800157DC();

    for (;;)
    {
        draw_buffer = next_buffer;
        ClearOTagR(draw_buffer, 0x10);
        *(u8**)(draw_buffer + 0x4040) = draw_buffer + 0x40;
        field_text_reset_scratch();
        func_800A9E78();
        golem_update_frame(draw_buffer);
        func_80063194();
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);

        if (g_golem_exit_requested != 0)
        {
            break;
        }

        ClearImage(draw_buffer + 0x4044, 0, 0, 0);
        other_buffer = g_golem_render_buffers;
        if (draw_buffer == g_golem_render_buffers)
        {
            other_buffer = draw_buffer + 0x40C0;
        }
        next_buffer = other_buffer;
        PutDispEnv(other_buffer + 0x4050);
        PutDrawEnv(next_buffer + 0x4064);
        DrawOTag(draw_buffer + 0x3C);
        draw_buffer = other_buffer;
        func_800157DC();
        func_800122C0();
    }

    func_800AA02C();
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
    u8* menu_data;
    u8* menu_data2;
    s32 stack_pad[2];

    g_golem_restore_slot_on_cancel = restore_slot_on_cancel;
    if (restore_slot_on_cancel != 0)
    {
        slot_index = 0;
        menu_data = g_menuLayoutBuffer;
        selected_logic_type = *(s8*)(menu_data + 0x29D7);
        g_golem_saved_logic_type_slot = D_80122C00;
        D_80122C00 = 0;
        do
        {
            if (menu_data[slot_index + 0x29D8] == selected_logic_type)
            {
                D_80122C00 = slot_index;
            }
            slot_index++;
        } while (slot_index < 3);
    }

    g_golem_grid_size_class = func_800CB758() - 4;
    menu_data2 = g_menuLayoutBuffer;
    logic_type = menu_data2[D_80122C00 + 0x29D8];
    g_golem_block_rotation = 0;
    g_golem_block_y = 0;
    g_golem_block_x = 0;
    g_golem_is_placing_block = 0;
    g_golem_cursor_target_x = 0xB8;
    g_golem_cursor_x = 0xB8;
    g_golem_cursor_target_y = 0x50;
    g_golem_cursor_y = 0x50;
    g_golem_active_logic_type = logic_type;
    golem_reset_cursor_motion(logic_type);
    g_golem_logic_block_count = func_800CBD70(g_golem_block_status);
    g_golem_scroll_steps = 0;
    g_golem_scroll_y = 0;
    g_golem_scroll_target_y = 0;
    g_golem_selected_block = 0;
    golem_upload_ui_image();
    func_800AA02C();
    golem_set_fade_target(0x100, 0x100, 0x100, 6);
    D_8014C26C = 0;
    D_8014C280 = 0;
    D_8014C278 = 0;
    D_8014C274 = 0;
    return work_buffer;
}

/**
 * @brief Upload the golem editor image and CLUT to their VRAM locations.
 * @return None.
 * @see decomp.me (100%)
 */
void golem_upload_ui_image(void)
{
    GolemImageClutPos destinations;

    destinations.x = 0x140;
    destinations.y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    golem_upload_image_archive(&destinations, g_golem_ui_image);
}

/**
 * @brief Upload a TIM image and its optional CLUT to VRAM.
 * @param destinations VRAM destinations for the image and CLUT blocks.
 * @param tim TIM resource to upload.
 * @return None.
 * @see decomp.me (100%)
 */
void golem_upload_image_archive(GolemImageClutPos* destinations, u8* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    u16* pixel_dimensions;

    flags = *(s32*)(tim + 4);
    clut_block_size = *(s32*)(tim + 8);

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
}

/**
 * @brief Render and update one editor frame.
 * @note The incoming a0 render-context argument is forwarded to golem_render.
 * @return None.
 * @see decomp.me (100%)
 */
void golem_update_frame(void)
{
    golem_render();
    g_frame_counter += 1;
    golem_handle_input();
    if (D_8014C274 != 0)
    {
        D_8014C278 += (D_8014C280 - D_8014C278) / D_8014C274;
        D_8014C274 -= 1;
        return;
    }
    D_8014C278 = D_8014C280;
}

/**
 * @brief Handle logic-block selection, placement, rotation, and cancellation.
 * @return None.
 * @see decomp.me (100%)
 */
void golem_handle_input(void)
{
    s32 saved_x, saved_y, repeat_count, block_index, scroll_steps, input, cancel_pressed;
    if ((g_pad_input & 0x800) && (g_golem_is_placing_block == 0))
    {
        goto cancel;
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
        g_golem_selected_block = g_golem_scroll_y / 40;
    }
    else
    {
        g_golem_scroll_y = g_golem_scroll_target_y;
        g_golem_selected_block = g_golem_scroll_target_y / 40;
    }
    if (g_golem_scroll_steps != 0)
    {
        return;
    }
    if (g_golem_is_placing_block != 0)
    {
        input = g_pad_input;
        if (input & 0xF000)
        {
            saved_x = g_golem_block_x;
            saved_y = g_golem_block_y;
            if (input & 0x1000)
            {
                g_golem_block_y = saved_y - 1;
            }
            else if (input & 0x4000)
            {
                g_golem_block_y = saved_y + 1;
            }
            else if (input & 0x8000)
            {
                g_golem_block_x = saved_x - 1;
            }
            else if (input & 0x2000)
            {
                g_golem_block_x = saved_x + 1;
            }
            if (func_800CBC0C(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y) == 0)
            {
                g_golem_block_x = saved_x;
                g_golem_block_y = saved_y;
                func_800A3938(0x78, 0x80);
            }
            else
            {
                func_800A3938(0x7D, 0x80);
            }
            return;
        }
        if (input & 0x90)
        {
            if (input & 0x10)
            {
                if (g_golem_block_rotation >= 3)
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
                    g_golem_block_rotation = 3;
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
            func_800A3938(0x7D, 0x80);
            g_golem_panel_records.rotate = GOLEM_ACTIVATE_PANEL(g_golem_panel_records.rotate);
            return;
        }
        if (input & 0xA20)
        {
            if (func_800CBA9C(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y) != 0)
            {
                g_golem_is_placing_block = 0;
                golem_reset_cursor_motion();
                func_800A3938(0x120, 0x80);
                func_800CB918(g_golem_selected_block, g_golem_block_rotation, g_golem_block_x, g_golem_block_y);
            }
            else
            {
                func_800A3938(0x78, 0x80);
            }
            return;
        }
        if (input & 0x40)
        {
            g_golem_is_placing_block = 0;
            golem_reset_cursor_motion();
            func_800A3938(0x78, 0x80);
            GOLEM_LOGIC_BLOCK(g_golem_selected_block) |= 3;
        }
        return;
    }
    if (g_golem_logic_block_count != 0)
    {
        repeat_count = 1;
        input = g_pad_input;
        if (input & 4)
        {
            g_pad_input = 0x1000;
            repeat_count = 3;
        }
        else if (input & 8)
        {
            g_pad_input = 0x4000;
            repeat_count = 3;
        }
        input = g_pad_input;
        if (input & 0x5000)
        {
            func_800A3938(0x7D, 0x80);
            while (repeat_count != 0)
            {
                if (g_pad_input & 0x1000)
                {
                    if (g_golem_scroll_target_y != 0)
                    {
                        g_golem_scroll_steps = 4;
                        g_golem_scroll_target_y -= 40;
                        g_golem_panel_records.scroll_up = GOLEM_ACTIVATE_PANEL(g_golem_panel_records.scroll_up);
                    }
                }
                else if ((g_pad_input & 0x4000) && ((g_golem_scroll_target_y / 40) != (g_golem_logic_block_count - 1)))
                {
                    g_golem_scroll_steps = 4;
                    g_golem_scroll_target_y += 40;
                    g_golem_panel_records.scroll_down = GOLEM_ACTIVATE_PANEL(g_golem_panel_records.scroll_down);
                }
                repeat_count--;
            }
            return;
        }
        if (input & 2)
        {
            s32 limit, logic_type;
            GolemMenuData* menu_data;
            block_index = g_golem_selected_block;
            repeat_count = 0;
            if (g_golem_logic_block_count > 0)
            {
                limit = g_golem_logic_block_count;
                menu_data = (GolemMenuData*)g_menuLayoutBuffer;
                logic_type = g_golem_active_logic_type;
                block_index++;
            forward_loop:
                if (block_index == limit)
                {
                    block_index = 0;
                }
                repeat_count++;
                if ((menu_data->logic_blocks[block_index] & 3) == logic_type)
                {
                    goto forward_done;
                }
                block_index++;
                if (repeat_count < limit)
                {
                    goto forward_loop;
                }
                block_index--;
            }
        forward_done:
            g_golem_scroll_target_y = block_index * 40;
            g_golem_scroll_steps = 4;
            g_golem_panel_records.scroll_down = GOLEM_ACTIVATE_PANEL(g_golem_panel_records.scroll_down);
            func_800A3938(0x7D, 0x80);
            return;
        }
        if (input & 1)
        {
            s32 limit, logic_type;
            GolemMenuData* menu_data;
            s32 selected_tmp;
            s32* countp;
            do { selected_tmp = g_golem_selected_block; } while (0);
            input = g_golem_logic_block_count;
            block_index = selected_tmp;
            repeat_count = 0;
            if (input > 0)
            {
                limit = input;
                menu_data = (GolemMenuData*)g_menuLayoutBuffer;
                logic_type = g_golem_active_logic_type;
                block_index--;
            backward_loop:
                if (block_index < 0)
                {
                    block_index = limit - 1;
                }
                countp = &g_golem_logic_block_count;
                if ((menu_data->logic_blocks[block_index] & 3) == logic_type)
                {
                    goto backward_done;
                }
                {
                    s32 loop_count;
                    do { loop_count = *countp; } while (0);
                    repeat_count++;
                    block_index--;
                    if (repeat_count < loop_count)
                    {
                        goto backward_loop;
                    }
                }
                block_index++;
            }
        backward_done:
            g_golem_scroll_target_y = block_index * 40;
            g_golem_scroll_steps = 4;
            g_golem_panel_records.scroll_up = GOLEM_ACTIVATE_PANEL(g_golem_panel_records.scroll_up);
            func_800A3938(0x7D, 0x80);
            return;
        }
        if (input & 0x220)
        {
            if (g_golem_block_status[g_golem_selected_block].is_unavailable == 0)
            {
                g_golem_is_placing_block = 1;
                if ((GOLEM_LOGIC_BLOCK(g_golem_selected_block) & 3) == g_golem_active_logic_type)
                {
                    g_golem_block_x = (s32)(GOLEM_LOGIC_BLOCK(g_golem_selected_block) << 8) >> 27;
                    g_golem_block_y = (s32)(GOLEM_LOGIC_BLOCK(g_golem_selected_block) << 3) >> 27;
                    g_golem_block_rotation = (GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 17) & 3;
                }
                else
                {
                    golem_reset_block_position();
                    g_golem_block_rotation = 0;
                }
                func_800CBE64(g_golem_selected_block);
                func_800A3938(0x7E, 0x80);
            }
            return;
        }
        cancel_pressed = input & 0x40;
    }
    else
    {
        cancel_pressed = g_pad_input & 0x40;
    }
    if (cancel_pressed != 0)
    {
    cancel:
        func_800A3938(0x7D, 0x80);
        g_golem_exit_requested = 1;
        if (g_golem_restore_slot_on_cancel != 0)
        {
            D_80122C00 = g_golem_saved_logic_type_slot;
        }
    }
}
/**
 * @brief Reset the active block to its layout-defined grid origin.
 * @return None.
 * @see decomp.me (100%)
 */
void golem_reset_block_position(void)
{
    if (g_golem_grid_size_class == 0)
    {
        GolemCompositeIconRow* rows;
        g_golem_block_x = (rows = g_golem_composite_icon_rows)[(GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 12) & 0xF].grid_width - 1;
        g_golem_block_y = rows[(GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 12) & 0xF].grid_height - 1;
        return;
    }
    if (g_golem_grid_size_class >= 0)
    {
        if (g_golem_grid_size_class < 3)
        {
            GolemCompositeIconRow* rows = g_golem_composite_icon_rows;
            g_golem_block_x = rows[(GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 12) & 0xF].grid_width;
            g_golem_block_y = rows[(GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 12) & 0xF].grid_height;
        }
    }
}

/**
 * @brief Emit the logic-grid divider markers and their texture-page packet.
 * @param packet_cursor Packet cursor, threaded through the marker emitters.
 * @param ordering_table Ordering-table tag for the marker packets.
 * @return Packet cursor after the marker run.
 * @note The 0x50 compare is materialized into @c glyph first, @c call_x holds a
 *       copy of @c x in both branches, and the remaining do{}while(0) wrappers
 *       and the recomputed marker_ptr pointer arithmetic are required to match
 *       the target's register allocation. The @c y increment does not need a
 *       wrapper.
 * @see decomp.me (100.00%)
 * @see working/func_80140DEC/
 */
s32 golem_draw_grid_markers(s32 packet_cursor, s32 ordering_table)
{
    s32 markers[60];
    s32 marker_index;
    s32 row;
    s32 column;
    s32 x;
    s32 y;
    s32 marker;
    s32 marker2;
    s32 glyph;
    s32 *marker_base;
    s32 *marker_ptr;
    s32 row_y;
    s32 call_x;

    func_800CBEC4(markers);

    marker_index = 0;
    row = marker_index;
    marker_base = markers;
    y = 4;
    for (; row < 6; row++)
    {
        column = 0;
        row_y = y;
        marker_ptr = (s32 *)((marker_index << 2) + (s32)marker_base);
        x = 0xC;
        for (; column < 5; )
        {
            marker = *marker_ptr;
            if (marker != 0)
            {
                glyph = 0x50;
                if (marker == glyph)
                {
                    call_x = x;
                    glyph = 2;
                }
                else
                {
                    call_x = x;
                    glyph = 3;
                }
                packet_cursor = golem_emit_grid_marker(packet_cursor, ordering_table, call_x, row_y, glyph);
            }
            x += 0x10;
            do { column++; } while (0);
            marker_ptr++;
            marker_index++;
        }
        y += 0x10;
    }

    for (row = 0; row < 5; row++)
    {
        column = 0;
        y = row << 4;
        marker_ptr = (s32 *)((marker_index << 2) + (s32)markers);
        x = 4;
        for (; column < 6; )
        {
            marker2 = *marker_ptr;
            if (marker2 != 0)
            {
                do { packet_cursor = golem_emit_grid_marker(packet_cursor, ordering_table, x, y + 0xC, marker2 != 0x4E); } while (0);
            }
            x += 0x10;
            do { column++; } while (0);
            marker_ptr++;
            marker_index++;
        }
    }

    return golem_finish_grid_marker_run(packet_cursor, ordering_table);
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
s32 golem_emit_grid_marker(s32 packet_cursor, s32 ordering_table, s32 x, s32 y, s32 glyph)
{
    SPRT* sprite = (SPRT*)packet_cursor;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setWH(sprite, 8, 8);
    setUV0(sprite, glyph * 8 - 0x70, 0x58);
    setXY0(sprite, x, y);
    sprite->clut = 0x7C87;
    addPrim(ordering_table, sprite);
    return packet_cursor + 0x14;
}

/**
 * @brief Return the animated cursor to its resting target.
 * @return None.
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
s32 golem_finish_grid_marker_run(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, getTPage(0, 1, 320, 0));
    addPrim(ordering_table, draw_mode);
    return packet_cursor + 8;
}

/**
 * @brief Draw and advance the animated logic-grid cursor.
 * @param packet_cursor Next free GPU packet.
 * @param render_context Render context containing the cursor ordering-table tag.
 * @return Packet cursor past the sprite and draw-mode packet.
 * @see decomp.me (100%)
 */
s32 golem_draw_cursor(s32 packet_cursor, s32 render_context)
{
    SPRT* sprite = (SPRT*)packet_cursor;
    DR_TPAGE* draw_mode;
    s32 ordering_table;
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
    sprite->clut = 0x7C87;
    setXY0(sprite, cursor_x + 8, cursor_y);
    addPrim(render_context + 0x2C, sprite);

    packet_cursor += 0x14;
    ordering_table = render_context + 0x2C;

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
    return packet_cursor + 8;
}

/**
 * @brief Emit the golem panel grid, the cursor, and the selected item's name
 *        and detail text into the render context's packet buffer.
 * @param render_context Render context with the packet cursor and ordering table.
 * @return None.
 * @see decomp.me (100.00%)
 */
void golem_render(s32 render_context)
{
    s32 stack_pad[2];
    u8 name_buf[0x100];
    u8 number_buf[0x100];
    GolemPanelRecord* panel_record;
    s32 archive;
    s32 archive_base;
    u8* offsets;
    u8* text_base;
    s32 packet_cursor;
    s32 panel_index;
    s32 panel_ordering_table;
    s32 text_ordering_table;
    s32 detail_block;

    panel_ordering_table = render_context + 0x3C;
    packet_cursor = *(s32*)(render_context + 0x4040);
    panel_index = 0;
    do
    {
        panel_record = (GolemPanelRecord*)&g_golem_panel_records + panel_index;
        packet_cursor =
            golem_draw_panel(packet_cursor, panel_ordering_table, panel_index, panel_record->x, panel_record->y, panel_record->width, panel_record->height);
        panel_index++;
    } while (panel_index < 0x9A);

    packet_cursor = golem_draw_block_list(packet_cursor, render_context);
    packet_cursor = golem_draw_logic_grid(packet_cursor, render_context);
    packet_cursor = golem_draw_cursor(packet_cursor, render_context);
    do { do { do { do { do { do { do { do { do { do { text_ordering_table = render_context + 0x28; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);

    if (g_golem_logic_block_count != 0)
    {
        golem_copy_encoded_string(name_buf,
                                  g_golem_text_archive_offset +
                                      (*(u16*)(((u8)GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 2) * 2 + g_golem_text_archive_offset +
                                                (archive = (archive_base = (s32)&g_golem_text_archive_offset) - 4)) +
                                       archive));
        if ((GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 8) & 0xF)
        {
            golem_append_encoded_string(name_buf, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(number_buf, (GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 8) & 0xF, 1);
            golem_append_encoded_string(name_buf, number_buf);
        }
        packet_cursor = func_800A88A0(packet_cursor, text_ordering_table, name_buf, 0, 0xA0, 0xA0, 2);
        detail_block = *(s32*)(archive + 8);
        packet_cursor = func_800A88A0(packet_cursor, text_ordering_table,
                                      detail_block +
                                          (*(u16*)(((u8)GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 2) * 2 + detail_block + archive) + archive),
                                      0, 0xA0, 0xB0, 2);
    }

    *(s32*)(render_context + 0x4040) = golem_render_fade(packet_cursor, render_context + 0x24);
}

/**
 * @brief Link a base primitive into the render context OT, emit a highlight
 *        primitive for each on-screen panel cell, then link a final frame prim.
 * @param packet_buffer Starting primitive pointer for this pass.
 * @param render_context Render context containing the panel ordering table.
 * @return Packet cursor after the final frame primitive.
 * @note The parameter is copied into @c packet_cursor (a local); the target
 *       keeps an addu copy of the incoming argument, so the copy is required
 *       to match. The per-cell y offset is @c block_index * 0x28 rather than a
 *       running accumulator for the same reason.
 * @see decomp.me (100.00%)
 * @see working/func_80141478/code.c
 */
s32 golem_draw_block_list(s32 packet_buffer, s32 render_context)
{
    u8 draw_env[0x60];
    s32 ordering_table;
    s32 next_packet;
    s32 block_y;
    s32 block_index;
    s32 draw_y;
    s32 scroll_above;
    s32 packet_cursor;

    packet_cursor = packet_buffer;
    block_index = 0;
    ordering_table = render_context + 0x38;
    SetDrawEnv(packet_cursor, g_golem_render_buffers + (*(s32*)(render_context + 0x404C) ^ 1) * 0x40C0 + 0x4064);
    next_packet = packet_cursor + 0x40;
    addPrim(ordering_table, packet_cursor);

    if (g_golem_logic_block_count > 0)
    {
        do
        {
            scroll_above = g_golem_scroll_y - 0x28;
            block_y = block_index * 0x28 - scroll_above;
            if (block_y >= -0x27)
            {
                if (block_y < 0x78)
                {
                    if (g_golem_selected_block == block_index && g_golem_block_status[block_index].is_unavailable == 0)
                    {
                        next_packet = golem_draw_composite_icon(next_packet, ordering_table, block_index, 0, 6, block_y + 4,
                                                                g_golem_block_status[block_index].clut, 1, g_golem_is_placing_block ? 0x80 : 2);
                    }
                    else
                    {
                        next_packet = golem_draw_composite_icon(next_packet, ordering_table, block_index, 0, 6,
                                                                block_index * 0x28 - g_golem_scroll_y + 0x2C,
                                                                g_golem_block_status[block_index].clut, 1, 0);
                    }
                }
            }
            block_index++;
        } while (block_index < g_golem_logic_block_count);
    }

    packet_cursor = next_packet;
    draw_y = 0x1E;
    if (*(s32*)(render_context + 0x404C) != 0)
    {
        draw_y = 0x106;
    }
    SetDefDrawEnv(draw_env, 0xCA, draw_y, 0x4C, 0x74);
    SetDrawEnv(packet_cursor, draw_env);
    addPrim(ordering_table, packet_cursor);
    return packet_cursor + 0x40;
}

/**
 * @brief Link a base primitive into the render context OT, emit a highlight prim
 *        for the selected cursor cell and for each active grid cell, then draw a
 *        framing box sized by the current panel mode.
 * @param packet_cursor Running primitive pointer, advanced for each primitive.
 * @param render_context Render context containing the grid ordering table.
 * @return Packet cursor after the final frame primitive.
 * @note The incoming @c packet_cursor is copied into @c cursor (an addu copy
 *       the target keeps and reuses for the final frame prim). The variant
 *       address is split into @c icon_base / @c icon_offset, the block loop
 *       re-reads GOLEM_LOGIC_BLOCK for the y coordinate, and each switch case
 *       emits its own SetDefDrawEnv with immediate args; all are required to
 *       match the target's register allocation.
 * @see decomp.me (100.00%)
 * @see working/func_801416C8/code.c
 */
s32 golem_draw_logic_grid(s32 packet_cursor, s32 render_context)
{
    u8 draw_env[0x60];
    s32 cursor;
    s32 ordering_table;
    s32 next_packet;
    s32 block_index;
    u32 logic_block;
    GolemIconVariantPosition* variant_position;
    s32 cursor_target_x;
    s32 cursor_target_y;
    s32 draw_x;
    s32 draw_y;
    s32 draw_size;
    GolemLogicBlockStatus* block_status;
    u8* logic_block_ptr;
    u8* icon_base;
    s32 icon_offset;

    cursor = packet_cursor;
    ordering_table = render_context + 0x34;
    func_8001A5D4(cursor, g_golem_render_buffers + (*(s32*)(render_context + 0x404C) ^ 1) * 0x40C0 + 0x4064);
    addPrim(ordering_table, cursor);
    cursor += 0x40;
    next_packet = cursor;

    if (g_golem_is_placing_block != 0)
    {
        next_packet = golem_draw_composite_icon(cursor, ordering_table, g_golem_selected_block, g_golem_block_rotation, g_golem_block_x * 0x10,
                                                g_golem_block_y * 0x10, g_golem_block_status[g_golem_selected_block].clut, 0, 3);
        icon_base = (u8*)g_golem_composite_icon_rows;
        icon_offset = g_golem_block_rotation * 0x14 + ((GOLEM_LOGIC_BLOCK(g_golem_selected_block) >> 12) & 0xF) * 0x58;
        variant_position = (GolemIconVariantPosition*)(icon_base + icon_offset);
        cursor_target_x = variant_position->base_x * 8 + g_golem_block_x * 0x10 - g_golem_grid_size_class * 8 + 0x3C;
        cursor_target_y = variant_position->base_y * 8 + g_golem_block_y * 0x10 - g_golem_grid_size_class * 8 + 0x3C;
        if ((cursor_target_x != g_golem_cursor_x || cursor_target_y != g_golem_cursor_y) && g_golem_cursor_steps == 0)
        {
            g_golem_cursor_target_x = cursor_target_x;
            g_golem_cursor_target_y = cursor_target_y;
            g_golem_cursor_steps = 4;
        }
    }

    next_packet = golem_draw_grid_markers(next_packet, ordering_table);

    block_index = 0;
    if (g_golem_logic_block_count > 0)
    {
        block_status = g_golem_block_status;
        logic_block_ptr = &g_menuLayoutBuffer[0];
        do
        {
            logic_block = GOLEM_LOGIC_BLOCK(block_index);
            if ((logic_block & 3) == g_golem_active_logic_type)
            {
                next_packet = golem_draw_composite_icon(next_packet, ordering_table, block_index, (logic_block >> 0x11) & 3,
                                                        ((s32)(logic_block << 8) >> 27) << 4, ((s32)(GOLEM_LOGIC_BLOCK(block_index) << 3) >> 27) << 4,
                                                        block_status->clut, 0,
                                                        block_index == g_golem_selected_block ? (g_golem_is_placing_block ? 0x80 : 2) : 0);
            }
            block_status++;
            logic_block_ptr += 4;
            block_index++;
        } while (block_index < g_golem_logic_block_count);
    }

    cursor = next_packet;

    switch (g_golem_grid_size_class)
    {
    case 0:
        draw_y = 0x38;
        if (*(s32*)(render_context + 0x404C) != 0)
        {
            draw_y = 0x120;
        }
        func_8001C56C(draw_env, 0x50, draw_y, 0x40, 0x40);
        break;
    case 1:
        draw_y = 0x30;
        if (*(s32*)(render_context + 0x404C) != 0)
        {
            draw_y = 0x118;
        }
        func_8001C56C(draw_env, 0x48, draw_y, 0x50, 0x50);
        break;
    case 2:
        draw_y = 0x28;
        if (*(s32*)(render_context + 0x404C) != 0)
        {
            draw_y = 0x110;
        }
        func_8001C56C(draw_env, 0x40, draw_y, 0x60, 0x60);
        break;
    }

    func_8001A5D4(cursor, draw_env);
    addPrim(ordering_table, cursor);
    return cursor + 0x40;
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
s32 golem_draw_panel(s32 packet_cursor, s32 ordering_table, s32 panel_index, s32 x, s32 y, s32 width, s32 height)
{
    SPRT* sprite;
    s32 animation;
    s32 color;
    s32 y_offset;
    s32 x_offset;
    s32 row_height;
    s32 cell_height;
    s32 segment_width;
    s32 available_width;
    s32 remaining_height;
    s32 packet_code;
    s32 bottom_texture_height;
    u8 stack_pad[0x10];

    color = 0x808080;

    switch ((GOLEM_PANEL_TEXTURE(panel_index).attributes >> 3) & 0xF)
    {
    case 0:
    case 7:
        break;
    case 1:
    case 2:
    case 3:
        if ((((GOLEM_PANEL_TEXTURE(panel_index).attributes >> 3) & 0xF) - 1) != g_golem_grid_size_class)
        {
            return packet_cursor;
        }
        break;
    case 4:
        if (g_golem_scroll_y == 0)
        {
            return packet_cursor;
        }
        animation = (GOLEM_PANEL_TEXTURE(panel_index).attributes >> 7) & 0xF;
        if (animation != 0)
        {
            color = 0xC0;
            GOLEM_PANEL_TEXTURE(panel_index).attributes = (GOLEM_PANEL_TEXTURE(panel_index).attributes & ~0x780) | (((animation - 1) & 0xF) << 7);
        }
        break;
    case 5:
        if (!((g_golem_scroll_y / 40) < (g_golem_logic_block_count - 1)))
        {
            return packet_cursor;
        }
        animation = (GOLEM_PANEL_TEXTURE(panel_index).attributes >> 7) & 0xF;
        if (animation != 0)
        {
            color = 0xC0;
            GOLEM_PANEL_TEXTURE(panel_index).attributes = (GOLEM_PANEL_TEXTURE(panel_index).attributes & ~0x780) | (((animation - 1) & 0xF) << 7);
        }
        break;
    case 6:
        animation = (GOLEM_PANEL_TEXTURE(panel_index).attributes >> 7) & 0xF;
        if (animation != 0)
        {
            color = 0xC0C0C0;
            GOLEM_PANEL_TEXTURE(panel_index).attributes = (GOLEM_PANEL_TEXTURE(panel_index).attributes & ~0x780) | (((animation - 1) & 0xF) << 7);
        }
        break;
    }

    ordering_table += 1;
    ordering_table -= 1;
    y_offset = 0;
    if (y_offset < height)
    {
        do
        {
            remaining_height = height - y_offset;
            row_height = remaining_height;
            segment_width = GOLEM_PANEL_TEXTURE(panel_index).texture >> 26;
            cell_height = ((GOLEM_PANEL_TEXTURE(panel_index).dimensions & 7) << 6) | segment_width;
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
                segment_width = (GOLEM_PANEL_TEXTURE(panel_index).texture >> 17) & 0x1FF;
                if (segment_width >= available_width)
                {
                    segment_width = available_width;
                }
                SET_BGR0_PACKED(sprite, color);
                setlen(sprite, 4);
                do { do { setcode(sprite, packet_code); } while (0); } while (0);
                if ((GOLEM_PANEL_TEXTURE(panel_index).attributes >> 2) & 1)
                {
                    setcode(sprite, 0x66);
                }
                sprite->x0 = (x + 8) + x_offset;
                sprite->y0 = y + y_offset;
                do { do { do { do { do { do { do { do { do { do { do { do { do { do { do { do { do { do { do { do { sprite->w = segment_width; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
                sprite->h = row_height;
                sprite->u0 = GOLEM_PANEL_TEXTURE(panel_index).attributes >> 11;
                sprite->v0 = GOLEM_PANEL_TEXTURE(panel_index).texture >> 3;
                sprite->clut = ((GOLEM_PANEL_TEXTURE(panel_index).texture >> 11) & 0x3F) | 0x7C80;
                addPrim(ordering_table, sprite);
                x_offset += (GOLEM_PANEL_TEXTURE(panel_index).texture >> 17) & 0x1FF;
                packet_cursor += 0x14;
            }
            bottom_texture_height = GOLEM_PANEL_TEXTURE(panel_index).texture >> 26;
            cell_height = ((GOLEM_PANEL_TEXTURE(panel_index).dimensions & 7) << 6) | bottom_texture_height;
            y_offset += cell_height;
            remaining_height = height - y_offset;
        } while (y_offset < height);
    }

    setlen(packet_cursor, 1);
    ((u_long*)packet_cursor)[1] = ((GOLEM_PANEL_TEXTURE(panel_index).attributes & 3) << 5) | 0xE1000005;
    addPrim(ordering_table, packet_cursor);
    return packet_cursor + 8;
}

/**
 * @brief Draw one grid cell's icon: the base glyph plus each positioned part
 *        from its layout row, then splice a draw-mode packet.
 * @param packet_cursor Running packet cursor.
 * @param ordering_table Ordering-table tag for the icon packets.
 * @param block_index Logic-block index selecting the packed record.
 * @param rotation Layout rotation index; each view has a 0x14-byte stride.
 * @param x          Screen x of the cell.
 * @param y          Screen y of the cell.
 * @param clut       CLUT selector passed to the part glyphs.
 * @param use_origin When 1, offset x/y by the layout row's origin fields.
 * @param style      Style flags forwarded to golem_emit_glyph for each part.
 * @return Packet cursor past the trailing draw-mode packet.
 * @note The part-loop advances @c part_offset / @c part_index after the
 *       golem_emit_glyph call, and the trailing draw-mode packet is built from
 *       named constant locals inside a @c do{}while(0); both shapes are required
 *       to match the target's register allocation.
 * @see decomp.me (100.00%)
 * @see working/func_80141EB4_golem/
 */
s32 golem_draw_composite_icon(s32 packet_cursor, s32* ordering_table, s32 block_index, s32 rotation, s32 x, s32 y, s32 clut, s32 use_origin, s32 style)
{
    u32 logic_block;
    s32 layout_index;
    s32 rotation_offset;
    s32 layout_offset;
    u8* table;
    u8* layout;
    s32 part_index;
    u32 low_mask;
    u32 high_mask;
    u32 draw_cmd;
    u32 one;

    logic_block = GOLEM_LOGIC_BLOCK(block_index);
    layout_index = logic_block >> 0xC;
    layout_index = layout_index & 0xF;
    if (use_origin == 1)
    {
        u8* base = (u8*)g_golem_composite_icon_rows;
        GolemCompositeIconRow* row = (GolemCompositeIconRow*)(base + layout_index * 0x58);
        x += row->origin_x * 8;
        y += row->origin_y * 8;
    }
    table = (u8*)g_golem_composite_icon_rows;
    rotation_offset = rotation * 0x14;
    layout_offset = layout_index * 0x58;
    {
        GolemCompositeIconRow* variant = (GolemCompositeIconRow*)(rotation_offset + layout_offset + table);
        packet_cursor =
            golem_emit_glyph(packet_cursor, ordering_table, ((logic_block >> 2) & 0x3F) + 0x13, (variant->base_x * 8) + x, (variant->base_y * 8) + y, 9, 0);
    }
    layout = layout_offset + table;
    part_index = 0;
    if (*layout != 0)
    {
        u8* table_base = table;
        s32 row_base = layout_offset;
        u8* part_count = layout;
        s32 part_offset = rotation_offset;
        do
        {
            GolemCompositeIconPartView* part = (GolemCompositeIconPartView*)(part_offset + row_base + (s32)table_base);
            packet_cursor = golem_emit_glyph(packet_cursor, ordering_table, part->glyph_id, (part->x * 0x10) + x, (part->y * 0x10) + y, clut, style);
            part_offset += 4;
            part_index += 1;
        } while (part_index < *part_count);
    }
    draw_cmd = 0xE1000025;
    low_mask = 0xFFFFFF;
    one = 1;
    high_mask = 0xFF000000;
    do
    {
        *(u8*)(packet_cursor + 3) = one;
        *(u32*)(packet_cursor + 4) = draw_cmd;
    } while (0);
    *(u32*)(packet_cursor + 0) = (*(u32*)(packet_cursor + 0) & high_mask) | (*ordering_table & low_mask);
    *ordering_table = (*ordering_table & high_mask) | (packet_cursor & low_mask);
    return packet_cursor + 8;
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
s32 golem_emit_glyph(s32 packet_cursor, s32 ordering_table, s32 glyph_id, s32 x, s32 y, s32 clut, s32 style)
{
    TILE* tile;
    SPRT* sprite;
    GolemGlyphMetric* metric;
    GolemGlyphMetric* metric2;
    u32 color;
    GolemGlyphMetric* metrics;
    GolemGlyphMetric* metrics2;

    if ((style & 0x7F) != 0)
    {
        tile = (TILE*)packet_cursor;
        switch (style & 0x7F)
        {
        case 1:
            color = 0x80;
            break;
        case 2:
            color = 0x800080;
            break;
        case 3:
            color = 0x8000;
            break;
        default:
            setlen(tile, 3);
            goto skip_color;
        }
        SET_BGR0_PACKED(packet_cursor, color);
        setlen(tile, 3);
    skip_color:
        setcode(tile, 0x62);
        metrics = g_golem_glyph_metrics;
        metric = metrics + glyph_id;
        setXY0(tile, x, y);
        setWH(tile, metric->width, metric->height);
        addPrim(ordering_table, tile);
        packet_cursor = (s32)tile + 0x10;
    }

    sprite = (SPRT*)packet_cursor;
    SET_BGR0_PACKED(sprite, 0x606060);
    setSprt(sprite);
    if (style & 0x80)
    {
        setRGB0(sprite, 0x38, 0x38, 0x38);
        sprite->code |= 2;
    }
    if (clut == 0xF)
    {
        setRGB0(sprite, 0x40, 0x40, 0x40);
        sprite->code |= 2;
    }
    if (clut != 9)
    {
        sprite->code |= 2;
    }
    metrics2 = g_golem_glyph_metrics;
    metric2 = metrics2 + glyph_id;
    setXY0(sprite, x, y);
    setWH(sprite, metric2->width, metric2->height);
    sprite->u0 = metric2->u0;
    sprite->v0 = metric2->v0;
    sprite->clut = (clut & 0x3F) | 0x7C80;
    addPrim(ordering_table, sprite);
    return packet_cursor + 0x14;
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
 * @note The first OT splice and reuse of @c temporary are required to match.
 * @see decomp.me (100%)
 */
GolemLinePacket* golem_emit_panel_outline(GolemLinePacket* packet, s32* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    s32 temporary;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y;
    temporary = GOLEM_GPU_TAG_HIGH_MASK;
    packet->tag = (packet->tag & GOLEM_GPU_TAG_HIGH_MASK) | (*ordering_table & GOLEM_GPU_ADDRESS_MASK);
    *ordering_table = (*ordering_table & temporary) | ((s32)packet & GOLEM_GPU_ADDRESS_MASK);
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
    temporary = y + height;
    packet->y0 = temporary;
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
 * @brief Set the screen-fade target color and step count.
 * @param red Target red component (0x100 = neutral).
 * @param green Target green component.
 * @param blue Target blue component.
 * @param steps Number of frames to reach the target.
 * @return None.
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
 * @param primitive           Running fade packet cursor.
 * @param ordering_table_tag  Ordering-table tag the packets are linked into.
 * @return The advanced packet cursor.
 * @see decomp.me (100%)
 */
GolemFadePrimitive* golem_render_fade(GolemFadePrimitive* primitive, u_long* ordering_table_tag)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

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
            primitive->tile.r0 = g_golem_fade_current.red - 1;
            primitive->tile.g0 = g_golem_fade_current.green - 1;
            primitive->tile.b0 = g_golem_fade_current.blue - 1;
        }
        else
        {
            if (g_golem_fade_current.red == GOLEM_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_golem_fade_current.red;
            }
            if (g_golem_fade_current.green == GOLEM_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_golem_fade_current.green;
            }
            if (g_golem_fade_current.blue == GOLEM_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_golem_fade_current.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        primitive->tile.w = SCREEN_WIDTH;
        draw_mode = GOLEM_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0(&primitive->tile, 0, 0);
        primitive->tile.h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, &primitive->tile);

        primitive = GOLEM_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_golem_fade_current.red < GOLEM_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = GOLEM_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = GOLEM_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    return primitive;
}

/**
 * @brief Append the encoded string @p src to the end of @p dest.
 * @param dest Destination encoded-text buffer (null-terminated).
 * @param src  Source encoded-text buffer (null-terminated).
 * @return None.
 * @note golem_encoded_string_length intentionally has no prototype here; the
 *       implicit declaration is required to match.
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
    u8 character;
    s32 byte_count;

    cursor = text;
    character = *cursor;
    byte_count = 0;
    while (character != 0)
    {
        if ((u32)(character - 0x19) < 7)
        {
            cursor += 2;
            byte_count += 2;
        }
        else
        {
            cursor += 1;
            byte_count += 1;
        }
        character = *cursor;
    }
    return byte_count;
}

/**
 * @brief Copy the encoded string @p src to @p dst, honoring two-byte
 *        characters, and null-terminate the destination.
 * @param dst Destination buffer.
 * @param src Source encoded-text buffer (null-terminated).
 * @return None.
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
