#include "common.h"
#include <abs.h>

#include "internal/menu_internal.h"
#include "main/audio/akao.h"
#include "main/audio/akao_cmd.h"
#include "main/cdrom.h"
#include "main/display.h"
#include "overlays/field/field_input.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_text.h"

/**
 * @brief Play-time display: hours/minutes separator text and the pen advances after
 *        the separator and before the minutes.
 * @note JP uses a comma and wider advances.
 */
#if defined(VERSION_JP)
#define MENU_PLAYTIME_SEPARATOR ","
#define MENU_PLAYTIME_SEPARATOR_ADVANCE 8
#define MENU_PLAYTIME_MINUTES_ADVANCE 20
#else
#define MENU_PLAYTIME_SEPARATOR ":"
#define MENU_PLAYTIME_SEPARATOR_ADVANCE 7
#define MENU_PLAYTIME_MINUTES_ADVANCE 16
#endif

/** @brief Largest number menu_draw_clamped_number draws. */
#define MENU_NUMBER_MAX 99

s32 menu_layout_node(s32 node_index, s32 base_pos);
s32 menu_handle_input(s32 process_actions);
s32 menu_find_nav_node_index(s32 node_id);
s32 menu_find_active_content_item(void);
s32 menu_count_inventory_items(void);
void menu_concat_encoded_text(u8* dst, u8* src1, u8* src2);
s32 menu_build_ability_nav_entries(void);
s32 menu_lookup_item_nibble(const FieldItemRecord* item, u32 index);
s32 menu_build_special_technique_nav_entries(void);
s32 menu_remove_technique_skills(void);

/**
 * @brief MENU overlay entry: run the menu frame loop until Start closes it or another screen is requested.
 * @param render_buffers Two render contexts drawn on alternate frames.
 * @return g_menu_transition_code: 0 when the menu closed, otherwise the screen FIELD opens next (see FIELD_MENU_*).
 */
s32 menu_run(RenderContext* render_buffers)
{
    RECT clear_rect;
    RenderContext* draw_buffer;
    RenderContext* next_buffer;
    RenderContext* other_buffer;
    RenderContext* draw_env_buffers;
    MenuControllerActuatorState* actuator_state = MENU_CONTROLLER_ACTUATORS;

    g_menu_draw_buf_base = render_buffers;
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
    setRECT(&clear_rect, 0, 0, SCREEN_WIDTH, (SCREEN_HEIGHT + VRAM_DRAW_HEIGHT));
    ClearImage(&clear_rect, 0, 0, 0);
    setRECT(&g_menu_draw_buf_base[0].clear_rect, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    setRECT(&g_menu_draw_buf_base[1].clear_rect, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDispEnv(&g_menu_draw_buf_base[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_menu_draw_buf_base[1].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_menu_draw_buf_base[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_menu_draw_buf_base[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    draw_env_buffers = g_menu_draw_buf_base;
    draw_env_buffers[1].draw_env.dtd = 0;
    draw_env_buffers[0].draw_env.dtd = 0;
    g_menu_load_request = 0;
    g_menu_transition_code = FIELD_MENU_CLOSED;
    g_menu_draw_buf_base[0].frame_parity = 0;
    g_menu_draw_buf_base[1].frame_parity = 1;
    menu_init();
    next_buffer = g_menu_draw_buf_base;
    ClearOTagR(next_buffer->ot, RENDER_CONTEXT_OT_ENTRY_COUNT);
    ClearOTagR(g_menu_draw_buf_base[1].ot, RENDER_CONTEXT_OT_ENTRY_COUNT);
    VSync(0);
    PutDispEnv(&next_buffer->disp_env);
    update_controllers();

    if (g_active_script == 0)
    {
        DrawSync(0);
        VSync(0);
        SetDispMask(1);
        g_menu_display_delay = 0;
    }
    else
    {
        g_menu_display_delay = 2;
    }

    while (1)
    {
        draw_buffer = next_buffer;
        ClearOTagR(draw_buffer->ot, RENDER_CONTEXT_OT_ENTRY_COUNT);
        draw_buffer->prim_cursor = &draw_buffer->ot[RENDER_CONTEXT_OT_ENTRY_COUNT];
        field_text_reset_scratch();
        menu_tick(draw_buffer);
        field_text_upload_immediate_cache();
        field_update_audio_timer();
        DrawSync(0);
        set_controller_vsync_interval(FRAME_VSYNC_INTERVAL);
        VSync(FRAME_VSYNC_INTERVAL);

        if (g_menu_load_request != 0 || g_pad_input == PAD_BTN_START)
        {
            break;
        }

        ClearImage(&draw_buffer->clear_rect, (*(u8*)&g_menu_initial_clut_pair & 0x1F) * 8, (g_menu_initial_clut_pair >> 2) & 0xF8,
                   (g_menu_initial_clut_pair >> 7) & 0xF8);
        other_buffer = g_menu_draw_buf_base;

        if (draw_buffer == g_menu_draw_buf_base)
        {
            other_buffer = draw_buffer + 1;
        }

        next_buffer = other_buffer;
        PutDispEnv(&other_buffer->disp_env);
        PutDrawEnv(&next_buffer->draw_env);
        DrawOTag(&draw_buffer->ot[MENU_GRID_OT_INDEX]);
        update_controllers();
        cdrom_process_state();

        if (g_active_script == 0)
        {
            if (g_menu_display_delay != 0)
            {
                g_menu_display_delay--;

                if (g_menu_display_delay != 0)
                {
                    DrawSync(0);
                    VSync(0);
                    SetDispMask(1);
                }
            }
        }
    }

    actuator_state->ports[1].large_motor_command = 0;
    actuator_state->ports[0].large_motor_command = 0;
    DrawSync(0);
    VSync(0);
    field_reset_input_repeat();
    field_text_reset_windows();
    return g_menu_transition_code;
}

/**
 * @brief Initialize menu graphics, runtime state, window slots, and the node tree.
 */
void menu_init(void)
{
    u8 unused_padding[4];
    menu_upload_graphics();
    menu_state_init();
    menu_reset_slots();
    g_active_slot = -1;
    field_reset_input_repeat();
    g_menu_compare_window_active = 0;
    menu_upload_party_portraits();
    g_menu_frame = 0;
    g_script_cursor = 0;
    menu_node_tree_init();
}

/**
 * @brief Upload the CLUTs and pixels of the three party portraits to VRAM.
 */
void menu_upload_party_portraits(void)
{
    s32 slot = 0;
    u8* portraits = g_field_party_portraits;
    s32 pixels_offset = FIELD_PORTRAIT_PIXELS_OFFSET;
    s32 clut_offset = 0;
    RECT rect;

    for (; slot < FIELD_PARTY_PORTRAIT_COUNT; slot++)
    {
        setRECT(&rect, FIELD_PARTY_PORTRAIT_CLUT_X, slot + FIELD_PARTY_PORTRAIT_CLUT_Y, GPU_CLUT_4BIT_COLORS, 1);
        LoadImage(&rect, menu_image_upload_source(portraits, clut_offset));

        setRECT(&rect, (slot == FIELD_PARTY_PORTRAIT_COUNT - 1) ? FIELD_PARTY_PORTRAIT_VRAM_X2 : FIELD_PARTY_PORTRAIT_VRAM_X,
                (slot == 0) ? FIELD_PARTY_PORTRAIT_VRAM_Y0 : FIELD_PARTY_PORTRAIT_VRAM_Y1, FIELD_PORTRAIT_SIZE / 4, FIELD_PORTRAIT_SIZE);
        LoadImage(&rect, menu_image_upload_source(portraits, pixels_offset));

        pixels_offset += FIELD_PORTRAIT_BYTES;
        clut_offset += FIELD_PORTRAIT_BYTES;
    }
}

/**
 * @brief Process input and render one menu frame.
 * @param render_ctx Render context receiving the menu primitives.
 */
void menu_tick(RenderContext* render_ctx)
{
    s32 menu_frame;
    s32 frame_counter;
    s32 input_mask;
    void* saved_prim_cursor;
    s32 repeat_index;
    u16 script_input;
    s32 stack_padding[2];

    menu_build_grid(render_ctx);
    menu_frame = g_menu_frame;
    frame_counter = g_frame_counter;
    /* Preserve the packet cursor established by the grid pass. */
    saved_prim_cursor = render_ctx->prim_cursor;
    g_menu_frame = menu_frame + 1;
    g_frame_counter = frame_counter + 1;
    field_update_input_repeat();

    /* Merge externally injected input when enabled by the pad context. */
    if ((g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word & MENU_PAD_INJECT_ENABLED) && g_saved_game_ctx->characters[FIELD_PARTY_GUEST].name[0])
    {
        g_pad_input |= g_pad_input_inject;
    }

    /* Keep only the highest-priority active button group. */
    input_mask = g_pad_input & MENU_PAD_VERTICAL;

    if (input_mask)
    {
        g_pad_input = input_mask;
    }

    input_mask = g_pad_input & PAD_BTN_DPAD;

    if (input_mask)
    {
        g_pad_input = input_mask;
    }

    input_mask = g_pad_input & MENU_PAD_SHOULDERS;

    if (input_mask)
    {
        g_pad_input = input_mask;
    }

    /* Prevent input from being accepted on consecutive frames. */
    if (g_pad_input_latched != 0)
    {
        g_pad_input = 0;
    }

    g_pad_input_latched = g_pad_input;

    /* Replace live input with the active scripted input sequence. */
    if (g_active_script != 0)
    {
        MenuScript* script_table = g_script_table;
        MenuScript* script_row;
        s32 cursor;

        script_row = &script_table[g_active_script];
        cursor = g_script_cursor;

        g_pad_input = 0;

        script_input = script_row->inputs[cursor];

        if (script_input == MENU_SCRIPT_END)
        {
            if (g_active_script < 4)
            {
                for (repeat_index = 0; repeat_index < g_script_repeat_count; repeat_index++)
                {
                    menu_step_item_selection(1);
                }

                g_script_repeat_last = g_script_repeat_count;
            }

            g_active_script = 0;
        }
        else
        {
            g_pad_input = script_input;
            g_script_cursor = cursor + 1;
        }
    }

    /* Render slots from the grid pass's packet cursor. */
    render_ctx->prim_cursor = saved_prim_cursor;
    menu_update_slots(render_ctx);
}

/**
 * @brief Build and queue a horizontally aligned run of glyph sprites.
 * @param sprite_cursor Start of the primitive-buffer region for the glyph sprites.
 * @param ot Ordering-table entry that receives the emitted packets.
 * @param src Address of the source text.
 * @param text_color Text-color index in the range 0-15.
 * @param x Horizontal anchor selected by @p alignment.
 * @param y Y coordinate applied to every glyph.
 * @param len Source byte length; must not exceed 0x7F.
 * @param alignment Horizontal alignment of the run relative to @p x.
 * @return Next free primitive-buffer address, immediately after the draw-mode packet.
 *
 * @see field_text_build_sprites
 */
void* menu_build_text_run(SPRT* sprite_cursor, u_long* ot, u8* src, s32 text_color, s32 x, s32 y, s32 len, s32 alignment)
{
    char buf[0x80];
    s32 count, i, acc;
    SPRT* sprite;
    DR_TPAGE* tpage;

    /* Prepare a null-terminated slice for the glyph decoder. */
    strncpy(buf, (char*)src, len);
    buf[len] = 0;

    /* Populate one SPRT per decoded glyph. */
    count = field_text_build_sprites(sprite_cursor, buf, text_color);

    /* Convert the requested anchor into the run's left edge. */
    if (alignment != FIELD_TEXT_ALIGN_RIGHT)
    {
        if (alignment == FIELD_TEXT_ALIGN_CENTER)
        {
            sprite = sprite_cursor;

            for (i = 0; i < count; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;

        for (i = 0; i < count; i++)
        {
            x -= sprite[i].w;
        }
    }

    /* Finish and link the prebuilt sprites using a running x offset. */
    acc = 0;

    while (count != 0)
    {
        sprite = sprite_cursor;
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);

        SET_SPRT_XY0_WORD(sprite, PACK_U16_PAIR(x, y) + acc);
        acc += sprite->w;

        addPrim(ot, sprite);
        sprite_cursor++;
        count--;
    }

    /* addPrim prepends, so link the texture-page packet after the sprites. */
    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, FIELD_UI_TPAGE(GPU_BLEND_HALF));
    addPrim(ot, tpage);

    return tpage + 1;
}

/**
 * @brief Builds and queues the GPU packet sequence for the menu grid.
 * @param render_ctx Render context providing the packet buffer and ordering table;
 */
void menu_build_grid(RenderContext* render_ctx)
{
    RECT texture_window;
    s32 sprite_index;
    SPRT* sprite;
    const MenuGridSpriteDef* sprite_def;
    u_long* packet_cursor;
    RenderContext* first_ctx = render_ctx;
    RenderContext* ot_ctx = first_ctx;

    /* Disable texture-window masking for the grid sprite batch. */
    packet_cursor = first_ctx->prim_cursor;
    texture_window.h = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.w = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.y = 0;
    texture_window.x = 0;

    setTexWindow((DR_TWIN*)packet_cursor, &texture_window);
    addPrim(&first_ctx->ot[MENU_GRID_OT_INDEX], packet_cursor);

    /* Expand each packed grid definition into one SPRT packet. */
    sprite_def = (const MenuGridSpriteDef*)g_menu_glyph_src;
    packet_cursor += PRIM_WORDS(DR_TWIN);
    sprite = (SPRT*)packet_cursor;

    for (sprite_index = 0; sprite_index < MENU_GRID_SPRITE_COUNT; sprite_index++, sprite++, sprite_def++)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        SET_SPRT_UV0_PACKED(sprite, sprite_def->uv);

        /* Copy the packed coordinate and size pairs as words. */
        SET_SPRT_XY0_WORD(sprite, sprite_def->packed_xy);
        SET_SPRT_WH_WORD(sprite, sprite_def->packed_wh);

        /* The final sprite group uses the alternate grid palette. */
        if (sprite_index >= MENU_GRID_ALT_CLUT_START)
        {
            SET_SPRT_CLUT(sprite, MENU_CLUT_GRID_ALT);
        }
        else
        {
            SET_SPRT_CLUT(sprite, MENU_CLUT_GRID_BASE);
        }

        addPrim(&ot_ctx->ot[MENU_GRID_OT_INDEX], sprite);
    }

    packet_cursor = (u_long*)sprite;

    /* Close the batch with texture-window and texture-page state packets. */
    setWH(&texture_window, MENU_GRID_TEXTURE_WINDOW_SIZE, MENU_GRID_TEXTURE_WINDOW_SIZE);
    texture_window.x = 0;
    texture_window.y = 0;

    setTexWindow((DR_TWIN*)packet_cursor, &texture_window);
    addPrim(&ot_ctx->ot[MENU_GRID_OT_INDEX], packet_cursor);

    packet_cursor += PRIM_WORDS(DR_TWIN);
    setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, MENU_GRID_TPAGE);
    addPrim(&ot_ctx->ot[MENU_GRID_OT_INDEX], packet_cursor);

    /* Publish the first unused packet word for subsequent builders. */
    render_ctx->prim_cursor = packet_cursor + PRIM_WORDS(DR_TPAGE);
}

/**
 * @brief Upload the menu texture and CLUTs to their reserved VRAM regions.
 */
void menu_upload_graphics(void)
{
    TimUploadDestination layout;

    layout.pixel_x = SCREEN_WIDTH;
    layout.pixel_y = 0;
    layout.clut_x = 0;
    layout.clut_y = VRAM_CLUT_Y;
    menu_upload_tim(&layout);
}

/**
 * @brief Initialize the base pointer for the menu's resource tables.
 */
void menu_state_init(void)
{
    g_menu_state_ptr = &g_menu_state_data;
}

/**
 * @brief Upload the menu texture and its two CLUTs to VRAM.
 * @param layout VRAM destinations for the texture and first CLUT; the second CLUT is placed on the following row.
 */
void menu_upload_tim(const TimUploadDestination* layout)
{
    MenuTimAsset* asset = (MenuTimAsset*)g_menu_tim;
    Tim* tim = &asset->tim;
    u32 clut_block_size = tim->clut_block.bnum;
    RECT vram_rect;
    u16* clut_color;
    s32 color_index;
    TimBlock* image_block;

    /* Preserve the first color pair before modifying the palette in place. */
    g_menu_initial_clut_pair = *(u32*)tim->clut_data;

    /* Enable STP on nonzero colors, then upload the first CLUT. */
    setRECT(&vram_rect, layout->clut_x, layout->clut_y, CLUT_ENTRY_COUNT, 1);

    clut_color = asset->tim.clut_data;

    for (color_index = 0; color_index < CLUT_ENTRY_COUNT; color_index++)
    {
        if (*clut_color != 0)
        {
            *clut_color |= GPU_STP_BIT;
        }

        clut_color++;
    }

    LoadImage(&vram_rect, (u_long*)tim->clut_data);

    /* Upload the image block following the variable-length first CLUT. */
    vram_rect.x = layout->pixel_x;
    vram_rect.y = layout->pixel_y;
    image_block = TIM_PIXEL_BLOCK(tim, clut_block_size);
    setWH(&vram_rect, image_block->dimensions.width, image_block->dimensions.height);
    LoadImage(&vram_rect, (u_long*)(image_block + 1));

    /* Apply the same STP treatment to the second CLUT. */
    setRECT(&vram_rect, layout->clut_x, layout->clut_y + 1, CLUT_ENTRY_COUNT, 1);

    clut_color = asset->second_clut;

    for (color_index = 0; color_index < CLUT_ENTRY_COUNT; color_index++)
    {
        if (*clut_color != 0)
        {
            *clut_color |= GPU_STP_BIT;
        }

        clut_color++;
    }

    LoadImage(&vram_rect, (u_long*)asset->second_clut);
}

/**
 * @brief Allocate and initialize the first available menu window slot.
 * @param ot_index Ordering-table entry used to link the slot's primitives.
 * @param rect Initial window position and dimensions.
 * @return Initialized menu slot.
 */
MenuSlot* menu_slot_alloc(s32 ot_index, const MenuSlotRect* rect)
{
    s32 slot_index;
    MenuSlot* slot;
    MenuSlot* slot_cursor;
    uintptr_t slot_pool;
    u32 slot_flags;
    u32 ot_index_clear_mask;

    /* Find the first slot whose active state is clear. */
    slot_index = 0;
    slot_pool = (uintptr_t)&g_menu_slots[0];
    slot_cursor = &g_menu_slots[0];

    while (slot_index < MENU_SLOT_COUNT)
    {
        if (slot_cursor->active == 0)
        {
            break;
        }

        slot_index++;
        slot_cursor++;
    }

    /* The original negative-index guard does not catch a full pool (index 4). */
    if (slot_index < 0)
    {
        return (MenuSlot*)(-1);
    }

    slot = (MenuSlot*)((slot_index * sizeof(MenuSlot)) + slot_pool);

    /* Clear low flags while retaining the slot's previous bits 24:16. */
    slot->navigation.fields.selected_index = 0;
    slot_flags = slot->navigation.packed;
    slot->active = 1;
    slot->content_cb = 0;
    slot->index = (u8)slot_index;
    slot->tick_cb = 0;
    slot->anim_frame = 0;
    ot_index_clear_mask = MENU_SLOT_OT_INDEX_CLEAR_MASK;
    slot_flags = slot_flags & ot_index_clear_mask;
    slot_flags = slot_flags | (((u32)ot_index) << MENU_SLOT_OT_INDEX_SHIFT);
    slot->navigation.packed = slot_flags;
    setRECT(slot, rect->x, rect->y, rect->w, rect->h);
    slot->lerp_cur_a = 0;
    slot->lerp_cur_b = 0;
    slot->lerp_target_a = 0;
    slot->lerp_target_b = 0;
    slot->lerp_steps = 0;
    slot->has_title = 0;
    g_active_slot = slot_index;
    return slot;
}

/**
 * @brief Mark every menu window slot as free.
 */
void menu_reset_slots(void)
{
    s32 slot_index;
    MenuSlot* slot;

    slot_index = MENU_SLOT_COUNT - 1;
    slot = &g_menu_slots[slot_index];

    while (slot_index >= 0)
    {
        slot->active = 0;
        slot_index--;
        slot--;
    }
}

/**
 * @brief Per-frame update/draw pump for the four menu slots.
 * @param render_ctx Per-frame render context.
 */
void menu_update_slots(RenderContext* render_ctx)
{
    Vec2s view_origin;
    s32 unused_pad[2];
    s32 has_active_slot;
    s32 slot_index;
    s32 allow_input;
    u_long* frame_ot;

    has_active_slot = 0;
    g_menu_help_text = 0;
    slot_index = MENU_SLOT_COUNT - 1;

    while (slot_index >= 0)
    {
        u8 anim_frame;
        s32 slot_state;

        slot_state = g_menu_slots[slot_index].active;

        switch (slot_state)
        {
            case MENU_SLOT_STATE_OPENING:
            {
                u8 previous_anim_frame;

                menu_draw_window_transition(render_ctx, &g_menu_slots[slot_index], g_menu_cursor_mode != MENU_CURSOR_MODE_NODE_TREE);
                previous_anim_frame = g_menu_slots[slot_index].anim_frame;
                anim_frame = previous_anim_frame + 1;
                g_menu_slots[slot_index].anim_frame = anim_frame;

                if ((anim_frame & 0xFF) == (MENU_WINDOW_TRANSITION_STEPS / 2))
                {
                    g_menu_slots[slot_index].anim_frame = previous_anim_frame;
                    g_menu_slots[slot_index].active = MENU_SLOT_STATE_OPEN;
                }
            }
                /* Fall through to the shared active-slot callback. */
            case MENU_SLOT_STATE_OPEN:
                if (slot_state == MENU_SLOT_STATE_OPEN)
                {
                    MenuRect* slot_rect;

                    slot_rect = (MenuRect*)&g_menu_slots[slot_index].x;
                    view_origin.y = 0;
                    view_origin.x = 0;
                    menu_draw_window(&g_menu_slots[slot_index], render_ctx, slot_rect, &view_origin, g_menu_cursor_mode != MENU_CURSOR_MODE_NODE_TREE);
                }

                if (slot_index == g_active_slot)
                {
                    if (g_menu_slots[slot_index].tick_cb != 0)
                    {
                        g_menu_slots[slot_index].tick_cb(&g_menu_slots[slot_index]);
                    }
                }

                has_active_slot = 1;
                slot_index -= 1;
                continue;

            case MENU_SLOT_STATE_CLOSING:
                menu_draw_window_transition(render_ctx, &g_menu_slots[slot_index], g_menu_cursor_mode != MENU_CURSOR_MODE_NODE_TREE);
                anim_frame = g_menu_slots[slot_index].anim_frame - 1;
                g_menu_slots[slot_index].anim_frame = anim_frame;

                if (!(anim_frame & 0xFF))
                {
                    g_menu_slots[slot_index].active = MENU_SLOT_STATE_FREE;
                    menu_update_active_slot();
                }

                has_active_slot = 1;
                slot_index -= 1;
                continue;
            default:
                slot_index -= 1;
                continue;
        }
    }

    allow_input = 0;

    if (has_active_slot == 0)
    {
        g_active_slot = -1;
    }

    frame_ot = &render_ctx->ot[MENU_FRAME_OT_INDEX];
    view_origin.y = 0;
    view_origin.x = 0;

    if ((g_active_slot == -1) || (g_menu_cursor_mode == MENU_CURSOR_MODE_NODE_TREE))
    {
        allow_input = 1;
    }

    render_ctx->prim_cursor = menu_draw_frame(render_ctx->prim_cursor, frame_ot, render_ctx->frame_parity, allow_input);

    if (g_menu_help_text != 0)
    {
        render_ctx->prim_cursor = field_draw_text(render_ctx->prim_cursor, &render_ctx->ot[MENU_FRAME_OT_INDEX], g_menu_help_text, FIELD_TEXT_COLOR_MENU, 0xA0,
                                                  0xCA, FIELD_TEXT_ALIGN_CENTER);
    }
}

/**
 * @brief Draw one frame of a menu window's opening or closing transition.
 * @param render_ctx Per-frame menu rendering context.
 * @param slot Window slot being animated.
 * @param cursor_enable Nonzero to allow the active-slot cursor highlight.
 */
void menu_draw_window_transition(RenderContext* render_ctx, MenuSlot* slot, s32 cursor_enable)
{
    MenuRect rect;
    Vec2s view_origin;
    s32 inset_x;
    s32 inset_y;
    s32 width;
    s32 height;

    /* Expand from the center as anim_frame advances; closing runs it in reverse. */
    inset_x = (slot->w >> 1) - ((s16)(slot->w / MENU_WINDOW_TRANSITION_STEPS) * slot->anim_frame);
    view_origin.x = inset_x;

    inset_y = (slot->h >> 1) - ((s16)(slot->h / MENU_WINDOW_TRANSITION_STEPS) * slot->anim_frame);
    view_origin.y = inset_y;

    if (inset_x > 0)
    {
        if (inset_y > 0)
        {
            width = slot->w - (inset_x * 2);

            if (width < MENU_WINDOW_MIN_WIDTH)
            {
                width = MENU_WINDOW_MIN_WIDTH;
            }

            height = slot->h - (inset_y * 2);

            if (height < MENU_WINDOW_MIN_HEIGHT)
            {
                height = MENU_WINDOW_MIN_HEIGHT;
            }

            setRECT(&rect, slot->x + inset_x, slot->y + inset_y, width, height);

            menu_draw_window(slot, render_ctx, &rect, &view_origin, cursor_enable);
        }
    }
}

/**
 * @brief Build all GPU primitives for one menu window at a given rectangle.
 * @param slot Slot descriptor (geometry, flags, content callback).
 * @param render_ctx Per-frame render context.
 * @param rect Window rectangle: x, y, w, h halfwords.
 * @param view_origin View-origin offset forwarded to the content callback.
 * @param cursor_enable Cursor-highlight enable for the active slot.
 */
void menu_draw_window(MenuSlot* slot, RenderContext* render_ctx, MenuRect* rect, Vec2s* view_origin, s32 cursor_enable)
{
    MenuRectU16 window_rect;
    DRAWENV content_draw_env;
    Vec2s title_pos;
    s32 content_draw_y;
    s32 draw_cursor;
    u_long* ot_entry;
    u_long* window_cursor;
    SPRT* sprite_cursor;
    u_long* packet_cursor;
    s32 content_draw_x;
    DRAWENV* draw_env;

    packet_cursor = render_ctx->prim_cursor;
    ot_entry = &render_ctx->ot[(u32)slot->navigation.packed >> MENU_SLOT_OT_INDEX_SHIFT];

    if (slot->lerp_steps != 0)
    {
        slot->lerp_cur_a += (slot->lerp_target_a - slot->lerp_cur_a) / slot->lerp_steps;
        slot->lerp_cur_b += (slot->lerp_target_b - slot->lerp_cur_b) / slot->lerp_steps;
        slot->lerp_steps--;
    }
    else
    {
        slot->lerp_cur_a = slot->lerp_target_a;
        slot->lerp_cur_b = slot->lerp_target_b;
    }

    if ((slot->content_cb != NULL) && ((rect->w - MENU_WINDOW_MIN_WIDTH) > 0) && ((rect->h - MENU_WINDOW_MIN_HEIGHT) > 0))
    {
        SetDrawEnv((DR_ENV*)packet_cursor, &g_menu_draw_buf_base[render_ctx->frame_parity ^ 1].draw_env);
        addPrim(ot_entry, packet_cursor);
        g_menu_draw_early_out = 0;
        packet_cursor += PRIM_WORDS(DR_ENV);
        draw_cursor = (slot->index == g_active_slot) && (cursor_enable != 0) && (g_menu_suppress_cursor == 0) && (slot->active == MENU_SLOT_STATE_OPEN);
        packet_cursor = slot->content_cb(ot_entry, slot, packet_cursor, view_origin, draw_cursor);

        if (g_menu_draw_early_out != 0)
        {
            render_ctx->prim_cursor = packet_cursor;
            return;
        }

        draw_env = &content_draw_env;
        content_draw_x = rect->x + 8;
        content_draw_y = rect->y + 16;

        if (render_ctx->frame_parity != 0)
        {
            content_draw_y = rect->y + SCREEN_HEIGHT + 8;
        }

        SetDefDrawEnv(draw_env, content_draw_x, content_draw_y, rect->w - MENU_WINDOW_MIN_HEIGHT, rect->h - MENU_WINDOW_MIN_HEIGHT);
        SetDrawEnv((DR_ENV*)packet_cursor, draw_env);
        addPrim(ot_entry, packet_cursor);
        packet_cursor += PRIM_WORDS(DR_ENV);

        if (slot->has_title != 0)
        {
            switch (g_menu_scene_type)
            {
                case 1:
                case 4:
                case 19:
                case 22:
                case 25:
                    title_pos.x = ((u16)rect->x + (u16)rect->w) - 104;
                    break;
                default:
                    title_pos.x = ((u16)rect->x + (u16)rect->w) - 72;
                    break;
            }

            title_pos.y = (u16)rect->y;

            if (slot->navigation.packed & MENU_LIST_COUNT_MASK)
            {
                packet_cursor = field_draw_sprite_number(ot_entry, packet_cursor, slot->navigation.fields.selected_index + 1, 3, &title_pos, 0);
            }
            else
            {
                packet_cursor = field_draw_sprite_number(ot_entry, packet_cursor, 0, 3, &title_pos, 0);
            }

            packet_cursor = field_draw_sprite_glyph((SPRT*)packet_cursor, ot_entry, FIELD_DIGIT_GLYPH_SEPARATOR, &title_pos, 0);
            title_pos.x += 8;
            packet_cursor =
                field_draw_sprite_number(ot_entry, packet_cursor, slot->navigation.fields.count_and_ot & MENU_ITEM_NAV_INDEX_MASK, 3, &title_pos, 0);
            switch (g_menu_scene_type)
            {
                case 1:
                case 4:
                case 19:
                case 22:
                case 25:
                    packet_cursor = field_draw_sprite_glyph((SPRT*)packet_cursor, ot_entry, FIELD_DIGIT_GLYPH_SEPARATOR, &title_pos, 0);
                    title_pos.x += 8;
                    packet_cursor = field_draw_sprite_number(ot_entry, packet_cursor, menu_count_inventory_items(), 3, &title_pos, 0);
                    break;
            }

            packet_cursor = menu_emit_slot_scroll_arrows((SPRT*)packet_cursor, ot_entry, slot);
        }
    }

    setWH(&window_rect, 0xFF, 0xFF);
    window_rect.x = 0;
    window_rect.y = 0;
    setTexWindow((DR_TWIN*)packet_cursor, &window_rect);
    addPrim(ot_entry, packet_cursor);
    window_cursor = packet_cursor + PRIM_WORDS(DR_TWIN);

    if (rect->h >= MENU_WINDOW_MIN_HEIGHT)
    {
        setRECT(&window_rect, (u16)rect->x + 8, (u16)rect->y, (u16)rect->w - MENU_WINDOW_MIN_HEIGHT, 8);
        window_cursor = menu_build_h_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_TOP);

        if (rect->h >= MENU_WINDOW_MIN_HEIGHT)
        {
            setRECT(&window_rect, (u16)rect->x + 8, ((u16)rect->y + (u16)rect->h) - 8, (u16)rect->w - MENU_WINDOW_MIN_HEIGHT, 8);
            window_cursor = menu_build_h_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_BOT);
        }
    }

    if (rect->w >= MENU_WINDOW_MIN_WIDTH)
    {
        setRECT(&window_rect, (u16)rect->x, (u16)rect->y + 8, 8, (u16)rect->h - MENU_WINDOW_MIN_HEIGHT);
        window_cursor = menu_build_v_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_LEFT);

        if (rect->w >= MENU_WINDOW_MIN_WIDTH)
        {
            setRECT(&window_rect, ((u16)rect->x + (u16)rect->w) - 8, (u16)rect->y + 8, 8, (u16)rect->h - MENU_WINDOW_MIN_HEIGHT);
            window_cursor = menu_build_v_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_RIGHT);
        }
    }

    setRECT(&window_rect, (u16)rect->x + 8, (u16)rect->y + 8, (u16)rect->w - MENU_WINDOW_MIN_HEIGHT, (u16)rect->h - MENU_WINDOW_MIN_HEIGHT);
    sprite_cursor = menu_fill_window_interior((SPRT*)window_cursor, ot_entry, &window_rect, MENU_TW_FILL);
    sprite_cursor = menu_emit_corner(sprite_cursor, ot_entry, rect->x, rect->y, MENU_TW_CORNER_TL);
    sprite_cursor = menu_emit_corner(sprite_cursor, ot_entry, rect->x + rect->w - 8, rect->y, MENU_TW_CORNER_TR);
    sprite_cursor = menu_emit_corner(sprite_cursor, ot_entry, rect->x, rect->y + rect->h - 8, MENU_TW_CORNER_BL);
    sprite_cursor = menu_emit_corner(sprite_cursor, ot_entry, rect->x + rect->w - 8, rect->y + rect->h - 8, MENU_TW_CORNER_BR);
    window_cursor = (u_long*)sprite_cursor;
    setDrawTPage((DR_TPAGE*)window_cursor, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot_entry, window_cursor);
    render_ctx->prim_cursor = window_cursor + PRIM_WORDS(DR_TPAGE);
}

