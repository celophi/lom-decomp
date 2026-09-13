#include "menu_internal.h"

/* ----- Initialization and Core Frame Processing ----- */

/**
 * @brief Run the menu overlay until an exit or follow-up screen is requested.
 * @param render_buffers Pair of render buffers used for alternating frames.
 * @return Requested follow-up screen code, or zero when the menu closes.
 */
s32 func_801405B0(RenderContext* render_buffers)
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
    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = SCREEN_WIDTH;
    clear_rect.h = (SCREEN_HEIGHT + VRAM_DRAW_HEIGHT);
    ClearImage(&clear_rect, 0, 0, 0);
    g_menu_draw_buf_base[0].clear_rect.x = 0;
    g_menu_draw_buf_base[0].clear_rect.y = VRAM_BACK_DRAW_Y;
    g_menu_draw_buf_base[0].clear_rect.w = SCREEN_WIDTH;
    g_menu_draw_buf_base[0].clear_rect.h = VRAM_DRAW_HEIGHT;
    g_menu_draw_buf_base[1].clear_rect.x = 0;
    g_menu_draw_buf_base[1].clear_rect.y = SCREEN_HEIGHT;
    g_menu_draw_buf_base[1].clear_rect.w = SCREEN_WIDTH;
    g_menu_draw_buf_base[1].clear_rect.h = VRAM_DRAW_HEIGHT;
    SetDefDispEnv(&g_menu_draw_buf_base[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_menu_draw_buf_base[1].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_menu_draw_buf_base[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_menu_draw_buf_base[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    draw_env_buffers = g_menu_draw_buf_base;
    draw_env_buffers[1].draw_env.dtd = 0;
    draw_env_buffers[0].draw_env.dtd = 0;
    g_menu_load_request = 0;
    g_menu_transition_code = 0;
    g_menu_draw_buf_base[0].frame_parity = 0;
    g_menu_draw_buf_base[1].frame_parity = 1;
    menu_init();
    next_buffer = g_menu_draw_buf_base;
    ClearOTagR(next_buffer->ot, MENU_OT_ENTRY_COUNT);
    ClearOTagR(g_menu_draw_buf_base[1].ot, MENU_OT_ENTRY_COUNT);
    VSync(0);
    PutDispEnv(&next_buffer->disp_env);
    update_controllers();
    if (g_active_script == 0)
    {
        DrawSync(0);
        VSync(0);
        SetDispMask(1);
        D_80168C08 = 0;
    }
    else
    {
        D_80168C08 = 2;
    }
    for (;;)
    {
        draw_buffer = next_buffer;
        ClearOTagR(draw_buffer->ot, MENU_OT_ENTRY_COUNT);
        draw_buffer->prim_cursor = &draw_buffer->ot[MENU_OT_ENTRY_COUNT];
        func_8006441C();
        menu_tick(draw_buffer);
        func_80063194();
        func_80068440();
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);

        if (g_menu_load_request != 0 || g_pad_input == PAD_BTN_START)
        {
            break;
        }

        ClearImage(&draw_buffer->clear_rect, (*(u8*)&g_menu_initial_clut_pair & 0x1F) * 8,
                   (g_menu_initial_clut_pair >> 2) & 0xF8, (g_menu_initial_clut_pair >> 7) & 0xF8);
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
            if (D_80168C08 != 0)
            {
                D_80168C08--;
                if (D_80168C08 != 0)
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
    func_800AA02C();
    func_800643E0();
    return g_menu_transition_code;
}

/**
 * @brief Initialize menu graphics, runtime state, window slots, and the node tree.
 */
void menu_init(void)
{
    volatile u8 padding;
    menu_upload_graphics();
    menu_state_init();
    menu_reset_slots();
    g_active_slot = -1;
    func_800AA02C();
    g_menu_compare_window_active = 0;
    menu_init_prim_rects();
    g_menu_frame = 0;
    g_script_cursor = 0;
    menu_node_tree_init();
}

/**
 * @brief Upload each menu slot's cursor strip and content texture to VRAM.
 * Each RAM slot stores the cursor strip first, followed by the content texture.
 */
void menu_init_prim_rects(void)
{
    s32 slot = 0;
    u8* upload_buffer = g_prim_rect_buf;
    s32 content_byte_offset = PRIM_CONTENT_BUF_OFFSET;
    s32 strip_byte_offset = 0;
    RECT rect;

    for (; slot < PRIM_SLOT_COUNT; slot++)
    {
        /* Upload the slot's cursor-highlight strip. */
        rect.x = PRIM_CURSOR_STRIP_VRAM_X;
        rect.y = slot + PRIM_CURSOR_STRIP_VRAM_Y0;
        rect.w = PRIM_CURSOR_STRIP_W;
        rect.h = PRIM_CURSOR_STRIP_H;
        LoadImage(&rect, menu_image_upload_source(upload_buffer, strip_byte_offset));

        /* Upload the slot's content texture block. */
        rect.x = (slot == PRIM_SLOT_COUNT - 1) ? PRIM_CONTENT_VRAM_X2 : PRIM_CONTENT_VRAM_X;
        rect.y = (slot == 0) ? PRIM_CONTENT_VRAM_Y0 : PRIM_CONTENT_VRAM_Y1;
        rect.w = PRIM_CONTENT_W;
        rect.h = PRIM_CONTENT_H;
        LoadImage(&rect, menu_image_upload_source(upload_buffer, content_byte_offset));

        content_byte_offset += PRIM_SLOT_BYTE_SIZE;
        strip_byte_offset += PRIM_SLOT_BYTE_SIZE;
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
    func_800A9E78();

    /* Merge externally injected input when enabled by the pad context. */
    if ((g_pad_ctx->inject_flags & MENU_PAD_INJECT_ENABLED) && g_pad_ctx->inject_enable)
    {
        g_pad_input |= g_pad_input_inject;
    }

    /* Keep only the highest-priority active button group. */
    input_mask = g_pad_input & MENU_PAD_VERTICAL;
    if (input_mask)
    {
        g_pad_input = input_mask;
    }
    input_mask = g_pad_input & MENU_PAD_DIRECTIONS;
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
void* menu_build_text_run(
    SPRT* sprite_cursor, s32* ot, s32 src, s32 text_color, s32 x, s32 y, s32 len, MenuTextAlignment alignment)
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
    if (alignment != MENU_TEXT_ALIGN_RIGHT)
    {
        if (alignment == MENU_TEXT_ALIGN_CENTER)
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

    if (count != 0)
    {
        do
        {
            sprite = sprite_cursor;
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setSprt(sprite);

            SET_SPRT_XY0_WORD(sprite, PACK_U16_PAIR(x, y) + acc);
            acc += sprite->w;

            addPrim(ot, sprite);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    /* addPrim prepends, so link the texture-page packet after the sprites. */
    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x1F);
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
    texture_window.w = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.h = MENU_GRID_TEXTURE_WINDOW_SIZE;
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
    MenuTimVramLayout layout;

    layout.texture_x = SCREEN_WIDTH;
    layout.texture_y = 0;
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
void menu_upload_tim(const MenuTimVramLayout* layout)
{
    MenuTimAsset* asset = (MenuTimAsset*)g_menu_tim;
    Tim* tim = &asset->tim;
    u32 clut_block_size = tim->clut_block.bnum;
    RECT vram_rect;
    u16* clut_color;
    s32 color_index;

    /* Preserve the first color pair before modifying the palette in place. */
    g_menu_initial_clut_pair = *(u32*)tim->clut_data;

    /* Enable STP on nonzero colors, then upload the first CLUT. */
    vram_rect.x = layout->clut_x;
    vram_rect.y = layout->clut_y;
    vram_rect.w = CLUT_ENTRY_COUNT;
    vram_rect.h = 1;

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
    vram_rect.x = layout->texture_x;
    vram_rect.y = layout->texture_y;
    {
        TimBlock* image_block = TIM_PIXEL_BLOCK(tim, clut_block_size);
        vram_rect.w = image_block->dimensions.width;
        vram_rect.h = image_block->dimensions.height;
        LoadImage(&vram_rect, (u_long*)(image_block + 1));
    }

    /* Apply the same STP treatment to the second CLUT. */
    vram_rect.x = layout->clut_x;
    vram_rect.y = layout->clut_y + 1;
    vram_rect.w = CLUT_ENTRY_COUNT;
    vram_rect.h = 1;

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

/* ----- Window Slots and Rendering ----- */

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
    MenuSlot* slot_pool;
    u32 slot_flags;
    u32 ot_index_clear_mask;

    /* Find the first slot whose active state is clear. */
    slot_index = 0;
    slot_pool = &g_menu_slots[0];
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
    slot = (MenuSlot*)((slot_index * sizeof(MenuSlot)) + (u32)slot_pool);

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
    slot->x = rect->x;
    slot->y = rect->y;
    slot->w = rect->w;
    slot->h = rect->h;
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
    ScreenPos view_origin;
    s32 unused_pad[2];
    s32 has_active_slot;
    s32 slot_index;

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

                menu_draw_window_transition(render_ctx, &g_menu_slots[slot_index], g_menu_cursor_enable != 0);
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
                menu_draw_window(&g_menu_slots[slot_index], render_ctx, slot_rect, &view_origin, g_menu_cursor_enable != 0);
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
            menu_draw_window_transition(render_ctx, &g_menu_slots[slot_index], g_menu_cursor_enable != 0);
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

    {
        s32 allow_input;
        u_long* frame_ot;

        allow_input = 0;
        if (has_active_slot == 0)
        {
            g_active_slot = -1;
        }

        frame_ot = &render_ctx->ot[MENU_FRAME_OT_INDEX];
        view_origin.y = 0;
        view_origin.x = 0;

        if ((g_active_slot == -1) || (g_menu_cursor_enable == 0))
        {
            allow_input = 1;
        }

        render_ctx->prim_cursor = menu_draw_frame(render_ctx->prim_cursor, frame_ot, render_ctx->frame_parity, allow_input);
        if (g_menu_help_text != 0)
        {
            render_ctx->prim_cursor = (void*)func_800A88A0(render_ctx->prim_cursor, &render_ctx->ot[MENU_FRAME_OT_INDEX], g_menu_help_text, 1, 0xA0, 0xCA, 2);
        }
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
    ScreenPos view_origin;
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

            rect.x = slot->x + inset_x;
            rect.y = slot->y + inset_y;
            rect.w = width;
            rect.h = height;

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
void menu_draw_window(MenuSlot* slot, RenderContext* render_ctx, MenuRect* rect, ScreenPos* view_origin, s32 cursor_enable)
{
    MenuRectU16 window_rect;
    DRAWENV content_draw_env;
    ScreenPos title_pos;
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
                packet_cursor = (u_long*)func_800AD208(ot_entry, packet_cursor, slot->navigation.fields.selected_index + 1, 3, &title_pos, 0);
            }
            else
            {
                packet_cursor = (u_long*)func_800AD208(ot_entry, packet_cursor, 0, 3, &title_pos, 0);
            }
            packet_cursor = func_800AD524((s32)packet_cursor, ot_entry, 0xB, &title_pos, 0);
            title_pos.x += 8;
            packet_cursor = (u_long*)func_800AD208(ot_entry, packet_cursor, slot->navigation.fields.count_and_ot & MENU_ITEM_NAV_INDEX_MASK, 3, &title_pos, 0);
            switch (g_menu_scene_type)
            {
            case 1:
            case 4:
            case 19:
            case 22:
            case 25:
                packet_cursor = func_800AD524((s32)packet_cursor, ot_entry, 0xB, &title_pos, 0);
                title_pos.x += 8;
                packet_cursor = (u_long*)func_800AD208(ot_entry, packet_cursor, menu_count_inventory_items(), 3, &title_pos, 0);
                break;
            }
            packet_cursor = menu_emit_slot_scroll_arrows((SPRT*)packet_cursor, ot_entry, slot);
        }
    }
    window_rect.w = 0xFF;
    window_rect.h = 0xFF;
    window_rect.x = 0;
    window_rect.y = 0;
    setTexWindow((DR_TWIN*)packet_cursor, &window_rect);
    addPrim(ot_entry, packet_cursor);
    window_cursor = packet_cursor + PRIM_WORDS(DR_TWIN);
    if (rect->h >= MENU_WINDOW_MIN_HEIGHT)
    {
        window_rect.x = (u16)rect->x + 8;
        window_rect.y = (u16)rect->y;
        window_rect.w = (u16)rect->w - MENU_WINDOW_MIN_HEIGHT;
        window_rect.h = 8;
        window_cursor = menu_build_h_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_TOP);
        if (rect->h >= MENU_WINDOW_MIN_HEIGHT)
        {
            window_rect.x = (u16)rect->x + 8;
            window_rect.y = ((u16)rect->y + (u16)rect->h) - 8;
            window_rect.w = (u16)rect->w - MENU_WINDOW_MIN_HEIGHT;
            window_rect.h = 8;
            window_cursor = menu_build_h_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_BOT);
        }
    }
    if (rect->w >= MENU_WINDOW_MIN_WIDTH)
    {
        window_rect.x = (u16)rect->x;
        window_rect.y = (u16)rect->y + 8;
        window_rect.w = 8;
        window_rect.h = (u16)rect->h - MENU_WINDOW_MIN_HEIGHT;
        window_cursor = menu_build_v_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_LEFT);
        if (rect->w >= MENU_WINDOW_MIN_WIDTH)
        {
            window_rect.x = ((u16)rect->x + (u16)rect->w) - 8;
            window_rect.y = (u16)rect->y + 8;
            window_rect.w = 8;
            window_rect.h = (u16)rect->h - MENU_WINDOW_MIN_HEIGHT;
            window_cursor = menu_build_v_edge(window_cursor, ot_entry, &window_rect, MENU_TW_EDGE_RIGHT);
        }
    }
    window_rect.x = (u16)rect->x + 8;
    window_rect.y = (u16)rect->y + 8;
    window_rect.w = (u16)rect->w - MENU_WINDOW_MIN_HEIGHT;
    window_rect.h = (u16)rect->h - MENU_WINDOW_MIN_HEIGHT;
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

        if (rect->w > 0)
        {
            do
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
            } while (x_offset < rect->w);
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
        sprite->w = rect->w;
        sprite->h = rect->h;
        sprite->x0 = rect->x;
        sprite->y0 = rect->y;
        sprite->clut = MENU_CLUT_CORNER;
        addPrim(ot_entry, sprite);
        packet_cursor += PRIM_WORDS(SPRT);

        /* Repeat the 16x8 edge texture across the sprite. */
        texture_window_primitive = (DR_TWIN*)packet_cursor;
        texture_window.x = texture_origin & 0xFF;
        texture_window.y = texture_origin >> 8;
        texture_window.w = MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE;
        texture_window.h = MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE;
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
        sprite->w = rect->w;
        sprite->h = rect->h;
        sprite->x0 = rect->x;
        sprite->y0 = rect->y;
        sprite->clut = MENU_CLUT_CORNER;
        addPrim(ot_entry, sprite);
        packet_cursor += PRIM_WORDS(SPRT);

        /* Repeat the 8x16 edge texture across the sprite. */
        texture_window_primitive = (DR_TWIN*)packet_cursor;
        texture_window.x = texture_origin & 0xFF;
        texture_window.y = texture_origin >> 8;
        texture_window.w = MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE;
        texture_window.h = MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE;
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
u_long* menu_draw_label(u_long* ot_entry, u_long* packet_cursor, const ScreenPos* position, s32 value)
{
    u8 label_buffer[MENU_LABEL_BUFFER_SIZE];

    menu_copy_sign_label(label_buffer, value);

    packet_cursor =
        (u_long*)func_800A88A0(packet_cursor, ot_entry, label_buffer, 1, position->x, position->y, 0);

    return packet_cursor;
}

/* ----- Node Tree and Navigation ----- */

/**
 * @brief Initialize the full menu node tree and global menu state.
 */
void menu_node_tree_init(void)
{
    u32 work_value_a;
    u32 work_value_b;
    u32 node3_flags;
    u32 node6_flags;
    u32 node9_flags;
    s32 zero_value;
    s32 layout_y;
    s32 node_index;
    s32 packed_value;
    s32 nav_x_low_bit;
    s8 scene_type;
    u16 flags_value;
    s32 layout_y_high;
    u16 node15_flags_base;
    u16 node15_flags;
    u16 node18_flags;
    u16 node29_flags;
    u16 node3_flags_base;
    u16 node6_flags_base;
    u16 node9_flags_base;
    u32 node12_flags;
    u16 node30_flags_base;
    u16 node30_flags;
    u32 layout_y_16;
    u32 layout_y_9;
    u32 layout_y_high_9;
    u16 node32_flags_base;
    u16 node32_flags;
    u16 node0_flags;
    u16 node0_flags_step1;
    u16 node0_flags_step2;
    u16 initial_flags;
    u32 node18_flags_copy;
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
    g_menu_cursor_enable = 0;
    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        initial_flags = g_menu_nodes[node_index].u2.unk2;
        g_menu_nodes[node_index].layout_frames_remaining = MENU_NODE_LAYOUT_IDLE;
        g_menu_nodes[node_index].icon_id = 0;
        g_menu_nodes[node_index].content_id = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[3] = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[2] = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[1] = MENU_NONE;
        g_menu_nodes[node_index].layout.s.children[0] = MENU_NONE;
        g_menu_nodes[node_index].u2.unk2 = (u16)((initial_flags & 0xFFFC) | 0x30);
        g_menu_nodes[node_index].u2.s.parent_idx = MENU_NONE;
    }

    g_menu_nodes[0].label_id = 1;
    g_menu_nodes[0].idx_nav.s.self_idx = 0;
    node0_flags = g_menu_nodes[0].u2.unk2;
    node0_flags_step1 = node0_flags & 0xFFCD;
    node0_flags_step2 = node0_flags & 0xFF0D;
    g_menu_nodes[0].u2.unk2 = node0_flags_step1;
    *(volatile u16*)&g_menu_nodes[0].u2.unk2 = node0_flags_step2;
    g_menu_nodes[0].u2.unk2 = node0_flags_step2 | MENU_NODE_FLAG_ACTIVE;
    g_menu_nodes[0].u2.s.parent_idx = MENU_NONE;
    if (D_800FD818.unk0 & 2)
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
    g_menu_nodes[3].label_id = 4;
    g_menu_nodes[3].idx_nav.s.self_idx = 3;
    g_menu_nodes[1].u2.unk2 = (u16)((g_menu_nodes[1].u2.unk2 & 0xFF0F) | 0x40);
    g_menu_nodes[1].u2.s.parent_idx = 0;
    g_menu_nodes[2].u2.unk2 = (u16)((g_menu_nodes[2].u2.unk2 & 0xFF0F) | 0x40);
    g_menu_nodes[2].u2.s.parent_idx = 0;
    node3_flags_base = (g_menu_nodes[3].u2.unk2 & 0xFFCD) | 0x10;
    g_menu_nodes[3].u2.unk2 = node3_flags_base;
    node3_flags = 0x10;
    node3_flags = node3_flags_base | node3_flags;
    *(volatile u16*)&g_menu_nodes[3].u2.unk2 = (u16)(node3_flags & 0xFF3F);
    g_menu_nodes[3].u2.unk2 = (u16)(node3_flags & 0xFF3E);
    g_menu_nodes[3].u2.s.parent_idx = MENU_NONE;
    if (D_800FDA80 & 2)
    {
        g_menu_nodes[3].icon_id = 0x6F;
    }
    else
    {
        g_menu_nodes[3].icon_id = 0x6E;
    }
    g_menu_nodes[4].u2.unk2 = (u16)((0xFF5F & g_menu_nodes[4].u2.unk2) | 0x50);
    g_menu_nodes[5].u2.unk2 = (u16)((g_menu_nodes[5].u2.unk2 & 0xFF5F) | 0x50);
    node6_flags_base = (g_menu_nodes[6].u2.unk2 & 0xFFCD) | 0x10;
    g_menu_nodes[6].u2.unk2 = node6_flags_base;
    node6_flags = 0x10;
    node6_flags = node6_flags_base | node6_flags;
    *(volatile u16*)&g_menu_nodes[6].u2.unk2 = (u16)(node6_flags & 0xFF3F);
    g_menu_nodes[6].u2.unk2 = (u16)(node6_flags & 0xFF3E);
    g_menu_nodes[3].layout.s.children[0] = 4;
    g_menu_nodes[3].layout.s.children[1] = 5;
    g_menu_nodes[4].label_id = 6;
    g_menu_nodes[4].idx_nav.s.self_idx = 4;
    g_menu_nodes[4].icon_id = 5;
    g_menu_nodes[5].label_id = 5;
    g_menu_nodes[5].idx_nav.s.self_idx = 5;
    g_menu_nodes[5].icon_id = 4;
    g_menu_nodes[6].label_id = 7;
    g_menu_nodes[6].idx_nav.s.self_idx = 6;
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
    g_menu_nodes[7].u2.unk2 = (u16)((g_menu_nodes[7].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[7].u2.s.parent_idx = 6;
    g_menu_nodes[8].u2.unk2 = (u16)((g_menu_nodes[8].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[8].idx_nav.s.self_idx = 8;
    node9_flags_base = (g_menu_nodes[9].u2.unk2 & 0xFFCD) | 0x20;
    g_menu_nodes[9].u2.unk2 = node9_flags_base;
    node9_flags = 0x20;
    node9_flags = node9_flags_base | node9_flags;
    *(volatile u16*)&g_menu_nodes[9].u2.unk2 = (u16)(node9_flags & 0xFF3F);
    g_menu_nodes[9].u2.unk2 = (u16)(node9_flags & 0xFF3E);
    g_menu_nodes[0xA].u2.unk2 = (u16)((g_menu_nodes[0xA].u2.unk2 & 0xFF6F) | 0x60);
    node12_flags = (g_menu_nodes[0xC].u2.unk2 & 0xFFCD) | 0x20;
    g_menu_nodes[0xC].u2.unk2 = node12_flags;
    *(volatile u16*)&g_menu_nodes[0xC].u2.unk2 = (u16)((node12_flags | 0x20) & 0xFF3F);
    g_menu_nodes[0xC].u2.unk2 = (u16)((node12_flags | 0x20) & 0xFF3E);
    g_menu_nodes[8].u2.s.parent_idx = 6;
    g_menu_nodes[8].icon_id = 4;
    g_menu_nodes[9].label_id = 0xA;
    g_menu_nodes[9].idx_nav.s.self_idx = 9;
    g_menu_nodes[9].icon_id = 6;
    g_menu_nodes[9].layout.s.children[0] = 0xA;
    g_menu_nodes[0xA].label_id = 0xB;
    g_menu_nodes[0xA].idx_nav.s.self_idx = 0xA;
    g_menu_nodes[0xA].icon_id = 7;
    g_menu_nodes[0xC].label_id = 0xA;
    g_menu_nodes[0xC].idx_nav.s.self_idx = 0xC;
    g_menu_nodes[0xC].icon_id = 6;
    g_menu_nodes[0xC].layout.s.children[0] = 0xD;
    g_menu_nodes[0xD].label_id = 0xB;
    g_menu_nodes[0xD].idx_nav.s.self_idx = 0xD;
    g_menu_nodes[0xD].icon_id = 7;
    g_menu_nodes[9].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xA].u2.s.parent_idx = 9;
    g_menu_nodes[0xC].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xD].u2.unk2 = (u16)((g_menu_nodes[0xD].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0xD].u2.s.parent_idx = 0xC;
    node15_flags_base = (g_menu_nodes[0xF].u2.unk2 & 0xFFCD) | 0x20;
    g_menu_nodes[0xF].u2.unk2 = node15_flags_base;
    node15_flags = (node15_flags_base & 0xFF6D) | 0x60;
    *(volatile u16*)&g_menu_nodes[0xF].u2.unk2 = node15_flags;
    g_menu_nodes[0xF].u2.unk2 = (u16)(node15_flags & 0xFFFE);
    g_menu_nodes[0xF].icon_id = 8;
    g_menu_nodes[0x10].icon_id = 7;
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
    g_menu_nodes[0x12].layout.s.children[3] = 0x1C;
    g_menu_nodes[0x10].u2.unk2 = (u16)((g_menu_nodes[0x10].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0x10].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x11].u2.unk2 = (u16)((g_menu_nodes[0x11].u2.unk2 & 0xFF6F) | 0x60);
    node18_flags_copy = g_menu_nodes[0x12].u2.unk2;
    flags_value = node18_flags_copy;
    g_menu_nodes[0x11].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x12].u2.unk2 = (u16)(flags_value & 0xFFFD);
    node18_flags = flags_value & 0xFF3D;
    *(volatile u16*)&g_menu_nodes[0x12].u2.unk2 = node18_flags;
    g_menu_nodes[0x12].u2.unk2 = (u16)(node18_flags | MENU_NODE_FLAG_ACTIVE);
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
    g_menu_nodes[0x13].u2.unk2 = (u16)((g_menu_nodes[0x13].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x14].u2.unk2 = (u16)((g_menu_nodes[0x14].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x13].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x14].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x16].u2.unk2 = (u16)((g_menu_nodes[0x16].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x16].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x15].u2.unk2 = (u16)((g_menu_nodes[0x15].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x15].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x17].u2.unk2 = (u16)((g_menu_nodes[0x17].u2.unk2 & 0xFF3F) | 0x80);
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
    g_menu_nodes[0x18].u2.unk2 = (u16)((g_menu_nodes[0x18].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x19].u2.unk2 = (u16)((g_menu_nodes[0x19].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x18].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x1B].u2.unk2 = (u16)((g_menu_nodes[0x1B].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x1B].u2.s.parent_idx = 0x19;
    g_menu_nodes[0x1A].u2.unk2 = (u16)((g_menu_nodes[0x1A].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x1C].u2.unk2 = (u16)((g_menu_nodes[0x1C].u2.unk2 & 0xFF3F) | 0x40);
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
    g_menu_nodes[0x2B].idx_nav.s.self_idx = 0x2B;
    flags_value = g_menu_nodes[0x1D].u2.unk2;
    g_menu_nodes[0x1D].u2.unk2 = (u16)(flags_value & 0xFFFD);
    node29_flags = flags_value & 0xFF3D;
    *(volatile u16*)&g_menu_nodes[0x1D].u2.unk2 = node29_flags;
    node30_flags_base = g_menu_nodes[0x1E].u2.unk2;
    g_menu_nodes[0x1D].u2.unk2 = (u16)(node29_flags | MENU_NODE_FLAG_ACTIVE);
    g_menu_nodes[0x1D].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x1F].u2.unk2 = (u16)((g_menu_nodes[0x1F].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1F].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1E].u2.unk2 = (u16)(node30_flags_base & 0xFFFD);
    node30_flags = node30_flags_base & 0xFF3D;
    *(volatile u16*)&g_menu_nodes[0x1E].u2.unk2 = node30_flags;
    g_menu_nodes[0x2B].u2.unk2 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1E].u2.unk2 = (u16)(node30_flags | MENU_NODE_FLAG_ACTIVE);
    g_menu_nodes[0x1E].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x2B].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1F].u2.unk2 = (u16)(g_menu_nodes[0x1F].u2.unk2 & 0xFFCF);
    g_menu_nodes[0x2B].u2.unk2 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFFCF) | 0x10);
    g_menu_nodes[0x2B].icon_id = 0x15;
    g_menu_nodes[0x20].label_id = 0x1B;
    g_menu_nodes[0x20].idx_nav.s.self_idx = 0x20;
    node32_flags_base = g_menu_nodes[0x20].u2.unk2;
    node32_flags = node32_flags_base & 0xFF3D;
    g_menu_nodes[0x20].u2.unk2 = (u16)(node32_flags_base & 0xFFFD);
    *(volatile u16*)&g_menu_nodes[0x20].u2.unk2 = (u16)node32_flags;
    g_menu_nodes[0x20].icon_id = 0x16;
    g_menu_nodes[0x20].u2.unk2 = (u16)(node32_flags | MENU_NODE_FLAG_ACTIVE);
    g_menu_nodes[0x20].u2.s.parent_idx = MENU_NONE;
    if (D_800FD818.unk268 & 1)
    {
        if (D_800FD818.unk26B)
        {
            g_menu_nodes[6].u2.unk2 = (u16)(g_menu_nodes[6].u2.unk2 | MENU_NODE_FLAG_ACTIVE);
        }
        else
        {
            g_menu_nodes[3].u2.unk2 = (u16)(g_menu_nodes[3].u2.unk2 | MENU_NODE_FLAG_ACTIVE);
        }
    }
    if (D_800FDCE8 & 1)
    {
        if ((g_pad_ctx->unkAA8 & 0x7F) == 4)
        {
            g_menu_nodes[0xF].u2.unk2 = (u16)(g_menu_nodes[0xF].u2.unk2 | MENU_NODE_FLAG_ACTIVE);
        }
        else
        {
            g_menu_nodes[9].u2.unk2 = (u16)(g_menu_nodes[9].u2.unk2 | MENU_NODE_FLAG_ACTIVE);
        }
    }
    if ((g_pad_ctx->inject_flags & 0x80) && g_pad_ctx->inject_enable)
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
    g_menu_ability_mask = packed_value;
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

    do
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
        node_index += 1;
    } while (node_index < MENU_NODE_COUNT);

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
    draw_env_packet = menu_draw_scene_content(packet_cursor, (s32*)ot_entry);
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
    switch (g_menu_cursor_enable)
    {
    case MENU_CURSOR_MODE_NODE_TREE:
        frame_cursor = (u8*)menu_draw_active_node_cursor((u8*)draw_env_packet, ot_entry - 1, allow_input);
        menu_handle_input(0);
        if (allow_input != 0)
        {
            menu_handle_node_input();
        }
        break;

    case MENU_CURSOR_MODE_CONTENT:
        frame_cursor = menu_draw_content_cursor(draw_env_packet, (s32*)(ot_entry - 1), allow_input);
        if (g_menu_suppress_cursor == 0)
        {
            menu_handle_input(allow_input);
        }
        break;

    case MENU_CURSOR_MODE_CONTENT_EXIT:
        frame_cursor = menu_draw_content_cursor(draw_env_packet, (s32*)(ot_entry - 1), allow_input);
        if (g_menu_suppress_cursor == 0)
        {
            g_menu_cursor_enable = MENU_CURSOR_MODE_NODE_TREE;
        }
        break;

    default:
        break;
    }

    /* Draw the node tree and its scroll indicators. */
    node_tree_end = menu_draw_node_tree(frame_cursor, (s32*)ot_entry);
    frame_cursor = menu_emit_tree_scroll_arrows(node_tree_end, (s32*)(ot_entry - 1));

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
            menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
            g_menu_load_request = 1;
            return;
        }
        g_menu_active_node = MENU_NODE_BROWSE_ALL;
    }
    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN | PAD_BTN_CIRCLE))
    {
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
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
    if (g_pad_input & (PAD_BTN_RIGHT | PAD_BTN_CROSS | PAD_BTN_L3))
    {
        menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
        if (g_menu_active_node == MENU_NODE_BROWSE_ALL)
        {
            if (!(g_pad_input & (PAD_BTN_CROSS | PAD_BTN_L3)))
            {
                return;
            }
            g_menu_load_request = 1;
        }
        if (g_menu_active_node == 0x11)
        {
            g_menu_load_request = 1;
            g_menu_transition_code = 0xA;
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
                    g_menu_ability_mask = 0;
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
            if (g_pad_input & (PAD_BTN_CROSS | PAD_BTN_L3))
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
            g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT;
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
                g_content_view_x = content_items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_X_MASK;
                view_y = content_items[g_menu_hit_item_idx].y - MENU_CONTENT_VIEW_Y_OFFSET;
                g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT;
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
        g_content_view_x = content_items[g_menu_hit_item_idx].packed_x & MENU_CONTENT_X_MASK;
        view_y = content_items[g_menu_hit_item_idx].y - MENU_CONTENT_VIEW_Y_OFFSET;
        g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
        g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT;
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
    MenuNode* walk_base;
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
    s32 node_base_addr;

    /* Clear the "expanded" bit (bit 1) on every node, then re-expand only the active path. */
    for (clear_index = 0; clear_index < MENU_NODE_COUNT; clear_index++)
    {
        g_menu_nodes[clear_index].u2.unk2 &= ~MENU_NODE_FLAG_EXPANDED;
    }

    /* Walk from g_menu_active_node up to the root, marking each ancestor expanded. */
    child_slot = g_menu_active_node;
    walk_base = g_menu_nodes;
    walk_off = child_slot << 4;
    if (((MenuNode*)((u8*)walk_base + walk_off))->u2.s.parent_idx != MENU_NONE)
    {
        do
        {
            child_slot = ((MenuNode*)(walk_off + (s32)walk_base))->u2.s.parent_idx;
            walk_off = child_slot << 4;
            ((MenuNode*)(walk_off + (s32)walk_base))->u2.unk2 |= MENU_NODE_FLAG_EXPANDED;
        } while (((MenuNode*)(walk_off + (s32)walk_base))->u2.s.parent_idx != MENU_NONE);
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
        node_base_addr = (s32)active_base;
        loop_active = active_node;
        for (; child_slot < MENU_MAX_CHILDREN; child_slot++)
        {
            child_index_value = loop_active->layout.s.children[child_slot];
            wide_child_index = child_index_value;
            if (child_index_value == (packed_value = MENU_NONE))
            {
                break;
            }
            child_node = (MenuNode*)(((u32)wide_child_index << 4) + node_base_addr);
            ((MenuNodeCoordinateView*)child_node)->layout_x =
                ((MenuNodeCoordinateView*)child_node)->nav_x = ((MenuNodeCoordinateView*)loop_active)->nav_x;
            reloaded_child_index = loop_active->layout.s.children[child_slot];
            ((MenuNodeCoordinateView*)(((u32)reloaded_child_index << 4) + node_base_addr))->nav_y =
                ((MenuNodeCoordinateView*)loop_active)->nav_y;
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
