#include "checkps_internal.h"

#include "akao.h"
#include "cd_resources.h"
#include "display.h"
#include "game_state.h"
#include "gpu_packet.h"
#include "pad.h"
#include "tim.h"
#include "psyq/libapi.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/memory.h"

#define CHECKPS_ORDERING_TABLE_LENGTH 0x1000
#define CHECKPS_PRIMITIVE_BUFFER_SIZE 0x4000
#define CHECKPS_SONG_BUFFER_SIZE 0x4000
#define CHECKPS_RESERVED_BSS_WORDS 32769

#define CHECKPS_FRAME_VSYNC_INTERVAL 2
#define CHECKPS_GPU_DRAW_MODE_COMMAND 0xE1000000
#define CHECKPS_FADE_ADDITIVE_DRAW_MODE 0x25
#define CHECKPS_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define CHECKPS_IMAGE_TPAGE 5
#define CHECKPS_GEOMETRY_SCREEN_DISTANCE 1500
#define CHECKPS_FADE_NEUTRAL 0x100
#define CHECKPS_FADE_ADDITIVE_THRESHOLD (CHECKPS_FADE_NEUTRAL + 1)
#define CHECKPS_DEFAULT_FADE_STEPS 20
#define CHECKPS_IMAGE_DISPLAY_FRAMES 120
#define CHECKPS_IMAGE_CLUT_Y 480
#define CHECKPS_GLYPH_VRAM_WIDTH 64
#define CHECKPS_GLYPH_VRAM_HEIGHT 256

#define CHECKPS_AUDIO_BANK_ADDRESS ((AkaoHeader*)0x8013C000)
#define CHECKPS_AUDIO_WORK_ADDRESS ((u8*)0x80180000)
#define CHECKPS_AUDIO_BANK_RESIDENT_STATE 6
#define CHECKPS_AKAO_COPIED_SECTION 0
#define CHECKPS_AKAO_UPLOAD_BANK_SECTION 1

#define CHECKPS_CONTROLLER_UNAVAILABLE 0xFE
#define CHECKPS_INITIAL_REPEAT_DELAY 15
#define CHECKPS_REPEAT_DELAY 2
#define CHECKPS_DPAD_MASK (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT)
#define CHECKPS_NON_REPEAT_BUTTON_MASK \
    (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1 | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_SELECT | PAD_BTN_L3 | PAD_BTN_START)

/**
 * @brief Conditions that end the CHECKPS display loop.
 */
typedef enum
{
    CHECKPS_EXIT_NONE = 0,
    CHECKPS_EXIT_IMAGE_TIMEOUT = 2,
} CheckPSExitReason;

/**
 * @brief Current and target RGB fade values with the remaining step count.
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeColor;

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} CheckPSFadePrimitive;

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define CHECKPS_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((CheckPSFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/** @brief Packet view for a CHECKPS image sprite or draw-mode command. */
typedef union
{
    SPRT sprite;
    DR_TPAGE draw_mode;
} CheckPSImagePrimitive;

/** Advance an image packet cursor by the concrete packet just emitted. */
#define CHECKPS_NEXT_IMAGE_PRIMITIVE(primitive, type) \
    ((CheckPSImagePrimitive*)((u8*)(primitive) + sizeof(type)))

/**
 * @brief GPU environments and clear rectangle for one display buffer.
 */
typedef struct
{
    DISPENV disp;
    DRAWENV draw;
    RECT clear_rect;
} CheckPSDisplayBuffer;

/**
 * @brief Ordering table, display state, and primitive storage for one frame.
 */
typedef struct
{
    u8 reserved_header[0x40];
    u_long ordering_table[CHECKPS_ORDERING_TABLE_LENGTH];
    CheckPSDisplayBuffer display;
    u8 primitive_buffer[CHECKPS_PRIMITIVE_BUFFER_SIZE];
    void* primitive_cursor;
    u8 reserved_tail[0x3C10];
} CheckPSFrame;

/**
 * @brief Double-buffered rendering workspace supplied to CHECKPS.
 */
struct CheckPSRenderState
{
    CheckPSFrame frames[2];
};

/* Count, two byte offsets, then variable-sized AKAO resources. */
extern u8 g_embedded_checkps_akao[];
extern TimPrefix g_checkps_image_asset;
extern u8 g_controller_device_type;