/**
 * @brief Emit one textured window-corner sprite.
 * @param sprite Primitive buffer location for the sprite.
 * @param ot_entry Ordering-table entry to link the sprite into.
 * @param x Screen X coordinate.
 * @param y Screen Y coordinate.
 * @param uv Packed texture coordinates: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the sprite.
 */
SPRT* menu_emit_corner(SPRT* sprite, u_long* ot_entry, s32 x, s32 y, u32 uv)
{
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);

    setSprt(sprite);

    SET_SPRT_WH_PACKED(sprite, MENU_WINDOW_CORNER_SIZE, MENU_WINDOW_CORNER_SIZE);

    setXY0(sprite, x, y);

    SET_SPRT_CLUT(sprite, MENU_CLUT_CORNER);
    SET_SPRT_UV0_PACKED(sprite, uv);

    addPrim(ot_entry, sprite);

    return sprite + 1;
}

/**
 * @brief Tile a window interior with textured sprites.
 * @param sprite Primitive buffer location for the first tile.
 * @param ot_entry Ordering-table entry to link the tiles into.
 * @param rect Screen-space region to fill.
 * @param uv Packed texture coordinates: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the emitted tiles.
 */
SPRT* menu_fill_window_interior(SPRT* sprite, u_long* ot_entry, const MenuRectU16* rect, u32 uv)
{
    s16 padding[2];
    s32 y_offset;

    for (y_offset = 0; y_offset < rect->h; y_offset += MENU_WINDOW_FILL_TILE_SIZE)
    {
        s32 x_offset = 0;

        while (x_offset < rect->w)
        {
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setSprt(sprite);
            SET_SPRT_UV0_PACKED(sprite, uv);

            /* Clamp the final tile in each row and column to the region. */
            if (x_offset + MENU_WINDOW_FILL_TILE_SIZE > rect->w)
            {
                sprite->w = rect->w - x_offset;
            }
            else
            {
                sprite->w = MENU_WINDOW_FILL_TILE_SIZE;
            }

            if (y_offset + MENU_WINDOW_FILL_TILE_SIZE > rect->h)
            {
                sprite->h = rect->h - y_offset;
            }
            else
            {
                sprite->h = MENU_WINDOW_FILL_TILE_SIZE;
            }

            sprite->x0 = rect->x + x_offset;
            x_offset += MENU_WINDOW_FILL_TILE_SIZE;
            sprite->y0 = rect->y + y_offset;
            SET_SPRT_CLUT(sprite, MENU_CLUT_GRID_ALT);
            addPrim(ot_entry, sprite);
            sprite++;
        }
    }

    return sprite;
}

/**
 * @brief Emit a textured top or bottom window edge.
 * @param packet_cursor Primitive buffer location for the edge.
 * @param ot_entry Ordering-table entry to link the primitives into.
 * @param rect Screen-space edge rectangle.
 * @param texture_origin Packed texture origin: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the emitted primitives.
 */
u_long* menu_build_h_edge(u_long* packet_cursor, u_long* ot_entry, const MenuRectU16* rect, s32 texture_origin)
{
    RECT texture_window;
    SPRT* sprite;
    DR_TWIN* texture_window_primitive;

    if (rect->w <= 0)
    {
        return packet_cursor;
    }

    if (rect->h > 0)
    {
        sprite = (SPRT*)packet_cursor;
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        SET_SPRT_UV0_PACKED(sprite, 0);
        setWH(sprite, rect->w, rect->h);
        setXY0(sprite, rect->x, rect->y);
        sprite->clut = MENU_CLUT_CORNER;
        addPrim(ot_entry, sprite);
        packet_cursor += PRIM_WORDS(SPRT);

        /* Repeat the 16x8 edge texture across the sprite. */
        texture_window_primitive = (DR_TWIN*)packet_cursor;
        setRECT(&texture_window, texture_origin & 0xFF, texture_origin >> 8, MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE, MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE);
        setTexWindow(texture_window_primitive, &texture_window);
        addPrim(ot_entry, texture_window_primitive);
        packet_cursor += PRIM_WORDS(DR_TWIN);
    }

    return packet_cursor;
}

/**
 * @brief Emit a textured left or right window edge.
 * @param packet_cursor Primitive buffer location for the edge.
 * @param ot_entry Ordering-table entry to link the primitives into.
 * @param rect Screen-space edge rectangle.
 * @param texture_origin Packed texture origin: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the emitted primitives.
 */
u_long* menu_build_v_edge(u_long* packet_cursor, u_long* ot_entry, const MenuRectU16* rect, s32 texture_origin)
{
    RECT texture_window;
    SPRT* sprite;
    DR_TWIN* texture_window_primitive;

    if (rect->w <= 0)
    {
        return packet_cursor;
    }

    if (rect->h > 0)
    {
        sprite = (SPRT*)packet_cursor;
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        SET_SPRT_UV0_PACKED(sprite, 0);
        setWH(sprite, rect->w, rect->h);
        setXY0(sprite, rect->x, rect->y);
        sprite->clut = MENU_CLUT_CORNER;
        addPrim(ot_entry, sprite);
        packet_cursor += PRIM_WORDS(SPRT);

        /* Repeat the 8x16 edge texture across the sprite. */
        texture_window_primitive = (DR_TWIN*)packet_cursor;
        setRECT(&texture_window, texture_origin & 0xFF, texture_origin >> 8, MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE, MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE);
        setTexWindow(texture_window_primitive, &texture_window);
        addPrim(ot_entry, texture_window_primitive);
        packet_cursor += PRIM_WORDS(DR_TWIN);
    }

    return packet_cursor;
}

/**
 * @brief Draw a sign-dependent label at a screen position.
 * @param ot_entry Ordering-table entry for the label primitives.
 * @param packet_cursor Primitive buffer location for the label.
 * @param position Label screen position.
 * @param value Selects the nonnegative or negative label.
 * @return Updated primitive buffer location.
 */
u_long* menu_draw_label(u_long* ot_entry, u_long* packet_cursor, const Vec2s* position, s32 value)
{
    u8 label_buffer[MENU_LABEL_BUFFER_SIZE];

    menu_copy_sign_label(label_buffer, value);

    packet_cursor = field_draw_text((SPRT*)packet_cursor, ot_entry, label_buffer, FIELD_TEXT_COLOR_MENU, position->x, position->y, FIELD_TEXT_ALIGN_LEFT);

    return packet_cursor;
}

/**
 * @brief Initialize the full menu node tree and global menu state.
 */
void menu_node_tree_init(void)
{
    u32 work_value_a;
    u32 work_value_b;
    s32 zero_value;
    s32 layout_y;
    s32 node_index;
    s32 packed_value;
    s32 nav_x_low_bit;
    s8 scene_type;
    s32 layout_y_high;
    u16 node30_flags;
    u32 layout_y_16;
    u32 layout_y_9;
    u32 layout_y_high_9;
    g_menu_prev_node = MENU_NONE;
    g_menu_content_ready = 0;
    g_item_slot_data[0] = 0;
    g_item_slot_data[1] = 0;
    g_item_slot_data[2] = 0;
    g_item_slot_data[3] = 0;
    g_item_slot_flags[0] = 0;
    g_item_slot_flags[1] = 0;
    g_item_slot_flags[2] = 0;
    g_item_slot_flags[3] = 0;
    g_menu_item_ptr = 0;
    g_menu_category0_item = 0;
    g_menu_category1_item = 0;
    g_menu_category2_item = 0;
    g_menu_active_equipped_item = 0;
    g_menu_saved_category0_item = 0;
    g_menu_saved_category1_item = 0;
    g_menu_saved_equipment_item = 0;
    g_menu_content_height = 0;
    g_menu_scroll_pos = 0;
    g_menu_redraw_state = 0;
    g_menu_active_node = 0;
    g_menu_cursor_mode = MENU_CURSOR_MODE_NODE_TREE;

    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        g_menu_nodes[node_index].layout_frames_remaining = MENU_NODE_LAYOUT_IDLE;
        g_menu_nodes[node_index].icon_id = 0;
        g_menu_nodes[node_index].content_id = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[3] = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[2] = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[1] = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[0] = MENU_NONE;
        g_menu_nodes[node_index].u2.bits.active = 0;
        g_menu_nodes[node_index].u2.bits.expanded = 0;
        g_menu_nodes[node_index].u2.bits.char_slot = 3;
        g_menu_nodes[node_index].u2.s.parent_idx = MENU_NONE;
    }

    g_menu_nodes[0].u2.bits.expanded = 0;
    g_menu_nodes[0].u2.bits.char_slot = 0;
    g_menu_nodes[0].label_id = 1;
    g_menu_nodes[0].u2.bits.depth = 0;
    g_menu_nodes[0].idx_nav.s.self_idx = 0;
    g_menu_nodes[0].u2.bits.active = 1;
    g_menu_nodes[0].u2.s.parent_idx = MENU_NONE;

    if (g_field_player_records[0].head.word & FIELD_PLAYER_ALT_APPEARANCE)
    {
        g_menu_nodes[0].icon_id = 2;
    }
    else
    {
        g_menu_nodes[0].icon_id = 1;
    }

    g_menu_nodes[0].layout.s.children[0] = 1;
    g_menu_nodes[1].idx_nav.s.self_idx = 1;
    g_menu_nodes[0].layout.s.children[1] = 2;
    g_menu_nodes[1].icon_id = 5;
    g_menu_nodes[2].label_id = 2;
    g_menu_nodes[2].idx_nav.s.self_idx = 2;
    g_menu_nodes[1].label_id = 3;
    g_menu_nodes[2].icon_id = 4;
    g_menu_nodes[1].u2.bits.char_slot = 0;
    g_menu_nodes[1].u2.bits.depth = 1;
    g_menu_nodes[1].u2.s.parent_idx = 0;
    g_menu_nodes[2].u2.bits.char_slot = 0;
    g_menu_nodes[2].u2.bits.depth = 1;
    g_menu_nodes[2].u2.s.parent_idx = 0;
    g_menu_nodes[3].u2.bits.expanded = 0;
    g_menu_nodes[3].u2.bits.char_slot = 1;
    g_menu_nodes[3].label_id = 4;
    g_menu_nodes[3].u2.bits.char_slot = 1;
    g_menu_nodes[3].u2.bits.depth = 0;
    g_menu_nodes[3].idx_nav.s.self_idx = 3;
    g_menu_nodes[3].u2.bits.active = 0;
    g_menu_nodes[3].u2.s.parent_idx = MENU_NONE;

    if (g_field_player_records[1].head.word & FIELD_PLAYER_ALT_APPEARANCE)
    {
        g_menu_nodes[3].icon_id = 0x6F;
    }
    else
    {
        g_menu_nodes[3].icon_id = 0x6E;
    }

    g_menu_nodes[4].u2.bits.char_slot = 1;
    g_menu_nodes[4].u2.bits.depth = 1;
    g_menu_nodes[5].u2.bits.char_slot = 1;
    g_menu_nodes[5].u2.bits.depth = 1;
    g_menu_nodes[3].layout.s.children[0] = 4;
    g_menu_nodes[3].layout.s.children[1] = 5;
    g_menu_nodes[4].label_id = 6;
    g_menu_nodes[4].idx_nav.s.self_idx = 4;
    g_menu_nodes[4].icon_id = 5;
    g_menu_nodes[5].label_id = 5;
    g_menu_nodes[5].idx_nav.s.self_idx = 5;
    g_menu_nodes[5].icon_id = 4;
    g_menu_nodes[6].u2.bits.expanded = 0;
    g_menu_nodes[6].u2.bits.char_slot = 1;
    g_menu_nodes[6].label_id = 7;
    g_menu_nodes[6].u2.bits.char_slot = 1;
    g_menu_nodes[6].u2.bits.depth = 0;
    g_menu_nodes[6].idx_nav.s.self_idx = 6;
    g_menu_nodes[6].u2.bits.active = 0;
    g_menu_nodes[6].icon_id = 3;
    g_menu_nodes[6].layout.s.children[0] = 7;
    g_menu_nodes[6].layout.s.children[1] = 8;
    g_menu_nodes[7].label_id = 9;
    g_menu_nodes[7].idx_nav.s.self_idx = 7;
    g_menu_nodes[7].icon_id = 5;
    g_menu_nodes[8].label_id = 8;
    g_menu_nodes[4].u2.s.parent_idx = 3;
    g_menu_nodes[5].u2.s.parent_idx = 3;
    g_menu_nodes[6].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[7].u2.bits.char_slot = 1;
    g_menu_nodes[7].u2.bits.depth = 1;
    g_menu_nodes[7].u2.s.parent_idx = 6;
    g_menu_nodes[8].u2.bits.char_slot = 1;
    g_menu_nodes[8].u2.bits.depth = 1;
    g_menu_nodes[8].idx_nav.s.self_idx = 8;
    g_menu_nodes[8].u2.s.parent_idx = 6;
    g_menu_nodes[8].icon_id = 4;
    g_menu_nodes[9].u2.bits.expanded = 0;
    g_menu_nodes[9].u2.bits.char_slot = 2;
    g_menu_nodes[9].label_id = 0xA;
    g_menu_nodes[9].u2.bits.char_slot = 2;
    g_menu_nodes[9].u2.bits.depth = 0;
    g_menu_nodes[9].idx_nav.s.self_idx = 9;
    g_menu_nodes[9].u2.bits.active = 0;
    g_menu_nodes[0xA].u2.bits.char_slot = 2;
    g_menu_nodes[0xA].u2.bits.depth = 1;
    g_menu_nodes[9].icon_id = 6;
    g_menu_nodes[9].layout.s.children[0] = 0xA;
    g_menu_nodes[0xA].label_id = 0xB;
    g_menu_nodes[0xA].idx_nav.s.self_idx = 0xA;
    g_menu_nodes[0xA].icon_id = 7;
    g_menu_nodes[0xC].u2.bits.expanded = 0;
    g_menu_nodes[0xC].u2.bits.char_slot = 2;
    g_menu_nodes[0xC].label_id = 0xA;
    g_menu_nodes[0xC].u2.bits.char_slot = 2;
    g_menu_nodes[0xC].u2.bits.depth = 0;
    g_menu_nodes[0xC].idx_nav.s.self_idx = 0xC;
    g_menu_nodes[0xC].u2.bits.active = 0;
    g_menu_nodes[0xC].icon_id = 6;
    g_menu_nodes[0xC].layout.s.children[0] = 0xD;
    g_menu_nodes[0xD].label_id = 0xB;
    g_menu_nodes[0xD].idx_nav.s.self_idx = 0xD;
    g_menu_nodes[0xD].icon_id = 7;
    g_menu_nodes[9].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xA].u2.s.parent_idx = 9;
    g_menu_nodes[0xC].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xD].u2.bits.char_slot = 2;
    g_menu_nodes[0xD].u2.bits.depth = 1;
    g_menu_nodes[0xD].u2.s.parent_idx = 0xC;
    g_menu_nodes[0xF].u2.bits.expanded = 0;
    g_menu_nodes[0xF].u2.bits.char_slot = 2;
    g_menu_nodes[0xF].icon_id = 8;
    g_menu_nodes[0xF].u2.bits.char_slot = 2;
    g_menu_nodes[0xF].u2.bits.depth = 1;
    g_menu_nodes[0x10].icon_id = 7;
    g_menu_nodes[0xF].u2.bits.active = 0;
    g_menu_nodes[0xF].label_id = 0xD;
    g_menu_nodes[0xF].idx_nav.s.self_idx = 0xF;
    g_menu_nodes[0xF].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xF].layout.s.children[0] = 0x10;
    g_menu_nodes[0xF].layout.s.children[1] = 0x11;
    g_menu_nodes[0x10].label_id = 0xC;
    g_menu_nodes[0x10].idx_nav.s.self_idx = 0x10;
    g_menu_nodes[0x11].label_id = 0xE;
    g_menu_nodes[0x11].idx_nav.s.self_idx = 0x11;
    g_menu_nodes[0x11].icon_id = 9;
    g_menu_nodes[0x12].label_id = 0x10;
    g_menu_nodes[0x12].idx_nav.s.self_idx = 0x12;
    g_menu_nodes[0x12].icon_id = 0xA;
    g_menu_nodes[0x12].content_id = 4;
    g_menu_nodes[0x12].layout.s.children[0] = 0x13;
    g_menu_nodes[0x12].layout.s.children[1] = 0x16;
    g_menu_nodes[0x12].layout.s.children[2] = 0x19;
    g_menu_nodes[0x10].u2.bits.char_slot = 2;
    g_menu_nodes[0x10].u2.bits.depth = 1;
    g_menu_nodes[0x10].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x11].u2.bits.char_slot = 2;
    g_menu_nodes[0x11].u2.bits.depth = 1;
    g_menu_nodes[0x12].u2.bits.expanded = 0;
    g_menu_nodes[0x11].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x12].u2.bits.depth = 0;
    g_menu_nodes[0x12].layout.s.children[3] = 0x1C;
    g_menu_nodes[0x12].u2.bits.active = 1;
    g_menu_nodes[0x12].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x13].label_id = 0x11;
    g_menu_nodes[0x14].icon_id = 0xF;
    g_menu_nodes[0x13].icon_id = 0xB;
    g_menu_nodes[0x13].idx_nav.s.self_idx = 0x13;
    g_menu_nodes[0x13].content_id = 0;
    g_menu_nodes[0x13].layout.s.children[0] = 0x14;
    g_menu_nodes[0x13].layout.s.children[1] = 0x15;
    g_menu_nodes[0x14].label_id = 0x12;
    g_menu_nodes[0x14].idx_nav.s.self_idx = 0x14;
    g_menu_nodes[0x15].label_id = 0x13;
    g_menu_nodes[0x16].content_id = 1;
    g_menu_nodes[0x15].idx_nav.s.self_idx = 0x15;
    g_menu_nodes[0x15].icon_id = 0x12;
    g_menu_nodes[0x16].label_id = 0x14;
    g_menu_nodes[0x16].idx_nav.s.self_idx = 0x16;
    g_menu_nodes[0x16].icon_id = 0xC;
    g_menu_nodes[0x16].layout.s.children[0] = 0x17;
    g_menu_nodes[0x16].layout.s.children[1] = 0x18;
    g_menu_nodes[0x17].label_id = 0x15;
    g_menu_nodes[0x17].idx_nav.s.self_idx = 0x17;
    g_menu_nodes[0x13].u2.bits.depth = 1;
    g_menu_nodes[0x14].u2.bits.depth = 2;
    g_menu_nodes[0x13].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x14].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x16].u2.bits.depth = 1;
    g_menu_nodes[0x16].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x15].u2.bits.depth = 2;
    g_menu_nodes[0x15].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x17].u2.bits.depth = 2;
    g_menu_nodes[0x17].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].content_id = 2;
    g_menu_nodes[0x17].icon_id = 0x10;
    g_menu_nodes[0x18].label_id = 0x13;
    g_menu_nodes[0x18].idx_nav.s.self_idx = 0x18;
    g_menu_nodes[0x18].icon_id = 0x12;
    g_menu_nodes[0x19].label_id = 0x16;
    g_menu_nodes[0x19].idx_nav.s.self_idx = 0x19;
    g_menu_nodes[0x19].icon_id = 0xD;
    g_menu_nodes[0x19].layout.s.children[0] = 0x1A;
    g_menu_nodes[0x19].layout.s.children[1] = 0x1B;
    g_menu_nodes[0x1A].label_id = 0x17;
    g_menu_nodes[0x1A].idx_nav.s.self_idx = 0x1A;
    g_menu_nodes[0x1A].icon_id = 0x11;
    g_menu_nodes[0x1B].label_id = 0x13;
    g_menu_nodes[0x1B].idx_nav.s.self_idx = 0x1B;
    g_menu_nodes[0x1B].icon_id = 0x12;
    g_menu_nodes[0x1C].label_id = 0x18;
    g_menu_nodes[0x1C].idx_nav.s.self_idx = 0x1C;
    g_menu_nodes[0x18].u2.bits.depth = 2;
    g_menu_nodes[0x19].u2.bits.depth = 1;
    g_menu_nodes[0x18].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x1B].u2.bits.depth = 2;
    g_menu_nodes[0x1B].u2.s.parent_idx = 0x19;
    g_menu_nodes[0x1A].u2.bits.depth = 2;
    g_menu_nodes[0x1C].u2.bits.depth = 1;
    g_menu_nodes[0x1C].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x1A].u2.s.parent_idx = 0x19;
    g_menu_nodes[0x1C].icon_id = 0xE;
    g_menu_nodes[0x1C].content_id = 5;
    g_menu_nodes[0x1D].label_id = 0x1C;
    g_menu_nodes[0x1D].content_id = 3;
    g_menu_nodes[0x1D].icon_id = 0x18;
    g_menu_nodes[0x1D].idx_nav.s.self_idx = 0x1D;
    g_menu_nodes[0x1E].label_id = 0x19;
    g_menu_nodes[0x1E].idx_nav.s.self_idx = 0x1E;
    g_menu_nodes[0x1E].layout.s.children[0] = 0x1F;
    g_menu_nodes[0x1F].idx_nav.s.self_idx = 0x1F;
    g_menu_nodes[0x1E].icon_id = 0x13;
    g_menu_nodes[0x1F].label_id = 0x1A;
    g_menu_nodes[0x1F].icon_id = 0x14;
    g_menu_nodes[0x2B].label_id = 0x1A;
    g_menu_nodes[0x1D].u2.bits.expanded = 0;
    g_menu_nodes[0x2B].idx_nav.s.self_idx = 0x2B;
    g_menu_nodes[0x1D].u2.bits.depth = 0;
    node30_flags = g_menu_nodes[0x1E].u2.unk2;
    g_menu_nodes[0x1D].u2.bits.active = 1;
    g_menu_nodes[0x1D].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x1F].u2.bits.depth = 1;
    g_menu_nodes[0x1E].u2.unk2 = node30_flags & ~MENU_NODE_FLAG_EXPANDED;
    g_menu_nodes[0x1F].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1E].u2.bits.depth = 0;
    g_menu_nodes[0x2B].u2.bits.depth = 1;
    g_menu_nodes[0x1E].u2.bits.active = 1;
    g_menu_nodes[0x1E].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x2B].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1F].u2.bits.char_slot = 0;
    g_menu_nodes[0x2B].u2.bits.char_slot = 1;
    g_menu_nodes[0x2B].icon_id = 0x15;
    g_menu_nodes[0x20].label_id = 0x1B;
    g_menu_nodes[0x20].u2.bits.expanded = 0;
    g_menu_nodes[0x20].idx_nav.s.self_idx = 0x20;
    g_menu_nodes[0x20].u2.bits.depth = 0;
    g_menu_nodes[0x20].icon_id = 0x16;
    g_menu_nodes[0x20].u2.bits.active = 1;
    g_menu_nodes[0x20].u2.s.parent_idx = MENU_NONE;

    if (g_field_player_records[1].head.word & FIELD_PLAYER_ACTIVE)
    {
        if (g_field_player_records[1].character_kind)
        {
            g_menu_nodes[6].u2.bits.active = 1;
        }
        else
        {
            g_menu_nodes[3].u2.bits.active = 1;
        }
    }

    if (g_field_player_records[2].head.word & FIELD_PLAYER_ACTIVE)
    {
        if ((g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.word & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GOLEM)
        {
            g_menu_nodes[0xF].u2.bits.active = 1;
        }
        else
        {
            g_menu_nodes[9].u2.bits.active = 1;
        }
    }

    if ((g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word & 0x80) && g_saved_game_ctx->characters[FIELD_PARTY_GUEST].name[0])
    {
        g_menu_companion_node = 0x2B;
    }

    layout_y = 0;
    node_index = 0;
    zero_value = 0;

    while (node_index < MENU_NODE_COUNT)
    {
        work_value_a = MENU_NONE;
        work_value_b = g_menu_nodes[node_index].u2.s.parent_idx;

        if (work_value_b == work_value_a)
        {
            work_value_a = g_menu_nodes[node_index].u2.s.flags & MENU_NODE_FLAG_ACTIVE;

            if (work_value_a)
            {
                layout_y_16 = layout_y & 0xFFFF;
                layout_y_9 = layout_y & 0x1FF;
                layout_y += MENU_ROW_HEIGHT;
                packed_value = (layout_y_16 & 1) << 15;
                layout_y_high = (layout_y_16 >> 1) & 0xFF;
                work_value_a = g_menu_nodes[node_index].u8_u.nav_y_packed & 0x80FF;
                g_menu_nodes[node_index].u8_u.nav_y_packed = (u16)work_value_a;
                work_value_b = g_menu_nodes[node_index].idx_nav.nav_x_packed & 0x80FF;
                g_menu_nodes[node_index].idx_nav.nav_x_packed = (u16)work_value_b;
                g_menu_nodes[node_index].layout.layout_child_packed = (u16)((g_menu_nodes[node_index].layout.layout_child_packed & 0xFF00) | layout_y_high);
                nav_x_low_bit = (layout_y_9 & 1) << 15;
                g_menu_nodes[node_index].u8_u.nav_y_packed = (u16)((g_menu_nodes[node_index].u8_u.nav_y_packed & 0x7FFF) | packed_value);
                g_menu_nodes[node_index].idx_nav.nav_x_packed = (u16)((g_menu_nodes[node_index].idx_nav.nav_x_packed & 0x7FFF) | nav_x_low_bit);
                layout_y_high_9 = layout_y_9 >> 1;
                g_menu_nodes[node_index].u8_u.nav_y_packed = (u16)((g_menu_nodes[node_index].u8_u.nav_y_packed & 0xFF00) | layout_y_high_9);
            }
        }

        node_index++;
    }

    if (g_active_script)
    {
        g_menu_scene_type = -1;
        return;
    }

    scene_type = zero_value;
    g_menu_scene_type = scene_type;
    packed_value = zero_value;
    g_menu_equipment_conflicts = packed_value;
    menu_open_content_page(g_menu_init_content_id);
    menu_set_active_node();
}

/**
 * @brief Collapse every menu node while preserving its other state flags.
 */
void menu_collapse_all(void)
{
    s32 node_index;

    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        g_menu_nodes[node_index].u2.unk2 &= ~MENU_NODE_FLAG_EXPANDED;
    }
}

/**
 * @brief Rebuild the visible node layout and update its scroll state.
 */
void menu_update_layout(void)
{
    s32 has_visible_children = 0;
    s32 layout_end = has_visible_children;
    s32 node_index = layout_end;

    for (; node_index < MENU_NODE_COUNT; node_index += 1)
    {
        if (g_menu_nodes[node_index].u2.s.parent_idx == MENU_NONE)
        {
            if (g_menu_nodes[node_index].u2.s.flags & MENU_NODE_FLAG_ACTIVE)
            {
                s32 root_y = layout_end;
                layout_end = menu_layout_node(node_index, layout_end);

                /* A root that contributes multiple rows has visible descendants. */
                if (root_y != (layout_end - MENU_ROW_HEIGHT))
                {
                    has_visible_children = 1;

                    if (layout_end > MENU_VIEW_HEIGHT)
                    {
                        g_menu_scroll_pos = layout_end - MENU_VIEW_HEIGHT;
                        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
                    }
                }
            }
        }
    }

    g_menu_layout_end = layout_end;

    if (has_visible_children == 0)
    {
        g_menu_scroll_pos = 0;
        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
    }
}
/**
 * @brief Assign rows to a node and its visible descendants.
 * @param node_index Node to lay out.
 * @param base_pos Top of the node's row.
 * @return Position following the last assigned row.
 */
s32 menu_layout_node(s32 node_index, s32 base_pos)
{
    s32 layout_pos;
    MenuNode* node;
    int is_expanded;
    u32 layout_y;
    MenuNode* child_source;

    layout_pos = base_pos;
    is_expanded = (((u16)(&g_menu_nodes[node_index])->u2.unk2) >> 1) & 1;
    layout_y = layout_pos & 0xFFFF;
    layout_pos += MENU_ROW_HEIGHT;
    node = &g_menu_nodes[node_index];

    g_menu_nodes[node_index].layout_frames_remaining = MENU_NODE_LAYOUT_STEPS;
    /* Pack the 9-bit layout Y across layout_x_y0 and layout_y_hi. */
    node->u8_u.nav_y_packed = node->u8_u.s.nav_y_hi | ((layout_y & 1) << 15);
    g_menu_nodes[node_index].layout.layout_child_packed = (g_menu_nodes[node_index].layout.layout_child_packed & 0xFF00) | (0xFF & (layout_y >> 1));
    node->layout.layout_child_packed = (node->layout.layout_child_packed & 0xFF00) | ((layout_y >> 1) & 0xFF);

    if (is_expanded)
    {
        s32 child_index;

        child_index = 0;
        child_source = node;

        while (child_index < MENU_MAX_CHILDREN)
        {
            if (child_source->layout.s.children[child_index] == MENU_NONE)
            {
                break;
            }

            layout_pos = menu_layout_node(child_source->layout.s.children[child_index++], layout_pos);
        }
    }

    return layout_pos;
}

/**
 * @brief Draw the menu frame layers and optionally dispatch navigation input.
 * @param packet_cursor Primitive buffer location for the frame.
 * @param ot_entry Ordering-table entry for the frame primitives.
 * @param frame_parity Selects the active double-buffered VRAM page.
 * @param allow_input Nonzero to dispatch navigation input.
 * @return Primitive buffer location immediately after the frame.
 */
u8* menu_draw_frame(u8* packet_cursor, u_long* ot_entry, s32 frame_parity, s32 allow_input)
{
    DRAWENV draw_env;
    DR_ENV* draw_env_packet;
    DR_TPAGE* draw_mode;
    u8* frame_cursor;
    s32 draw_y;
    void* node_tree_end;
    MenuControllerActuatorState* actuator_state;
    u8* frame_end;

    actuator_state = MENU_CONTROLLER_ACTUATORS;

    /* Emit the full-screen draw environment before the menu layers. */
    draw_env_packet = menu_draw_scene_content(packet_cursor, ot_entry);
    draw_y = frame_parity ? SCREEN_HEIGHT : VRAM_BACK_DRAW_Y;
    SetDefDrawEnv(&draw_env, 0, draw_y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDrawEnv(draw_env_packet, &draw_env);
    addPrim(ot_entry, draw_env_packet);

    draw_env_packet++;
    frame_cursor = (u8*)draw_env_packet;

    /* Fade the large-motor command and mirror it to the second port. */
    if (actuator_state->ports[0].large_motor_command != 0)
    {
        u8 motor_command = actuator_state->ports[0].large_motor_command - 1;
        actuator_state->ports[1].large_motor_command = motor_command;
        actuator_state->ports[0].large_motor_command = motor_command;
    }

    /* Render and update the active node or content cursor mode. */
    switch (g_menu_cursor_mode)
    {
        case MENU_CURSOR_MODE_NODE_TREE:
            frame_cursor = menu_draw_active_node_cursor(draw_env_packet, ot_entry - 1, allow_input);
            menu_handle_input(0);

            if (allow_input != 0)
            {
                menu_handle_node_input();
            }

            break;

        case MENU_CURSOR_MODE_CONTENT:
            frame_cursor = menu_draw_content_cursor(draw_env_packet, ot_entry - 1, allow_input);

            if (g_menu_suppress_cursor == 0)
            {
                menu_handle_input(allow_input);
            }

            break;

        case MENU_CURSOR_MODE_CONTENT_EXIT:
            frame_cursor = menu_draw_content_cursor(draw_env_packet, ot_entry - 1, allow_input);

            if (g_menu_suppress_cursor == 0)
            {
                g_menu_cursor_mode = MENU_CURSOR_MODE_NODE_TREE;
            }

            break;

        default:
            break;
    }

    /* Draw the node tree and its scroll indicators. */
    node_tree_end = menu_draw_node_tree(frame_cursor, ot_entry);
    frame_cursor = menu_emit_tree_scroll_arrows(node_tree_end, ot_entry - 1);

    draw_mode = (DR_TPAGE*)frame_cursor;
    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot_entry, draw_mode);
    draw_env_packet = (DR_ENV*)(draw_mode + 1);

    /* Restrict the final draw environment to the node-tree viewport. */
    draw_y = frame_parity ? SCREEN_HEIGHT + MENU_TREE_DRAW_Y_OFFSET : VRAM_BACK_DRAW_Y + MENU_TREE_DRAW_Y_OFFSET;
    frame_end = (u8*)(draw_env_packet + 1);
    SetDefDrawEnv(&draw_env, MENU_TREE_DRAW_X, draw_y, MENU_TREE_DRAW_WIDTH, MENU_TREE_DRAW_HEIGHT);
    SetDrawEnv(draw_env_packet, &draw_env);
    addPrim(ot_entry, draw_env_packet);

    return frame_end;
}

/**
 * @brief Process D-pad and face-button input to navigate and select menu nodes.
 * @return Unspecified; callers ignore the return value.
 */
