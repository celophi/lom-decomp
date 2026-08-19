#include "checkps.h"

/* Nonzero ends the CHECKPS display loop; value 2 is used for image timeout. */
s32 g_checkps_exit_reason;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkps_unused_word0;

FadeColor g_fade_target;

FadeColor g_fade_current;

/* Sequence data copied from a CHECKPS disc asset before playback. */
u8 g_checkps_song_buffer[16384];

/* Destination address used when registering the embedded AKAO bank. */
AkaoSeqHeader* g_checkps_akao_bank;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkps_unused_word1;

s32 g_debounced_input;

s32 g_checkps_image_height;

/* Width from the TIM-style image header, in VRAM words (4 pixels per word). */
s32 g_checkps_image_width_words;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkps_unused_word2;

s32 g_checkps_image_frames_remaining;

s32 g_last_input_state;

s32 g_input_repeat_timer;

/*
 * Unreferenced BSS extent between init.c and cdrom_data.c.  Keep the exact
 * element count: it preserves the linked address of the following CD state globals.
 */
s32 g_checkps_reserved_bss[32769];

s32 run_checkps(s32 render_state_address)
{
    load_embedded_checkps_audio();
    init_checkps_display((CheckPSRenderState*)render_state_address);
    do
    {
        run_checkps_display_loop((CheckPSRenderState*)render_state_address);
    } while (g_checkps_exit_reason == 0);

    return 8;
}