void run_checkps_display_loop(CheckPSRenderState* render_state);
void init_checkps_display(CheckPSRenderState* render_state);
void load_embedded_checkps_audio(void);
void load_checkps_song_from_disc(s32 song_index);
void stop_checkps_song(void);
void play_loaded_checkps_song(void);
void play_checkps_sfx(u32 sound_id, u32 volume, u32 pan);
void reset_fade_state(void);
void update_and_draw_fade(CheckPSFrame* frame);
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps);
void update_checkps_input_and_timeout(void);
void draw_checkps_image(CheckPSFrame* frame);
void load_checkps_image(void);
s32 poll_input_device(void);
void process_controller_input(void);
void update_controller_input(void);

/* Nonzero ends the CHECKPS display loop; value 2 is used for image timeout. */
s32 g_checkps_exit_reason;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkps_unused_word0;

FadeColor g_fade_target;

FadeColor g_fade_current;

/* Sequence data copied from a CHECKPS disc asset before playback. */
u8 g_checkps_song_buffer[CHECKPS_SONG_BUFFER_SIZE];

/* Destination address used when registering the embedded AKAO bank. */
AkaoHeader* g_checkps_akao_bank;

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
s32 g_checkps_reserved_bss[CHECKPS_RESERVED_BSS_WORDS];

/**
 * @brief Run the CHECKPS startup screen until it requests exit.
 * @param render_state Double-buffered render workspace.
 * @return Top-level game state to enter after CHECKPS.
 */
s32 run_checkps(CheckPSRenderState* render_state)
{
    load_embedded_checkps_audio();
    init_checkps_display(render_state);
    do
    {
        run_checkps_display_loop(render_state);
    } while (g_checkps_exit_reason == CHECKPS_EXIT_NONE);

    return GAME_STATE_INTRO_MOVIE;
}

/**
 * @brief Render and update CHECKPS frames until an exit condition is reached.
 * @param render_state Double-buffered render workspace.
 */
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
        set_controller_vsync_interval(CHECKPS_FRAME_VSYNC_INTERVAL);
        VSync(CHECKPS_FRAME_VSYNC_INTERVAL);
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
    } while (g_checkps_exit_reason == CHECKPS_EXIT_NONE);

    reset_controller_vsync_state();
    VSync(0);
}

/**
 * @brief Configure CHECKPS display buffers and reset renderer state.
 * @param render_state Double-buffered render workspace to initialize.
 */
void init_checkps_display(CheckPSRenderState* render_state)
{
    RECT rect;
    SetGeomScreen(CHECKPS_GEOMETRY_SCREEN_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
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

    rect.w = VRAM_WIDTH;
    rect.x = 0;
    rect.y = 0;
    rect.h = VRAM_HEIGHT;
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv(&render_state->frames[0].display.disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&render_state->frames[1].display.disp, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&render_state->frames[0].display.draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&render_state->frames[1].display.draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    render_state->frames[1].display.draw.dtd = 0;
    render_state->frames[0].display.draw.dtd = 0;

    rect.x = CHECKPS_GLYPH_VRAM_X;
    rect.w = CHECKPS_GLYPH_VRAM_WIDTH;
    rect.y = 0;
    rect.h = CHECKPS_GLYPH_VRAM_HEIGHT;
    ClearImage(&rect, 0, 0, 0);
    reset_glyph_renderer();
    reset_fade_state();
    set_fade_target(CHECKPS_FADE_NEUTRAL, CHECKPS_FADE_NEUTRAL, CHECKPS_FADE_NEUTRAL, CHECKPS_DEFAULT_FADE_STEPS);
    load_checkps_image();
    g_checkps_exit_reason = CHECKPS_EXIT_NONE;
    update_controller_input();
}

/**
 * @brief Register the embedded CHECKPS program data and upload its sample bank.
 * @details The embedded container stores both resources behind byte offsets.
 */
void load_embedded_checkps_audio(void)
{
    u8* bank_data;
    u8* upload_bank_data;
    u8* bank_destination;
    u32 bank_size;
    AkaoHeader** bank_slot;
    u32* section_offsets;

    if (((((g_previousGameState == GAME_STATE_TITLE) || (g_previousGameState == GAME_STATE_GNAME)) || (g_previousGameState == GAME_STATE_FIELD)) || (g_previousGameState == CHECKPS_AUDIO_BANK_RESIDENT_STATE)) ||
        (g_previousGameState == GAME_STATE_MENU_LOAD) || (g_previousGameState == GAME_STATE_WORLD_SELECT))
    {
        return;
    }

    bank_slot = &g_checkps_akao_bank;
    *bank_slot = CHECKPS_AUDIO_BANK_ADDRESS;

    section_offsets = (u32*)g_embedded_checkps_akao;
    section_offsets++; /* Skip the section count to reach the offset table. */

    bank_data = &g_embedded_checkps_akao[section_offsets[CHECKPS_AKAO_COPIED_SECTION]];
    bank_destination = (u8*)*bank_slot;
    bank_size = section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION] - section_offsets[CHECKPS_AKAO_COPIED_SECTION];
    bcopy(bank_data, bank_destination, bank_size);
    akao_register_bank(*bank_slot);
    upload_bank_data = &g_embedded_checkps_akao[section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION]];
    akao_upload_bank_blocking((AkaoBankHeader*)upload_bank_data, 1);
}