s32 menu_handle_node_input(void)
{
    MenuNode* active_node;
    s32 nav_index;
    s32 content_height;
    MenuNode* nodes;
    s32 cursor_y_hi;
    s32 cursor_y_lsb;
    s32 view_y;
    s32 active_row_y;
    s32 work_index;
    MenuContentItem* content_items;
    nav_index = menu_find_nav_node_index(g_menu_active_node);

    if (nav_index == (-1))
    {
        return;
    }

    if (g_pad_input & PAD_BTN_UP)
    {
        if (nav_index != 0)
        {
            g_menu_active_node = g_menu_nav_nodes[nav_index - 1];
        }
        else
        {
            g_menu_active_node = g_menu_nav_nodes[g_menu_nav_count - 1];
        }
    }

    if (g_pad_input & PAD_BTN_DOWN)
    {
        if (nav_index >= (g_menu_nav_count - 1))
        {
            g_menu_active_node = g_menu_nav_nodes[0];
        }
        else
        {
            g_menu_active_node = g_menu_nav_nodes[nav_index + 1];
        }
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        if (g_menu_active_node == MENU_NODE_BROWSE_ALL)
        {
            menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
            g_menu_load_request = 1;
            return;
        }

        g_menu_active_node = MENU_NODE_BROWSE_ALL;
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN | PAD_BTN_CIRCLE))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        active_row_y = menu_find_nav_node_index(g_menu_active_node);
        active_row_y *= MENU_ROW_HEIGHT;
        work_index = active_row_y - g_menu_scroll_pos;

        if (work_index < 0)
        {
            g_menu_scroll_pos = active_row_y;
            g_menu_redraw_state = MENU_REDRAW_NAVIGATE;
        }
        else if (work_index >= MENU_VIEW_HEIGHT)
        {
            g_menu_scroll_pos = active_row_y - (MENU_VIEW_HEIGHT - MENU_ROW_HEIGHT);
            g_menu_redraw_state = MENU_REDRAW_NAVIGATE;
        }

        return;
    }

    if (g_pad_input & (PAD_BTN_RIGHT | PAD_CONFIRM_BUTTONS))
    {
        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);

        if (g_menu_active_node == MENU_NODE_BROWSE_ALL)
        {
            if (!(g_pad_input & PAD_CONFIRM_BUTTONS))
            {
                return;
            }

            g_menu_load_request = 1;
        }

        if (g_menu_active_node == 0x11)
        {
            g_menu_load_request = 1;
            g_menu_transition_code = FIELD_MENU_GOLEM;
            return;
        }

        nodes = g_menu_nodes;
        active_node = nodes + g_menu_active_node;
        active_node->u2.unk2 |= 0xC;

        if (active_node->u2.s.parent_idx == MENU_NONE)
        {
            g_menu_item_ptr = 0;
            g_menu_category0_item = 0;
            g_menu_category1_item = 0;
            g_menu_category2_item = 0;
        }

        g_menu_prev_node = MENU_NONE;

        if (g_menu_scene_type != g_menu_active_node)
        {
            if (g_menu_content_table[active_node->idx_nav.s.self_idx] != NULL)
            {
                g_menu_scene_type = g_menu_active_node;
                menu_clear_slots();

                if (g_menu_nodes[g_menu_scene_type].content_id != MENU_NONE)
                {
                    g_menu_equipment_conflicts = 0;
                    menu_open_content_page(g_menu_nodes[g_menu_scene_type].content_id);
                }
            }

            menu_set_active_node();
            return;
        }

        if (g_menu_scene_type == (-1))
        {
            return;
        }

        if (g_menu_scene_type == MENU_NODE_BROWSE_ALL)
        {
            if (g_pad_input & PAD_CONFIRM_BUTTONS)
            {
                g_menu_load_request = 1;
            }

            return;
        }

        cursor_y_lsb = active_node->idx_nav.nav_x_packed >> 15;
        cursor_y_hi = active_node->u8_u.s.nav_y_hi;
        content_height = g_menu_content_height;
        g_content_cursor_y = MENU_CURSOR_Y_MIN;
        g_content_cursor_y = ((cursor_y_hi * 2) | cursor_y_lsb) - (content_height - g_content_cursor_y);

        if (g_content_cursor_y < MENU_CURSOR_Y_MIN)
        {
            g_content_cursor_y = MENU_CURSOR_Y_MIN;
        }

        if (g_content_cursor_y >= MENU_CURSOR_Y_MAX)
        {
            g_content_cursor_y = MENU_CURSOR_Y_MAX;
        }

        g_content_cursor_x = menu_nav_x(active_node->idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;

        if (active_node->content_id != MENU_NONE)
        {
            g_menu_cursor_mode = MENU_CURSOR_MODE_CONTENT;
            work_index = 0;

            while (work_index < MENU_SLOT_COUNT)
            {
                if (g_menu_slots[work_index].active != 0)
                {
                    g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                    g_content_view_x = g_menu_default_view_pos.x;
                    g_content_view_y = g_menu_default_view_pos.y;
                    break;
                }

                work_index++;
            }
        }
        else
        {
            g_menu_hit_item_idx = menu_find_active_content_item();

            if (g_menu_hit_item_idx != (-1))
            {
                content_items = g_menu_content_table[nodes[g_menu_scene_type].idx_nav.s.self_idx];
                g_content_view_x = MENU_CONTENT_X(&content_items[g_menu_hit_item_idx]);
                view_y = MENU_CONTENT_Y(&content_items[g_menu_hit_item_idx]);
                g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                g_menu_cursor_mode = MENU_CURSOR_MODE_CONTENT;
                g_content_view_y = view_y;
            }
        }
    }
}

/**
 * @brief Focus the active content item and snap the viewport to its position.
 * @return 1 if an active item was found; otherwise 0.
 */
inline s32 menu_focus_active_content_item(void)
{
    MenuContentItem* content_items;
    s32 view_y;

    g_menu_hit_item_idx = menu_find_active_content_item();

    if (g_menu_hit_item_idx != (-1))
    {
        MenuNode* nodes = g_menu_nodes;
        u8 content_table_idx = (nodes + g_menu_scene_type)->idx_nav.s.self_idx;

        content_items = g_menu_content_table[content_table_idx];
        g_content_view_x = MENU_CONTENT_X(&content_items[g_menu_hit_item_idx]);
        view_y = MENU_CONTENT_Y(&content_items[g_menu_hit_item_idx]);
        g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
        g_menu_cursor_mode = MENU_CURSOR_MODE_CONTENT;
        g_content_view_y = view_y;
        return 1;
    }

    return 0;
}
/**
 * @brief Expand the active path, initialize child cursor positions, and rebuild the layout.
 */
void menu_set_active_node(void)
{
    s32 clear_index;
    s32 walk_off;
    uintptr_t walk_base;
    MenuNode* active_node;
    MenuNode* active_base;
    MenuNode* loop_active;
    u16 packed_value;
    s32 active_idx;
    s32 char_slot_bits;
    s32 has_visible_children;
    s32 layout_pos;
    s32 node_index;
    s32 prev_layout_y;
    long child_slot;
    s32 child_index_value;
    s32 reloaded_child_index;
    u16 wide_child_index;
    MenuNode* child_node;
    uintptr_t node_base_addr;

    /* Clear the "expanded" bit (bit 1) on every node, then re-expand only the active path. */
    for (clear_index = 0; clear_index < MENU_NODE_COUNT; clear_index++)
    {
        g_menu_nodes[clear_index].u2.unk2 &= ~MENU_NODE_FLAG_EXPANDED;
    }

    /* Walk from g_menu_active_node up to the root, marking each ancestor expanded. */
    child_slot = g_menu_active_node;
    walk_base = (uintptr_t)g_menu_nodes;
    walk_off = child_slot << 4;

    if (((MenuNode*)(walk_base + walk_off))->u2.s.parent_idx != MENU_NONE)
    {
        do
        {
            child_slot = ((MenuNode*)(walk_off + walk_base))->u2.s.parent_idx;
            walk_off = child_slot << 4;
            ((MenuNode*)(walk_off + walk_base))->u2.unk2 |= MENU_NODE_FLAG_EXPANDED;
        } while (((MenuNode*)(walk_off + walk_base))->u2.s.parent_idx != MENU_NONE);
    }

    /* Mark the active node itself expanded, then propagate its nav cursor to children. */
    active_base = g_menu_nodes;
    active_idx = g_menu_active_node;
    active_node = active_base + active_idx;
    packed_value = active_node->u2.unk2 | MENU_NODE_FLAG_EXPANDED;
    active_node->u2.unk2 = packed_value;

    if ((packed_value >> 1) & 1)
    {
        child_slot = 0;
        node_base_addr = (uintptr_t)active_base;
        loop_active = active_node;

        for (; child_slot < MENU_MAX_CHILDREN; child_slot++)
        {
            child_index_value = loop_active->layout.s.children[child_slot];
            wide_child_index = child_index_value;
            packed_value = MENU_NONE;

            if (child_index_value == packed_value)
            {
                break;
            }

            child_node = (MenuNode*)(((u32)wide_child_index << 4) + node_base_addr);
            ((MenuNodeCoordinateView*)child_node)->layout_x = ((MenuNodeCoordinateView*)child_node)->nav_x = ((MenuNodeCoordinateView*)loop_active)->nav_x;
            reloaded_child_index = loop_active->layout.s.children[child_slot];
            ((MenuNodeCoordinateView*)(((u32)reloaded_child_index << 4) + node_base_addr))->nav_y = ((MenuNodeCoordinateView*)loop_active)->nav_y;
        }
    }

    /* Update g_menu_char_slot from bits [5:4] of u2.unk2 (3 = preserve current value). */
    char_slot_bits = (((u16)g_menu_nodes[g_menu_active_node].u2.unk2) >> 4) & 3;

    if (char_slot_bits != 3)
    {
        g_menu_char_slot = char_slot_bits;
    }

    /* Re-run layout for all root nodes and adjust scroll if content overflows the viewport. */
    has_visible_children = 0;
    layout_pos = 0;

    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        packed_value = MENU_VIEW_HEIGHT;

        if (g_menu_nodes[node_index].u2.s.parent_idx == MENU_NONE)
        {
            if (g_menu_nodes[node_index].u2.s.flags & MENU_NODE_FLAG_ACTIVE)
            {
                prev_layout_y = layout_pos;
                layout_pos = menu_layout_node(node_index, layout_pos);

                /* If this node contributed more than one row, it has visible children. */
                if (prev_layout_y != (layout_pos - MENU_ROW_HEIGHT))
                {
                    has_visible_children = 1;

                    if (layout_pos > MENU_VIEW_HEIGHT)
                    {
                        g_menu_scroll_pos = layout_pos - packed_value;
                        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
                    }
                }
            }
        }
    }

    g_menu_layout_end = layout_pos;

    if (has_visible_children == 0)
    {
        g_menu_scroll_pos = 0;
        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
    }
}

/**
 * @brief Return the start of the active character's block in the saved game.
 * @return Byte pointer to character block g_menu_char_slot of g_saved_game_ctx.
 */
static inline u8* menu_character_block(void)
{
    return (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
}

/** @brief Return the saved game shifted so that characters[0] is the character whose button bindings the active page edits (party slot 0 or 1). */
static inline SavedGameLayout* menu_button_page_context(void)
{
    u8* base = (u8*)g_saved_game_ctx;
    s32 off = (g_menu_scene_type == MENU_SCENE_BUTTONS_CHARACTER_0) ? 0 : MENU_CHARACTER_BLOCK_SIZE;

    return (SavedGameLayout*)(base + off);
}

/**
 * @brief Per-frame menu navigation, confirm/cancel and cursor-move input handler.
 * @param process_actions Non-zero to also process confirm/cancel and the four cursor-move buttons; 0 runs only node switching.
 * @return Unspecified; callers do not consume the return value.
 */
s32 menu_handle_input(s32 process_actions)
{
    MenuControllerActuatorState* actuator_state = MENU_CONTROLLER_ACTUATORS;
    const MenuContentItem* content_items;
    u32 content_type;
    MenuSlot* submenu_slot;
    s32 node_self_index;
    s32 button_a, button_b;
    u8* binding;
    u16 item_packed_x;
    u16 item_type;
    u8 action_group;
    u8 action_code;
    s32 work_index;
    MenuNode* selected_node;
    MenuNode* node_base;
    s32 node_nav_y;
    s32 node_nav_y_low_bit;
    s32 scroll_extent;
    u8 item_flag;
    u32* inject_flags;
    u32 item_word;
    s32 text_variant;
    uintptr_t name_table;
    u8* label_table;
    uintptr_t description_table;
    uintptr_t detail_table;
    MenuSlotRect rect;
    s8 text_buffer[0x40];
    s8* text_cursor;

    if ((u32)(g_menu_scene_type - 0x14) < 2 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1A || g_menu_scene_type == 0x1B)
    {
        if (g_pad_input & (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1))
        {
            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

            if (g_pad_input & (PAD_BTN_L2 | PAD_BTN_R2))
            {
                if (g_menu_item_ptr != NULL)
                {
                    if (g_pad_input & PAD_BTN_L2)
                    {
                        menu_step_item_selection(-1);

                        if (g_script_repeat_last == 0)
                        {
                            g_script_repeat_last = g_menu_page_count - 1;
                        }
                        else
                        {
                            g_script_repeat_last -= 1;
                        }
                    }
                    else if (g_pad_input & PAD_BTN_R2)
                    {
                        menu_step_item_selection(1);

                        if (g_script_repeat_last == (g_menu_page_count - 1))
                        {
                            g_script_repeat_last = 0;
                        }
                        else
                        {
                            g_script_repeat_last += 1;
                        }
                    }
                }
            }
            else
            {
                if (g_menu_scene_type == 0x14 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x1A)
                {
                    g_menu_scene_type += 1;
                    g_menu_active_node = g_menu_scene_type;
                }
                else if (g_menu_scene_type == 0x15 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1B)
                {
                    g_menu_scene_type -= 1;
                    g_menu_active_node = g_menu_scene_type;
                }

                if (g_menu_cursor_mode == MENU_CURSOR_MODE_NODE_TREE)
                {
                    menu_snap_view_to_cursor();
                }

                menu_set_active_node();
                menu_focus_active_content_item();
            }
        }
    }

    if (g_menu_scene_type < 0x11U && (g_pad_input & (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1)))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

        switch (g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx - 0x14)
        {
            case 0:
            case 3:
            case 6:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx += 1;
                menu_focus_active_content_item();
                break;

            case 1:
            case 4:
            case 7:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx -= 1;
                menu_focus_active_content_item();
                break;

            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
                break;

            case 2:
            case 5:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            default:
                while (1)
                {
                    if (g_pad_input & PAD_BTN_L1)
                    {
                        if (g_menu_scene_type == ((g_menu_scene_type / 3) * 3))
                        {
                            if (g_menu_scene_type >= 9)
                            {
                                g_menu_scene_type += 1;
                                g_menu_active_node = g_menu_scene_type;
                            }
                            else
                            {
                                g_menu_scene_type += 2;
                                g_menu_active_node = g_menu_scene_type;
                            }
                        }
                        else
                        {
                            g_menu_scene_type -= 1;
                            g_menu_active_node = g_menu_scene_type;
                        }
                    }

                    if (g_pad_input & PAD_BTN_R1)
                    {
                        if (g_menu_scene_type < 9)
                        {
                            if ((g_menu_scene_type % 3) == 2)
                            {
                                g_menu_scene_type -= 2;
                                g_menu_active_node = g_menu_scene_type;
                            }
                            else
                            {
                                g_menu_scene_type += 1;
                                g_menu_active_node = g_menu_scene_type;
                            }
                        }
                        else
                        {
                            if ((g_menu_scene_type % 3) == 1)
                            {
                                g_menu_scene_type -= 1;
                                g_menu_active_node = g_menu_scene_type;
                            }
                            else
                            {
                                g_menu_scene_type += 1;
                                g_menu_active_node = g_menu_scene_type;
                            }
                        }
                    }

                    if ((g_pad_input & PAD_BTN_L2) && (g_menu_scene_type < 9))
                    {
                        if ((g_menu_scene_type / 3) == 0)
                        {
                            g_menu_scene_type += 6;
                            g_menu_active_node = g_menu_scene_type;
                        }
                        else
                        {
                            g_menu_scene_type -= 3;
                            g_menu_active_node = g_menu_scene_type;
                        }

                        menu_set_active_node();
                    }

                    if ((g_pad_input & PAD_BTN_R2) && (g_menu_scene_type < 9))
                    {
                        if ((g_menu_scene_type / 3) == 2)
                        {
                            g_menu_scene_type -= 6;
                            g_menu_active_node = g_menu_scene_type;
                        }
                        else
                        {
                            g_menu_scene_type += 3;
                            g_menu_active_node = g_menu_scene_type;
                        }

                        menu_set_active_node();
                    }

                    if (!(g_menu_nodes[(g_menu_scene_type / 3) * 3].u2.s.flags & MENU_NODE_FLAG_ACTIVE))
                    {
                        continue;
                    }

                    if (g_menu_cursor_mode == MENU_CURSOR_MODE_NODE_TREE)
                    {
                        menu_snap_view_to_cursor();
                    }

                    if (menu_focus_active_content_item())
                    {
                        break;
                    }
                }

                menu_clear_slots();
                break;
        }
    }

    if (process_actions != 0)
    {
        content_items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];

        if ((g_menu_scene_type == MENU_SCENE_BUTTONS_CHARACTER_0 || g_menu_scene_type == MENU_SCENE_BUTTONS_CHARACTER_1) &&
            g_menu_hit_item_idx >= MENU_BUTTON_ITEM_FIRST && g_menu_hit_item_idx < MENU_BUTTON_ITEM_FIRST + MENU_BUTTON_COUNT)
        {
            if (g_pad_input & PAD_BTN_CIRCLE)
            {
                if (g_menu_button_swap_marker.selected_idx != MENU_NONE)
                {
                    g_pad_input &= ~PAD_BTN_CIRCLE;
                    g_menu_button_swap_marker.selected_idx = MENU_NONE;
                }
                else
                {
                    g_pad_input &= ~PAD_BTN_CIRCLE;
                    menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                    menu_reset_content_view();
                }
            }

            if (g_pad_input & PAD_CONFIRM_BUTTONS)
            {
                if (g_menu_button_swap_marker.selected_idx == MENU_NONE)
                {
                    menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                    g_menu_button_swap_marker.x = (s8)(((MenuContentItemBits*)content_items)[g_menu_hit_item_idx].x - 2);
                    g_menu_button_swap_marker.y = (s8)MENU_CONTENT_Y(&content_items[g_menu_hit_item_idx]);
                    g_menu_button_swap_marker.selected_idx = (u8)g_menu_hit_item_idx;
                }
                else if (g_menu_button_swap_marker.selected_idx != g_menu_hit_item_idx)
                {
                    menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);

                    for (button_a = 0; button_a < MENU_BUTTON_COUNT; button_a++)
                    {
                        if (menu_button_page_context()->characters[0].button_actions[button_a] == (g_menu_hit_item_idx - MENU_BUTTON_ITEM_FIRST))
                        {
                            break;
                        }
                    }

                    for (button_b = 0; button_b < MENU_BUTTON_COUNT; button_b++)
                    {
                        if (menu_button_page_context()->characters[0].button_actions[button_b] ==
                            (g_menu_button_swap_marker.selected_idx - MENU_BUTTON_ITEM_FIRST))
                        {
                            break;
                        }
                    }

                    work_index = menu_button_page_context()->characters[0].button_actions[button_a];
                    binding = (u8*)menu_button_page_context() + button_a + MENU_CHARACTER_FIELD_OFFSET(button_actions);
                    *binding = menu_button_page_context()->characters[0].button_actions[button_b];
                    menu_button_page_context()->characters[0].button_actions[button_b] = work_index;
                    g_menu_button_swap_marker.selected_idx = MENU_NONE;
                }
                else
                {
                    menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
                    g_menu_button_swap_marker.selected_idx = MENU_NONE;
                }
            }
            else if (g_pad_input & PAD_BTN_SQUARE)
            {
                menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
                menu_button_page_context()->characters[0].button_actions[0] = 0;
                menu_button_page_context()->characters[0].button_actions[1] = 1;
                menu_button_page_context()->characters[0].button_actions[2] = 2;
                menu_button_page_context()->characters[0].button_actions[3] = 3;
                menu_button_page_context()->characters[0].button_actions[4] = 4;
                menu_button_page_context()->characters[0].button_actions[5] = 5;
                menu_button_page_context()->characters[0].button_actions[6] = 6;
                menu_button_page_context()->characters[0].button_actions[7] = 7;
            }
        }
        else if (g_pad_input & PAD_CONFIRM_BUTTONS)
        {
            item_packed_x = content_items[g_menu_hit_item_idx].packed_x;
            item_type = item_packed_x & MENU_CONTENT_ITEM_TYPE_MASK;

            if (item_type == MENU_CONTENT_ITEM_TYPE_SUBMENU)
            {
                g_menu_active_subtype = content_items[g_menu_hit_item_idx].action_type;
                content_type = content_items[g_menu_hit_item_idx].action_type;

                switch (content_type)
                {
                    case 1:
                    case 2:
                        if (g_menu_char_slot == 0)
                        {
                            setRECT(&rect, 0x40, 0x60, 0xF0, 0x60);
                            submenu_slot = menu_slot_alloc(3, &rect);
                            submenu_slot->content_cb = &menu_ability_list_callback;
                            submenu_slot->navigation.packed = MENU_LIST_WITH_COUNT(submenu_slot->navigation.packed, menu_build_ability_nav_entries());
                            submenu_slot->has_title = 1;
                            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                        }

                        break;

                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        if (g_menu_char_slot == 0)
                        {
                            item_flag = *((u8*)g_saved_game_ctx + content_type + MENU_ACTION_SLOT_OFFSET);

                            if ((item_flag != MENU_NONE) && (item_flag & 0x80))
                            {
                                setRECT(&rect, 0xB0, 0x60, 0x70, 0x50);
                            }
                            else
                            {
                                setRECT(&rect, 0xB0, 0x60, 0x70, 0x40);
                            }

                            submenu_slot = menu_slot_alloc(3, &rect);
                            submenu_slot->content_cb = &menu_skill_action_callback;
                            item_flag = *(menu_character_block() + content_type + MENU_ACTION_SLOT_OFFSET);

                            if ((item_flag != MENU_NONE) && (item_flag & 0x80))
                            {
                                submenu_slot->navigation.packed = MENU_LIST_WITH_COUNT(submenu_slot->navigation.packed, 4);
                                menu_init_item_nav_entries(4);
                            }
                            else
                            {
                                submenu_slot->navigation.packed = MENU_LIST_WITH_COUNT(submenu_slot->navigation.packed, 3);
                                menu_init_item_nav_entries(3);
                            }

                            scroll_list_update_target((ScrollListState*)submenu_slot, g_menu_scroll_nav_entries);

                            g_menu_active_equipped_item =
                                (FieldItemRecord*)((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET) +
                                                   ((content_type * MENU_ITEM_RECORD_SIZE) + (MENU_CHARACTER_ITEMS_OFFSET + MENU_ITEM_RECORD_SIZE)));
                            g_menu_item_ptr = g_menu_active_equipped_item;
                            g_menu_saved_category0_item = g_menu_item_ptr;
                            g_menu_category0_item = g_menu_item_ptr;
                            g_menu_saved_category1_item = g_menu_item_ptr;
                            g_menu_category1_item = g_menu_item_ptr;
                            g_menu_saved_equipment_item = g_menu_item_ptr;
                            g_menu_category2_item = g_menu_item_ptr;
                            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                        }

                        break;

                    case 7:
                    case 8:
                    case 9:
                    case 10:
                        if (g_menu_char_slot == 0)
                        {
                            setRECT(&rect, 0xB0, 0x30, 0x70, 0x60);
                            submenu_slot = menu_slot_alloc(3, &rect);
                            submenu_slot->content_cb = &menu_equipment_action_callback;
                            submenu_slot->navigation.packed = MENU_LIST_WITH_COUNT(submenu_slot->navigation.packed, 5);
                            menu_init_item_nav_entries(5);
                            g_menu_active_equipped_item =
                                (FieldItemRecord*)((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET) +
                                                   ((content_type * MENU_ITEM_RECORD_SIZE) +
                                                    (MENU_CHARACTER_ITEMS_OFFSET - MENU_EQUIPMENT_SUBTYPE_BASE * MENU_ITEM_RECORD_SIZE)));
                            g_menu_saved_category0_item = g_menu_active_equipped_item;
                            g_menu_saved_category1_item = g_menu_active_equipped_item;
                            g_menu_saved_equipment_item = g_menu_active_equipped_item;
                            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                        }

                        break;

                    case 15:
                        if ((g_menu_scene_type >= 0x12) || (g_menu_scene_type == 1) || (g_menu_scene_type == 2))
                        {
                            if ((g_menu_item_ptr != NULL) && (*((u8*)g_menu_item_ptr) != 0))
                            {
                                g_field_rename_script_index = FIELD_ITEM_CATEGORY(g_menu_item_ptr->info.word);
                                field_copy_name(g_field_rename_initial_name, (u8*)g_menu_item_ptr);
                                text_buffer[0] = 0;
                                g_field_rename_custom_name[0] = 0;
                                g_field_rename_target = (u8*)g_menu_item_ptr;

                                if (menu_item_differs_from_bare_hands(g_menu_item_ptr) != 0)
                                {
                                    text_cursor = text_buffer;
                                    label_table = menu_text_table_base(MENU_TEXT_ITEM_NAME_PARTS);
                                    field_copy_name(text_cursor,
                                                    (s8*)(label_table + (*((u16*)(label_table + (((g_menu_item_ptr->info.halves[1]) & 0x3F) * 2) + 0x48)))));
                                    item_word = g_menu_item_ptr->info.word;
                                    text_variant = (item_word >> 8) & 3;

                                    switch (text_variant)
                                    {
                                        case 0:
                                            name_table = (uintptr_t)menu_text_table_base(MENU_TEXT_ITEM_NAME_PARTS);
                                            menu_concat_encoded_text(g_field_rename_custom_name, text_buffer,
                                                                     (s8*)(name_table + (*((u16*)(((item_word >> 9) & 0x7E) + name_table)))));
                                            break;

                                        case 1:
                                            description_table = (uintptr_t)menu_text_table_base(MENU_TEXT_ITEM_NAME_PARTS);
                                            menu_concat_encoded_text(
                                                g_field_rename_custom_name, text_buffer,
                                                (s8*)(description_table + (*((u16*)(((item_word >> 9) & 0x7E) + description_table + 16 * sizeof(u16))))));
                                            break;

                                        default:
                                            detail_table = (uintptr_t)menu_text_table_base(MENU_TEXT_ITEM_NAME_PARTS);
                                            menu_concat_encoded_text(g_field_rename_custom_name, text_buffer,
                                                                     (s8*)(detail_table + (*((u16*)((((((u32)g_menu_item_ptr->info.word) >> 9) & 0x7E)) +
                                                                                                    detail_table + 32 * sizeof(u16))))));
                                            break;
                                    }

                                    g_menu_load_request = 1;
                                    g_field_rename_source = g_script_repeat_last;

                                    if (g_menu_scene_type == 1)
                                    {
                                        g_menu_transition_code = FIELD_MENU_NAME_EQUIPMENT_ITEM;
                                    }
                                    else if (g_menu_scene_type == 2)
                                    {
                                        g_menu_transition_code = FIELD_MENU_NAME_ABILITY_ITEM;
                                    }
                                    else
                                    {
                                        g_menu_transition_code = FIELD_MENU_NAME_ITEM;
                                    }
                                }
                            }
                        }

                        break;
                }
            }
            else if (item_type == MENU_CONTENT_ITEM_TYPE_ACTION)
            {
                action_group = g_menu_content_group_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                action_code = g_menu_content_action_codes[action_group][(item_packed_x >> 9) & 7];

                if (action_code != 0)
                {
                    menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);

                    switch (action_code)
                    {
                        case 1:
                        {
                            FieldPlayerRecord* player;
                            s32 idx;
                            s32 count;
                            u8* slot_base;

                            slot_base = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);

                            if (slot_base[MENU_CHARACTER_RECORD_OFFSET] != 0)
                            {
                                if ((slot_base[MENU_CHARACTER_INFO_OFFSET] & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GOLEM)
                                {
                                    g_field_rename_source = slot_base[MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)] + 0x80;
                                }
                                else
                                {
                                    g_field_rename_source = slot_base[MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)];
                                }

                                field_copy_name(g_field_rename_initial_name,
                                                (u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET));
                                count = 0;
                                idx = 0;
                                player = g_field_player_records;
                                g_field_rename_target =
                                    (u8*)((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET));
                                for (; idx < 3; idx++)
                                {
                                    if (idx == g_menu_char_slot)
                                    {
                                        break;
                                    }

                                    if (player->head.bytes.flags & FIELD_PLAYER_ACTIVE)
                                    {
                                        count += 1;
                                    }

                                    player++;
                                }

                                g_field_rename_script_index = count + 3;
                                g_menu_load_request = 1;
                                g_menu_transition_code = FIELD_MENU_NAME_CHARACTER;
                            }
                        }

                        break;

                        case 6:
                            g_saved_game_ctx->options.word |= MENU_OPTION_AUDIO_ENABLED;
                            akao_set_mono_output(0);
                            break;

                        case 7:
                            g_saved_game_ctx->options.word &= ~MENU_OPTION_AUDIO_ENABLED;
                            akao_set_mono_output(1);
                            break;

                        case 8:
                            g_saved_game_ctx->options.word |= MENU_OPTION_VIBRATION_ENABLED;
                            actuator_state->ports[0].small_motor_command = 1;
                            actuator_state->ports[0].large_motor_command = AKAO_SFX_DEFAULT_VOLUME;

                            if (g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word & MENU_PAD_INJECT_ENABLED)
                            {
                                actuator_state->ports[1].small_motor_command = 1;
                                actuator_state->ports[1].large_motor_command = AKAO_SFX_DEFAULT_VOLUME;
                            }

                            break;

                        case 9:
                            g_saved_game_ctx->options.word &= ~MENU_OPTION_VIBRATION_ENABLED;
                            actuator_state->ports[0].small_motor_command = 0;
                            actuator_state->ports[1].small_motor_command = 0;
                            break;

                        case 10:
                            if (actuator_state->ports[1].padding0[0] == MENU_NONE)
                            {
                                g_menu_message_line1 = menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_NO_CONTROLLER_LINE1);
                                g_menu_message_line2 = menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_NO_CONTROLLER_LINE2);
                                menu_clear_slots();
                                menu_open_content_page(MENU_PAGE_TWO_LINE_MESSAGE);
                            }
                            else
                            {
                                inject_flags = &g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word;
                                *inject_flags |= MENU_PAD_INJECT_ENABLED;
                                g_menu_companion_node = 0x2B;
                                menu_set_active_node();
                            }

                            break;

                        case 11:
                            inject_flags = &g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word;
                            *inject_flags &= ~MENU_PAD_INJECT_ENABLED;
                            g_menu_companion_node = MENU_NONE;
                            menu_set_active_node();
                            break;
                    }
                }
            }
        }
        else if (g_pad_input & PAD_BTN_CIRCLE)
        {
            menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);

            if ((u32)g_menu_scene_type >= 0x11U)
            {
                menu_reset_content_view();
            }
            else
            {
                node_self_index = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;

                if ((u8)node_self_index < 0x14)
                {
                    menu_reset_content_view();
                }
                else if ((u8)node_self_index >= 0x1C)
                {
                    menu_reset_content_view();
                }
                else
                {
                    if ((u32)(node_self_index - 0x14) < 5)
                    {
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 1;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = (g_menu_char_slot * 3) + 3;
                        menu_focus_active_content_item();
                    }
                    else if ((u32)(node_self_index - 0x1A) < 2)
                    {
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = 2;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = (g_menu_char_slot * 3) + 3;
                        menu_focus_active_content_item();
                    }
                }
            }
        }

        work_index = 0;
        content_type = PAD_BTN_UP;

        for (; work_index < MENU_CONTENT_DIRECTION_COUNT; work_index++)
        {
            if (g_pad_input & content_type)
            {
                break;
            }

            content_type <<= 1;
        }

        if (work_index != MENU_CONTENT_DIRECTION_COUNT)
        {
            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

            if (content_items[g_menu_hit_item_idx].params[work_index] == MENU_NONE)
            {
                if ((u32)g_menu_scene_type >= 0x11U || g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x14 ||
                    g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx >= 0x1C)
                {
                    s32* view_y_ptr = &g_content_view_y;
                    g_menu_cursor_mode = MENU_CURSOR_MODE_CONTENT_EXIT;
                    node_base = g_menu_nodes;
                    selected_node = &node_base[g_menu_active_node];
                    node_nav_y_low_bit = selected_node->idx_nav.nav_x_packed >> 15;
                    node_nav_y = (selected_node->u8_u.s.nav_y_hi << 1) | node_nav_y_low_bit;
                    scroll_extent = g_menu_content_height - MENU_CURSOR_Y_MIN;

                    *view_y_ptr = node_nav_y - scroll_extent;
                    if (*view_y_ptr < MENU_CURSOR_Y_MIN)
                    {
                        *view_y_ptr = MENU_CURSOR_Y_MIN;
                    }

                    if (*view_y_ptr >= MENU_CURSOR_Y_MAX)
                    {
                        *view_y_ptr = MENU_CURSOR_Y_MAX;
                    }

                    g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                    g_content_view_x = menu_nav_x(selected_node->idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
                }
            }
            else if (content_items[g_menu_hit_item_idx].params[work_index] != 0)
            {
                if (g_menu_scene_type < 0x11)
                {
                    if ((content_items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_ITEM_TYPE_MASK) == MENU_CONTENT_ITEM_TYPE_SUBMENU)
                    {
                        if (g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x11)
                        {
                            g_menu_item_ptr = NULL;
                            g_menu_category0_item = NULL;
                            g_menu_category2_item = NULL;
                            g_menu_category1_item = NULL;
                        }
                    }
                }

                g_menu_hit_item_idx = content_items[g_menu_hit_item_idx].params[work_index];
                g_content_view_x = MENU_CONTENT_X(&content_items[g_menu_hit_item_idx]);
                g_content_view_y = MENU_CONTENT_Y(&content_items[g_menu_hit_item_idx]);
                g_menu_suppress_cursor = 3;
            }
        }
    }
}