void run_checkps_display_loop(CheckPSRenderState* render_state)
{
    RECT rect;
    u_long* ordering_table_end;
    CheckPSFrame* frame;
    CheckPSFrame* next_frame;

    DrawSync(0);
    VSync(0);

    frame = &render_state->frames[0];
    reset_controller_vsync_state();

    rect.w = SCREEN_WIDTH;
    rect.x = 0;
    rect.y = 0;
    rect.h = VRAM_BACK_DISP_Y + SCREEN_HEIGHT;
    ClearImage(&rect, 0, 0, 0);
    ClearOTagR(frame->ordering_table, CHECKPS_ORDERING_TABLE_LENGTH);
    ClearOTagR(render_state->frames[1].ordering_table, CHECKPS_ORDERING_TABLE_LENGTH);
    PutDispEnv(&frame->display.disp);
    update_controllers();
    SetDispMask(1);
    do
    {
        ordering_table_end = frame->ordering_table;
        ClearOTagR(ordering_table_end, CHECKPS_ORDERING_TABLE_LENGTH);
        frame->primitive_cursor = frame->primitive_buffer;
        begin_glyph_cache_frame();
        update_and_draw_fade(frame);
        draw_checkps_image(frame);
        update_checkps_input_and_timeout();
        evict_unused_glyphs();
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        ClearImage(&frame->display.clear_rect, 0, 0, 0);
        next_frame = &render_state->frames[0];
        if (frame == next_frame)
        {
            next_frame = &render_state->frames[1];
        }
        frame = next_frame;
        PutDispEnv(&frame->display.disp);
        PutDrawEnv(&frame->display.draw);
        ordering_table_end += CHECKPS_ORDERING_TABLE_LENGTH - 1;
        DrawOTag(ordering_table_end);

        update_controllers();
        cdrom_process_state();
    } while (g_checkps_exit_reason == 0);

    reset_controller_vsync_state();
    VSync(0);
}
/* Configure the two 320x240 frame environments and reset CHECKPS rendering state. */
void init_checkps_display(CheckPSRenderState* render_state)
{
    RECT rect;
    SetGeomScreen(1500);
    SetGeomOffset(160, 120);
    render_state->frames[0].display.clear_rect.x = 0;
    render_state->frames[0].display.clear_rect.y = 0;
    render_state->frames[0].display.clear_rect.w = SCREEN_WIDTH;
    render_state->frames[0].display.clear_rect.h = SCREEN_HEIGHT;
    render_state->frames[1].display.clear_rect.x = 0;
    render_state->frames[1].display.clear_rect.y = VRAM_BACK_DISP_Y;
    render_state->frames[1].display.clear_rect.w = SCREEN_WIDTH;
    render_state->frames[1].display.clear_rect.h = SCREEN_HEIGHT;
    DrawSync(0);
    VSync(0);

    rect.w = 1024;
    rect.x = 0;
    rect.y = 0;
    rect.h = 512;
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv(&render_state->frames[0].display.disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&render_state->frames[1].display.disp, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&render_state->frames[0].display.draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&render_state->frames[1].display.draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    render_state->frames[1].display.draw.dtd = 0;
    render_state->frames[0].display.draw.dtd = 0;

    rect.x = 960;
    rect.w = 64;
    rect.y = 0;
    rect.h = 256;
    ClearImage(&rect, 0, 0, 0);
    reset_glyph_renderer();
    reset_fade_state();
    set_fade_target(256, 256, 256, 20);
    load_checkps_image();
    g_checkps_exit_reason = 0;
    update_controller_input();
}

void load_embedded_checkps_audio(void)
{
    u8* source;
    u8* bank_destination;
    u32 byte_count;
    AkaoSeqHeader** bank_slot;
    u32* section_offsets;
    if (((((g_previousGameState == 2) || (g_previousGameState == 3)) || (g_previousGameState == 0)) || (g_previousGameState == 6)) ||
        (g_previousGameState == 7) || (g_previousGameState == 5))
    {
        return;
    }

    bank_slot = &g_checkps_akao_bank;
    *bank_slot = (AkaoSeqHeader*)0x8013C000;

    section_offsets = &g_embedded_checkps_akao;
    section_offsets++;

    source = (u8*)&g_embedded_checkps_akao + section_offsets[0];
    bank_destination = (u8*)*bank_slot;
    byte_count = section_offsets[1] - section_offsets[0];
    bcopy(source, bank_destination, byte_count);
    akao_register_bank(*bank_slot);
    akao_play_sequence_blocking((AkaoSeqHeader*)((u32)&g_embedded_checkps_akao + section_offsets[1]), 1);
}

void load_checkps_song_from_disc(s32 song_index)
{
    u32* section_offsets;
    u8* song_container;

    cdrom_queue_read((song_index + 0x17) & 0xFFFF, 0x80180000);
    cdrom_wait_queue_empty();

    section_offsets = (u32*)0x80180004;
    song_container = (u8*)0x80180000;
    bcopy(song_container + section_offsets[0], g_checkps_song_buffer, section_offsets[1] - section_offsets[0]);
    akao_play_sequence_blocking((AkaoSeqHeader*)(section_offsets[1] + (u32)song_container), 1);
}

void stop_checkps_song(void)
{
    akao_stop_song(0);
}

void play_loaded_checkps_song(void)
{
    akao_play_song(&g_checkps_song_buffer);
    akao_cmd_c0(0, 0x7F);
}
void play_checkps_sfx(u32 sound_id, u32 volume, u32 pan)
{
    akao_play_sfx(sound_id, 0, volume, pan);
}
/* Reset both fade endpoints to black. */
void reset_fade_state(void)
{
    g_fade_current.red = 0;
    g_fade_current.green = 0;
    g_fade_current.blue = 0;
    g_fade_target.red = 0;
    g_fade_target.green = 0;
    g_fade_target.blue = 0;
    g_fade_target.steps = 0;
}

void update_and_draw_fade(CheckPSFrame* frame)
{
    s32 red_step, green_step, blue_step;
    s32 draw_mode;
    u32* primitive;
    u32* ordering_table_tag = (u32*)frame->ordering_table;

    primitive = (u32*)frame->primitive_cursor;
    if (g_fade_target.steps != 0)
    {
        red_step = (g_fade_target.red - g_fade_current.red) / g_fade_target.steps;
        green_step = (g_fade_target.green - g_fade_current.green) / g_fade_target.steps;
        blue_step = (g_fade_target.blue - g_fade_current.blue) / g_fade_target.steps;
        g_fade_target.steps--;
        g_fade_current.red += red_step;
        g_fade_current.green += green_step;
        g_fade_current.blue += blue_step;
    }
    else
    {
        g_fade_current.red = g_fade_target.red;
        g_fade_current.green = g_fade_target.green;
        g_fade_current.blue = g_fade_target.blue;
    }
    if (g_fade_current.red != 0x100 || g_fade_current.green != g_fade_current.red || g_fade_current.blue != g_fade_current.green)
    {
        if (g_fade_current.red >= 0x101)
        {
            ((u8*)primitive)[4] = (u8)(g_fade_current.red - 1);
            ((u8*)primitive)[5] = (u8)(g_fade_current.green - 1);
            ((u8*)primitive)[6] = (u8)(g_fade_current.blue - 1);
        }
        else
        {
            if (g_fade_current.red == 0x100)
            {
                ((u8*)primitive)[4] = 0;
            }
            else
            {
                ((u8*)primitive)[4] = ~(u8)(g_fade_current.red);
            }
            if (g_fade_current.green == 0x100)
            {
                ((u8*)primitive)[5] = 0;
            }
            else
            {
                ((u8*)primitive)[5] = ~(u8)(g_fade_current.green);
            }
            if (g_fade_current.blue == 0x100)
            {
                ((u8*)primitive)[6] = 0;
            }
            else
            {
                ((u8*)primitive)[6] = ~(u8)(g_fade_current.blue);
            }
        }
        ((u8*)primitive)[3] = 3;
        ((u8*)primitive)[7] = 0x62;
        *(u16*)((u8*)primitive + 12) = 0x140;
        *(u16*)((u8*)primitive + 10) = 0;
        *(u16*)((u8*)primitive + 8) = 0;
        *(u16*)((u8*)primitive + 14) = 0xF0;

        *primitive = (*primitive & 0xFF000000) | (*ordering_table_tag & 0x00FFFFFF);
        *ordering_table_tag = (*ordering_table_tag & 0xFF000000) | ((u32)primitive & 0x00FFFFFF);
        draw_mode = 0x25;
        primitive = (u32*)((u8*)primitive + 0x10); // advance in-place; lands in delay slot
        if (g_fade_current.red < 0x101)
        {
            draw_mode = 0x45;
        }
        ((u8*)primitive)[3] = 1;
        *(u32*)((u8*)primitive + 4) = (u32)draw_mode | 0xE1000000;

        *primitive = (*primitive & 0xFF000000) | (*ordering_table_tag & 0x00FFFFFF);
        *ordering_table_tag = (*ordering_table_tag & 0xFF000000) | ((u32)primitive & 0x00FFFFFF);

        primitive = (u32*)((u8*)primitive + 8); // second advance (+8), total = +0x18
    }
    frame->primitive_cursor = (u8*)primitive;
}
/* Set the RGB fade target; 0x100 is normal brightness. */
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fade_target.red = red;
    g_fade_target.green = green;
    g_fade_target.blue = blue;
    g_fade_target.steps = steps;
}
void update_checkps_input_and_timeout(void)
{
    s32 timer;

    process_controller_input();
    timer = g_checkps_image_frames_remaining - 1;
    g_checkps_image_frames_remaining = timer;

    if (timer == 0)
    {
        g_checkps_exit_reason = 2;
    }
}