/**
 * @brief Load a CHECKPS song container from disc and prepare it for playback.
 * @details Copies the persistent song block into the CHECKPS song buffer, then
 *          uploads the trailing instrument bank to the audio driver.
 * @param song_index Music-file index; 0 selects MSC_DATA.DAT.
 */
void load_checkps_song_from_disc(s32 song_index)
{
    u8* song_data;
    u8* bank_data;
    u32 song_size;
    u32* section_offsets;
    u8* song_container;

    cdrom_queue_read(CD_RES_MUSIC_FILE(song_index), CHECKPS_AUDIO_WORK_ADDRESS);
    cdrom_wait_queue_empty();

    section_offsets = (u32*)(CHECKPS_AUDIO_WORK_ADDRESS + sizeof(u32));
    song_container = CHECKPS_AUDIO_WORK_ADDRESS;
    song_data = &song_container[section_offsets[CHECKPS_AKAO_COPIED_SECTION]];
    song_size = section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION] - section_offsets[CHECKPS_AKAO_COPIED_SECTION];
    bcopy(song_data, g_checkps_song_buffer, song_size);
    bank_data = &song_container[section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION]];
    akao_upload_bank_blocking((AkaoBankHeader*)bank_data, 1);
}

/**
 * @brief Stop the currently playing CHECKPS song.
 */
void stop_checkps_song(void)
{
    akao_stop_song(0);
}

/**
 * @brief Start playback from the CHECKPS song buffer.
 */
void play_loaded_checkps_song(void)
{
    akao_play_song((AkaoHeader*)g_checkps_song_buffer);
    akao_set_song_volume(0, AKAO_VOLUME_MAX);
}

/**
 * @brief Play a CHECKPS sound effect.
 * @param sound_id AKAO sound-effect identifier.
 * @param volume Playback volume.
 * @param pan Stereo pan value.
 */
void play_checkps_sfx(u32 sound_id, u32 volume, u32 pan)
{
    akao_play_sfx(sound_id, 0, volume, pan);
}

/**
 * @brief Reset the current and target fade colors to black.
 */
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

/**
 * @brief Advance the fade interpolation and emit its fullscreen GPU packets.
 * @param frame Frame receiving the fade primitives.
 */
void update_and_draw_fade(CheckPSFrame* frame)
{
    s32 red_step, green_step, blue_step;
    s32 draw_mode;
    CheckPSFadePrimitive* primitive;
    u_long* ordering_table_tag = frame->ordering_table;

    primitive = frame->primitive_cursor;
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
    if (g_fade_current.red != CHECKPS_FADE_NEUTRAL || g_fade_current.green != g_fade_current.red || g_fade_current.blue != g_fade_current.green)
    {
        if (g_fade_current.red >= CHECKPS_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = g_fade_current.red - 1;
            primitive->tile.g0 = g_fade_current.green - 1;
            primitive->tile.b0 = g_fade_current.blue - 1;
        }
        else
        {
            if (g_fade_current.red == CHECKPS_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_fade_current.red;
            }
            if (g_fade_current.green == CHECKPS_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_fade_current.green;
            }
            if (g_fade_current.blue == CHECKPS_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_fade_current.blue;
            }
        }
        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        SET_YX0(&primitive->tile, 0, 0);
        setWH(&primitive->tile, SCREEN_WIDTH, SCREEN_HEIGHT);

        addPrim(ordering_table_tag, &primitive->tile);
        draw_mode = CHECKPS_FADE_ADDITIVE_DRAW_MODE;
        primitive = CHECKPS_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_fade_current.red < CHECKPS_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = CHECKPS_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);

        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = CHECKPS_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    frame->primitive_cursor = primitive;
}