/**
 * @brief Clamp the content cursor and snap the viewport to its position.
 */
void menu_snap_view_to_cursor(void)
{
    if (g_content_cursor_y < MENU_CURSOR_Y_MIN)
    {
        g_content_cursor_y = MENU_CURSOR_Y_MIN;
    }

    if (g_content_cursor_y >= MENU_CURSOR_Y_MAX)
    {
        g_content_cursor_y = MENU_CURSOR_Y_MAX;
    }

    g_content_view_y = g_content_cursor_y;

    /* Restore X from the active node's packed navigation column. */
    g_content_cursor_x = menu_nav_x(g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
    g_content_view_x = g_content_cursor_x;
}

/**
 * @brief Return non-zero if the item currently under the cursor has a confirm action.
 * @return 1 if the item will trigger a sub-menu or named action on confirm, 0 otherwise.
 */
s32 menu_item_has_action(void)
{
    MenuContentItem* items;
    MenuContentItem* item;
    u16 item_type;
    s16 item_subtype;
    s32 action_code;

    items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];

    if ((g_menu_scene_type == MENU_SCENE_BUTTONS_CHARACTER_0) || (g_menu_scene_type == MENU_SCENE_BUTTONS_CHARACTER_1))
    {
        if (g_menu_hit_item_idx >= MENU_BUTTON_ITEM_FIRST)
        {
            if (g_menu_hit_item_idx < MENU_BUTTON_ITEM_FIRST + MENU_BUTTON_COUNT)
            {
                return 1;
            }
        }
    }

    item = (MenuContentItem*)(g_menu_hit_item_idx * sizeof(*items) + (uintptr_t)items);
    item_type = item->packed_x & MENU_CONTENT_ITEM_TYPE_MASK;

    if (item_type == MENU_CONTENT_ITEM_TYPE_SUBMENU)
    {
        g_menu_active_subtype = item->action_type;
        item_subtype = item->action_type;

        if (item_subtype != 0)
        {
            if (item_subtype >= 11)
            {
                if (item_subtype == 15)
                {
                    goto success;
                }

                return 0;
            }
            else
            {
                return g_menu_char_slot == 0;
            }
        }
    }
    else if (item_type == MENU_CONTENT_ITEM_TYPE_ACTION)
    {
        action_code = g_menu_content_action_codes[g_menu_content_group_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx]][(item->packed_x >> 9) & 7];

        if (action_code != 0)
        {
            if (action_code == 1)
            {
                return 1;
            }
            else if ((action_code != 0) && (action_code < 12) && (action_code >= 6))
            {
            success:
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Exit content focus and restore the node-tree viewport to the active node.
 */
void menu_reset_content_view(void)
{
    MenuNode* active_node;
    s32 node_y;
    s32 node_y_low_bit;
    s32 scroll_offset;
    s32* view_y_ptr;

    if (g_menu_nodes[g_menu_scene_type].content_id != MENU_NONE)
    {
        g_content_cursor_x = g_menu_default_view_pos.x;
        g_content_cursor_y = g_menu_default_view_pos.y;
    }

    g_menu_cursor_mode = MENU_CURSOR_MODE_CONTENT_EXIT;
    view_y_ptr = &g_content_view_y;
    active_node = &g_menu_nodes[g_menu_active_node];
    node_y_low_bit = active_node->idx_nav.nav_x_packed >> 15;
    node_y = (active_node->u8_u.s.nav_y_hi << 1) | node_y_low_bit;

    /* Convert the node's layout position into the scrolled viewport. */
    scroll_offset = g_menu_content_height - MENU_CURSOR_Y_MIN;

    *view_y_ptr = node_y - scroll_offset;
    if (*view_y_ptr < MENU_CURSOR_Y_MIN)
    {
        *view_y_ptr = MENU_CURSOR_Y_MIN;
    }

    if (*view_y_ptr >= MENU_CURSOR_Y_MAX)
    {
        *view_y_ptr = MENU_CURSOR_Y_MAX;
    }

    g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
    g_content_view_x = menu_nav_x(active_node->idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
}

/**
 * @brief Initialize packed positions and circular links for item navigation.
 * @param count Number of entries to initialize (no-op if <= 0).
 */
void menu_init_item_nav_entries(s32 count)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 entry_index;
    s32 wrapped_next_index;
    s32* entry;
    s32 packed_entry;
    s32 entry_with_previous;

    for (entry_index = 0; entry_index < count; entry_index++)
    {
        entry = entry_index + g_menu_item_nav_entries;
        packed_entry = *entry;
        previous_index = entry_index - 1;

        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;

        next_index = entry_index + 1;
        wrapped_next_index = 0;

        if (next_index < count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
}

/**
 * @brief Build circular navigation entries for the learned abilities.
 * @return Number of learned abilities.
 */
s32 menu_build_ability_nav_entries(void)
{
    s32 learned_bits;
    s32 next_index;
    u32 entry_with_position;
    s32 index;
    s32 previous_index;
    s32 row;
    s32 item_count;
    s32 bit_mask;
    s32 wrapped_next_index;
    u32* entry;
    u8* learned_bytes;
    u32 packed_entry;
    u32 entry_with_previous;

    item_count = 0;
    learned_bytes = (u8*)g_saved_game_ctx->ability_bits;

    for (row = MENU_ABILITY_BYTE_COUNT - 1; row >= 0; row--)
    {
        bit_mask = 1;
        learned_bits = *learned_bytes;

        for (index = MENU_ABILITIES_PER_BYTE - 1; index >= 0; index--)
        {
            if (learned_bits & bit_mask)
            {
                item_count++;
            }

            bit_mask <<= 1;
        }

        learned_bytes++;
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (index = 0; index < item_count; index++)
    {
        entry = g_menu_scroll_nav_entries + index;
        packed_entry = *entry;
        previous_index = index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;

        next_index = index + 1;
        wrapped_next_index = 0;

        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }

    return item_count;
}

/**
 * @brief Build navigation entries for the learned Special Technique list.
 * @return Item count in the low halfword; initial selected row in the high halfword.
 */
s32 menu_build_special_technique_nav_entries(void)
{
    s32 group_index;
    s32 index;
    s32 previous_index;
    s32 next_index;
    s32 item_count;
    s32 wrapped_next_index;
    s32 entry_with_previous;
    s32 selected_index;
    s32 none_index;
    s32 bit_mask;
    s32 group_flags;
    s32* group_flags_ptr;

    item_count = 0;
    selected_index = MENU_NONE;
    group_index = 0;
    none_index = selected_index;
    group_flags_ptr = (s32*)g_saved_game_ctx->technique_bits;

    while (group_index < FIELD_WEAPON_CATEGORY_COUNT)
    {
        bit_mask = 1;
        group_flags = *group_flags_ptr;
        index = FIELD_TECHNIQUES_PER_WEAPON - 1;

        while (index >= 0)
        {
            if ((group_flags & bit_mask) != 0)
            {
                if ((group_index == (s32)FIELD_ITEM_TYPE(g_menu_equipment_base->info.word)) && (selected_index == none_index))
                {
                    selected_index = item_count;
                }

                item_count += 1;
            }

            index -= 1;
            bit_mask <<= 1;
        }

        group_index += 1;
        group_flags_ptr += 1;
    }

    if (selected_index == MENU_NONE)
    {
        selected_index = 0;
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (index = 0; index < item_count; index++)
    {
        s32* nav_entry = (s32*)g_menu_scroll_nav_entries + index;
        s32 previous_entry = *nav_entry;
        s32 entry_with_position;

        previous_index = index - 1;
        entry_with_position = previous_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position |= (index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK;

        *nav_entry = entry_with_position;
        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *nav_entry = entry_with_previous;
        next_index = index + 1;
        wrapped_next_index = 0;

        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }

        *nav_entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }

    return item_count | (selected_index << 16);
}

/**
 * @brief Build circular navigation entries for inventory items of one kind.
 * @param item_kind Item kind to match against FieldItemRecord::attributes.
 * @return Number of matching inventory items.
 */
s32 menu_build_inventory_nav_entries(s32 item_kind)
{
    s32 entry_index;
    s32 previous_index;
    s32 next_index;
    s32 item_count;
    s32 wrapped_next_index;
    s32 entry_with_previous;
    FieldItemRecord* item;

    entry_index = 0;
    item_count = 0;
    item = (FieldItemRecord*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET);

    for (; entry_index < MENU_ITEM_TABLE_COUNT; entry_index++, item++)
    {
        u8 active = item->name[0];

        if (active == 0)
        {
            break;
        }

        if (FIELD_ITEM_CATEGORY(item->info.word) == (u32)item_kind)
        {
            item_count += 1;
        }
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (entry_index = 0; entry_index < item_count; entry_index++)
    {
        s32* nav_entry = (s32*)g_menu_scroll_nav_entries + entry_index;
        s32 previous_entry = *nav_entry;
        s32 entry_with_position;

        previous_index = entry_index - 1;
        entry_with_position = previous_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position |= (entry_index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK;

        *nav_entry = entry_with_position;
        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *nav_entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;

        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }

        *nav_entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }

    return item_count;
}

/**
 * @brief Return a golem record from the shared context.
 * @param context Context containing the golem records.
 * @param index Golem record index.
 * @return Selected golem record.
 */
static inline GolemRecord* menu_golem_record(SavedGameLayout* context, s32 index)
{
    u8* records = (u8*)context->golem_records;
    return (GolemRecord*)(records + index * sizeof(GolemRecord));
}

/**
 * @brief Read one packed value from the active character's menu data.
 * @param context Shared context containing the character records.
 * @param index Value index, 0 through 7.
 * @return Packed value; the character panel displays its high seven bits.
 */
static inline u16 menu_active_character_value(SavedGameLayout* context, s32 index)
{
    s32 offset = index * sizeof(u16);
    offset += g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
    return *(u16*)((u8*)context + offset + MENU_CHARACTER_PACKED_VALUES_OFFSET);
}

/**
 * @brief Return the active character's record in the shared context.
 * @param context Context containing the character records.
 * @return Active character record.
 */
static inline FieldCharacterRecord* menu_active_character(SavedGameLayout* context)
{
    s32 offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
    return (FieldCharacterRecord*)((u8*)context + offset + MENU_CHARACTER_RECORD_OFFSET);
}

/**
 * @brief Draw a text string at a content item's position in its style.
 * @param packet_cursor GPU packet cursor.
 * @param ot Ordering-table entry.
 * @param text Encoded string to draw.
 * @param text_color Field text color index.
 * @param item Content item giving the position and style.
 * @return Updated GPU packet cursor.
 */
static inline void* menu_draw_content_text(void* packet_cursor, u_long* ot, u8* text, s32 text_color, MenuContentItem* item)
{
    return field_draw_text(packet_cursor, ot, text, text_color, MENU_CONTENT_X(item), MENU_CONTENT_Y(item), MENU_CONTENT_ALIGN(item));
}

/**
 * @brief Draw the active scene's content entries.
 * @param packet_cursor GPU packet cursor advanced as primitives are emitted.
 * @param ot_entry Ordering-table entry passed to render helpers.
 * @return Updated GPU packet cursor after drawing.
 * @note JP tests the category 0 item's effect flags for actions 0x22-0x29 without
 *       first checking that the item exists.
 */
void* menu_draw_scene_content(void* packet_cursor, u_long* ot_entry)
{
    u8 text_buffer[0x40];
    u8 prefix_buffer[0x40];
    Vec2s pos;
    u8 label_buffer[16];
    s32 remaining_count;
    MenuContentItem* content_item;
    void* draw_packet_cursor;
    uintptr_t technique_names;
    s32 k;
    u32 shared_s0;
    intptr_t content_index;
    s32 two_outer = 2;
    s32 val;
    s32 zero = 0;

    if (g_menu_scene_type == -1)
    {
        content_item = &g_menu_default_content_items;
        remaining_count = g_menu_default_content_count_minus_one + 1;
    }
    else
    {
        u8 idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
        content_item = g_menu_content_table[idx];
        remaining_count = g_menu_content_item_counts[g_menu_content_group_ids[idx]];
    }

    if (content_item != NULL)
    {
        if (content_item == (MenuContentItem*)1)
        {
            g_menu_load_request = 1;
        }
        else
        {
            u8* character_equipment_area;
            FieldItemRecord* equipped_items;
            MenuNode* nodes;
            MenuNode* node;

            g_menu_equipment_base =
                (equipped_items = (FieldItemRecord*)((character_equipment_area = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE +
                                                                                                          MENU_CHARACTER_RECORD_OFFSET)) +
                                                     MENU_CHARACTER_ITEMS_OFFSET));
            g_menu_shown_equipment = equipped_items;
            g_menu_shown_armor = equipped_items;
            g_menu_shown_equipment_copy = equipped_items;

            if (g_menu_scene_type == 0x10)
            {
                FieldItemRecord* second_equipped_item;
                g_menu_category0_item = equipped_items;
                g_menu_category1_item = second_equipped_item = &equipped_items[1];
                g_menu_category2_item = second_equipped_item;
            }

            while (remaining_count != 0)
            {
                s32 icon_id;
                u16 item_word;
                s32 item_type;

                pos.x = (s16)MENU_CONTENT_X(content_item);
                pos.y = (s16)MENU_CONTENT_Y(content_item);
                icon_id = 0xFF;
                item_word = content_item->packed_x;
                item_type = item_word >> MENU_CONTENT_TYPE_SHIFT;

                switch (item_type)
                {
                    case 1:
                        packet_cursor = menu_draw_content_text(packet_cursor, ot_entry,
                                                               menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), content_item->action_type),
                                                               FIELD_TEXT_COLOR_MENU, content_item);
                        break;

                    case 2:
                    {
                        s32 icon_selector;
                        u8 action_type;

                        if (content_item->action_type < 2)
                        {
                            icon_id = 0xFF;

                            if (two_outer == 2)
                            {
                                icon_id = 0x1E;
                            }
                        }

                        action_type = content_item->action_type;
                        icon_selector = action_type;

                        if (icon_selector >= 0x02 && icon_selector <= 0x09)
                        {
                            s32 action;
                            val = 0;

                            if (*(u8*)g_menu_category1_item != 0 && g_menu_category1_item != 0)
                            {
                                val = g_menu_category1_item->status_flags;
                            }

                            action = content_item->action_type;

                            if (((val >> (action - 2)) & 1) != 0)
                            {
                                icon_id = action + 0x3B;
                            }
                            else
                            {
                                icon_id = 0x21;
                            }
                        }
                        else if (icon_selector >= 0x0A && icon_selector <= 0x11)
                        {
                            s32 action;
                            val = 0;

                            if (*(u8*)g_menu_category1_item != 0 && g_menu_category1_item != 0)
                            {
                                val = g_menu_category1_item->element_flags;
                            }

                            icon_id = 0x21;
                            action = content_item->action_type;

                            if (((val >> (action - 0xA)) & 1) != 0)
                            {
                                icon_id = action + 0x2B;
                            }
                        }
                        else if (icon_selector >= 0x12 && icon_selector <= 0x19)
                        {
                            s32 action;
                            val = 0;

                            if (g_menu_shown_armor[1].name[0] != 0)
                            {
                                val = g_menu_shown_armor[1].status_flags;
                            }

                            if (g_menu_shown_armor[2].name[0] != 0)
                            {
                                val |= g_menu_shown_armor[2].status_flags;
                            }

                            if (g_menu_shown_armor[3].name[0] != 0)
                            {
                                val |= g_menu_shown_armor[3].status_flags;
                            }

                            icon_id = 0x21;
                            action = content_item->action_type;

                            if (((val >> (action - 0x12)) & 1) != 0)
                            {
                                icon_id = action + 0x2B;
                            }
                        }
                        else if (icon_selector >= 0x1A && icon_selector <= 0x21)
                        {
                            s32 action;
                            val = 0;

                            if (g_menu_shown_armor[1].name[0] != 0)
                            {
                                val = g_menu_shown_armor[1].element_flags;
                            }

                            if (g_menu_shown_armor[2].name[0] != 0)
                            {
                                val |= g_menu_shown_armor[2].element_flags;
                            }

                            if (g_menu_shown_armor[3].name[0] != 0)
                            {
                                val |= g_menu_shown_armor[3].element_flags;
                            }

                            action = content_item->action_type;

                            if (((val >> (action - 0x1A)) & 1) != 0)
                            {
                                icon_id = g_menu_element_icon_ids[action];
                            }
                            else
                            {
                                icon_id = 0x21;
                            }
                        }
                        else if (icon_selector >= 0x22 && icon_selector <= 0x29)
                        {
#if defined(VERSION_JP)
                            s32 action;
                            action = content_item->action_type;

                            if (((g_menu_category0_item->status_flags >> (action - 0x22)) & 1) != 0)
                            {
                                icon_id = action + 0x13;
                            }
                            else
                            {
                                icon_id = 0x21;
                            }
#else
                            icon_id = 0x21;

                            if (g_menu_category0_item != 0)
                            {
                                s32 action;
                                action = content_item->action_type;

                                if (((g_menu_category0_item->status_flags >> (action - 0x22)) & 1) != 0)
                                {
                                    icon_id = action + 0x13;
                                }
                            }
#endif
                        }
                        else if (icon_selector >= 0x2A && icon_selector <= 0x31)
                        {
                            s32 button;
                            s32 action;
                            action = content_item->action_type;

                            for (button = 0; button < MENU_BUTTON_COUNT; button++)
                            {
                                u8* saved_game = (u8*)g_saved_game_ctx;
                                s32 character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;

                                if (*(u8*)(saved_game + character_offset + button + MENU_CHARACTER_FIELD_OFFSET(button_actions)) == action - 0x2A)
                                {
                                    break;
                                }
                            }

                            icon_id = g_menu_button_icon_ids[button];
                        }
                        else if (icon_selector >= 0x32 && icon_selector <= 0x35)
                        {
                            if (g_menu_char_slot != 2)
                            {
                                FieldItemRecord* equipment = (FieldItemRecord*)((content_item->action_type * sizeof(FieldItemRecord)) +
                                                                                (uintptr_t)g_menu_equipment_base - 0x32 * sizeof(FieldItemRecord));
                                icon_id = 0x21;

                                if (equipment->name[0] != 0)
                                {
                                    u32 attributes = equipment->info.word;
                                    s32 item_kind = FIELD_ITEM_CATEGORY(attributes);

                                    switch (item_kind)
                                    {
                                        case 1:
                                            icon_id = FIELD_ITEM_TYPE(attributes) + 0x50;
                                            break;
                                        case 2:
                                            icon_id = FIELD_ITEM_TYPE(attributes) + 0x5C;
                                            break;
                                        case 0:
                                            icon_id = FIELD_ITEM_TYPE(attributes) + 0x45;
                                            break;
                                    }
                                }
                            }
                        }
                        else if ((u8)icon_selector == 0x36)
                        {
                            icon_id = 0xFF;

                            if (g_menu_item_ptr != 0)
                            {
                                if (g_menu_item_ptr->name[0] != 0)
                                {
                                    u32 attributes = g_menu_item_ptr->info.word;
                                    s32 item_kind = FIELD_ITEM_CATEGORY(attributes);

                                    switch (item_kind)
                                    {
                                        case 0:
                                            icon_id = FIELD_ITEM_TYPE(attributes) + 0x45;
                                            break;
                                        case 1:
                                            icon_id = FIELD_ITEM_TYPE(attributes) + 0x50;
                                            break;
                                        case 2:
                                            icon_id = FIELD_ITEM_TYPE(attributes) + 0x5C;
                                            break;
                                    }
                                }
                            }
                        }
                        else if (icon_selector >= 0x37 && icon_selector <= 0x54)
                        {
                            icon_id = g_menu_action_icon_ids[content_item->action_type];
                        }
                        else if ((u8)icon_selector == 0x55)
                        {
                            icon_id = 0xFF;

                            if (((g_saved_game_ctx->options.word >> 1) & 1) != 0)
                            {
                                icon_id = 0x6A;
                            }
                        }
                        else
                        {
                            icon_selector = content_item->action_type;

                            if (icon_selector == 0x56)
                            {
                                icon_id = 0x6A;

                                if (((g_saved_game_ctx->options.word >> 1) & 1) != 0)
                                {
                                    icon_id = 0xFF;
                                }
                            }
                            else if (icon_selector == 0x57)
                            {
                                icon_id = 0xFF;

                                if ((g_saved_game_ctx->options.word & 1) != 0)
                                {
                                    icon_id = 0x6A;
                                }
                            }
                            else if (icon_selector == 0x58)
                            {
                                icon_id = 0x6A;

                                if ((g_saved_game_ctx->options.word & 1) != 0)
                                {
                                    icon_id = 0xFF;
                                }
                            }
                            else if (icon_selector == 0x59)
                            {
                                icon_id = 0xFF;

                                if ((g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word & 0x80) != 0)
                                {
                                    icon_id = 0x6A;
                                }
                            }
                            else if (icon_selector == 0x5A)
                            {
                                icon_id = 0x6A;

                                if ((g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word & 0x80) != 0)
                                {
                                    icon_id = 0xFF;
                                }
                            }
                            else
                            {
                                icon_id = 0xFF;
                            }
                        }

                        if (icon_id != 0xFF)
                        {
                            packet_cursor = menu_emit_draw_mode_primitive(
                                menu_emit_icon_sprite(packet_cursor, ot_entry, icon_id, MENU_CONTENT_X(content_item), MENU_CONTENT_Y(content_item), 0, 0, 0, 0),
                                ot_entry);
                        }
                    }

                    break;

                    case 3:
                    {
                        content_index = content_item->action_type;

                        switch (content_index)
                        {
                            case 0x1:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry, (u8*)g_menu_item_ptr, FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x2:
                                if (g_menu_item_ptr != 0)
                                {
                                    u32 v;
                                    u32 v1;

                                    if (menu_item_differs_from_bare_hands(g_menu_item_ptr) != 0)
                                    {
                                        void* a3 = (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_NAMES]);
                                        menu_concat_encoded_text(
                                            prefix_buffer, (void*)((u8*)a3 + *(u16*)((u8*)a3 + (g_menu_item_ptr->info.halves[1] & 0x3F) * 2)),
                                            (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES] +
                                                    *(u16*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES] +
                                                            0xB4)));
                                    }
                                    else
                                    {
                                        prefix_buffer[0] = 0;
                                    }

                                    v = g_menu_item_ptr->info.word;
                                    v1 = (v >> 8) & 3;

                                    switch (v1)
                                    {
                                        case 0:
                                        {
                                            uintptr_t base0 = ((uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES));
                                            menu_concat_encoded_text(text_buffer, prefix_buffer, (void*)(base0 + *(u16*)(((v >> 9) & 0x7E) + base0)));
                                            break;
                                        }
                                        case 1:
                                        {
                                            uintptr_t base1 = ((uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES));
                                            menu_concat_encoded_text(text_buffer, prefix_buffer,
                                                                     (void*)(base1 + *(u16*)(((v >> 9) & 0x7E) + base1 + 11 * sizeof(u16))));
                                            break;
                                        }
                                        default:
                                        {
                                            uintptr_t base2 = ((uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES));
                                            menu_concat_encoded_text(
                                                text_buffer, prefix_buffer,
                                                (void*)(base2 + *(u16*)(((g_menu_item_ptr->info.word >> 9) & 0x7E) + base2 + 23 * sizeof(u16))));
                                            break;
                                        }
                                    }

                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry, (u8*)(&text_buffer), FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x3:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, g_menu_item_ptr->bonus_nibbles.word & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x4:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (g_menu_item_ptr->bonus_nibbles.word >> 4) & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x5:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (g_menu_item_ptr->bonus_nibbles.word >> 8) & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x6:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (g_menu_item_ptr->bonus_nibbles.word >> 12) & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x7:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, g_menu_item_ptr->bonus_nibbles.halves[1] & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x8:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (g_menu_item_ptr->bonus_nibbles.word >> 20) & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x9:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, g_menu_item_ptr->bonus_nibbles.bytes[3] & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0xA:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (g_menu_item_ptr->bonus_nibbles.word >> 28) & 0xF,
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0xB:
                            case 0xC:
                            case 0xD:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_content_text(
                                        packet_cursor, ot_entry,
                                        menu_text_entry(menu_text_table_base(MENU_TEXT_ITEM_SPECIAL_NAMES), g_menu_item_ptr->special_ids[content_index - 0xB]),
                                        FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0xE:
                            case 0xF:
                            case 0x10:
                                if (g_menu_category0_item != 0)
                                {
                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry,
                                                                           menu_text_entry(menu_text_table_base(MENU_TEXT_WEAPON_ATTACK_NAMES),
                                                                                           g_menu_category0_item->derived.weapon.stats[content_index - 0xC]),
                                                                           FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x11:
                                if (g_menu_category0_item != 0)
                                {
                                    packet_cursor = field_draw_number(ot_entry, packet_cursor, g_menu_category0_item->derived.values[0], FIELD_TEXT_COLOR_MENU,
                                                                      &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x12:
                                if (g_menu_category1_item != 0)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, g_menu_category1_item->derived.values[0],
                                                                             FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x13:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_content_text(
                                        packet_cursor, ot_entry,
                                        menu_text_entry(menu_text_table_base(MENU_TEXT_MAGIC_NAMES), g_menu_category2_item->derived.instrument.spirit * 14 +
                                                                                                         g_menu_category2_item->derived.instrument.spell),
                                        FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x14:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_content_text(
                                        packet_cursor, ot_entry,
                                        menu_text_entry(menu_text_table_base(MENU_TEXT_SPIRIT_NAMES), g_menu_category2_item->derived.instrument.spirit),
                                        FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x15:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = menu_draw_content_text(
                                        packet_cursor, ot_entry,
                                        menu_text_entry(menu_text_table_base(MENU_TEXT_MAGIC_SHAPE_NAMES), g_menu_category2_item->derived.instrument.spell),
                                        FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x16:
                                if (g_menu_item_ptr != 0)
                                {
                                    packet_cursor = field_draw_number(ot_entry, packet_cursor, g_menu_category2_item->derived.instrument.power,
                                                                      FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x17:
                            case 0x18:
                            case 0x19:
                            case 0x1A:
                            {
                                val = 0;

                                if (g_item_slot_flags[content_index - 0x17] != 0)
                                {
                                    FieldItemRecord* equipment = (FieldItemRecord*)((content_index * sizeof(FieldItemRecord)) +
                                                                                    (uintptr_t)g_menu_shown_equipment - 0x17 * sizeof(FieldItemRecord));
                                    FieldItemRecord* comparison_item;

                                    if (equipment->name[0] != 0)
                                    {
                                        val = equipment->derived.values[0];
                                    }

                                    comparison_item = g_item_slot_data[content_index - 0x17];

                                    if (comparison_item != 0 && comparison_item->name[0] != 0)
                                    {
                                        val -= comparison_item->derived.values[0];
                                    }
                                    else
                                    {
                                        val -= g_field_bare_hands_item.derived.weapon.power;
                                    }

                                    packet_cursor =
                                        field_draw_number(ot_entry, packet_cursor, abs(val), FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            /* fallthrough */
                            case 0x1B:
                            case 0x1C:
                            case 0x1D:
                            case 0x1E:
                            {
                                val = 0;

                                if (g_item_slot_flags[content_index - 0x1B] != 0)
                                {
                                    FieldItemRecord* equipment = (FieldItemRecord*)((content_index * sizeof(FieldItemRecord)) +
                                                                                    (uintptr_t)g_menu_shown_equipment - 0x1B * sizeof(FieldItemRecord));
                                    FieldItemRecord* comparison_item;

                                    if (equipment->name[0] != 0)
                                    {
                                        val = equipment->derived.values[0];
                                    }

                                    comparison_item = g_item_slot_data[content_index - 0x1B];

                                    if (comparison_item != 0 && comparison_item->name[0] != 0)
                                    {
                                        val -= comparison_item->derived.values[0];
                                    }
                                    else
                                    {
                                        val -= g_field_bare_hands_item.derived.weapon.power;
                                    }

                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, (u8*)(&label_buffer), FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }
                            }

                            break;
                            case 0x1F:
                            case 0x20:
                            case 0x21:
                            case 0x22:
                            case 0x23:
                            case 0x24:
                            case 0x25:
                            case 0x26:
                                if (g_menu_item_ptr != 0)
                                {
                                    s32 res = menu_lookup_item_nibble(g_menu_item_ptr, content_index - 0x1F);
                                    menu_copy_sign_label(label_buffer, res);
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, (u8*)(&label_buffer), FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }

                                break;
                            case 0x27:
                            case 0x28:
                            case 0x29:
                            case 0x2A:
                            case 0x2B:
                            case 0x2C:
                            case 0x2D:
                            case 0x2E:
                                if (g_menu_item_ptr != 0)
                                {
                                    s32 diff = menu_lookup_item_nibble(g_menu_item_ptr, content_index - 0x27);
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (u32)abs(diff), FIELD_TEXT_COLOR_MENU, &pos,
                                                                             MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x2F:
                            case 0x30:
                            case 0x31:
                            case 0x32:
                            case 0x33:
                            case 0x34:
                            case 0x35:
                            case 0x36:
                            {
                                s32 idx2 = content_index - 0x2F;

                                if (g_menu_item_ptr != 0)
                                {
                                    s32 new_value = menu_lookup_item_nibble(g_menu_item_ptr, idx2);
                                    s32 old_value = menu_lookup_item_nibble((void*)g_menu_active_equipped_item, idx2);
                                    s32 diff = new_value - old_value;
                                    menu_copy_sign_label(label_buffer, diff);
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, (u8*)(&label_buffer), FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }
                            }

                            break;
                            case 0x37:
                            case 0x38:
                            case 0x39:
                            case 0x3A:
                            case 0x3B:
                            case 0x3C:
                            case 0x3D:
                            case 0x3E:
                            {
                                s32 idx2 = content_index - 0x2F;

                                if (g_menu_item_ptr != 0)
                                {
                                    s32 new_value = menu_lookup_item_nibble(g_menu_item_ptr, idx2);
                                    s32 old_value = menu_lookup_item_nibble((void*)g_menu_active_equipped_item, idx2);
                                    s32 diff = new_value - old_value;
                                    packet_cursor =
                                        field_draw_number(ot_entry, packet_cursor, abs(diff), FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            case 0x3F:
                            case 0x40:
                            case 0x41:
                            case 0x42:
                                if (g_menu_category1_item != 0)
                                {
                                    packet_cursor =
                                        menu_draw_clamped_number(ot_entry, packet_cursor, g_menu_category1_item->derived.values[content_index - 0x3F],
                                                                 FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 0x43:
                            case 0x44:
                            case 0x45:
                            case 0x46:
                            {
                                uintptr_t has = 0;

                                val = has;

                                for (k = 1; k < 4; k++)
                                {
                                    FieldItemRecord* slot = &g_menu_shown_armor[k];

                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (slot->name[0] != 0)
                                        {
                                            val += slot->derived.values[content_index - 0x43];
                                        }

                                        has = (uintptr_t)g_item_slot_data[k];

                                        if (has != 0 && ((FieldItemRecord*)has)->name[0] != 0)
                                        {
                                            val -= ((FieldItemRecord*)has)->derived.values[content_index - 0x43];
                                        }

                                        has = 1;
                                    }
                                }

                                if (has)
                                {
                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, (u8*)(&label_buffer), FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }
                            }

                            break;
                            case 0x47:
                            case 0x48:
                            case 0x49:
                            case 0x4A:
                            {
                                uintptr_t has = 0;
                                s32 total = has;

                                for (k = 1; k < 4; k++)
                                {
                                    FieldItemRecord* slot = &g_menu_shown_armor[k];

                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (slot->name[0] != 0)
                                        {
                                            total += slot->derived.values[content_index - 0x47];
                                        }

                                        has = (uintptr_t)g_item_slot_data[k];

                                        if (has != 0 && ((FieldItemRecord*)has)->name[0] != 0)
                                        {
                                            total -= ((FieldItemRecord*)has)->derived.values[content_index - 0x47];
                                        }

                                        has = 1;
                                    }
                                }

                                if (has)
                                {
                                    packet_cursor =
                                        field_draw_number(ot_entry, packet_cursor, abs(total), FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            default:
                                break;
                        }
                    }

                    break;

                    case 4:
                    {
                        content_index = content_item->action_type;

                        switch (content_index)
                        {
                            case 0x1:
                            {
                                u8 tex_u;
                                u8 tex_v;
                                SET_BGR0_PACKED(packet_cursor, GPU_TINT_NEUTRAL);
                                setSprt(packet_cursor);
                                setXY0((SPRT*)packet_cursor, MENU_CONTENT_X(content_item), MENU_CONTENT_Y(content_item));
                                tex_u = 0xD0;

                                if (g_menu_char_slot == 2)
                                {
                                    tex_u = 0xA0;
                                }

                                ((SPRT*)packet_cursor)->u0 = tex_u;
                                tex_v = 0x50;

                                if (g_menu_char_slot == 0)
                                {
                                    tex_v = 0x20;
                                }

                                ((SPRT*)packet_cursor)->v0 = tex_v;
                                SET_SPRT_WH_PACKED(packet_cursor, 0x30, 0x30);
                                SET_SPRT_CLUT(packet_cursor, (((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                                addPrim(ot_entry, packet_cursor);
                                packet_cursor = (SPRT*)packet_cursor + 1;
                                SET_BGR0_PACKED(packet_cursor, 0);
                                setSprt(packet_cursor);
                                setSemiTrans(packet_cursor, 1);
                                setXY0((SPRT*)packet_cursor, (MENU_CONTENT_X(content_item)) + 2, content_item->y - 6);
                                tex_u = 0xD0;

                                if (g_menu_char_slot == 2)
                                {
                                    tex_u = 0xA0;
                                }

                                ((SPRT*)packet_cursor)->u0 = tex_u;
                                tex_v = 0x50;

                                if (g_menu_char_slot == 0)
                                {
                                    tex_v = 0x20;
                                }

                                ((SPRT*)packet_cursor)->v0 = tex_v;
                                SET_SPRT_WH_PACKED(packet_cursor, 0x30, 0x30);
                                SET_SPRT_CLUT(packet_cursor, (((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                                addPrim(ot_entry, packet_cursor);
                                packet_cursor = (SPRT*)packet_cursor + 1;
                                setDrawTPage(packet_cursor, 0, 0, FIELD_UI_TPAGE(GPU_BLEND_HALF));
                                addPrim(ot_entry, packet_cursor);
                                packet_cursor = (DR_TPAGE*)packet_cursor + 1;
                            }

                            break;
                            case 0x2:
                            {
                                u8* saved_game = (u8*)g_saved_game_ctx;
                                s32 off = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE + MENU_CHARACTER_RECORD_OFFSET;
                                packet_cursor = field_draw_text(packet_cursor, ot_entry, saved_game + off, FIELD_TEXT_COLOR_MENU, MENU_CONTENT_X(content_item),
                                                                MENU_CONTENT_Y(content_item), MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x3:
                            {
                                SavedGameLayout* context = g_saved_game_ctx;
                                u8 v = menu_active_character(context)->progress.level;
                                packet_cursor =
                                    menu_draw_clamped_number(ot_entry, packet_cursor, v, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x4:
                            {
                                packet_cursor = field_draw_number(ot_entry, packet_cursor, g_field_object_states[g_menu_char_slot].current_hp.word,
                                                                  FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x5:
                            {
                                SavedGameLayout* context = g_saved_game_ctx;
                                u16 v = menu_active_character(context)->hp;
                                packet_cursor = field_draw_number(ot_entry, packet_cursor, v, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x6:
                            {
                                SavedGameLayout* context = g_saved_game_ctx;
                                u32 v = menu_active_character(context)->progress.word;
                                packet_cursor =
                                    field_draw_number(ot_entry, packet_cursor, (v >> 8), FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x7:
                            case 0x8:
                            case 0x9:
                            case 0xA:
                            case 0xB:
                            case 0xC:
                            case 0xD:
                            case 0xE:
                            {
                                void* base = (void*)g_saved_game_ctx;
                                s32 idx = content_index - 7;
                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, menu_active_character_value((SavedGameLayout*)base, idx) >> 9,
                                                                         FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0xF:
                            {
                                void* text = g_menu_equipment_base;

                                if (*(u8*)text != 0)
                                {
                                    s32 text_color = FIELD_TEXT_COLOR_MENU;

                                    if (g_item_slot_flags[0] != 0)
                                    {
                                        text_color = FIELD_TEXT_COLOR_MENU_HIGHLIGHT;
                                    }

                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry, text, text_color, content_item);
                                }
                            }

                            break;
                            case 0x10:
                            case 0x11:
                            case 0x12:
                                if (g_menu_shown_equipment->name[0] != 0)
                                {
                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry,
                                                                           menu_text_entry(menu_text_table_base(MENU_TEXT_WEAPON_ATTACK_NAMES),
                                                                                           g_menu_shown_equipment->derived.weapon.stats[content_index - 0xE]),
                                                                           FIELD_TEXT_COLOR_MENU, content_item);
                                }

                                break;
                            case 0x13:
                            {
                                s32 displayed_stat = zero;

                                if (g_item_slot_flags[0] != 0)
                                {
                                    FieldItemRecord* comparison_item = g_item_slot_data[0];

                                    if (comparison_item == 0 || comparison_item->name[0] == 0)
                                    {
                                        displayed_stat += g_field_bare_hands_item.derived.weapon.power;
                                    }
                                    else
                                    {
                                        displayed_stat = comparison_item->derived.values[0];
                                    }
                                }
                                else
                                {
                                    if (g_menu_shown_equipment->name[0] != 0)
                                    {
                                        displayed_stat = g_menu_shown_equipment->derived.values[0];
                                    }
                                }

                                packet_cursor =
                                    field_draw_number(ot_entry, packet_cursor, displayed_stat, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x14:
                            case 0x15:
                            case 0x16:
                            {
                                uintptr_t base = (uintptr_t)g_menu_equipment_base;
                                s32 slot_offset = content_index * MENU_ITEM_RECORD_SIZE;

                                if (*(u8*)(slot_offset + base - 0x4C0) != 0)
                                {
                                    s32 text_color = FIELD_TEXT_COLOR_MENU;
                                    s32 off = slot_offset - 0x4C0;
                                    void* text = (void*)(base + off);

                                    if (g_item_slot_flags[content_index - 0x13] != 0)
                                    {
                                        text_color = FIELD_TEXT_COLOR_MENU_HIGHLIGHT;
                                    }

                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry, text, text_color, content_item);
                                }
                            }

                            break;
                            case 0x17:
                            case 0x18:
                                packet_cursor = menu_draw_content_text(
                                    packet_cursor, ot_entry,
                                    menu_text_entry(menu_text_table_base(MENU_TEXT_ABILITY_NAMES),
                                                    *(menu_character_block() + content_index + (MENU_CHARACTER_FIELD_OFFSET(info.actions.commands) - 0x17))),
                                    FIELD_TEXT_COLOR_MENU, content_item);
                                break;
                            case 0x19:
                            {
                                s32 v = field_get_next_level_experience(g_menu_char_slot);
                                SavedGameLayout* context = g_saved_game_ctx;
                                u32 shift = menu_active_character(context)->progress.word >> 8;
                                packet_cursor =
                                    field_draw_number(ot_entry, packet_cursor, v - shift, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x1B:
                            case 0x1C:
                            case 0x1D:
                            case 0x1E:
                            {
                                void* base = (void*)g_saved_game_ctx;
                                u8* ptr;
                                u8 val;
                                ptr = (u8*)base + g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                                ptr += content_index;
                                content_index = (intptr_t)ptr;
                                val = ((MenuNameSelection*)(ptr + (MENU_SKILL_OFFSET - 0x1B)))->packed;

                                if (val != 0xFF)
                                {
                                    draw_packet_cursor = packet_cursor;

                                    if (val & 0x80)
                                    {
                                        u16* saved_game = (u16*)g_saved_game_ctx;
                                        void* character_record =
                                            (void*)((u8*)saved_game + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE + MENU_CHARACTER_RECORD_OFFSET));
                                        void* text = (void*)((u8*)character_record +
                                                             ((((MenuNameSelection*)(ptr + (MENU_SKILL_OFFSET - 0x1B)))->fields.index * MENU_ITEM_RECORD_SIZE) +
                                                              OFFSETOF(FieldCharacterRecord, unk150)));
                                        packet_cursor = field_draw_text(draw_packet_cursor, ot_entry, text, FIELD_TEXT_COLOR_MENU, MENU_CONTENT_X(content_item),
                                                                        MENU_CONTENT_Y(content_item), MENU_CONTENT_ALIGN(content_item));
                                    }
                                    else
                                    {
                                        u32 weapon_type;
                                        u8 idx;
                                        u16 technique_offset;
                                        void* technique_name;

                                        technique_names =
                                            (uintptr_t)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_TECHNIQUE_NAMES];
                                        weapon_type = ((g_menu_equipment_base->info.word >> 10) & 0x3F);

                                        idx = ((MenuNameSelection*)(ptr + (MENU_SKILL_OFFSET - 0x1B)))->fields.index;
                                        technique_offset = idx * 2;
                                        technique_name = (void*)(technique_names + *(u16*)(technique_offset + (weapon_type * 0x30 + technique_names)));
                                        packet_cursor =
                                            field_draw_text(draw_packet_cursor, ot_entry, technique_name, FIELD_TEXT_COLOR_MENU, MENU_CONTENT_X(content_item),
                                                            MENU_CONTENT_Y(content_item), MENU_CONTENT_ALIGN(content_item));
                                    }
                                }
                            }

                            break;
                            case 0x1F:
                                packet_cursor = menu_draw_content_text(packet_cursor, ot_entry,
                                                                       menu_text_entry(menu_text_table_base(MENU_TEXT_MONSTER_NAMES),
                                                                                       menu_character_block()[MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)]),
                                                                       FIELD_TEXT_COLOR_MENU, content_item);
                                break;
                            case 0x20:
                            {
                                u32 total = 0;

                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        FieldItemRecord* ptr = g_item_slot_data[k];

                                        if (ptr != 0 && ptr->name[0] != 0)
                                        {
                                            total += ptr->derived.values[0];
                                        }
                                    }
                                    else
                                    {
                                        FieldItemRecord* v = (FieldItemRecord*)((u8*)g_menu_shown_armor + 0x40 + (k - 1) * 0x40);

                                        if (v->name[0] != 0)
                                        {
                                            total += v->derived.values[0];
                                        }
                                    }
                                }

                                packet_cursor =
                                    menu_draw_clamped_number(ot_entry, packet_cursor, total, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x21:
                            {
                                u32 total = 0;

                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        FieldItemRecord* ptr = g_item_slot_data[k];

                                        if (ptr != 0 && ptr->name[0] != 0)
                                        {
                                            total += ptr->derived.values[1];
                                        }
                                    }
                                    else
                                    {
                                        FieldItemRecord* v = (FieldItemRecord*)((u8*)g_menu_shown_armor + 0x40 + (k - 1) * 0x40);

                                        if (v->name[0] != 0)
                                        {
                                            total += v->derived.values[1];
                                        }
                                    }
                                }

                                packet_cursor =
                                    menu_draw_clamped_number(ot_entry, packet_cursor, total, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x22:
                            {
                                u32 total = 0;

                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        FieldItemRecord* ptr = g_item_slot_data[k];

                                        if (ptr != 0 && ptr->name[0] != 0)
                                        {
                                            total += ptr->derived.values[2];
                                        }
                                    }
                                    else
                                    {
                                        FieldItemRecord* v = (FieldItemRecord*)((u8*)g_menu_shown_armor + 0x40 + (k - 1) * 0x40);

                                        if (v->name[0] != 0)
                                        {
                                            total += v->derived.values[2];
                                        }
                                    }
                                }

                                packet_cursor =
                                    menu_draw_clamped_number(ot_entry, packet_cursor, total, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x23:
                            {
                                u32 total = 0;

                                for (k = 1; k < 4; k++)
                                {
                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        FieldItemRecord* ptr = g_item_slot_data[k];

                                        if (ptr != 0 && ptr->name[0] != 0)
                                        {
                                            total += ptr->derived.values[3];
                                        }
                                    }
                                    else
                                    {
                                        FieldItemRecord* v = (FieldItemRecord*)((u8*)g_menu_shown_armor + 0x40 + (k - 1) * 0x40);

                                        if (v->name[0] != 0)
                                        {
                                            total += v->derived.values[3];
                                        }
                                    }
                                }

                                packet_cursor =
                                    menu_draw_clamped_number(ot_entry, packet_cursor, total, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            case 0x3F:
                            {
                                val = 0;

                                if (g_item_slot_flags[0] != 0)
                                {
                                    if (g_menu_shown_armor->name[0] != 0)
                                    {
                                        val = g_menu_shown_equipment->derived.values[0];
                                    }

                                    if (g_item_slot_data[0] != 0 && g_item_slot_data[0]->name[0] != 0)
                                    {
                                        val = val - (g_item_slot_data[0])->derived.values[0];
                                    }
                                    else
                                    {
                                        val = val - g_field_bare_hands_item.derived.weapon.power;
                                    }

                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, (u8*)(&label_buffer), FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }
                            }

                            break;
                            case 0x40:
                            case 0x41:
                            case 0x42:
                            case 0x43:
                            {
                                uintptr_t has = 0;

                                val = has;

                                for (k = 1; k < 4; k++)
                                {
                                    FieldItemRecord* slot = &g_menu_shown_armor[k];

                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (slot->name[0] != 0)
                                        {
                                            val += slot->derived.values[content_index - 0x40];
                                        }

                                        has = (uintptr_t)g_item_slot_data[k];

                                        if (has != 0 && ((FieldItemRecord*)has)->name[0] != 0)
                                        {
                                            val -= ((FieldItemRecord*)has)->derived.values[content_index - 0x40];
                                        }

                                        has = 1;
                                    }
                                }

                                if (has)
                                {
                                    menu_copy_sign_label(label_buffer, val);
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, (u8*)(&label_buffer), FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }
                            }

                            break;
                            case 0x44:
                            {
                                u16 current_stat = 0;

                                if (g_item_slot_flags[0] != 0)
                                {
                                    s32 diff;

                                    if (g_menu_shown_armor->name[0] != 0)
                                    {
                                        current_stat = g_menu_shown_equipment->derived.values[0];
                                    }

                                    if (g_item_slot_data[0] != 0 && g_item_slot_data[0]->name[0] != 0)
                                    {
                                        diff = current_stat - (g_item_slot_data[0])->derived.values[0];
                                    }
                                    else
                                    {
                                        diff = current_stat - g_field_bare_hands_item.derived.weapon.power;
                                    }

                                    packet_cursor =
                                        field_draw_number(ot_entry, packet_cursor, abs(diff), FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            case 0x45:
                            case 0x46:
                            case 0x47:
                            case 0x48:
                            {
                                uintptr_t has = 0;
                                s32 total = has;

                                for (k = 1; k < 4; k++)
                                {
                                    FieldItemRecord* slot = &g_menu_shown_armor[k];

                                    if (g_item_slot_flags[k] != 0)
                                    {
                                        if (slot->name[0] != 0)
                                        {
                                            total += slot->derived.values[content_index - 0x45];
                                        }

                                        has = (uintptr_t)g_item_slot_data[k];

                                        if (has != 0 && ((FieldItemRecord*)has)->name[0] != 0)
                                        {
                                            total -= ((FieldItemRecord*)has)->derived.values[content_index - 0x45];
                                        }

                                        has = 1;
                                    }
                                }

                                if (has)
                                {
                                    packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, (u32)abs(total), FIELD_TEXT_COLOR_MENU, &pos,
                                                                             MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            case 0x49:
                                packet_cursor = menu_draw_content_text(packet_cursor, ot_entry,
                                                                       menu_text_entry(menu_text_table_base(MENU_TEXT_COMPANION_ABILITY_NAMES),
                                                                                       menu_character_block()[MENU_CHARACTER_FIELD_OFFSET(companion_ability)]),
                                                                       FIELD_TEXT_COLOR_MENU, content_item);
                                break;
                            case 0x4A:
                            {
                                s8 golem_index = g_saved_game_ctx->joined_golem;

                                if (golem_index >= 0)
                                {
                                    SavedGameLayout* context = g_saved_game_ctx;
                                    u8 v = menu_golem_record(context, golem_index)->logic_layout & GOLEM_LOGIC_CLASS_MASK;
                                    void* a2 =
                                        (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_GOLEM_CLASS_NAMES]);
                                    packet_cursor =
                                        field_draw_text(packet_cursor, ot_entry, (void*)((u8*)a2 + *(u16*)((u8*)a2 + (v * 2))), FIELD_TEXT_COLOR_MENU,
                                                        MENU_CONTENT_X(content_item), MENU_CONTENT_Y(content_item), MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            case 0x4B:
                            {
                                s8 golem_index = g_saved_game_ctx->joined_golem;

                                if (golem_index >= 0)
                                {
                                    SavedGameLayout* context = g_saved_game_ctx;
                                    u8 v = menu_golem_record(context, golem_index)->logic_layout >> GOLEM_GRID_BOUND_SHIFT;
                                    void* a2 =
                                        (void*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_GOLEM_GRID_SIZES]);
                                    packet_cursor =
                                        field_draw_text(packet_cursor, ot_entry, (void*)((u8*)a2 + *(u16*)((u8*)a2 + (v * 2))), FIELD_TEXT_COLOR_MENU,
                                                        MENU_CONTENT_X(content_item), MENU_CONTENT_Y(content_item), MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            case 0x4C:
                            {
                                s8 golem_index = g_saved_game_ctx->joined_golem;

                                if (golem_index >= 0)
                                {
                                    SavedGameLayout* context = g_saved_game_ctx;
                                    u8 v = menu_golem_record(context, golem_index)->unknown_0x46;
                                    packet_cursor =
                                        field_draw_number(ot_entry, packet_cursor, v, FIELD_TEXT_COLOR_MENU, &pos, MENU_CONTENT_ALIGN(content_item));
                                }
                            }

                            break;
                            case 0x4D:
                            {
                                s8 golem_index = g_saved_game_ctx->joined_golem;

                                if (golem_index >= 0)
                                {
                                    packet_cursor = menu_draw_content_text(packet_cursor, ot_entry,
                                                                           menu_text_entry(menu_text_table_base(MENU_TEXT_GOLEM_PALETTE_NAMES),
                                                                                           menu_golem_record(g_saved_game_ctx, golem_index)->palette),
                                                                           FIELD_TEXT_COLOR_MENU, content_item);
                                }
                            }

                            break;
                            case 0x4E:
                            case 0x4F:
                            case 0x50:
                            case 0x51:
                            case 0x52:
                            case 0x53:
                            case 0x54:
                            case 0x55:
                            {
                                u32 display_value = 0;

                                switch (content_index)
                                {
                                    case 0x4E:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.word & 0xF;
                                        break;
                                    case 0x4F:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.bytes[0];
                                        display_value >>= 4;
                                        break;
                                    case 0x50:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.word >> 8;
                                        display_value &= 0xF;
                                        break;
                                    case 0x51:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.word >> 12;
                                        display_value &= 0xF;
                                        break;
                                    case 0x52:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.halves[1];
                                        display_value &= 0xF;
                                        break;
                                    case 0x53:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.word >> 20;
                                        display_value &= 0xF;
                                        break;
                                    case 0x54:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.bytes[3];
                                        display_value &= 0xF;
                                        break;
                                    case 0x55:
                                        display_value = menu_equipped_item(g_saved_game_ctx, 0)->bonus_nibbles.word >> 28;
                                        break;
                                }

                                packet_cursor = menu_draw_clamped_number(ot_entry, packet_cursor, display_value, FIELD_TEXT_COLOR_MENU, &pos,
                                                                         MENU_CONTENT_ALIGN(content_item));
                            }

                            break;
                            default:
                                break;
                        }
                    }

                    break;

                    case 6:
                    {
                        shared_s0 = content_item->action_type;

                        switch (shared_s0)
                        {
                            case 1:
                                if (g_saved_game_ctx->money > SAVED_MONEY_MAX)
                                {
                                    packet_cursor = field_draw_number(ot_entry, packet_cursor, SAVED_MONEY_MAX, FIELD_TEXT_COLOR_MENU, &pos,
                                                                      MENU_CONTENT_ALIGN(content_item));
                                }
                                else
                                {
                                    packet_cursor = field_draw_number(ot_entry, packet_cursor, g_saved_game_ctx->money, FIELD_TEXT_COLOR_MENU, &pos,
                                                                      MENU_CONTENT_ALIGN(content_item));
                                }

                                break;
                            case 2:
                            {
                                u32 align;
                                s32 total_frames;
                                s32 hours;
                                s32 hour_minutes;
                                align = MENU_CONTENT_ALIGN(content_item);

                                switch (align)
                                {
                                    case 1:
                                        pos.x -= 0x32;
                                        break;
                                    case 2:
                                        pos.x -= 0x19;
                                        break;
                                }

                                total_frames = g_saved_game_ctx->play_time + VSync(-1);
                                shared_s0 = total_frames - g_playtime_vsync_origin;
                                pos.x += 0x14;
                                hours = shared_s0 / SAVED_PLAY_TIME_TICKS_PER_HOUR;
                                packet_cursor = field_draw_number(ot_entry, packet_cursor, hours, FIELD_TEXT_COLOR_MENU, &pos, FIELD_TEXT_ALIGN_RIGHT);

                                if ((g_frame_counter / 15) & 1)
                                {
                                    packet_cursor = field_draw_text(packet_cursor, ot_entry, MENU_PLAYTIME_SEPARATOR, FIELD_TEXT_COLOR_MENU, pos.x, pos.y,
                                                                    FIELD_TEXT_ALIGN_LEFT);
                                }

                                pos.x += MENU_PLAYTIME_SEPARATOR_ADVANCE;
                                hour_minutes = hours * 0x3C;
                                shared_s0 = (shared_s0 / SAVED_PLAY_TIME_TICKS_PER_MINUTE) - hour_minutes;

                                if (shared_s0 < 0xA)
                                {
                                    packet_cursor = field_draw_number(ot_entry, packet_cursor, 0U, FIELD_TEXT_COLOR_MENU, &pos, FIELD_TEXT_ALIGN_LEFT);
                                }

                                pos.x += MENU_PLAYTIME_MINUTES_ADVANCE;
                                packet_cursor = field_draw_number(ot_entry, packet_cursor, shared_s0, FIELD_TEXT_COLOR_MENU, &pos, FIELD_TEXT_ALIGN_RIGHT);
                            }

                            break;
                        }
                    }

                    break;

                    case 7:
                    {
                        shared_s0 = content_item->action_type;

                        switch (shared_s0)
                        {
                            case 1:
                            {
                                void* base = (void*)g_saved_game_ctx + g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;

                                if ((*(u8*)((u8*)base + MENU_CHARACTER_INFO_OFFSET) & FIELD_CHARACTER_TYPE_MASK) != FIELD_CHARACTER_GUEST ||
                                    ((*(u8*)((u8*)base + MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)) != 5) &&
                                     (*(u8*)((u8*)base + MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)) != 8)))
                                {
                                    packet_cursor = menu_draw_content_text(
                                        packet_cursor, ot_entry, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_QUICK_ATTACKS),
                                        FIELD_TEXT_COLOR_MENU, content_item);
                                }
                            }

                            break;
                            case 2:
                            {
                                void* base = (void*)g_saved_game_ctx + g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;

                                if ((*(u8*)((u8*)base + MENU_CHARACTER_INFO_OFFSET) & FIELD_CHARACTER_TYPE_MASK) != FIELD_CHARACTER_GUEST ||
                                    ((*(u8*)((u8*)base + MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)) != 5) &&
                                     (*(u8*)((u8*)base + MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)) != 8)))
                                {
                                    packet_cursor = menu_draw_content_text(
                                        packet_cursor, ot_entry, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_POWER_ATTACKS),
                                        FIELD_TEXT_COLOR_MENU, content_item);
                                }
                            }

                            break;
                        }
                    }

                    break;

                    default:
                        break;
                }

                content_item++;
                remaining_count--;
            }

            nodes = g_menu_nodes;
            node = nodes + g_menu_scene_type;

            if (node->label_id == 0x13)
            {
                u8* title = (u8*)g_menu_item_ptr;

                if (*title != 0)
                {
                    packet_cursor = field_draw_text(packet_cursor, ot_entry, title, FIELD_TEXT_COLOR_MENU, 0xAC, 0xC, FIELD_TEXT_ALIGN_CENTER);
                }
            }
            else if (g_menu_scene_type == 0x1D)
            {
                packet_cursor = field_draw_text(packet_cursor, ot_entry, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_EVENT_LIST),
                                                FIELD_TEXT_COLOR_MENU, 0xAC, 0xC, FIELD_TEXT_ALIGN_CENTER);
            }
            else if (g_menu_scene_type != -1)
            {
                packet_cursor = field_draw_text(packet_cursor, ot_entry, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), node->label_id),
                                                FIELD_TEXT_COLOR_MENU, 0xAC, 0xC, FIELD_TEXT_ALIGN_CENTER);
            }

            switch (g_menu_scene_type)
            {
                case 20:
                case 21:
                case 23:
                case 24:
                case 26:
                case 27:
                {
                    void* number_cursor;
                    pos.x = 0x88;
                    pos.y = 0x28;

                    if (g_menu_page_count != 0)
                    {
                        number_cursor = field_draw_sprite_number(ot_entry, packet_cursor, g_script_repeat_last + 1, 3, &pos, 0);
                    }
                    else
                    {
                        number_cursor = field_draw_sprite_number(ot_entry, packet_cursor, 0, 3, &pos, 0);
                    }

                    number_cursor = field_draw_sprite_glyph(number_cursor, ot_entry, FIELD_DIGIT_GLYPH_SEPARATOR, &pos, 0);
                    pos.x += 8;
                    number_cursor = field_draw_sprite_number(ot_entry, number_cursor, g_menu_page_count, 3, &pos, 0);
                    number_cursor = field_draw_sprite_glyph(number_cursor, ot_entry, FIELD_DIGIT_GLYPH_SEPARATOR, &pos, 0);
                    pos.x += 8;
                    packet_cursor = field_draw_sprite_number(ot_entry, number_cursor, menu_count_inventory_items(), 3, &pos, 0);
                }

                break;
                default:
                    break;
            }
        }
    }

    return packet_cursor;
}

/**
 * @brief Combine the type-conflict bits of the active character's equipment, skipping one slot.
 * @param excluded_slot Equipment slot to leave out.
 * @return Conflict bits of the other equipped items.
 */
inline s32 menu_get_equipment_conflicts(s32 excluded_slot)
{
    s32 equipment_index;
    s32 conflicts;
    FieldItemRecord* equipment;
    u32 item_info;

    conflicts = 0;
    equipment = g_menu_equipment_base;

    for (equipment_index = 0; equipment_index < MENU_EQUIPMENT_SLOT_COUNT; equipment_index++)
    {
        if ((equipment_index != excluded_slot) && (equipment->name[0] != 0))
        {
            item_info = equipment->info.word;

            if (item_info & FIELD_ITEM_CATEGORY_BITS)
            {
                conflicts |= g_field_armor_type_conflicts[FIELD_ITEM_TYPE(item_info)];
            }
            else
            {
                conflicts |= g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(item_info)];
            }
        }

        equipment++;
    }

    return conflicts;
}

/**
 * @brief Emit and link the menu texture-page command.
 * @param draw_mode Primitive-buffer cursor.
 * @param ot Ordering-table entry to prepend to.
 * @return Primitive-buffer cursor after the command.
 */
void* menu_emit_draw_mode_primitive(DR_TPAGE* draw_mode, u_long* ot)
{
    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot, draw_mode);
    draw_mode++;
    return draw_mode;
}

/**
 * @brief Concatenate two encoded text command streams into a destination buffer.
 * @param dst Buffer large enough for both streams and the terminator.
 * @param src1 First null-terminated stream.
 * @param src2 Second null-terminated stream.
 * @note Bytes 0x19-0x1F introduce a second byte, which may be zero.
 */
void menu_concat_encoded_text(u8* dst, u8* src1, u8* src2)
{
    menu_copy_encoded_pair(dst, src1, src2);
}

/**
 * @brief Extract a 4-bit nibble from a packed u32 field in the item struct and look it up in a byte table.
 * @param item Item record containing the packed nibble field.
 * @param index Nibble selector (0-7), selecting four bits at a time.
 * @return Signed byte from g_field_stat_modifier_values at the selected nibble index.
 * @note An index of 8 or more reads the table at an undefined position, as
 *       the original does; no caller passes one.
 */
s32 menu_lookup_item_nibble(const FieldItemRecord* item, u32 index)
{
    u32 nibble;

    if (index < 8U)
    {
        switch (index)
        {
            case 0:
                nibble = item->stat_nibbles.word & 0xF;
                break;
            case 1:
                nibble = item->stat_nibbles.bytes[0];
                nibble = nibble >> 4;
                break;
            case 2:
                nibble = item->stat_nibbles.word >> 8;
                nibble = nibble & 0xF;
                break;
            case 3:
                nibble = item->stat_nibbles.word >> 12;
                nibble = nibble & 0xF;
                break;
            case 4:
                nibble = item->stat_nibbles.halves[1];
                nibble = nibble & 0xF;
                break;
            case 5:
                nibble = item->stat_nibbles.word >> 20;
                nibble = nibble & 0xF;
                break;
            case 6:
                nibble = item->stat_nibbles.bytes[3];
                nibble = nibble & 0xF;
                break;
            case 7:
                nibble = item->stat_nibbles.word >> 28;
                break;
        }
    }

    return g_field_stat_modifier_values[nibble];
}

/**
 * @brief Find the first active action or submenu item in the current scene.
 * @return Content-item index, or -1 if there is no scene or matching item.
 */
s32 menu_find_active_content_item(void)
{
    u8 content_id;
    s32 item_count;
    MenuContentItem* item;
    s32 item_index;
    u16 packed_x;
    u16 content_type;

    if (g_menu_scene_type == -1)
    {
        return -1;
    }

    content_id = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
    item = g_menu_content_table[content_id];
    item_count = g_menu_content_item_counts[g_menu_content_group_ids[content_id]];

    for (item_index = 0; item_index < item_count; item_index++, item++)
    {
        packed_x = item->packed_x;
        content_type = packed_x & MENU_CONTENT_ITEM_TYPE_MASK;

        if ((content_type == MENU_CONTENT_ITEM_TYPE_ACTION || content_type == MENU_CONTENT_ITEM_TYPE_SUBMENU) &&
            (packed_x & MENU_CONTENT_ITEM_ACTIVE_MASK) == MENU_CONTENT_ITEM_ACTIVE_MASK)
        {
            return item_index;
        }
    }

    return -1;
}

/**
 * @brief Find the navigation-list index of a given node ID.
 * @param node_id Node ID to locate.
 * @return Navigation index, or -1 if the node is absent.
 */
s32 menu_find_nav_node_index(s32 node_id)
{
    s32* nav_entry;
    s32 nav_index;
    s32 nav_count;

    nav_index = 0;

    if (g_menu_nav_count > 0)
    {
        nav_count = g_menu_nav_count;
        nav_entry = g_menu_nav_nodes;

        do
        {
            if (*nav_entry == node_id)
            {
                return nav_index;
            }

            nav_index++;
            nav_entry++;
        } while (nav_index < nav_count);
    }

    return -1;
}

/**
 * @brief Draw a slot's scroll arrows for content outside its viewport.
 * @param sprite Primitive write cursor.
 * @param ot_entry Ordering-table entry for the arrows.
 * @param slot Menu slot whose scroll fields drive the arrows.
 * @return Updated primitive write cursor.
 */
void* menu_emit_slot_scroll_arrows(SPRT* sprite, u_long* ot_entry, MenuSlot* slot)
{
    s32 arrow_count = 0;
    s32 remaining_height;
    u8* packet_cursor;

    if (slot->lerp_cur_b != 0)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, slot->x + slot->w - MENU_SCROLL_ARROW_SIZE, slot->y);
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_UP);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        addPrim(ot_entry, sprite);
        arrow_count = 1;
        sprite++;
    }

    remaining_height = ((slot->navigation.fields.count_and_ot & MENU_ITEM_NAV_INDEX_MASK) << 4) - slot->lerp_cur_b;

    if ((slot->h - MENU_SCROLL_ARROW_SIZE) < remaining_height)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, slot->x + slot->w - MENU_SCROLL_ARROW_SIZE, slot->y + slot->h - 8);
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_DOWN);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        addPrim(ot_entry, sprite);
        arrow_count += 1;
        sprite++;
    }

    packet_cursor = (u8*)sprite;

    if (arrow_count != 0)
    {
        DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;
        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
        addPrim(ot_entry, draw_mode);
        packet_cursor = (u8*)(draw_mode + 1);
    }

    return packet_cursor;
}

/**
 * @brief Draw navigation-tree scroll arrows for content outside the viewport.
 * @param sprite Primitive write cursor.
 * @param ot_entry Pointer to the ordering-table entry to prepend each arrow_count primitive to.
 * @return Pointer to the next free byte in @p sprite after all arrow_count primitives.
 */
void* menu_emit_tree_scroll_arrows(SPRT* sprite, u_long* ot_entry)
{
    u8* packet_cursor;

    if (g_menu_content_height != 0)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, 0x20, 3);
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_UP);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        addPrim(ot_entry, sprite);
        sprite++;
    }

    if ((g_menu_layout_end - g_menu_content_height) > MENU_VIEW_HEIGHT)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, 0x20, 0xBA);
        SET_SPRT_UV0_PACKED(sprite, MENU_SCROLL_ARROW_UV_DOWN);
        SET_SPRT_CLUT(sprite, MENU_SCROLL_ARROW_CLUT);
        SET_SPRT_WH_PACKED(sprite, MENU_SCROLL_ARROW_SIZE, MENU_SCROLL_ARROW_SIZE);
        addPrim(ot_entry, sprite);
        sprite++;
    }

    packet_cursor = (u8*)sprite;

    if ((g_menu_content_height != 0) || (g_menu_layout_end > MENU_VIEW_HEIGHT))
    {
        DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;
        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
        addPrim(ot_entry, draw_mode);
        packet_cursor = (u8*)(draw_mode + 1);
    }

    return packet_cursor;
}

/**
 * @brief Draw the active node's cursor, clamped to the visible viewport.
 * @param packet_cursor Primitive write cursor.
 * @param ot_entry Ordering-table entry for the arrow_count primitives.
 * @param draw_label Non-zero to also draw the node's label.
 * @return Updated primitive write cursor.
 */
void* menu_draw_active_node_cursor(void* packet_cursor, u_long* ot_entry, s32 draw_label)
{
    u16 nav_x_packed;
    u8 nav_y_high;
    s32 nav_y_low;
    s32 node_y;
    s32 cursor_y;
    s32 viewport_y;
    u8* label_table;

    nav_x_packed = g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed;
    nav_y_high = g_menu_nodes[g_menu_active_node].u8_u.s.nav_y_hi;

    nav_y_low = nav_x_packed >> 15;
    node_y = (nav_y_high << 1) | nav_y_low;
    viewport_y = g_menu_content_height - MENU_CURSOR_Y_MIN;
    cursor_y = node_y - viewport_y;

    if (cursor_y < MENU_CURSOR_Y_MIN)
    {
        cursor_y = MENU_CURSOR_Y_MIN;
    }

    if (cursor_y >= MENU_CURSOR_Y_MAX)
    {
        cursor_y = MENU_CURSOR_Y_MAX;
    }

    packet_cursor = menu_emit_cursor(packet_cursor, ot_entry, menu_nav_x(nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET, cursor_y, 1);

    if (draw_label != 0)
    {
        label_table = menu_text_table_base(MENU_TEXT_GENERAL);
        packet_cursor = field_draw_text(packet_cursor, ot_entry, menu_text_entry(label_table, g_menu_nodes[g_menu_active_node].label_id), FIELD_TEXT_COLOR_MENU,
                                        0xA0, 0xCA, FIELD_TEXT_ALIGN_CENTER);
    }

    return packet_cursor;
}

/**
 * @brief Render the content cursor, update its lerped position, and optionally render the active hit-item label.
 * @param prim_buf Current primitive buffer pointer, reused as the running cursor.
 * @param ot Pointer to the current ordering-table entry.
 * @param draw_label Non-zero to also render the label string for the active hit item.
 * @return Updated primitive buffer pointer after all emitted primitives.
 */
void* menu_draw_content_cursor(void* prim_buf, u_long* ot, s32 draw_label)
{
    u8 label_buffer[0x40];
    u8 item_name_buffer[0x40];
    u_long* cursor_ot;
    s32 cursor_active;
    s16 text_index;
    MenuContentItem* content_base;
    u16 content_type;
    s32 action_type;
    DR_TPAGE* draw_mode;

    if (g_menu_content_ready == 0)
    {
        cursor_active = 0;

        if (menu_item_has_action() != 0)
        {
            cursor_active = (draw_label != 0);
        }

        cursor_ot = ot;

        if (g_menu_suppress_cursor != 0)
        {
            cursor_ot = ot - 10;
        }

        prim_buf = menu_emit_cursor(prim_buf, cursor_ot, g_content_cursor_x, g_content_cursor_y, cursor_active);
    }

    if (g_menu_suppress_cursor != 0)
    {
        s32* cxp = &g_content_cursor_x;
        s32 dx = (g_content_view_x - *cxp) / g_menu_suppress_cursor;
        s32* cyp = &g_content_cursor_y;
        s32 dy = (g_content_view_y - *cyp) / g_menu_suppress_cursor;
        g_menu_suppress_cursor -= 1;
        *cxp = dx + *cxp;
        *cyp = dy + *cyp;
    }
    else
    {
        g_content_cursor_x = g_content_view_x;
        g_content_cursor_y = g_content_view_y;
    }

    if (draw_label != 0)
    {
        content_base = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
        content_type = content_base[g_menu_hit_item_idx].packed_x & MENU_CONTENT_ITEM_TYPE_MASK;

        if (content_type == MENU_CONTENT_ITEM_TYPE_ACTION)
        {
            if (content_base[g_menu_hit_item_idx].action_type < 0xF0U)
            {
                prim_buf = menu_emit_content_label(prim_buf, ot,
                                                   menu_text_entry(menu_text_table_base(MENU_TEXT_GENERAL), content_base[g_menu_hit_item_idx].action_type));
            }
            else
            {
                action_type = content_base[g_menu_hit_item_idx].action_type;

                switch (action_type)
                {
                    case 0xF0:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MONSTER_FAMILIES), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF1:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_ARTIFACT_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF2:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MAGIC_SHAPE_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF3:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_SPIRIT_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF4:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MAGIC_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF5:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_ITEM_SPECIAL_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF6:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot,
                            menu_text_entry(menu_text_table_base(MENU_TEXT_MONSTER_TECHNIQUE_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF7:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_WEAPON_ATTACK_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xF8:
                        if (g_menu_help_skill_selection != MENU_NONE)
                        {
                            if (g_menu_help_skill_selection & 0x80)
                            {
                                prim_buf = menu_emit_content_label(
                                    prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MAGIC_HELP), content_base[g_menu_hit_item_idx].action_type));
                            }
                            else
                            {
                                prim_buf = menu_emit_content_label(
                                    prim_buf, ot,
                                    menu_text_entry(menu_text_table_base(MENU_TEXT_TECHNIQUE_HELP), content_base[g_menu_hit_item_idx].action_type));
                            }
                        }

                        break;
                    case 0xF9:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_TECHNIQUE_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xFA:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_ABILITY_HELP), content_base[g_menu_hit_item_idx].action_type));
                        break;
                    case 0xFB:
                    case 0xFC:
                    case 0xFD:
                    case 0xFE:
                        prim_buf = menu_emit_content_label(
                            prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_ITEM_HELP), content_base[g_menu_hit_item_idx].action_type));
                }
            }
        }
        else if (content_type == MENU_CONTENT_ITEM_TYPE_SUBMENU)
        {
            action_type = content_base[g_menu_hit_item_idx].action_type;

            switch (action_type)
            {
                case 1:
                case 2:
                {
                    u8* text_table = menu_text_table_base(MENU_TEXT_ABILITY_HELP);
                    u8* character_base = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                    u8 entry_index = *(character_base + action_type + MENU_ACTION_SLOT_OFFSET);

                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                    break;
                }
                case 3:
                case 4:
                case 5:
                case 6:
                {
                    u8* pad_base = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                    u8* flag_addr = pad_base + action_type;
                    u8 flag = ((MenuNameSelection*)(flag_addr + MENU_ACTION_SLOT_OFFSET))->packed;

                    if (flag != MENU_NONE)
                    {
                        if (flag & 0x80)
                        {
                            u8* item_ptr = pad_base + (((MenuNameSelection*)(flag_addr + MENU_ACTION_SLOT_OFFSET))->fields.index * MENU_ITEM_RECORD_SIZE) +
                                           (MENU_CHARACTER_RECORD_OFFSET + OFFSETOF(FieldCharacterRecord, unk150));
                            u8 cat = *(item_ptr + OFFSETOF(FieldItemRecord, derived.instrument.spirit));
                            u8 entry = *(item_ptr + OFFSETOF(FieldItemRecord, derived.instrument.spell));
                            u8* state44 = (u8*)g_menu_state_ptr;
                            state44 += ((MenuTextResources*)state44)->table_offsets[MENU_TEXT_MAGIC_NAMES];
                            prim_buf = menu_emit_content_label(prim_buf, ot, state44 + *(u16*)(state44 + (cat) * 0x1C + (entry) * 2));
                        }
                        else
                        {
                            u32 weapon_info = *(u32*)(pad_base + MENU_CHARACTER_FIELD_OFFSET(equipment[FIELD_WEAPON_SLOT].info));
                            u8* state1c = (u8*)g_menu_state_ptr;
                            state1c += ((MenuTextResources*)state1c)->table_offsets[MENU_TEXT_TECHNIQUE_HELP];
                            prim_buf = menu_emit_content_label(prim_buf, ot,
                                                               state1c + *(u16*)(state1c +
                                                                                 ((MenuNameSelection*)(flag_addr + MENU_ACTION_SLOT_OFFSET))->fields.index * 2 +
                                                                                 FIELD_ITEM_TYPE(weapon_info) * 0x30));
                        }
                    }

                    break;
                }
                case 7:
                case 8:
                case 9:
                case 10:
                    if (g_menu_char_slot < 2)
                    {
                        SavedGameLayout* context = g_saved_game_ctx;

                        if (menu_equipped_item(context, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->name[0] != 0)
                        {
                            u32 item_attributes;
                            u32 kind;

                            if (menu_item_differs_from_bare_hands(
                                    &menu_character_record(context, g_menu_char_slot)->equipment[action_type - MENU_EQUIPMENT_SUBTYPE_BASE]) != 0)
                            {
                                s32 name_index = menu_equipped_item(g_saved_game_ctx, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->info.halves[1] & 0x3F;
                                u16 name_offset = *(u16*)((u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_NAMES] +
                                                          name_index * 2);
                                u8* message_table = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES];
                                u16 suffix_offset = *(u16*)((u8*)message_table + 0xB4);
                                u8* name = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_ITEM_NAMES] + name_offset;
                                u8* suffix = (u8*)g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES] + suffix_offset;
                                u8* out = item_name_buffer;
                                menu_copy_encoded_pair(out, name, suffix);
                            }
                            else
                            {
                                item_name_buffer[0] = 0;
                            }

                            item_attributes = menu_equipped_item(g_saved_game_ctx, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->info.word;
                            kind = (item_attributes >> 8) & 3;

                            switch (kind)
                            {
                                case 0:
                                {
                                    u32 idx = (item_attributes >> 9) & 0x7E;
                                    uintptr_t category_help_table = (uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES);
                                    u8* str2 = (u8*)(category_help_table + *(u16*)(idx + category_help_table));

                                    u8* out = label_buffer;
                                    u8* first = item_name_buffer;
                                    menu_copy_encoded_pair(out, first, str2);
                                    break;
                                }
                                case 1:
                                {
                                    u32 idx = (item_attributes >> 9) & 0x7E;
                                    uintptr_t category_help_table = (uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES);
                                    u8* str2 = (u8*)(category_help_table + *(u16*)(idx + category_help_table + 11 * sizeof(u16)));

                                    u8* out = label_buffer;
                                    u8* first = item_name_buffer;
                                    menu_copy_encoded_pair(out, first, str2);
                                    break;
                                }
                                default:
                                {
                                    s32* category_help_offset = &((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_EQUIPMENT_TYPE_NAMES];
                                    SavedGameLayout* context = g_saved_game_ctx;
                                    u32 reloaded_item_attributes = menu_equipped_item(context, action_type - MENU_EQUIPMENT_SUBTYPE_BASE)->info.word;
                                    u32 idx = (reloaded_item_attributes >> 9) & 0x7E;
                                    uintptr_t category_help_table = (uintptr_t)g_menu_state_ptr + *category_help_offset;
                                    u8* str2 = (u8*)(category_help_table + *(u16*)(idx + category_help_table + 23 * sizeof(u16)));

                                    u8* out = label_buffer;
                                    u8* first = item_name_buffer;
                                    menu_copy_encoded_pair(out, first, str2);
                                    break;
                                }
                            }

                            prim_buf = menu_emit_content_label(prim_buf, ot, label_buffer);
                        }
                    }

                    break;
                case 11:
                case 12:
                case 13:
                {
                    u8* char_base = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);

                    if (*(char_base + MENU_FIRST_EQUIPMENT_OFFSET) != 0)
                    {
                        text_index = *(char_base + action_type + (MENU_CHARACTER_FIELD_OFFSET(equipment[FIELD_WEAPON_SLOT].derived.weapon.stats[2]) - 11));
                        prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_WEAPON_ATTACK_HELP), text_index));
                    }

                    break;
                }
                case 14:
                {
                    u8* text_table = menu_text_table_base(MENU_TEXT_MONSTER_FAMILIES);
                    u8* character_base = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                    u8 entry_index = character_base[MENU_CHARACTER_FIELD_OFFSET(info.actions.unk19)];

                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                    break;
                }
                case 15:
                    if (g_menu_item_ptr != NULL)
                    {
                        prim_buf = menu_emit_content_label(prim_buf, ot, (u8*)g_menu_item_ptr);
                    }

                    break;
                case 16:
                case 17:
                case 18:
                    if (g_menu_item_ptr != 0)
                    {
                        text_index = g_menu_item_ptr->special_ids[action_type - 16];
                        prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_ITEM_SPECIAL_HELP), text_index));
                    }

                    break;
                case 19:
                case 20:
                case 21:
                    if (g_menu_item_ptr != 0)
                    {
                        text_index = g_menu_item_ptr->derived.weapon.stats[action_type - 17];
                        prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_WEAPON_ATTACK_HELP), text_index));
                    }

                    break;
                case 22:
                    if (g_menu_item_ptr != 0)
                    {
                        u8* help_table = menu_text_table_base(MENU_TEXT_MAGIC_HELP);
                        u8 cat = g_menu_category2_item->derived.instrument.spirit;
                        u8 entry = g_menu_category2_item->derived.instrument.spell;
                        prim_buf = menu_emit_content_label(prim_buf, ot, help_table + *(u16*)(help_table + cat * 0x1C + entry * 2));
                    }

                    break;
                case 23:
                    if (g_menu_item_ptr != 0)
                    {
                        u8* text_table = menu_text_table_base(MENU_TEXT_SPIRIT_HELP);
                        u8* item = (u8*)g_menu_category2_item;
                        u8 entry_index = item[OFFSETOF(FieldItemRecord, derived.instrument.spirit)];
                        prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                    }

                    break;
                case 24:
                    if (g_menu_item_ptr != 0)
                    {
                        u8* text_table = menu_text_table_base(MENU_TEXT_MAGIC_SHAPE_HELP);
                        u8* item = (u8*)g_menu_category2_item;
                        u8 entry_index = item[OFFSETOF(FieldItemRecord, derived.instrument.spell)];
                        prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                    }

                    break;
                case 25:
                {
                    u8* text_table = menu_text_table_base(MENU_TEXT_COMPANION_ABILITY_HELP);
                    u8* character_base = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                    u8 entry_index = character_base[MENU_CHARACTER_FIELD_OFFSET(companion_ability)];

                    prim_buf = menu_emit_content_label(prim_buf, ot, menu_text_entry(text_table, entry_index));
                    break;
                }
            }
        }
    }

    if (g_menu_button_swap_marker.selected_idx != MENU_NONE)
    {
        prim_buf = menu_emit_swap_marker(prim_buf, ot, g_menu_button_swap_marker.x, g_menu_button_swap_marker.y);
        draw_mode = (DR_TPAGE*)prim_buf;
        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
        addPrim(ot, draw_mode);
        prim_buf = draw_mode + 1;
    }

    return prim_buf;
}

/**
 * @brief Render the root nodes and advance the tree's scroll animation.
 * @param prim_buf Current primitive buffer pointer.
 * @param ot Pointer to the ordering-table entry used by node-rendering helpers.
 * @return Updated primitive buffer pointer after rendering all active root nodes.
 */
void* menu_draw_node_tree(void* prim_buf, u_long* ot)
{
    s32 node_index;
    s32 scroll_step;

    g_menu_nav_count = 0;

    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        if ((g_menu_nodes[node_index].u2.s.parent_idx == MENU_NONE) && (g_menu_nodes[node_index].u2.s.flags & MENU_NODE_FLAG_ACTIVE))
        {
            prim_buf = menu_draw_node_recursive(node_index, prim_buf, ot);
        }
    }

    if ((g_menu_redraw_state == 0) || (g_menu_scroll_pos == g_menu_content_height))
    {
        g_menu_redraw_state = 0;
        g_menu_content_height = g_menu_scroll_pos;
    }
    else
    {
        scroll_step = (g_menu_scroll_pos - g_menu_content_height) / g_menu_redraw_state;
        g_menu_redraw_state -= 1;
        g_menu_content_height += scroll_step;
    }

    return prim_buf;
}

/**
 * @brief Render one menu node's panel and update its animated Y position; recurse into children.
 * @param node_index Node index within g_menu_nodes.
 * @param prim_buf Current primitive buffer pointer.
 * @param ot Pointer to the ordering-table entry.
 * @return Updated primitive buffer pointer after rendering this node and all expanded children.
 */
void* menu_draw_node_recursive(s32 node_index, void* prim_buf, u_long* ot)
{
    MenuNode* node;
    MenuNode* node_base;
    int work_mask;
    int work_value;
    MenuNode* child_source;
    void* next_prim;
    int animation_count;
    MenuNode* tree_node;
    MenuNode* tree_base;
    u16 flags;
    u32 animation_frames;

    g_menu_nav_nodes[g_menu_nav_count] = node_index;
    node_base = g_menu_nodes;
    node = node_base + node_index;
    g_menu_nav_count += 1;
    next_prim = menu_emit_icon_sprite(prim_buf, ot, node->icon_id, ((node->idx_nav.nav_x_packed >> 8) & (MENU_NAV_X_MASK >> 8)) + 1,
                                      ((node->u8_u.s.nav_y_hi << 1) | (work_value = node->idx_nav.nav_x_packed >> 15)) - g_menu_content_height, 1,
                                      ((node->u2.unk2 >> 2) & 3) != 0, g_menu_scene_type == node_index, (node->u2.unk2 >> 6) & 3);

    flags = node->u2.unk2;
    animation_frames = (flags >> 2) & 3;

    if (animation_frames != 0)
    {
        s32 updated_flags;
        work_value = animation_frames - 1;
        animation_count = work_value & 3;
        work_mask = animation_count << 2;
        updated_flags = flags & 0xFFF3;
        node->u2.unk2 = updated_flags | work_mask;
    }

    if (node->layout_frames_remaining == MENU_NODE_LAYOUT_IDLE)
    {
        s32 target_y_low_bit = node->u8_u.nav_y_packed & 0x8000;
        u16 packed_y;

        node->idx_nav.nav_x_packed = (node->idx_nav.nav_x_packed & 0x7FFF) | target_y_low_bit;
        packed_y = node->u8_u.nav_y_packed;
        packed_y &= 0xFF00;
        packed_y |= node->layout.s.layout_y_hi;
        node->u8_u.nav_y_packed = packed_y;
    }
    else
    {
        u16 nav_x_packed = node->idx_nav.nav_x_packed;
        u16 nav_y_packed = node->u8_u.nav_y_packed;
        s32 current_y_low_bit = nav_x_packed >> 15;
        s32 current_y = ((nav_y_packed & 0xFF) << 1) | current_y_low_bit;
        u16 target_y = (nav_y_packed >> 15) | (node->layout.s.layout_y_hi << 1);

        if (((u16)current_y) == target_y)
        {
            node->layout_frames_remaining = MENU_NODE_LAYOUT_IDLE;
        }
        else
        {
            s32 step = ((s32)target_y - (u16)current_y) / node->layout_frames_remaining;
            u32 new_y = current_y + step;
            u32 new_y_hi;
            u32 new_y_low_bit;
            new_y &= 0xFFFF;
            new_y_hi = (new_y >> 1) & 0xFF;
            new_y_low_bit = (new_y & 1) << 15;
            node->idx_nav.nav_x_packed = (nav_x_packed & 0x7FFF) | new_y_low_bit;
            node->layout_frames_remaining -= 1;
            node->u8_u.nav_y_packed = (node->u8_u.nav_y_packed & 0xFF00) | new_y_hi;
        }
    }

    tree_base = g_menu_nodes;
    tree_node = tree_base + node_index;

    if ((tree_node->u2.unk2 >> 1) & 1)
    {
        s32 child_slot = 0;
        s32 child_sentinel;
        child_source = tree_node;

        for (; child_slot < MENU_MAX_CHILDREN; child_slot++)
        {
            child_sentinel = MENU_NONE;

            if (child_source->layout.s.children[child_slot] == (u8)child_sentinel)
            {
                break;
            }

            next_prim = menu_draw_node_recursive(child_source->layout.s.children[child_slot], next_prim, ot);
            child_sentinel = 0;
        }
    }

    return next_prim;
}

/**
 * @brief Emit the sprite primitives for one menu icon.
 * @param prim_buf Primitive-buffer cursor.
 * @param ot Ordering-table entry to update.
 * @param icon_id Icon definition index.
 * @param x Base X coordinate.
 * @param y Base Y coordinate.
 * @param secondary_offset Nonzero to emit the secondary sprite and offset the primary sprite.
 * @param animation_offset Pixel offset applied to the animated sprite positions.
 * @param active Nonzero for an opaque secondary sprite; zero enables its semitransparency.
 * @param style_bits Packed node style bits; currently unused.
 * @return Next free primitive-buffer address.
 */
void* menu_emit_icon_sprite(void* prim_buf, u_long* ot, s32 icon_id, s32 x, s32 y, s32 secondary_offset, s32 animation_offset, s32 active, s32 style_bits)
{
    SPRT* sprite = (SPRT*)prim_buf;
    s32 sprite_x;

    (void)style_bits;
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    sprite_x = x - secondary_offset;
    setXY0(sprite, (s16)(sprite_x + animation_offset), (s16)((y - secondary_offset) + animation_offset));
    setUV0(sprite, g_menu_icon_sprite_defs[icon_id].u_coord, g_menu_icon_sprite_defs[icon_id].v_coord);
    setWH(sprite, (s16)g_menu_icon_sprite_defs[icon_id].w, (s16)g_menu_icon_sprite_defs[icon_id].h);
    SET_SPRT_CLUT(sprite, menu_icon_clut(g_menu_icon_clut_codes[icon_id]));
    addPrim((u_long*)ot, sprite);
    sprite += 1;

    if (secondary_offset != 0)
    {
        if (active != 0)
        {
            SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0, 0, 0xA0));
        }
        else
        {
            SET_BGR0_PACKED(sprite, 0);
        }

        setSprt(sprite);

        if (active == 0)
        {
            setSemiTrans(sprite, 1);
        }

        setXY0(sprite, (s16)(x + (secondary_offset - animation_offset) * 2), (s16)(y + (secondary_offset - animation_offset) * 2));
        setUV0(sprite, g_menu_icon_sprite_defs[icon_id].u_coord, g_menu_icon_sprite_defs[icon_id].v_coord);
        setWH(sprite, (s16)g_menu_icon_sprite_defs[icon_id].w, (s16)g_menu_icon_sprite_defs[icon_id].h);
        SET_SPRT_CLUT(sprite, menu_icon_clut(g_menu_icon_clut_codes[icon_id]));
        addPrim((u_long*)ot, sprite);
        sprite += 1;
    }

    return sprite;
}

/**
 * @brief Emit a single 16x16 gray SPRT primitive and OT-link it.
 * @param prim_buf Current primitive buffer pointer; must have at least 0x14 bytes of space.
 * @param ot Pointer to the ordering-table entry; updated to prepend this SPRT.
 * @param x X screen position of the sprite.
 * @param y Y screen position of the sprite.
 * @return Pointer to the next free byte after the emitted 0x14-byte SPRT.
 */
void* menu_emit_swap_marker(void* prim_buf, u_long* ot, s16 x, s16 y)
{
    SPRT* sprite = (SPRT*)prim_buf;

    SET_BGR0_PACKED(sprite, GPU_COLOR_WORD(0x50, 0x50, 0x50));
    setSprt(sprite);
    SET_SPRT_WH_PACKED(sprite, 16, 16);
    SET_SPRT_UV0_PACKED(sprite, 0x80);
    setXY0(sprite, x, y);
    SET_SPRT_CLUT(sprite, getClut(0x60, VRAM_CLUT_Y));
    addPrim((u_long*)ot, sprite);
    return sprite + 1;
}

/**
 * @brief Handle list navigation and draw its cursor.
 * @param prim_buf Primitive-buffer cursor.
 * @param ot Ordering-table entry for the cursor.
 * @param state List state to update.
 * @param entries Packed row positions and circular navigation links.
 * @param view_origin Viewport origin in list coordinates.
 * @param active Nonzero to process input and animate the cursor.
 * @return Primitive-buffer cursor after drawing.
 */
void* scroll_list_draw(void* prim_buf, u_long* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active)
{
    int steps_remaining;

    if (active)
    {
        if (g_pad_input & PADR1)
        {
            g_pad_input = PADLdown;
            steps_remaining = (state->viewport_h - MENU_LIST_ROW_HEIGHT) >> MENU_LIST_ROW_SHIFT;
        }
        else if (g_pad_input & PADL1)
        {
            g_pad_input = PADLup;
            steps_remaining = (state->viewport_h - MENU_LIST_ROW_HEIGHT) >> MENU_LIST_ROW_SHIFT;
        }
        else
        {
            steps_remaining = 1;
        }

        while (steps_remaining != 0)
        {
            if (g_pad_input & MENU_PAD_VERTICAL)
            {
                if (g_pad_input & PADLup)
                {
                    state->navigation.fields.selected_index =
                        (entries[state->navigation.fields.selected_index] >> MENU_ITEM_NAV_PREVIOUS_SHIFT) & MENU_ITEM_NAV_INDEX_MASK;
                }
                else
                {
                    state->navigation.fields.selected_index = entries[state->navigation.fields.selected_index] >> MENU_ITEM_NAV_NEXT_SHIFT;
                }

                scroll_list_update_target(state, entries);

                if (state->navigation.fields.selected_index == ((state->navigation.fields.count_and_ot & MENU_ITEM_NAV_INDEX_MASK) - 1))
                {
                    steps_remaining = 1;
                }

                if (state->navigation.fields.selected_index == 0)
                {
                    steps_remaining = 1;
                }
            }

            steps_remaining--;
        }

        if (g_pad_input & MENU_PAD_VERTICAL)
        {
            menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
        }

        if (g_pad_input & PADLleft)
        {
            g_pad_input |= PAD_BTN_CIRCLE;
        }
    }

    prim_buf = menu_emit_cursor(prim_buf, ot, (4 - view_origin->x) - state->scroll_x,
                                ((entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - view_origin->y) - state->scroll_y, active);
    g_menu_default_view_pos.x = (state->base_x + ((4 - (u32)view_origin->x) - state->scroll_x)) + 8;
    g_menu_default_view_pos.y =
        (state->base_y + (((entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - view_origin->y) - state->scroll_y)) + 8;
    return prim_buf;
}

/**
 * @brief Set scroll targets to keep the selected row visible.
 * @param state List state to update.
 * @param entries Packed row positions and circular navigation links.
 */
void scroll_list_update_target(ScrollListState* state, u32* entries)
{
    s32 item_y;

    if (4 - state->scroll_x > state->viewport_w - 32)
    {
        state->target_x = 4 - 32 - state->viewport_w;
    }

    if (4 - state->scroll_x < 0)
    {
        state->target_x = 4;
    }

    if ((s32)((entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - state->scroll_y) > state->viewport_h - 32)
    {
        state->target_y = (entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK) - state->viewport_h + 32;
    }

    item_y = entries[state->navigation.fields.selected_index] & MENU_ITEM_NAV_POSITION_MASK;

    if (item_y - state->scroll_y < 0)
    {
        state->target_y = item_y;
    }

    state->lerp_steps = 4;
}

/**
 * @brief Draw the menu cursor with an optional animated shadow.
 * @param prim_buf Primitive write cursor; the SPRTs are built here.
 * @param ot Ordering-table entry; updated after each emitted primitive.
 * @param x Cursor screen X before the bob offset is applied.
 * @param y Cursor screen Y before the bob offset is applied.
 * @param active Nonzero to animate the cursor and draw its semitransparent shadow.
 * @return Primitive-buffer cursor after the sprites and draw-mode packet.
 */
void* menu_emit_cursor(void* prim_buf, u_long* ot, s32 x, s32 y, s32 active)
{
    SPRT* sprite = prim_buf;
    DR_TPAGE* draw_mode;
    u8* icon_id;
    MenuIconSpriteInfo* sprite_defs;
    u8* clut_codes;
    s32 bob_phase;

    if (active == 0 || g_menu_frame < 9)
    {
        bob_phase = 0;
    }
    else
    {
        if (g_menu_frame < 16)
        {
            bob_phase = 1;
        }
        else if (g_menu_frame < 23)
        {
            bob_phase = 2;
        }
        else if (g_menu_frame < 30)
        {
            bob_phase = 1;
        }
        else
        {
            bob_phase = 0;
            g_menu_frame = 0;
        }
    }

    sprite_defs = g_menu_icon_sprite_defs;
    icon_id = &g_menu_cursor_icon_ids[bob_phase];
    clut_codes = g_menu_icon_clut_codes;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setXY0(sprite, x - bob_phase, y + bob_phase);
    setUV0(sprite, sprite_defs[*icon_id].u_coord, sprite_defs[*icon_id].v_coord);
    setWH(sprite, sprite_defs[*icon_id].w, sprite_defs[*icon_id].h);
    SET_SPRT_CLUT(sprite, menu_icon_clut(clut_codes[*icon_id]));
    addPrim(ot, sprite);
    sprite++;

    if (active != 0)
    {
        SET_BGR0_PACKED(sprite, 0);
        setSprt(sprite);
        setSemiTrans(sprite, 1);
        setXY0(sprite, (x - bob_phase) + 2, (y + bob_phase) + 2);
        setUV0(sprite, sprite_defs[*icon_id].u_coord, sprite_defs[*icon_id].v_coord);
        setWH(sprite, sprite_defs[*icon_id].w, sprite_defs[*icon_id].h);
        SET_SPRT_CLUT(sprite, menu_icon_clut(clut_codes[*icon_id]));
        addPrim(ot, sprite);
        sprite++;
    }

    draw_mode = (DR_TPAGE*)sprite;
    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Draw the current inventory category and handle item selection.
 * @param ot Ordering table the primitives are linked into.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input this frame.
 * @return The advanced primitive write cursor.
 */

void* menu_inventory_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, s32 active)
{
    ScrollListState* list;
    void* packet_cursor;
    s32 scroll_y;
    void* pending_item_record;
    s32 item_index;
    s32 y;
    FieldItemRecord* item_record;

    MenuSlotRect rect;
    u8 item_name_buffer[0x40];

    list = state;
    packet_cursor = prim_buf;

    if ((g_pad_input & 0x10) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

        switch (g_menu_active_item_category)
        {
            case 0:
            {
                s32 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;

                switch (v)
                {
                    case 0x21:
                    case 0x22:
                    case 0x23:
                        g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = v + 1;
                        break;
                    case 0x13:
                    case 0x24:
                        g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x21;
                        break;
                }

                g_menu_nodes[g_menu_scene_type].label_id = g_menu_list_page_label_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                break;
            }
            case 1:
            {
                s32 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;

                switch (v)
                {
                    case 0x25:
                    case 0x26:
                        g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = v + 1;
                        break;
                    case 0x16:
                    case 0x27:
                        g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x25;
                        break;
                }

                g_menu_nodes[g_menu_scene_type].label_id = g_menu_list_page_label_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                break;
            }
            case 2:
            {
                u8 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;

                switch (v)
                {
                    case 0x28:
                        g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x29;
                        break;
                    case 0x29:
                    case 0x19:
                        g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x28;
                        break;
                }

                g_menu_nodes[g_menu_scene_type].label_id = g_menu_technique_page_label_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                break;
            }
        }
    }

    packet_cursor = scroll_list_draw(packet_cursor, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if (g_menu_scene_type < 0x13 && (g_pad_input & 0x8000))
    {
        g_pad_input &= ~0x40;
    }

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);

        if (g_menu_scene_type < 0x13)
        {
            s32 item_kind;
            g_menu_content_ready = 0;
            item_kind = g_menu_active_item_category;

            if (item_kind == 2)
            {
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = (s8)item_kind;
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = (u8)((g_menu_char_slot * 3) + 2);
                menu_clear_slots();
                return packet_cursor;
            }

            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 1;
            g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = (u8)((g_menu_char_slot * 3) + 3);
            menu_clear_slots();
            return packet_cursor;
        }

        if (g_menu_pending_item_row != MENU_NONE)
        {
            g_menu_pending_item_row = MENU_NONE;
        }
        else
        {
            g_menu_nodes[0x13].label_id = 0x11;
            g_menu_nodes[0x16].label_id = 0x14;
            g_menu_nodes[0x19].label_id = 0x16;
            g_menu_content_ready = 0;
            menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
            menu_reset_content_view();
            g_pad_input = 0;
        }
    }

    item_index = 0;
    y = item_index;
    scroll_y = list->scroll_y;
    g_menu_item_ptr = 0;
    g_menu_category0_item = 0;
    g_menu_category1_item = 0;
    g_menu_category2_item = 0;
    item_record = (FieldItemRecord*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET);

    while (item_index < MENU_ITEM_TABLE_COUNT)
    {
        u32 item_attributes;
        s32 item_kind;
        s32 icon_id;

        if (item_record->name[0] == 0)
        {
            break;
        }

        item_attributes = item_record->info.word;
        item_kind = FIELD_ITEM_CATEGORY(item_attributes);
        icon_id = 0x45;

        if (item_kind == g_menu_active_item_category)
        {
            switch (item_kind)
            {
                case 0:
                    icon_id = FIELD_ITEM_TYPE(item_attributes) + 0x45;
                    break;
                case 1:
                    icon_id = FIELD_ITEM_TYPE(item_attributes) + 0x50;
                    break;
                case 2:
                    icon_id = FIELD_ITEM_TYPE(item_attributes) + 0x5C;
                    break;
            }

            if ((y - scroll_y) > -MENU_LIST_ROW_HEIGHT && (y - scroll_y) < (list->viewport_h - MENU_LIST_ROW_HEIGHT))
            {
                u32 display_attributes;
                s32 text_palette;
                u8 ability_flags;

                packet_cursor = menu_emit_icon_sprite(packet_cursor, ot, icon_id, 0x10 - view_origin->x, (y - scroll_y) - view_origin->y, 0, 0, 0, 0);
                setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, MENU_GRID_TPAGE);
                addPrim(ot, (DR_TPAGE*)packet_cursor);
                packet_cursor = (DR_TPAGE*)packet_cursor + 1;

                display_attributes = item_record->info.word;

                if (display_attributes & FIELD_ITEM_CATEGORY_BITS)
                {
                    ability_flags = g_field_armor_type_conflicts[FIELD_ITEM_TYPE(display_attributes)];
                }
                else
                {
                    ability_flags = g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(display_attributes)];
                }

                text_palette = FIELD_TEXT_COLOR_MENU;

                if (ability_flags & (u8)g_menu_equipment_conflicts)
                {
                    text_palette = FIELD_TEXT_COLOR_MENU_DIM;
                }

                packet_cursor = field_draw_text(packet_cursor, ot, (u8*)item_record, text_palette, 0x22 - view_origin->x, (y - scroll_y) - view_origin->y,
                                                FIELD_TEXT_ALIGN_LEFT);

                if (g_menu_pending_item_row != MENU_NONE)
                {
                    if ((y >> MENU_LIST_ROW_SHIFT) == g_menu_pending_item_row)
                    {
                        s32 view_x = view_origin->x;
                        s32 view_y = view_origin->y;
                        SET_BGR0_PACKED((SPRT*)packet_cursor, GPU_COLOR_WORD(0x50, 0x50, 0x50));
                        setSprt((SPRT*)packet_cursor);
                        SET_SPRT_UV0_PACKED((SPRT*)packet_cursor, 0x80);
                        SET_SPRT_CLUT((SPRT*)packet_cursor, getClut(0x60, VRAM_CLUT_Y));
                        SET_SPRT_WH_PACKED((SPRT*)packet_cursor, 16, 16);
                        SET_YX0((SPRT*)packet_cursor, (y - scroll_y) - view_y, -2 - view_x);
                        addPrim(ot, (SPRT*)packet_cursor);
                        packet_cursor = (SPRT*)packet_cursor + 1;
                        setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, MENU_GRID_TPAGE);
                        addPrim(ot, (DR_TPAGE*)packet_cursor);
                        packet_cursor = (DR_TPAGE*)packet_cursor + 1;
                    }
                }
            }

            if (g_menu_pending_item_row != MENU_NONE && (y >> MENU_LIST_ROW_SHIFT) == g_menu_pending_item_row)
            {
                pending_item_record = &((FieldItemRecord*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET))[item_index];
            }

            if ((y >> MENU_LIST_ROW_SHIFT) == list->navigation.fields.selected_index)
            {
                g_menu_inventory_index = item_index;
                g_menu_item_ptr = &((FieldItemRecord*)((u8*)g_saved_game_ctx + MENU_ITEM_TABLE_OFFSET))[item_index];

                switch (g_menu_active_item_category)
                {
                    case 0:
                        g_menu_category0_item = g_menu_item_ptr;
                        break;
                    case 1:
                        g_menu_category1_item = g_menu_item_ptr;
                        break;
                    case 2:
                        g_menu_category2_item = g_menu_item_ptr;
                        break;
                }
            }

            y += MENU_LIST_ROW_HEIGHT;
        }

        item_index += 1;
        item_record += 1;
    }

    if ((y - scroll_y) <= 0 && list->lerp_steps == 0)
    {
        u16 cur = list->scroll_y;

        if (cur != 0)
        {
            list->target_y = cur - 0x10;
            list->lerp_steps = 4;
        }
    }

    list->navigation.packed = (list->navigation.packed & ~MENU_LIST_COUNT_MASK) |
                              ((menu_build_inventory_nav_entries(g_menu_active_item_category) & MENU_ITEM_NAV_INDEX_MASK) << MENU_LIST_COUNT_SHIFT);
    g_menu_page_count = (list->navigation.packed >> MENU_LIST_COUNT_SHIFT) & MENU_ITEM_NAV_INDEX_MASK;
    g_script_repeat_last = list->navigation.fields.selected_index;

    if (list->navigation.packed & MENU_LIST_COUNT_MASK)
    {
        if ((u32)((list->navigation.packed >> MENU_LIST_COUNT_SHIFT) & MENU_ITEM_NAV_INDEX_MASK) <= (u32)list->navigation.fields.selected_index)
        {
            menu_step_item_selection(-1);
            list->navigation.fields.selected_index = (u16)((list->navigation.fields.count_and_ot & 0x1FF) - 1);

            if (((list->navigation.fields.selected_index * MENU_LIST_ROW_HEIGHT) - scroll_y) < (list->viewport_h - MENU_LIST_ROW_HEIGHT))
            {
                u16 cur = list->scroll_y;

                if (cur != 0)
                {
                    list->target_y = cur - 0x10;
                    list->lerp_steps = 4;
                }
            }
        }

        g_menu_page_count = list->navigation.fields.count_and_ot & 0x1FF;
        g_script_repeat_last = list->navigation.fields.selected_index;

        if (g_menu_item_ptr != 0)
        {
            if (active != 0)
            {
                u32 item_attributes;
                u32 item_kind;

                if (menu_item_differs_from_bare_hands(g_menu_item_ptr) != 0)
                {
                    u8* resources = g_menu_state_ptr;
                    u16 name_attributes = g_menu_item_ptr->info.halves[1];
                    s32 name_offset = (name_attributes & 0x3F) * 2;
                    s32 names_offset = ((MenuTextResources*)resources)->table_offsets[MENU_TEXT_ITEM_NAMES];
                    s32 general_offset = ((MenuTextResources*)resources)->table_offsets[MENU_TEXT_GENERAL];
                    uintptr_t name_table = (uintptr_t)resources + names_offset;
                    u8* name = (u8*)(name_table + *(u16*)(name_offset + name_table));
                    u8* general_table = resources + general_offset;
                    u8* suffix = menu_text_entry(general_table, 109);
                    u8* text_cursor = item_name_buffer;
                    menu_copy_encoded_pair(text_cursor, name, suffix);
                }
                else
                {
                    item_name_buffer[0] = 0;
                }

                item_attributes = g_menu_item_ptr->info.word;
                item_kind = FIELD_ITEM_CATEGORY(item_attributes);

                switch (item_kind)
                {
                    case 0:
                    {
                        u32 category_offset = (item_attributes >> 9) & 0x7E;
                        uintptr_t description_table = (uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES);
                        u8* description = (u8*)(description_table + *(u16*)(category_offset + description_table));

                        u8* text_cursor = g_menu_item_description_buffer;
                        u8* name_prefix = item_name_buffer;
                        menu_copy_encoded_pair(text_cursor, name_prefix, description);
                        break;
                    }
                    case 1:
                    {
                        u32 category_offset = (item_attributes >> 9) & 0x7E;
                        uintptr_t description_table = (uintptr_t)menu_text_table_base(MENU_TEXT_EQUIPMENT_TYPE_NAMES);
                        u8* description = (u8*)(description_table + *(u16*)(category_offset + description_table + 11 * sizeof(u16)));

                        u8* text_cursor = g_menu_item_description_buffer;
                        u8* name_prefix = item_name_buffer;
                        menu_copy_encoded_pair(text_cursor, name_prefix, description);
                        break;
                    }
                    default:
                    {
                        s32* description_offset = &((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_EQUIPMENT_TYPE_NAMES];
                        u32 item_attributes = g_menu_item_ptr->info.word;
                        u32 category_offset = (item_attributes >> 9) & 0x7E;
                        uintptr_t description_table = (uintptr_t)g_menu_state_ptr + *description_offset;
                        u8* description = (u8*)(description_table + *(u16*)(category_offset + description_table + 23 * sizeof(u16)));

                        u8* text_cursor = g_menu_item_description_buffer;
                        u8* name_prefix = item_name_buffer;
                        menu_copy_encoded_pair(text_cursor, name_prefix, description);
                        break;
                    }
                }

                g_menu_help_text = (u8*)g_menu_item_description_buffer;
            }
        }

        if (g_pad_input & PAD_CONFIRM_BUTTONS)
        {
            if (active != 0)
            {
                if (g_menu_scene_type < 0x13)
                {
                    s32 sub = g_menu_active_item_category;

                    if (sub == 2)
                    {
                        menu_swap_item_records(g_menu_item_ptr, g_menu_active_equipped_item);
                        field_compact_inventory();
                        *(menu_character_block() + g_menu_active_subtype + MENU_ACTION_SLOT_OFFSET) =
                            (s8)((u8)g_menu_active_subtype + (FIELD_SKILL_INSTRUMENT - MENU_SKILL_SUBTYPE_BASE));
                        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = (s8)sub;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = (u8)((g_menu_char_slot * 3) + 2);
                        menu_clear_slots();
                        g_menu_content_ready = 0;
                    }
                    else
                    {
                        u32 w = g_menu_item_ptr->info.word;
                        u8 ability_flags;

                        if (w & FIELD_ITEM_CATEGORY_BITS)
                        {
                            ability_flags = g_field_armor_type_conflicts[FIELD_ITEM_TYPE(w)];
                        }
                        else
                        {
                            ability_flags = g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(w)];
                        }

                        if (ability_flags & (u8)g_menu_equipment_conflicts)
                        {
                            menu_play_se(SFX_ERROR, AKAO_SFX_DEFAULT_VOLUME);
                            return packet_cursor;
                        }

                        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
                        menu_clear_slots();

                        if (menu_item_differs_from_bare_hands(g_menu_active_equipped_item) != 0)
                        {
                            menu_swap_item_records(g_menu_item_ptr, g_menu_active_equipped_item);
                            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = g_menu_item_ptr;
                            g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
                        }
                        else
                        {
                            field_copy_inventory_record(g_menu_active_equipped_item, g_menu_item_ptr);
                            g_menu_item_ptr->name[0] = 0;
                            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 0;
                            g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
                        }

                        menu_clear_slots();
                        rect.x = 0xB0;
                        rect.y = 0x40;
                        rect.w = 0x70;
                        rect.h = 0x30;
                        list = (ScrollListState*)menu_slot_alloc(0, &rect);
                        ((MenuSlot*)list)->content_cb = &menu_equipment_compare_callback;
                        g_menu_compare_window_active = 1;
                        ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);
                        menu_relink_pair();
                        g_menu_content_ready = 0;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 1;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = (u8)((g_menu_char_slot * 3) + 3);
                        return packet_cursor;
                    }
                }
                else
                {
                    u16 sel;

                    g_menu_nodes[0x13].label_id = 0x11;
                    g_menu_nodes[0x16].label_id = 0x14;
                    g_menu_nodes[0x19].label_id = 0x16;
                    menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
                    sel = list->navigation.fields.selected_index;

                    if (sel != g_menu_pending_item_row)
                    {
                        if (g_menu_pending_item_row != MENU_NONE)
                        {
                            menu_swap_item_records(pending_item_record, g_menu_item_ptr);
                            g_menu_pending_item_row = MENU_NONE;
                        }
                        else
                        {
                            g_menu_pending_item_row = sel;
                        }
                    }
                    else
                    {
                        rect.x = 0xB0;
                        rect.y = 0x60;
                        rect.w = 0x70;
                        rect.h = 0x30;
                        list = (ScrollListState*)menu_slot_alloc(0, &rect);
                        ((MenuSlot*)list)->content_cb = &menu_item_action_callback;
                        ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);
                        menu_relink_pair();
                    }
                }
            }
        }
    }

    return packet_cursor;
}

/**
 * @brief Clear every skill slot of the active character that holds a technique; instruments stay.
 * @return 1 if at least one technique was removed, 0 if none were.
 */
s32 menu_remove_technique_skills(void)
{
    s32 changed;
    s32 i;

    changed = 0;

    for (i = 0; i < FIELD_SKILL_SLOT_COUNT; i++)
    {
        u8* skill = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + i;

        if (skill[MENU_SKILL_OFFSET] != FIELD_SKILL_NONE)
        {
            if ((skill[MENU_SKILL_OFFSET] & FIELD_SKILL_INSTRUMENT) == 0)
            {
                skill[MENU_SKILL_OFFSET] = FIELD_SKILL_NONE;
                changed = 1;
            }
        }
    }

    return changed;
}

/**
 * @brief Scan the item table for the next entry whose kind matches g_menu_active_item_category.
 * @param step Direction/stride to walk the table (1 = forward, -1 = backward).
 * @return Selected item-table index.
 */
u32 menu_step_item_selection(s32 step)
{
    u32 start;
    u32 index;
    s32 kind;
    FieldItemRecord* item;
    u32 result;
    u8* base;

    g_menu_item_ptr = 0;
    base = (u8*)g_saved_game_ctx;
    start = g_menu_inventory_index + step;
    item = (FieldItemRecord*)(base + ((start * MENU_ITEM_RECORD_SIZE) + MENU_ITEM_TABLE_OFFSET));
    index = start;

    while (index < MENU_ITEM_TABLE_COUNT)
    {
        if (item->name[0] != 0)
        {
            kind = FIELD_ITEM_CATEGORY(item->info.word);

            if (kind == g_menu_active_item_category)
            {
                g_menu_inventory_index = index;
                g_menu_item_ptr = (FieldItemRecord*)((u8*)g_saved_game_ctx + ((index * MENU_ITEM_RECORD_SIZE) + MENU_ITEM_TABLE_OFFSET));

                switch (kind)
                {
                    case 0:
                        g_menu_category0_item = g_menu_item_ptr;
                        break;
                    case 1:
                        g_menu_category1_item = g_menu_item_ptr;
                        break;
                    case 2:
                        g_menu_category2_item = g_menu_item_ptr;
                        break;
                }

                return index;
            }
        }

        index += step;
        item += step;
    }

    result = (uintptr_t)g_menu_item_ptr;

    if (result == 0)
    {
        if (step == 1)
        {
            g_menu_inventory_index = -1;
        }
        else
        {
            g_menu_inventory_index = 0x64;
        }

        result = menu_step_item_selection(step);
    }

    return result;
}

/**
 * @brief Draw the learned abilities and assign the chosen one to the active battle-action slot.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_ability_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    s32 selected_index;
    s32 byte_index;
    s32 bit_index;
    s32 bit_mask;
    s32 y;
    s32 relative_y;
    u8* learned_byte;
    u32 scroll_y;
    void* name_table;
    void* help_table;
    ScrollListState* list;

    list = state;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
        g_pad_input = 0;
    }

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    y = 0;
    selected_index = -1;
    byte_index = 0;
    learned_byte = (u8*)g_saved_game_ctx->ability_bits;
    scroll_y = list->scroll_y;

    while (byte_index < MENU_ABILITY_BYTE_COUNT)
    {
        bit_index = 0;
        bit_mask = 1;

        while (bit_index < MENU_ABILITIES_PER_BYTE)
        {
            if (*learned_byte & bit_mask)
            {
                relative_y = y - scroll_y;

                if ((relative_y > -MENU_LIST_ROW_HEIGHT) && (relative_y < (list->viewport_h - MENU_LIST_ROW_HEIGHT)))
                {
                    name_table = menu_text_table_base(MENU_TEXT_ABILITY_NAMES);
                    prim_buf = field_draw_text(
                        prim_buf, ot, (void*)((u8*)name_table + *(u16*)((u8*)name_table + (bit_index * 2) + (byte_index * (MENU_ABILITIES_PER_BYTE * 2)))),
                        FIELD_TEXT_COLOR_MENU, 0x10 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                }

                if (list->navigation.fields.selected_index == (y >> MENU_LIST_ROW_SHIFT))
                {
                    selected_index = bit_index + (byte_index * MENU_ABILITIES_PER_BYTE);
                }

                y += MENU_LIST_ROW_HEIGHT;
            }

            bit_index++;
            bit_mask <<= 1;
        }

        byte_index++;
        learned_byte++;
    }

    if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        *((u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + g_menu_active_subtype + (byte_index = MENU_ACTION_SLOT_OFFSET)) =
            selected_index;
        menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }

    if (selected_index != -1)
    {
        help_table = menu_text_table_base(MENU_TEXT_ABILITY_HELP);
        g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
    }

    return prim_buf;
}

/**
 * @brief Draw the Event List (every event past its listed stage, with progress marks) and handle its input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x20 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_event_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    ScrollListState* list;
    u32 scroll_y;
    s32 selected_index;
    s32 word_index;
    s32 event_index;
    s32 list_y;
    s32 relative_y;
    s32 stage;
    u32 stages;
    void* name_table;
    void* help_table;
    DR_TPAGE* draw_mode;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    list_y = 0;
    scroll_y = list->scroll_y;
    selected_index = -1;

    for (word_index = 0; word_index < MENU_EVENT_WORD_COUNT; word_index++)
    {
        stages = g_saved_game_ctx->words[SAVED_GAME_WORD_EVENT_PROGRESS + word_index];

        for (event_index = 0; event_index < MENU_EVENTS_PER_WORD; event_index++)
        {
            stage = stages & MENU_EVENT_STAGE_MASK;

            if (stage >= MENU_EVENT_STAGE_LISTED)
            {
                relative_y = list_y - scroll_y;

                if ((relative_y > -MENU_LIST_ROW_HEIGHT) && (relative_y < (list->viewport_h - MENU_LIST_ROW_HEIGHT)))
                {
                    name_table = menu_text_table_base(MENU_TEXT_EVENT_NAMES);
                    prim_buf = field_draw_text(
                        prim_buf, ot, (void*)((u8*)name_table + *(u16*)((u8*)name_table + (event_index * 2) + (word_index * (MENU_EVENTS_PER_WORD * 2)))),
                        FIELD_TEXT_COLOR_MENU, 0x20 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                    if (stage >= MENU_EVENT_STAGE_MARKED)
                    {
                        if (stage >= MENU_EVENT_STAGE_COMPLETE)
                        {
                            prim_buf = menu_emit_icon_sprite(prim_buf, ot, 0x70, 0x10 - view_origin->x, relative_y - view_origin->y, 0, 0, 0, 0);
                        }
                        else
                        {
                            prim_buf = menu_emit_icon_sprite(prim_buf, ot, 0x69, 0x10 - view_origin->x, relative_y - view_origin->y, 0, 0, 0, 0);
                        }

                        draw_mode = prim_buf;
                        setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
                        addPrim(ot, draw_mode);
                        prim_buf = draw_mode + 1;
                    }
                }

                if (list->navigation.fields.selected_index == (list_y >> MENU_LIST_ROW_SHIFT))
                {
                    selected_index = event_index + (word_index * MENU_EVENTS_PER_WORD);
                }

                list_y += MENU_LIST_ROW_HEIGHT;
            }

            stages >>= MENU_EVENT_STAGE_BITS;
        }
    }

    if ((active != 0) && (selected_index != -1))
    {
        help_table = menu_text_table_base(MENU_TEXT_EVENT_HELP);
        g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
    }

    return prim_buf;
}

/**
 * @brief Draw the Items list (held item kinds and their quantities) and handle its input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y) and the quantity anchor is (0xC0 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_item_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    s32 item_index;
    s32 y;
    s32 relative_y;
    u32 scroll_y;
    u8* quantity;
    s32 selected_index;
    void* name_table;
    void* help_table;
    ScrollListState* list;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    y = 0;
    selected_index = -1;
    quantity = g_saved_game_ctx->item_counts;
    scroll_y = list->scroll_y;

    for (item_index = 0; item_index < FIELD_ITEM_KIND_COUNT; item_index += 1)
    {
        if (*quantity != 0)
        {
            relative_y = y - scroll_y;

            if ((relative_y > -MENU_LIST_ROW_HEIGHT) && (relative_y < (list->viewport_h - MENU_LIST_ROW_HEIGHT)))
            {
                pos.x = 0xC0 - (u16)view_origin->x;
                pos.y = relative_y - (u16)view_origin->y;

                name_table = menu_text_table_base(MENU_TEXT_ITEM_NAMES);
                prim_buf = field_draw_text(prim_buf, ot, ((u8*)name_table + ((u16*)name_table)[item_index]), FIELD_TEXT_COLOR_MENU, 0x10 - view_origin->x,
                                           relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                prim_buf = menu_draw_clamped_number(ot, prim_buf, *quantity, FIELD_TEXT_COLOR_MENU, &pos, FIELD_TEXT_ALIGN_RIGHT);
            }

            if (list->navigation.fields.selected_index == (y >> MENU_LIST_ROW_SHIFT))
            {
                selected_index = item_index;
            }

            y += MENU_LIST_ROW_HEIGHT;
        }

        quantity += 1;
    }

    if (selected_index != -1)
    {
        if (active != 0)
        {
            help_table = menu_text_table_base(MENU_TEXT_ITEM_HELP);
            g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
        }
    }

    return prim_buf;
}

/**
 * @brief Draw the Artifacts list (one entry per placed land) and handle its input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x20 - x, rel_y - y) and the marker origin is (0x10 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_artifact_list_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    u32 scroll_y;
    s32 selected_index;
    s32 land_index;
    s32 list_y;
    s32 relative_y;
    FieldLandRecord* land;
    void* name_table;
    void* help_table;
    DR_TPAGE* draw_mode;
    ScrollListState* list;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    list_y = 0;
    selected_index = -1;
    land = g_saved_game_ctx->lands;
    scroll_y = list->scroll_y;

    for (land_index = 0; land_index < FIELD_LAND_COUNT; land_index += 1)
    {
        if (land->flags & FIELD_LAND_ARTIFACT_OBTAINED)
        {
            relative_y = list_y - scroll_y;

            if ((relative_y > -MENU_LIST_ROW_HEIGHT) && (relative_y < (list->viewport_h - MENU_LIST_ROW_HEIGHT)))
            {
                name_table = menu_text_table_base(MENU_TEXT_ARTIFACT_NAMES);
                prim_buf = field_draw_text(prim_buf, ot, (void*)((u8*)name_table + ((u16*)name_table)[land_index]), FIELD_TEXT_COLOR_MENU,
                                           0x20 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                if (land->flags & FIELD_LAND_PLACED)
                {
                    prim_buf = menu_emit_icon_sprite(prim_buf, ot, 0x2C, 0x10 - view_origin->x, relative_y - view_origin->y, 0, 0, 0, 0);

                    draw_mode = prim_buf;
                    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
                    addPrim(ot, draw_mode);
                    prim_buf = draw_mode + 1;
                }
            }

            if (list->navigation.fields.selected_index == (list_y >> MENU_LIST_ROW_SHIFT))
            {
                selected_index = land_index;
            }

            list_y += MENU_LIST_ROW_HEIGHT;
        }

        land += 1;
    }

    if (selected_index != -1)
    {
        if (active != 0)
        {
            help_table = menu_text_table_base(MENU_TEXT_ARTIFACT_HELP);
            g_menu_help_text = (u8*)((u8*)help_table + ((u16*)help_table)[selected_index]);
        }
    }

    return prim_buf;
}

/**
 * @brief Draw the ST / Magic / Remove / Status window of a skill slot and run the chosen command.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor (unchanged on the early-return paths).
 */
void* menu_skill_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    s32 technique_nav;
    s32 selected_row;
    FieldItemRecord* free_record;
    s32 item_offset;
    u8* character_bytes;
    u8 skill;
    ScrollListState* list;
    void* buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }
    else if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

        switch (list->navigation.fields.selected_index)
        {
            case 0:
                list->anim_frame = 0;
                list->active = MENU_SLOT_STATE_FREE;
                rect.x = 0x40;
                rect.y = 0x60;
                rect.w = 0xF0;
                rect.h = 0x60;
                list = (ScrollListState*)menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
                ((MenuSlot*)list)->content_cb = &menu_special_technique_list_callback;
                technique_nav = menu_build_special_technique_nav_entries();
                selected_row = technique_nav >> 16;
                ((MenuSlot*)list)->lerp_target_b = selected_row * MENU_LIST_ROW_HEIGHT;
                ((MenuSlot*)list)->lerp_cur_b = selected_row * MENU_LIST_ROW_HEIGHT;
                ((MenuSlot*)list)->anim_frame = MENU_WINDOW_OPEN_FRAME;
                ((MenuSlot*)list)->active = MENU_SLOT_STATE_OPEN;
                ((MenuSlot*)list)->has_title = 1;
                g_menu_draw_early_out = 1;
                ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, technique_nav);
                ((MenuSlot*)list)->navigation.fields.selected_index = (u16)selected_row;
                return buf;

            case 1:
                menu_clear_slots();
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = 0x29;
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id =
                    g_menu_technique_page_label_ids[g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES]
                                                        .idx_nav.s.self_idx];
                g_menu_equipment_conflicts = 0;
                g_menu_content_ready = 1;
                menu_open_content_page(MENU_PAGE_INSTRUMENTS);
                g_menu_draw_early_out = 1;
                g_menu_item_ptr = NULL;
                g_menu_category0_item = NULL;
                g_menu_category1_item = NULL;
                g_menu_category2_item = NULL;
                return buf;

            case 2:
                character_bytes = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                character_bytes += g_menu_active_subtype;
                skill = character_bytes[MENU_ACTION_SLOT_OFFSET];

                if (skill == FIELD_SKILL_NONE)
                {
                    break;
                }

                /* An instrument goes back to the inventory; without room the slot keeps it. */
                if (skill & FIELD_SKILL_INSTRUMENT)
                {
                    free_record = field_find_free_inventory_record();

                    if (free_record != NULL)
                    {
                        field_copy_inventory_record(
                            free_record, (((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET)) +
                                          ((g_menu_active_subtype * MENU_ITEM_RECORD_SIZE) + (MENU_CHARACTER_ITEMS_OFFSET + MENU_ITEM_RECORD_SIZE))));
                        item_offset = ((g_menu_active_subtype + 1) * MENU_ITEM_RECORD_SIZE) + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                        character_bytes = (u8*)g_saved_game_ctx + item_offset;
                        character_bytes[MENU_FIRST_EQUIPMENT_OFFSET] = 0;
                        field_compact_inventory(item_offset);
                    }
                    else
                    {
                        menu_show_message(MENU_MESSAGE_INVENTORY_FULL);
                        return buf;
                    }
                }

                character_bytes = (u8*)g_saved_game_ctx + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + g_menu_active_subtype;
                character_bytes[MENU_ACTION_SLOT_OFFSET] = FIELD_SKILL_NONE;
                list->active = MENU_SLOT_STATE_CLOSING;
                break;

            case 3:
                if (g_menu_active_equipped_item->name[0] == 0)
                {
                    break;
                }

                menu_clear_slots();
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].idx_nav.s.self_idx = 0x1A;
                g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_ABILITIES].label_id = 0x17;
                menu_focus_active_content_item();

                g_menu_draw_early_out = 1;
                return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_TECHNIQUE), FIELD_TEXT_COLOR_MENU,
                          0x30 - view_origin->x, -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_MAGIC), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_REMOVE), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x20 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_STATUS), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x30 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draw the one-line message window; a confirm or cancel press closes it.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x88 - x, -y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_message_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    void* buf;

    buf = prim_buf;

    if ((g_pad_input & PAD_DISMISS_BUTTONS) && (active != 0))
    {
        state->anim_frame = 0;
        state->active = MENU_SLOT_STATE_FREE;
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
    }

    buf = field_draw_text(buf, ot, g_menu_message_line1, FIELD_TEXT_COLOR_MENU, 0x88 - view_origin->x, -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draw the two-line message window; a confirm or cancel press closes it.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origins are (0x88 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_two_line_message_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    void* buf;

    buf = prim_buf;

    if ((g_pad_input & PAD_DISMISS_BUTTONS) && (active != 0))
    {
        state->anim_frame = 0;
        state->active = MENU_SLOT_STATE_FREE;
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);
    }

    buf = field_draw_text(buf, ot, g_menu_message_line1, FIELD_TEXT_COLOR_MENU, 0x88 - view_origin->x, -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, g_menu_message_line2, FIELD_TEXT_COLOR_MENU, 0x88 - view_origin->x, 0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draw the Status / Trash window of an inventory item and run the chosen command.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_item_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    ScrollListState* list;
    Vec2s pos;
    void* buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }
    else if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

        if (list->navigation.fields.selected_index != 0)
        {
            /* Trash: drop the item record and close up the inventory. */
            list->active = MENU_SLOT_STATE_CLOSING;
            g_menu_item_ptr->name[0] = 0;
            field_compact_inventory();
            g_menu_pending_item_row = MENU_NONE;
        }
        else
        {
            /* Status: show the item's status page, the next scene node. */
            g_menu_scene_type++;
            g_menu_active_node = g_menu_scene_type;
            menu_clear_slots();
            menu_focus_active_content_item();
        }

        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_ITEM_STATUS), FIELD_TEXT_COLOR_MENU,
                          0x30 - view_origin->x, -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_TRASH), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Locate the equipment record of the active character's active equipment slot.
 * @return Equipment record selected by g_menu_active_subtype.
 */