void draw_checkps_image(CheckPSFrame* frame)
{
    u8* primitive_cursor_page;
    u8* primitive;
    s32 width_words;
    s32 image_width_words;
    s32 image_height;
    u_long* ordering_table;
    u32 draw_mode_command;
    volatile u32 stack_layout_scratch; /* Required to preserve the original stack frame. */
    draw_mode_command = 0xE1000000;
    /* Keep the split +0x8000/+0xB8 addressing shape; GCC 2.7.2 emits the matched cursor load this way. */
    primitive_cursor_page = ((u8*)frame) + 0x8000;
    primitive = *((u8**)(primitive_cursor_page + 0xB8));

    // Set RGB
    *((u32*)(primitive + 4)) = 0x808080;

    setSprt((SPRT*)primitive);

    width_words = g_checkps_image_width_words;
    image_height = g_checkps_image_height;

    setUV0((SPRT*)primitive, 0, 0);
    setClut((SPRT*)primitive, 0, 480);
    setXY0((SPRT*)primitive, (0x140 - (width_words * 4)) >> 1, (0xE0 - image_height) / 2);

    image_width_words = g_checkps_image_width_words;
    width_words = image_width_words;

    ordering_table = frame->ordering_table;

    setWH((SPRT*)primitive, width_words * 4, g_checkps_image_height);
    addPrim(ordering_table, (SPRT*)primitive);
    primitive += 0x14;

    setDrawTPage((SPRT*)primitive, 0, 0, 5);
    addPrim(ordering_table, (SPRT*)primitive);

    primitive += 8;
    *((u8**)(primitive_cursor_page + 0xB8)) = primitive;
}