/**
 * @brief Set the RGB fade target and interpolation duration.
 * @param red Target red level.
 * @param green Target green level.
 * @param blue Target blue level.
 * @param steps Number of interpolation steps.
 */
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fade_target.red = red;
    g_fade_target.green = green;
    g_fade_target.blue = blue;
    g_fade_target.steps = steps;
}

/**
 * @brief Update controller debounce state and the CHECKPS image timeout.
 */
void update_checkps_input_and_timeout(void)
{
    s32 timer;

    process_controller_input();
    timer = g_checkps_image_frames_remaining - 1;
    g_checkps_image_frames_remaining = timer;

    if (timer == 0)
    {
        g_checkps_exit_reason = CHECKPS_EXIT_IMAGE_TIMEOUT;
    }
}

/**
 * @brief Emit the CHECKPS image sprite and draw-mode packets.
 * @param frame Frame receiving the image primitives.
 */
void draw_checkps_image(CheckPSFrame* frame)
{
    CheckPSImagePrimitive* primitive;
    s32 width_words;
    s32 image_width_words;
    s32 image_height;
    u_long* ordering_table;
    u32 draw_mode_command;
    volatile u32 stack_layout_scratch; /* Required to preserve the original stack frame. */
    draw_mode_command = CHECKPS_GPU_DRAW_MODE_COMMAND;
    primitive = frame->primitive_cursor;

    SET_BGR0_PACKED(&primitive->sprite, GPU_TINT_NEUTRAL);

    setSprt(&primitive->sprite);

    width_words = g_checkps_image_width_words;
    image_height = g_checkps_image_height;

    setUV0(&primitive->sprite, 0, 0);
    setClut(&primitive->sprite, 0, CHECKPS_IMAGE_CLUT_Y);
    setXY0(&primitive->sprite, (SCREEN_WIDTH - (width_words * 4)) >> 1, (VRAM_DRAW_HEIGHT - image_height) / 2);

    image_width_words = g_checkps_image_width_words;
    width_words = image_width_words;

    ordering_table = frame->ordering_table;

    setWH(&primitive->sprite, width_words * 4, g_checkps_image_height);
    addPrim(ordering_table, &primitive->sprite);

    primitive = CHECKPS_NEXT_IMAGE_PRIMITIVE(primitive, SPRT);
    setDrawTPage(&primitive->draw_mode, 0, 0, CHECKPS_IMAGE_TPAGE);
    addPrim(ordering_table, &primitive->draw_mode);

    primitive = CHECKPS_NEXT_IMAGE_PRIMITIVE(primitive, DR_TPAGE);
    frame->primitive_cursor = primitive;
}

/**
 * @brief Upload the embedded CHECKPS CLUT and image pixels to VRAM.
 */
void load_checkps_image(void)
{
    RECT image_destination;
    RECT upload_rect;
    RECT* upload_rect_ptr;
    TimPrefix* image_asset;
    u32 clut_block_size;
    TimBlock* pixel_block;
    u16 image_x, image_y;
    TimDimensions* pixel_dimensions;

    upload_rect_ptr = &upload_rect;
    image_asset = &g_checkps_image_asset;

    g_checkps_image_frames_remaining = CHECKPS_IMAGE_DISPLAY_FRAMES;
    image_destination.x = SCREEN_WIDTH;
    image_destination.y = 0;
    image_destination.w = 0;
    image_destination.h = CHECKPS_IMAGE_CLUT_Y;

    upload_rect.x = 0;
    upload_rect.y = CHECKPS_IMAGE_CLUT_Y;
    upload_rect.w = image_asset->clut_block.dimensions.width * image_asset->clut_block.dimensions.height;
    upload_rect.h = 1;

    clut_block_size = image_asset->clut_block.bnum;
    LoadImage(upload_rect_ptr, image_asset->clut_data);

    image_x = image_destination.x;
    image_y = image_destination.y;

    pixel_block = TIM_PIXEL_BLOCK(image_asset, clut_block_size);

    pixel_dimensions = &pixel_block->dimensions;
    upload_rect.x = image_x;
    upload_rect.y = image_y;
    upload_rect.w = pixel_dimensions->width;
    upload_rect.h = pixel_dimensions->height;

    g_checkps_image_width_words = pixel_dimensions->width;
    g_checkps_image_height = pixel_dimensions->height;

    pixel_dimensions++;
    LoadImage(upload_rect_ptr, (u_long*)pixel_dimensions);
}

/**
 * @brief Read and normalize the current controller sample.
 * @return Logical CHECKPS button mask, or zero when no device is available.
 */