static inline FieldItemRecord* menu_active_equipment(void)
{
    return (FieldItemRecord*)((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET) +
                              ((g_menu_active_subtype * MENU_ITEM_RECORD_SIZE) +
                               (MENU_CHARACTER_ITEMS_OFFSET - MENU_EQUIPMENT_SUBTYPE_BASE * MENU_ITEM_RECORD_SIZE)));
}

/**
 * @brief Locate the weapon record of the active character.
 * @return Equipment record in FIELD_WEAPON_SLOT.
 */
static inline FieldItemRecord* menu_weapon_record(void)
{
    return (FieldItemRecord*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (uintptr_t)g_saved_game_ctx + MENU_FIRST_EQUIPMENT_OFFSET);
}

/**
 * @brief Draw the Equip / Auto-Select / Total-Auto / Status / Remove window of an equipment slot and run the chosen command.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
void* menu_equipment_action_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    ScrollListState* list;
    u8* armor_record;
    FieldItemRecord* bare_hands;
    u8* saved_game;
    u8* equipment_base;
    s32 character_offset;
    u32 weapon_info;
    u32 bare_hands_info;
    void* buf;
    FieldItemRecord* free_record;
    FieldItemRecord* shield_record;
    s32 armor_offset;
    s32 equipment_offset;
    s32 equipment_index;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }
    else if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        menu_play_se(SFX_CURSOR, AKAO_SFX_DEFAULT_VOLUME);

        switch (list->navigation.fields.selected_index)
        {
            case 0:
                switch (g_menu_active_subtype)
                {
                    case MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT):
                    {
                        menu_clear_slots();
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x24;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id =
                            g_menu_list_page_label_ids[g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT]
                                                           .idx_nav.s.self_idx];

                        g_menu_equipment_conflicts = menu_get_equipment_conflicts(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);
                        g_menu_content_ready = 1;
                        menu_open_content_page(MENU_PAGE_WEAPONS);
                        g_menu_draw_early_out = 1;
                        return buf;
                    }

                    case MENU_EQUIPMENT_SUBTYPE(1):
                    case MENU_EQUIPMENT_SUBTYPE(2):
                    case MENU_EQUIPMENT_SUBTYPE(3):
                    {
                        menu_clear_slots();
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x27;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id =
                            g_menu_armor_page_label_ids[g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT]
                                                            .idx_nav.s.self_idx];

                        g_menu_equipment_conflicts = menu_get_equipment_conflicts(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);
                        g_menu_content_ready = 1;
                        menu_open_content_page(MENU_PAGE_ARMOR);
                        g_menu_draw_early_out = 1;
                        return buf;
                    }

                    default:
                        break;
                }

                break;

            case 1:
                if (g_menu_active_subtype == MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT))
                {
                    if (menu_stage_best_weapon() == 0)
                    {
                        break;
                    }

                    menu_clear_slots();
                    rect.x = 0xB0;
                    rect.y = 0x40;
                    rect.w = 0x70;
                    rect.h = 0x30;
                    list = (ScrollListState*)menu_slot_alloc(0, &rect);
                    ((MenuSlot*)list)->content_cb = &menu_equipment_compare_callback;
                    g_menu_compare_window_active = 1;
                    ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);

                    menu_relink_pair();
                    break;
                }

                g_item_slot_data[3] = 0;
                g_item_slot_data[2] = 0;
                g_item_slot_data[1] = 0;

                if (menu_stage_best_armor() == 0)
                {
                    break;
                }

                menu_clear_slots();
                rect.x = 0xB0;
                rect.y = 0x40;
                rect.w = 0x70;
                rect.h = 0x30;
                list = (ScrollListState*)menu_slot_alloc(0, &rect);
                ((MenuSlot*)list)->content_cb = &menu_equipment_compare_callback;
                g_menu_compare_window_active = 1;
                ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);

                menu_relink_pair();
                break;

            case 2:
                g_item_slot_data[3] = 0;
                g_item_slot_data[2] = 0;
                g_item_slot_data[1] = 0;
                menu_stage_best_weapon();
                g_menu_active_subtype = MENU_EQUIPMENT_SUBTYPE(1);
                menu_stage_best_armor();
                g_menu_active_subtype = MENU_EQUIPMENT_SUBTYPE(2);
                menu_stage_best_armor();
                g_menu_active_subtype = MENU_EQUIPMENT_SUBTYPE(3);
                menu_stage_best_armor();

                menu_clear_slots();
                rect.x = 0xB0;
                rect.y = 0x40;
                rect.w = 0x70;
                rect.h = 0x30;
                list = (ScrollListState*)menu_slot_alloc(0, &rect);
                ((MenuSlot*)list)->content_cb = &menu_equipment_compare_callback;
                g_menu_compare_window_active = 1;
                ((MenuSlot*)list)->navigation.packed = MENU_LIST_WITH_COUNT(((MenuSlot*)list)->navigation.packed, 2);

                menu_relink_pair();
                break;

            case 3:
                if (g_menu_active_equipped_item->name[0] == 0)
                {
                    break;
                }

                menu_clear_slots();

                switch (g_menu_active_subtype)
                {
                    case MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT):
                        g_menu_item_ptr = g_menu_active_equipped_item;
                        g_menu_category0_item = g_menu_saved_category0_item;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x14;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = 0x12;
                        break;

                    case MENU_EQUIPMENT_SUBTYPE(1):
                    case MENU_EQUIPMENT_SUBTYPE(2):
                    case MENU_EQUIPMENT_SUBTYPE(3):
                        g_menu_item_ptr = g_menu_active_equipped_item;
                        g_menu_category1_item = g_menu_saved_category1_item;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].idx_nav.s.self_idx = 0x17;
                        g_menu_nodes[(g_menu_char_slot * MENU_NODES_PER_CHARACTER) + MENU_CHARACTER_NODE_EQUIPMENT].label_id = 0x15;
                        break;
                }

                menu_focus_active_content_item();

                g_menu_draw_early_out = 1;
                return buf;

            case 4:
                if (g_menu_active_equipped_item->name[0] == 0)
                {
                    break;
                }

                free_record = field_find_free_inventory_record();

                if (free_record != NULL)
                {
                    if (menu_item_differs_from_bare_hands(menu_active_equipment()) != 0)
                    {
                        field_copy_inventory_record(free_record, menu_active_equipment());

                        switch (g_menu_active_subtype)
                        {
                            case MENU_EQUIPMENT_SUBTYPE(FIELD_WEAPON_SLOT):
                                g_menu_active_equipped_item = NULL;
                                g_menu_saved_category0_item = NULL;
                                character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                                saved_game = (u8*)g_saved_game_ctx;
                                weapon_info = ((FieldCharacterRecord*)(saved_game + character_offset + MENU_CHARACTER_RECORD_OFFSET))->equipment[0].info.word >>
                                              FIELD_ITEM_TYPE_SHIFT;
                                bare_hands = &g_field_bare_hands_item;
                                bare_hands_info = bare_hands->info.word >> FIELD_ITEM_TYPE_SHIFT;

                                /* Bare hands are a glove: removing a glove changes nothing else. */
                                if ((weapon_info & FIELD_ITEM_TYPE_MASK) == (bare_hands_info & FIELD_ITEM_TYPE_MASK))
                                {
                                    field_copy_inventory_record(
                                        &menu_character_record((SavedGameLayout*)saved_game, g_menu_char_slot)->equipment[FIELD_WEAPON_SLOT], bare_hands);
                                    break;
                                }

                                field_copy_inventory_record(
                                    &menu_character_record((SavedGameLayout*)saved_game, g_menu_char_slot)->equipment[FIELD_WEAPON_SLOT], bare_hands);

                                /* Bare hands cannot hold a shield: move it to the inventory, or undo the removal when there is no room. */
                                for (equipment_index = FIELD_ARMOR_SLOT; equipment_index < MENU_EQUIPMENT_SLOT_COUNT; equipment_index++)
                                {
                                    armor_offset = equipment_index * MENU_ITEM_RECORD_SIZE;
                                    armor_record = (u8*)g_saved_game_ctx + (armor_offset + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE));

                                    if (armor_record[MENU_FIRST_EQUIPMENT_OFFSET] != 0)
                                    {
                                        if (FIELD_ITEM_TYPE(*(u32*)(armor_record + MENU_FIRST_EQUIPMENT_OFFSET + OFFSETOF(FieldItemRecord, info))) ==
                                            FIELD_ARMOR_SHIELD)
                                        {
                                            shield_record = field_find_free_inventory_record();

                                            if (shield_record != NULL)
                                            {
                                                field_copy_inventory_record(
                                                    shield_record,
                                                    (((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET)) +
                                                     (armor_offset + MENU_CHARACTER_ITEMS_OFFSET)));
                                                menu_equipped_item(g_saved_game_ctx, equipment_index)->name[0] = 0;
                                                field_compact_inventory();
                                                break;
                                            }

                                            field_copy_inventory_record(menu_weapon_record(), free_record);
                                            free_record->name[0] = 0;
                                            menu_show_message(MENU_MESSAGE_INVENTORY_FULL);
                                            return buf;
                                        }
                                    }
                                }

                                if (menu_remove_technique_skills() == 0)
                                {
                                    break;
                                }

                                menu_show_message(MENU_MESSAGE_TECHNIQUES_REMOVED);
                                return buf;

                            case MENU_EQUIPMENT_SUBTYPE(1):
                            case MENU_EQUIPMENT_SUBTYPE(2):
                            case MENU_EQUIPMENT_SUBTYPE(3):
                                g_menu_active_equipped_item = NULL;
                                g_menu_saved_category1_item = NULL;
                                equipment_base = (u8*)g_saved_game_ctx;
                                equipment_offset = ((g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE) * MENU_ITEM_RECORD_SIZE) +
                                                   (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                                *(equipment_base + equipment_offset + MENU_FIRST_EQUIPMENT_OFFSET) = 0;
                                field_compact_inventory();
                                break;
                        }
                    }

                    list->active = MENU_SLOT_STATE_CLOSING;
                    break;
                }

                menu_show_message(MENU_MESSAGE_INVENTORY_FULL);
                return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_EQUIP), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_AUTO_SELECT), FIELD_TEXT_COLOR_MENU,
                          0x30 - view_origin->x, 0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_TOTAL_AUTO), FIELD_TEXT_COLOR_MENU,
                          0x30 - view_origin->x, 0x20 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_STATUS), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x30 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_REMOVE), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x40 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    return buf;
}