void load_checkps_image(void)
{
    RECT image_destination;
    RECT upload_rect;
    RECT* upload_rect_ptr;
    u8* image_asset;
    u32 clut_block_size;
    u8* pixel_block;
    u16 image_x, image_y;
    u16* image_header;

    upload_rect_ptr = &upload_rect;
    image_asset = g_checkps_image_asset;

    g_checkps_image_frames_remaining = 120;
    image_destination.x = 0x140;
    image_destination.y = 0;
    image_destination.w = 0;
    image_destination.h = 0x1E0;

    upload_rect.x = 0;
    upload_rect.y = 0x1E0;
    upload_rect.w = *(u16*)(image_asset + 0x10) * *(u16*)(image_asset + 0x12);
    upload_rect.h = 1;

    clut_block_size = *(u32*)(image_asset + 8);
    LoadImage(upload_rect_ptr, (u_long*)(image_asset + 0x14));

    image_x = image_destination.x;
    image_y = image_destination.y;

    pixel_block = image_asset + (clut_block_size + 8);

    image_header = (u16*)(pixel_block + 8);
    upload_rect.x = image_x;
    upload_rect.y = image_y;
    upload_rect.w = image_header[0];
    upload_rect.h = image_header[1];

    g_checkps_image_width_words = image_header[0];
    g_checkps_image_height = image_header[1];

    image_header += 2;
    LoadImage(upload_rect_ptr, (u_long*)image_header);
}