s32 poll_input_device(void)
{
    SCDRegs* regs = SCD_REGS;

    u32 input_mask;
    u32 raw_buttons;

    s16 axis_x;
    s16 axis_y;
    u16 hi_read;
    u16 lo_read;

    if (regs->device_type >= CHECKPS_CONTROLLER_UNAVAILABLE)
    {
        return 0;
    }

    /* Read twice because the controller register may change asynchronously. */
    hi_read = regs->held_buttons;
    lo_read = regs->held_buttons;
    input_mask = (hi_read >> 8) | (lo_read << 8);

    raw_buttons = input_mask;
    /* Convert the controller protocol bits to the game's logical layout. */
    input_mask = PAD_REMAP_FACE_BITS(raw_buttons);
    if (regs->device_type != 0)
    {
        /* Convert signed analog-axis thresholds to digital directions. */
        axis_x = regs->axis_x.signed_value;

        if (axis_x < -1)
        {
            input_mask |= PAD_BTN_LEFT;
        }
        else if (axis_x >= 2)
        {
            input_mask |= PAD_BTN_RIGHT;
        }

        axis_y = regs->axis_y.signed_value;
        if (axis_y < -1)
        {
            input_mask |= PAD_BTN_UP;
        }
        else if (axis_y >= 2)
        {
            input_mask |= PAD_BTN_DOWN;
        }
    }

    return input_mask;
}

/**
 * @brief Apply CHECKPS debounce and key-repeat behavior to controller input.
 */
void process_controller_input(void)
{
    SCDRegs* controller_regs;
    u32 buttons;
    s16 axis_x;
    s16 axis_y;
    s32 input_state;
    controller_regs = SCD_REGS;

    /* Unavailable controller types produce no input. */
    if (g_controller_device_type >= CHECKPS_CONTROLLER_UNAVAILABLE)
    {
        input_state = 0;
    }
    else
    {
        /* Convert the controller protocol bits to the game's logical layout. */
        buttons = (controller_regs->held_buttons >> 8) | (controller_regs->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);

        if (controller_regs->device_type != 0)
        {
            axis_x = controller_regs->axis_x.signed_value;

            if (axis_x < -1)
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }

            axis_y = controller_regs->axis_y.signed_value;
            if (axis_y < -1)
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis_y >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        input_state = buttons;
    }

    /* Publish input only when a new press or key-repeat event fires. */
    g_debounced_input = 0;
    if (((input_state == g_last_input_state) ||
         ((g_last_input_state != 0) &&
          ((input_state & (g_last_input_state | CHECKPS_NON_REPEAT_BUTTON_MASK))))) &&
        (input_state != 0))
    {
        /* Held input repeats directional buttons only. */
        if ((input_state & CHECKPS_DPAD_MASK) != 0)
        {
            input_state &= CHECKPS_DPAD_MASK;
        }
        if (g_input_repeat_timer == 0)
        {
            g_debounced_input = input_state;
            g_input_repeat_timer = CHECKPS_REPEAT_DELAY;
        }
        else
        {
            g_input_repeat_timer--;
            g_debounced_input = 0;
        }
    }
    else if (input_state == 0)
    {
        g_input_repeat_timer = 0;
        g_last_input_state = 0;
    }
    else
    {
        g_debounced_input = input_state;
        g_last_input_state = input_state;
        g_input_repeat_timer = CHECKPS_INITIAL_REPEAT_DELAY;
    }
}

/**
 * @brief Seed the CHECKPS controller snapshot and repeat timer.
 */
void update_controller_input(void)
{
    SCDRegs* regs;
    PadButton processed_buttons;
    short axis_x;
    short axis_y;
    unsigned int final_button_state;
    regs = SCD_REGS;

    g_debounced_input = 0;

    /* Unavailable controller types produce no input. */
    if (g_controller_device_type >= CHECKPS_CONTROLLER_UNAVAILABLE)
    {
        final_button_state = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processed_buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
        // Reorder face buttons from controller protocol order to the game's logical order.
        processed_buttons = PAD_REMAP_FACE_BITS(processed_buttons);
        if (regs->device_type != 0)
        {
            axis_x = regs->axis_x.signed_value;

            if (axis_x < -1)
            {
                processed_buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                processed_buttons |= PAD_BTN_RIGHT;
            }

            axis_y = regs->axis_y.signed_value;
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
    g_input_repeat_timer = CHECKPS_INITIAL_REPEAT_DELAY;
}