/**
 * @brief Draw the Change / Cancel window that follows Auto-Select and Total-Auto, and keep or undo the staged equipment.
 * @param ot Ordering-table pointer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input.
 * @return Updated primitive write cursor; the unchanged @p prim_buf when the window closes.
 */
void* menu_equipment_compare_callback(u_long* ot, ScrollListState* state, void* prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    ScrollListState* list;
    s32 equipment_index;
    FieldItemRecord* replaced_item;
    s32 character_offset;
    void* buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & PAD_DISMISS_BUTTONS) && (active != 0))
    {
        g_menu_compare_window_active = 0;

        if ((list->navigation.fields.selected_index != 0) || (g_pad_input & PAD_BTN_CIRCLE))
        {
            /* Cancel: put every staged slot back the way it was. */
            menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
            list->active = MENU_SLOT_STATE_CLOSING;

            for (equipment_index = 0; equipment_index < MENU_EQUIPMENT_SLOT_COUNT; equipment_index++)
            {
                if (g_item_slot_flags[equipment_index] != 0)
                {
                    replaced_item = g_item_slot_data[equipment_index];

                    if (replaced_item != NULL)
                    {
                        menu_swap_item_records(replaced_item, (FieldItemRecord*)(((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) +
                                                                                                           MENU_CHARACTER_RECORD_OFFSET)) +
                                                                                 ((equipment_index * MENU_ITEM_RECORD_SIZE) + MENU_CHARACTER_ITEMS_OFFSET)));
                    }
                    else if (equipment_index == FIELD_WEAPON_SLOT)
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(), menu_weapon_record());
                        field_copy_inventory_record(menu_weapon_record(), &g_field_bare_hands_item);
                    }
                    else
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(),
                                                    (((u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET)) +
                                                     ((equipment_index * MENU_ITEM_RECORD_SIZE) + MENU_CHARACTER_ITEMS_OFFSET)));
                        character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                        *((u8*)g_saved_game_ctx + ((equipment_index * MENU_ITEM_RECORD_SIZE) + character_offset) + MENU_FIRST_EQUIPMENT_OFFSET) = 0;
                    }
                }
            }
        }
        else
        {
            /* Change: keep the staged equipment; a new weapon type invalidates the assigned techniques. */
            menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
            list->active = MENU_SLOT_STATE_CLOSING;

            if (g_item_slot_flags[FIELD_WEAPON_SLOT] != 0)
            {
                if (((g_item_slot_data[FIELD_WEAPON_SLOT] != NULL) &&
                     ((g_item_slot_data[FIELD_WEAPON_SLOT]->info.word & FIELD_ITEM_TYPE_BITS) != (g_menu_equipment_base->info.word & FIELD_ITEM_TYPE_BITS))) ||
                    ((g_item_slot_data[FIELD_WEAPON_SLOT] == NULL) &&
                     ((g_field_bare_hands_info & FIELD_ITEM_TYPE_BITS) != (g_menu_equipment_base->info.word & FIELD_ITEM_TYPE_BITS))))
                {
                    if (menu_remove_technique_skills() != 0)
                    {
                        menu_show_message(MENU_MESSAGE_TECHNIQUES_REMOVED);
                    }
                }
            }
        }

        field_compact_inventory();
        g_item_slot_flags[0] = 0;
        g_item_slot_flags[1] = 0;
        g_item_slot_flags[2] = 0;
        g_item_slot_flags[3] = 0;
        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_CHANGE), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          -view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    buf = field_draw_text(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), MENU_MESSAGE_CANCEL), FIELD_TEXT_COLOR_MENU, 0x30 - view_origin->x,
                          0x10 - view_origin->y, FIELD_TEXT_ALIGN_CENTER);
    g_pad_input = 0;
    return buf;
}