s32 poll_input_device(void)
{
    SCDRegs* regs = SCD_REGS;

    u32 input_mask;
    u32 raw_buttons;
    u32 remapped_upper;
    u32 cross_bit;
    u32 triangle_bit;
    u32 square_bit;
    u32 non_face_bits;

    s32 axis_x;
    s32 axis_y;
    u16 hi_read;
    u16 lo_read;
    u16 axis_raw;

    if (regs->device_type >= 0xFEU)
    {
        return 0;
    }

    // Raw button read (byte-swapped via two reads)
    hi_read = *((volatile u16*)(((u8*)regs) + 2));
    lo_read = *((volatile u16*)(((u8*)regs) + 2));
    input_mask = (((u32)hi_read) >> 8) | (((u32)lo_read) << 8);

    raw_buttons = input_mask;
    // Remap upper nibble bits (hardware → logical layout)
    remapped_upper = (raw_buttons & 0x40) >> 1;
    cross_bit = (raw_buttons & 0x20) << 1;
    remapped_upper |= cross_bit;

    triangle_bit = (raw_buttons & 0x80) >> 3;
    remapped_upper |= triangle_bit;

    square_bit = (raw_buttons & 0x10) << 3;
    remapped_upper |= square_bit;

    non_face_bits = raw_buttons & (~0xF0U);
    input_mask = remapped_upper | non_face_bits;
    if (regs->device_type != 0)
    {
        // X axis → left/right flags
        axis_raw = *(volatile u16*)&regs->axis_x;
        axis_x = (s32)((s16)axis_raw);

        if (axis_x < -1)
        {
            input_mask |= 0x8000U;
        }
        else if (axis_x >= 2)
        {
            input_mask |= 0x2000U;
        }

        // Y axis → up/down flags
        axis_raw = *(u16*)&regs->axis_y;
        axis_y = (s32)((s16)axis_raw);
        if (axis_y < -1)
        {
            input_mask |= 0x1000U;
        }
        else if (axis_y >= 2)
        {
            input_mask |= 0x4000U;
        }
    }

    return (s32)input_mask;
}
/* Read the merged controller sample and apply CHECKPS key-repeat/debounce behavior. */
void process_controller_input(void)
{
    SCDRegs* controller_regs;
    u32 processed_buttons;
    s16 axis_x;
    s16 axis_y;
    s32 final_button_state;
    controller_regs = SCD_REGS;

    /* Device types 0xFE/0xFF are unavailable controller states. */
    if (((u8)g_controller_device_type) >= 0xFEU)
    {
        final_button_state = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processed_buttons = (controller_regs->held_buttons >> 8) | (controller_regs->held_buttons << 8);
        // Remap face buttons from hardware order to game order.
        processed_buttons = PAD_REMAP_FACE_BITS(processed_buttons);

        if (controller_regs->device_type != 0)
        {
            axis_x = (s16)controller_regs->axis_x;

            if (axis_x < -1)
            {
                processed_buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                processed_buttons |= PAD_BTN_RIGHT;
            }

            axis_y = (s16)controller_regs->axis_y;
            if (axis_y < -1)
            {
                processed_buttons |= PAD_BTN_UP;
            }
            else if (axis_y >= 2)
            {
                processed_buttons |= PAD_BTN_DOWN;
            }
        }
        final_button_state = processed_buttons;
    }

    g_debounced_input = 0; // current active input
    // 0x0B6F = L2 | R2 | L1 | R1 | Cross | Circle | Select | L3 | Start
    // = 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0020 | 0x0040 | 0x0100 | 0x0200 | 0x0800
    if (((final_button_state == g_last_input_state) || ((g_last_input_state != 0) && ((final_button_state & (g_last_input_state | 0xB6F))))) && (final_button_state != 0))
    {
        // Keep only directional bits
        if ((final_button_state & 0xF000) != 0)
        {
            final_button_state &= 0xF000;
        }
        if (g_input_repeat_timer == 0)
        {
            g_debounced_input = final_button_state;
            g_input_repeat_timer = 2;
        }
        else
        {
            g_input_repeat_timer--;
            g_debounced_input = 0;
        }
    }
    else if (final_button_state == 0)
    {
        g_input_repeat_timer = 0;
        g_last_input_state = 0;
    }
    else
    {
        g_debounced_input = final_button_state;
        g_last_input_state = final_button_state;
        g_input_repeat_timer = 15;
    }
}
/* Seed the CHECKPS input snapshot from the merged controller sample. */
void update_controller_input(void)
{
    SCDRegs* regs;
    PadButton processed_buttons;
    short axis_x;
    short axis_y;
    unsigned int final_button_state;
    regs = SCD_REGS;

    g_debounced_input = 0;

    /* Device types 0xFE/0xFF are unavailable controller states. */
    if (g_controller_device_type >= 0xFEU)
    {
        final_button_state = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processed_buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
        // Reorder face button bits 4-7 from hardware order (Triangle, Circle, Cross, Square)
        // to game order (Square, Cross, Circle, Triangle) by swapping Triangle<->Square and Circle<->Cross.
        // Keep D-pad and shoulder button bits (0-3, 8-15) unchanged.
        processed_buttons =
            (((((processed_buttons & PAD_BTN_CIRCLE) >> 1) | ((processed_buttons & PAD_BTN_CROSS) << 1)) | ((processed_buttons & PAD_BTN_TRIANGLE) >> 3)) |
             ((processed_buttons & PAD_BTN_SQUARE) << 3)) |
            (processed_buttons & ~0xF0);
        if (regs->device_type != 0)
        {
            axis_x = regs->axis_x;

            if (axis_x < -1)
            {
                processed_buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                processed_buttons |= PAD_BTN_RIGHT;
            }

            axis_y = regs->axis_y;
            if (axis_y < -1)
            {
                processed_buttons |= PAD_BTN_UP;
            }
            else if (axis_y >= 2)
            {
                processed_buttons |= PAD_BTN_DOWN;
            }
        }
        final_button_state = processed_buttons;
    }

    g_last_input_state = final_button_state;
    g_input_repeat_timer = 15;
}