/**
 * @brief Test whether an item record differs from FIELD's BareHands record.
 * @param item_record Item record to compare.
 * @return 1 as soon as a byte differs; 0 if the whole record is equal.
 */
inline s32 menu_item_differs_from_bare_hands(const FieldItemRecord* item_record)
{
    const u8* item;
    const u8* bare_hands;
    u32 i;

    item = (const u8*)item_record;
    bare_hands = (const u8*)&g_field_bare_hands_item;

    for (i = 0; i < MENU_ITEM_RECORD_SIZE; i++, bare_hands++, item++)
    {
        if (*bare_hands != *item)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Swap two item records through a stack buffer.
 * @param first_item First item record.
 * @param second_item Second item record.
 */
inline void menu_swap_item_records(FieldItemRecord* first_item, FieldItemRecord* second_item)
{
    u8 swap_buffer[MENU_ITEM_RECORD_SIZE];

    field_copy_inventory_record(swap_buffer, first_item);
    field_copy_inventory_record(first_item, second_item);
    field_copy_inventory_record(second_item, swap_buffer);
}

/**
 * @brief Point @ref g_active_slot at the highest-numbered menu slot still in use.
 */
void menu_update_active_slot(void)
{
    s32 i;

    for (i = 0; i < MENU_SLOT_COUNT; i++)
    {
        if (g_menu_slots[i].active != 0)
        {
            g_active_slot = i;
        }
    }
}

/**
 * @brief Draw the learned Special Technique list and assign the chosen technique to the active skill slot.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state_arg Scroll-list state for this page.
 * @param prim_arg Primitive buffer write cursor.
 * @param view_origin Viewport anchor; glyph origins are (0x10 - x, rel - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @note Only techniques of the equipped weapon's type can be assigned.
 */
void* menu_special_technique_list_callback(u_long* ot, ScrollListState* state_arg, void* prim_arg, Vec2s* view_origin, s32 active)
{
    ScrollListState* state = state_arg;
    void* prim = prim_arg;
    s32 selected_technique;
    s32 list_y;
    s32 weapon_type;
    s32 technique_index;
    s32 bit_mask;
    s32 learned_techniques;
    s32* learned_ptr;
    s32 relative_y;
    s32 scroll_y;
    u8 skill;
    FieldItemRecord* free_record;

    if ((g_pad_input & PAD_BTN_CIRCLE) && (active != 0))
    {
        menu_play_se(SFX_CANCEL, AKAO_SFX_DEFAULT_VOLUME);
        state->active = MENU_SLOT_STATE_CLOSING;
        return prim;
    }

    selected_technique = -1;
    list_y = 0;
    weapon_type = 0;
    learned_ptr = (s32*)g_saved_game_ctx->technique_bits;

    while (weapon_type < FIELD_WEAPON_CATEGORY_COUNT)
    {
        technique_index = 0;
        bit_mask = 1;
        learned_techniques = *learned_ptr;

        while (technique_index < FIELD_TECHNIQUES_PER_WEAPON)
        {
            if (learned_techniques & bit_mask)
            {
                if (state->navigation.fields.selected_index == (list_y >> MENU_LIST_ROW_SHIFT))
                {
                    selected_technique = technique_index + (weapon_type * FIELD_TECHNIQUES_PER_WEAPON);
                }

                list_y += MENU_LIST_ROW_HEIGHT;
            }

            technique_index++;
            bit_mask <<= 1;
        }

        weapon_type++;
        learned_ptr++;
    }

    if ((g_pad_input & PAD_CONFIRM_BUTTONS) && (active != 0))
    {
        if ((selected_technique / FIELD_TECHNIQUES_PER_WEAPON) == (s32)FIELD_ITEM_TYPE(g_menu_equipment_base->info.word))
        {
            skill = g_saved_game_ctx->characters[g_menu_char_slot].info.actions.skills[g_menu_active_subtype - MENU_SKILL_SUBTYPE_BASE];

            if (skill != FIELD_SKILL_NONE)
            {
                /* The slot holds an instrument: return its item record to the inventory first. */
                if (skill & FIELD_SKILL_INSTRUMENT)
                {
                    free_record = field_find_free_inventory_record();

                    if (free_record != NULL)
                    {
                        s32 item_offset;

                        field_copy_inventory_record(
                            free_record, (u8*)g_saved_game_ctx + ((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + MENU_CHARACTER_RECORD_OFFSET) +
                                             ((g_menu_active_subtype * MENU_ITEM_RECORD_SIZE) + (MENU_CHARACTER_ITEMS_OFFSET + MENU_ITEM_RECORD_SIZE)));
                        item_offset = ((g_menu_active_subtype + 1) * MENU_ITEM_RECORD_SIZE) + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                        *((u8*)g_saved_game_ctx + item_offset + MENU_FIRST_EQUIPMENT_OFFSET) = 0;
                        field_compact_inventory();
                    }
                    else
                    {
                        menu_show_message(MENU_MESSAGE_INVENTORY_FULL);
                        return prim;
                    }
                }
            }

            g_saved_game_ctx->characters[g_menu_char_slot].info.actions.skills[g_menu_active_subtype - MENU_SKILL_SUBTYPE_BASE] =
                selected_technique % FIELD_TECHNIQUES_PER_WEAPON;
            menu_play_se(SFX_CONFIRM, AKAO_SFX_DEFAULT_VOLUME);
            state->active = MENU_SLOT_STATE_CLOSING;
        }
        else
        {
            menu_play_se(SFX_ERROR, AKAO_SFX_DEFAULT_VOLUME);
        }
    }

    prim = scroll_list_draw(prim, ot, state, g_menu_scroll_nav_entries, view_origin, active);

    selected_technique = -1;
    list_y = 0;
    weapon_type = 0;
    learned_ptr = (s32*)g_saved_game_ctx->technique_bits;
    scroll_y = state->scroll_y;

    while (weapon_type < FIELD_WEAPON_CATEGORY_COUNT)
    {
        technique_index = 0;
        bit_mask = 1;

        while (technique_index < FIELD_TECHNIQUES_PER_WEAPON)
        {
            if (*learned_ptr & bit_mask)
            {
                relative_y = list_y - scroll_y;

                if (relative_y > -MENU_LIST_ROW_HEIGHT)
                {
                    if (relative_y < state->viewport_h - MENU_LIST_ROW_HEIGHT)
                    {
                        s32 glyph_color;
                        u8* name_table;
                        s32 name_offset;
                        u8* name;

                        /* Techniques of other weapon types are listed dimmed. */
                        glyph_color = FIELD_TEXT_COLOR_MENU_DIM;
                        name_table = g_menu_state_ptr;
                        name_table += ((MenuTextResources*)name_table)->table_offsets[MENU_TEXT_TECHNIQUE_NAMES];
                        name_offset = *(u16*)((u8*)name_table + (technique_index * 2) + (weapon_type * (FIELD_TECHNIQUES_PER_WEAPON * 2)));
                        name = name_table + name_offset;

                        if (weapon_type == (s32)FIELD_ITEM_TYPE(g_menu_equipment_base->info.word))
                        {
                            glyph_color = FIELD_TEXT_COLOR_MENU;
                        }

                        prim = field_draw_text(prim, ot, name, glyph_color, 0x10 - view_origin->x, relative_y - view_origin->y, FIELD_TEXT_ALIGN_LEFT);
                    }
                }

                if (state->navigation.fields.selected_index == (list_y >> MENU_LIST_ROW_SHIFT))
                {
                    selected_technique = technique_index + (weapon_type * FIELD_TECHNIQUES_PER_WEAPON);
                }

                list_y += MENU_LIST_ROW_HEIGHT;
            }

            technique_index++;
            bit_mask <<= 1;
        }

        weapon_type++;
        learned_ptr++;
    }

    if (selected_technique != -1)
    {
        u8* help_table = menu_text_table_base(MENU_TEXT_TECHNIQUE_HELP);

        g_menu_help_text = (u8*)(help_table + ((u16*)help_table)[selected_technique]);
    }

    return prim;
}

/**
 * @brief Open the menu content window for the given content page.
 * @param content_id MenuContentPage to open; any other value only resets g_menu_pending_item_row.
 */
void menu_open_content_page(u32 content_id)
{
    MenuSlotRect rect;
    MenuSlot* slot;
    s32 entry_count;

    g_menu_pending_item_row = MENU_NONE;

    switch (content_id)
    {
        case MENU_PAGE_WEAPONS:
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_inventory_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_inventory_nav_entries(FIELD_ITEM_CATEGORY_WEAPON);
            g_menu_active_item_category = FIELD_ITEM_CATEGORY_WEAPON;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_ARMOR:
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_inventory_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_inventory_nav_entries(FIELD_ITEM_CATEGORY_ARMOR);
            g_menu_active_item_category = FIELD_ITEM_CATEGORY_ARMOR;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_INSTRUMENTS:
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_inventory_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_inventory_nav_entries(FIELD_ITEM_CATEGORY_INSTRUMENT);
            g_menu_active_item_category = FIELD_ITEM_CATEGORY_INSTRUMENT;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_EVENTS:
            rect.x = 0x40;
            rect.y = 0x2C;
            rect.w = 0xE8;
            rect.h = 0x90;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_event_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_event_nav_entries();
            g_menu_active_item_category = MENU_ITEM_CATEGORY_NONE;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_ITEMS:
            rect.x = 0x40;
            rect.y = 0x2C;
            rect.w = 0xE8;
            rect.h = 0x80;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_item_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_item_nav_entries();
            g_menu_active_item_category = MENU_ITEM_CATEGORY_NONE;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_ARTIFACTS:
            rect.x = 0x40;
            rect.y = 0x2C;
            rect.w = 0xE8;
            rect.h = 0x90;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_artifact_list_callback;
            slot->has_title = 1;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            entry_count = menu_build_artifact_nav_entries();
            g_menu_active_item_category = MENU_ITEM_CATEGORY_NONE;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, entry_count);
            break;

        case MENU_PAGE_MESSAGE:
            rect.x = 0x10;
            rect.y = 0x60;
            rect.w = 0x120;
            rect.h = 0x20;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_message_callback;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, 1);

            menu_relink_item_nav(1);
            g_menu_draw_early_out = 1;
            break;

        case MENU_PAGE_TWO_LINE_MESSAGE:
            rect.x = 0x10;
            rect.y = 0x60;
            rect.w = 0x120;
            rect.h = 0x30;
            slot = menu_slot_alloc(MENU_CONTENT_OT_INDEX, &rect);
            slot->content_cb = &menu_two_line_message_callback;
            slot->anim_frame = MENU_WINDOW_OPEN_FRAME;
            slot->active = MENU_SLOT_STATE_OPEN;
            slot->navigation.packed = MENU_LIST_WITH_COUNT(slot->navigation.packed, 1);

            menu_relink_item_nav(1);
            g_menu_draw_early_out = 1;
            break;
    }
}

/**
 * @brief Build circular navigation for the Event List: every event that has reached its listed stage.
 * @return Number of listed events.
 */
s32 menu_build_event_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 event_count;
    s32 word_index;
    s32 event_index;
    u32 stages;
    s32 stage_mask;

    event_count = 0;
    word_index = 0;
    stage_mask = MENU_EVENT_STAGE_MASK;

    while (word_index < MENU_EVENT_WORD_COUNT)
    {
        stages = g_saved_game_ctx->words[SAVED_GAME_WORD_EVENT_PROGRESS + word_index];
        event_index = MENU_EVENTS_PER_WORD - 1;

        while (event_index >= 0)
        {
            if ((stages & stage_mask) >= MENU_EVENT_STAGE_LISTED)
            {
                event_count++;
            }

            event_index--;
            stages = stages >> MENU_EVENT_STAGE_BITS;
        }

        word_index++;
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (event_index = 0; event_index < event_count; event_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + event_index;

        packed_entry = *entry;
        previous_index = event_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((event_index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = event_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = event_index + 1;
        wrapped_next_index = 0;

        if (next_index < event_count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }

    return event_count;
}

/**
 * @brief Clear the unused last item kind and build circular navigation for the Items list.
 * @return Number of item kinds held.
 */
s32 menu_build_item_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 item_count;
    s32 remaining_kinds;
    s32 entry_index;
    u8* quantity;

    item_count = 0;
    g_saved_game_ctx->item_counts[FIELD_ITEM_KIND_COUNT - 1] = 0;
    quantity = g_saved_game_ctx->item_counts;

    for (remaining_kinds = FIELD_ITEM_KIND_COUNT - 1; remaining_kinds >= 0; remaining_kinds--)
    {
        if (*quantity != 0)
        {
            item_count++;
        }

        quantity++;
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (entry_index = 0; entry_index < item_count; entry_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + entry_index;

        packed_entry = *entry;
        previous_index = entry_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;

        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }

    return item_count;
}

/**
 * @brief Build circular navigation for the Artifacts list: one entry per obtained land artifact.
 * @return Number of land artifacts obtained.
 */
s32 menu_build_artifact_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 land_count;
    s32 remaining_lands;
    s32 entry_index;
    FieldLandRecord* land;

    land_count = 0;
    land = g_saved_game_ctx->lands;

    for (remaining_lands = FIELD_LAND_COUNT - 1; remaining_lands >= 0; remaining_lands--)
    {
        if (land->flags & FIELD_LAND_ARTIFACT_OBTAINED)
        {
            land_count++;
        }

        land++;
    }

    g_menu_scroll_nav_entries[0] = 0;

    for (entry_index = 0; entry_index < land_count; entry_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + entry_index;

        packed_entry = *entry;
        previous_index = entry_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_LIST_ROW_HEIGHT) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = land_count - 1;
        }

        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;

        if (next_index < land_count)
        {
            wrapped_next_index = next_index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }

    return land_count;
}

/**
 * @brief Stage the best weapon from the inventory in the weapon slot for comparison.
 * @return 1 when a weapon was staged, otherwise 0.
 */
s32 menu_stage_best_weapon(void)
{
    FieldItemRecord* candidate;

    menu_stage_stack_shape(0, 0, 0, 0, 0, 0);

    candidate = menu_find_best_weapon();

    if (candidate != NULL)
    {
        if (menu_item_differs_from_bare_hands(menu_weapon_record()) == 0)
        {
            field_copy_inventory_record(&menu_character_record(g_saved_game_ctx, g_menu_char_slot)->equipment[FIELD_WEAPON_SLOT], candidate);
            candidate->name[0] = 0;
            g_item_slot_data[FIELD_WEAPON_SLOT] = NULL;
        }
        else
        {
            menu_swap_item_records(menu_weapon_record(), candidate);
            g_item_slot_data[FIELD_WEAPON_SLOT] = candidate;
        }

        g_item_slot_flags[FIELD_WEAPON_SLOT] = 1;
        return 1;
    }

    return 0;
}

/**
 * @brief Find the inventory weapon with the highest power that does not conflict with the equipped armor.
 * @return First weapon stronger than the equipped one, or NULL if none is.
 */
FieldItemRecord* menu_find_best_weapon(void)
{
    s32 item_index;
    s32 conflicts;
    s32 item_value;
    s32 best_value;
    u32 item_info;
    FieldItemRecord* item;
    FieldItemRecord* best_item;

    best_value = 0;
    best_item = &menu_character_record(g_saved_game_ctx, g_menu_char_slot)->equipment[FIELD_WEAPON_SLOT];

    if (best_item->name[0] != 0)
    {
        best_value = best_item->derived.values[0];
    }

    item = (FieldItemRecord*)g_saved_game_ctx->items;
    best_item = NULL;
    conflicts = menu_get_equipment_conflicts(FIELD_WEAPON_SLOT);

    for (item_index = 0; item_index < MENU_ITEM_TABLE_COUNT; item_index++, item++)
    {
        if (item->name[0] != 0)
        {
            item_info = item->info.word;

            if (!(item_info & FIELD_ITEM_CATEGORY_BITS) && !(conflicts & g_field_weapon_type_conflicts[FIELD_ITEM_TYPE(item_info)]))
            {
                item_value = item->derived.values[0];

                if (best_value < item_value)
                {
                    best_item = item;
                    best_value = item_value;
                }
            }
        }
    }

    return best_item;
}

/**
 * @brief Stage the best armor from the inventory in the active armor slot for comparison.
 * @return 1 if an armor was staged, otherwise 0.
 */
s32 menu_stage_best_armor(void)
{
    FieldItemRecord* candidate;
    FieldItemRecord** staged_items;
    SavedGameLayout* saved_game;
    s32 armor_slot;
    s32 slot_index;

    if (0)
    {
        field_copy_inventory_record(0, 0, 0, 0, 0, 0);
    }

    candidate = menu_find_best_armor();

    if (candidate != NULL)
    {
        slot_index = FIELD_ARMOR_SLOT;
        armor_slot = g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE;
        saved_game = g_saved_game_ctx;

        if (menu_equipped_item(saved_game, armor_slot)->name[0] == 0)
        {
            /* A candidate staged for another slot is now the record being replaced here. */
            staged_items = &g_item_slot_data[0];

            for (; slot_index < MENU_EQUIPMENT_SLOT_COUNT; slot_index++)
            {
                if (candidate == staged_items[slot_index])
                {
                    staged_items[slot_index] = menu_active_equipment();
                    break;
                }
            }

            field_copy_inventory_record(menu_active_equipment(), candidate);
            candidate->name[0] = 0;
            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = NULL;
        }
        else
        {
            staged_items = &g_item_slot_data[0];

            for (; slot_index < MENU_EQUIPMENT_SLOT_COUNT; slot_index++)
            {
                if (candidate == staged_items[slot_index])
                {
                    staged_items[slot_index] = menu_active_equipment();
                    break;
                }
            }

            menu_swap_item_records((FieldItemRecord*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (uintptr_t)g_saved_game_ctx +
                                                      (g_menu_active_subtype << 6) +
                                                      (MENU_FIRST_EQUIPMENT_OFFSET - MENU_EQUIPMENT_SUBTYPE_BASE * MENU_ITEM_RECORD_SIZE)),
                                   candidate);
            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = candidate;
        }

        g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
        return 1;
    }

    return 0;
}

/**
 * @brief Find the inventory armor for the active armor slot with the highest sum of its four defense values.
 * @return First armor better than the equipped one that does not conflict with the other equipment, or NULL if none is.
 */
FieldItemRecord* menu_find_best_armor(void)
{
    s32 item_index;
    s32 conflicts;
    s32 item_value;
    s32 best_value;
    u32 item_info;
    FieldItemRecord* item;
    FieldItemRecord* best_item;

    best_item = &menu_character_record(g_saved_game_ctx, g_menu_char_slot)->equipment[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE];

    if (best_item->name[0] == 0)
    {
        best_value = 0;
    }
    else
    {
        best_value = best_item->derived.values[0] + best_item->derived.values[1] + best_item->derived.values[2] + best_item->derived.values[3];
    }

    best_item = NULL;
    item = (FieldItemRecord*)g_saved_game_ctx->items;
    conflicts = menu_get_equipment_conflicts(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);

    for (item_index = 0; item_index < MENU_ITEM_TABLE_COUNT; item_index++, item++)
    {
        if (item->name[0] != 0)
        {
            item_info = item->info.word;

            if (((item_info & FIELD_ITEM_CATEGORY_BITS) == (FIELD_ITEM_CATEGORY_ARMOR << FIELD_ITEM_CATEGORY_SHIFT)) &&
                !(conflicts & g_field_armor_type_conflicts[FIELD_ITEM_TYPE(item_info)]))
            {
                item_value = item->derived.values[0] + item->derived.values[1] + item->derived.values[2] + item->derived.values[3];

                if (best_value < item_value)
                {
                    best_item = item;
                    best_value = item_value;
                }
            }
        }
    }

    return best_item;
}

/**
 * @brief Play a menu sound effect, unless a menu script is currently driving input.
 * @param sound_id Sound effect ID, such as SFX_CURSOR.
 * @param volume Playback volume (menu callers always pass AKAO_SFX_DEFAULT_VOLUME).
 */
void menu_play_se(s32 sound_id, s32 volume)
{
    if (g_active_script == 0)
    {
        field_play_sound(sound_id, volume);
    }
}

/**
 * @brief Count occupied records at the start of the inventory item table.
 * @return Number of occupied records before the first empty entry.
 */
s32 menu_count_inventory_items(void)
{
    s32 count;
    FieldItemRecord* item;

    item = (FieldItemRecord*)g_saved_game_ctx->items;

    for (count = 0; count < MENU_ITEM_TABLE_COUNT; count++)
    {
        if (item->name[0] == 0)
        {
            break;
        }

        item++;
    }

    return count;
}

/**
 * @brief Draw a number, showing values above MENU_NUMBER_MAX as MENU_NUMBER_MAX.
 * @param ot_entry Ordering-table entry for the emitted primitives.
 * @param packet_cursor Primitive write cursor.
 * @param value Number to draw; negative values pass through.
 * @param text_color Field text color index.
 * @param origin Number position.
 * @param flags FIELD_TEXT_ALIGN_* alignment.
 * @return Updated primitive write cursor.
 */
void* menu_draw_clamped_number(u_long* ot_entry, void* packet_cursor, s32 value, s32 text_color, Vec2s* origin, s32 flags)
{
    if (value > MENU_NUMBER_MAX)
    {
        value = MENU_NUMBER_MAX;
    }

    return field_draw_number(ot_entry, packet_cursor, value, text_color, origin, flags);
}
