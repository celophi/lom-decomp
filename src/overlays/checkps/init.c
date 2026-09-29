#include "init.h"
#include "include_asm.h"

#include "checkps_internal.h"

#if defined(VERSION_JP)
#include <cdrom.h>
#include "cdrom.h"
#endif

#include "font.h"
#include "akao.h"
#include "cd_resources.h"
#include "display.h"
#include "game_state.h"
#include "gpu_packet.h"
#include "pad.h"
#include "tim.h"
#include "sdk/libapi.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"
#include "sdk/rand.h"

#define CHECKPS_ORDERING_TABLE_LENGTH 4096
#define CHECKPS_PRIMITIVE_BUFFER_SIZE 16384
#define CHECKPS_SONG_BUFFER_SIZE 16384
#define CHECKPS_RESERVED_BSS_WORDS 32769

#define CHECKPS_FRAME_VSYNC_INTERVAL 2
#define CHECKPS_FADE_ADDITIVE_DRAW_MODE 0x25
#define CHECKPS_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define CHECKPS_IMAGE_TPAGE 5
#define CHECKPS_GEOMETRY_SCREEN_DISTANCE 1500
#define CHECKPS_FADE_NEUTRAL 256
#define CHECKPS_FADE_ADDITIVE_THRESHOLD (CHECKPS_FADE_NEUTRAL + 1)
#define CHECKPS_DEFAULT_FADE_STEPS 20
#define CHECKPS_IMAGE_DISPLAY_FRAMES 120
#define CHECKPS_IMAGE_COUNT 100
#define CHECKPS_IMAGE_INITIAL_Y 112
#define CHECKPS_IMAGE_HIDDEN_FRAME 15
#define CHECKPS_IMAGE_FRAME_DELAY 2
#define CHECKPS_IMAGE_SPEED 4
#define CHECKPS_IMAGE_TINT 128
#define CHECKPS_IMAGE_WIDTH 32
#define CHECKPS_IMAGE_HEIGHT 64
#define CHECKPS_IMAGE_ANIMATION_FRAMES 7
#define CHECKPS_IMAGE_SOUND_COUNT 10
#define CHECKPS_IMAGE_SOUND 0xB2

#define CHECKPS_IMAGE_CLUT_Y 480
#define CHECKPS_GLYPH_VRAM_WIDTH 64
#define CHECKPS_GLYPH_VRAM_HEIGHT 256

#define CHECKPS_AUDIO_BANK_ADDRESS ((AkaoHeader*)0x8013C000)
#define CHECKPS_AUDIO_WORK_ADDRESS ((AkaoContainerHeader*)0x80180000)
#define CHECKPS_AUDIO_BANK_RESIDENT_STATE 6
#define CHECKPS_AKAO_COPIED_SECTION 0
#define CHECKPS_AKAO_UPLOAD_BANK_SECTION 1

#define CHECKPS_CONTROLLER_UNAVAILABLE 0xFE
#define CHECKPS_INITIAL_REPEAT_DELAY 15
#define CHECKPS_REPEAT_DELAY 2
#define CHECKPS_DPAD_MASK (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT)
#define CHECKPS_NON_REPEAT_BUTTON_MASK                                                                                                                         \
    (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1 | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_SELECT | PAD_BTN_L3 | PAD_BTN_START)

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define CHECKPS_NEXT_FADE_PRIMITIVE(primitive, type) ((CheckPSFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/** Advance an image packet cursor by the concrete packet just emitted. */
#define CHECKPS_NEXT_IMAGE_PRIMITIVE(primitive, type) ((CheckPSImagePrimitive*)((u8*)(primitive) + sizeof(type)))

/**
 * @brief Exit state of the CHECKPS display loop.
 */
typedef enum
{
    CHECKPS_EXIT_NONE = 0,
    CHECKPS_EXIT_COMPLETE = 2,
} CheckPSExitReason;

/**
 * @brief RGB fade values with the remaining interpolation step count.
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps_remaining;
} CheckPSFadeState;

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} CheckPSFadePrimitive;

/** @brief Packet view for a CHECKPS image sprite or draw-mode command. */
typedef union
{
    SPRT sprite;
    DR_TPAGE draw_mode;
} CheckPSImagePrimitive;

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

#if defined(VERSION_JP)
/** @brief Steps from CD-driver handoff through the completed disc check. */
typedef enum
{
    CHECKPS_STARTUP_ENTER_RECOVERY,
    CHECKPS_STARTUP_BEGIN_CHECK,
    CHECKPS_STARTUP_WAIT_CHECK,
    CHECKPS_STARTUP_RESTORE_DRIVE
} CheckPSStartupStep;

/** @brief Position, color and animation state of a CHECKPS image. */
typedef struct
{
    u8 red;
    u8 green;
    u8 blue;
    u8 reserved;
    s16 x;
    s16 y : 12;
    s16 frame_delay : 4;
    union
    {
        u16 word;
        struct
        {
            u16 frame : 4;
            u16 speed : 4;
            u16 frame_timer : 7;
            u16 moving_right : 1;
        } bits;
        u8 bytes[2];
    } animation;
} CheckPSImage;

/** @brief VRAM destinations for an image and its palette. */
typedef struct
{
    u16 pixel_x;
    u16 pixel_y;
    u16 clut_x;
    u16 clut_y;
} CheckPSImageDestinations;
#endif

extern AkaoContainerHeader g_embedded_checkps_akao;
extern TimPrefix g_checkps_image_asset;

static void run_checkps_display_loop(CheckPSRenderState* render_state);
static void init_checkps_display(CheckPSRenderState* render_state);
static void load_embedded_checkps_audio(void);
static void reset_fade_state(void);
static void update_and_draw_fade(CheckPSFrame* frame);
static void set_fade_target(s32 red, s32 green, s32 blue, s32 step_count);
static void update_checkps_input_and_timeout(void);
static void draw_checkps_image(CheckPSFrame* frame);
static void load_checkps_image(void);
#if defined(VERSION_JP)
static u32 checkps_upload_image_to_vram(TimPrefix* tim, CheckPSImageDestinations* destinations);
#endif
#if !defined(VERSION_JP)
static void process_controller_input(void);
static void update_controller_input(void);
#endif

#if defined(VERSION_JP)
extern CheckPSExitReason g_checkps_exit_reason;
extern CheckPSFadeState g_fade_target;
extern CheckPSFadeState g_fade_current;
extern u8 g_checkps_song_buffer[CHECKPS_SONG_BUFFER_SIZE];
extern AkaoHeader* g_checkps_akao_bank;
extern s32 g_checkps_startup_step;
extern CheckPSImage g_checkps_images[CHECKPS_IMAGE_COUNT];
extern s32 g_checkps_image_burst_active;
#else
/* Nonzero ends the CHECKPS display loop; value 2 is used for image timeout. */
CheckPSExitReason g_checkps_exit_reason;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkps_unused_word0;

CheckPSFadeState g_fade_target;

CheckPSFadeState g_fade_current;

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
 * Unreferenced BSS extent at the end of init.c.  Keep the exact element count:
 * it preserves the linked address of cdrom.c's CD state variables that follow.
 */
s32 g_checkps_reserved_bss[CHECKPS_RESERVED_BSS_WORDS];

#endif

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
static void run_checkps_display_loop(CheckPSRenderState* render_state)
{
    RECT rect;
    u_long* drawn_ordering_table;
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
#if !defined(VERSION_JP)
    update_controllers();
#endif
    SetDispMask(1);
    do
    {
        drawn_ordering_table = frame->ordering_table;
        ClearOTagR(drawn_ordering_table, CHECKPS_ORDERING_TABLE_LENGTH);
        frame->primitive_cursor = frame->primitive_buffer;
        begin_glyph_cache_frame();
        update_and_draw_fade(frame);
        draw_checkps_image(frame);
        update_checkps_input_and_timeout();
        evict_unused_glyphs();
        DrawSync(0);
#if !defined(VERSION_JP)
        set_controller_vsync_interval(CHECKPS_FRAME_VSYNC_INTERVAL);
#endif
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
        DrawOTag(&drawn_ordering_table[CHECKPS_ORDERING_TABLE_LENGTH - 1]);

#if !defined(VERSION_JP)
        update_controllers();
        cdrom_process_state();
#endif
    } while (g_checkps_exit_reason == CHECKPS_EXIT_NONE);

    reset_controller_vsync_state();
    VSync(0);
}

/**
 * @brief Configure CHECKPS display buffers and reset renderer state.
 * @param render_state Double-buffered render workspace to initialize.
 */
static void init_checkps_display(CheckPSRenderState* render_state)
{
    RECT rect;
    SetGeomScreen(CHECKPS_GEOMETRY_SCREEN_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    setRECT(&render_state->frames[0].display.clear_rect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    setRECT(&render_state->frames[1].display.clear_rect, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
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
#if defined(VERSION_JP)
    cdrom_enter_recovery_mode();
#else
    update_controller_input();
#endif
}

/**
 * @brief Register the embedded CHECKPS program data and upload its sample bank.
 * @details The embedded container stores both resources behind byte offsets.
 */
static void load_embedded_checkps_audio(void)
{
    u8* bank_data;
    u8* upload_bank_data;
    u32 bank_size;
    u32* section_offsets;

    if (g_previous_game_state == GAME_STATE_TITLE || g_previous_game_state == GAME_STATE_GNAME || g_previous_game_state == GAME_STATE_FIELD ||
        g_previous_game_state == CHECKPS_AUDIO_BANK_RESIDENT_STATE || g_previous_game_state == GAME_STATE_MENU_LOAD ||
        g_previous_game_state == GAME_STATE_WORLD_SELECT)
    {
        return;
    }

    g_checkps_akao_bank = CHECKPS_AUDIO_BANK_ADDRESS;

    /* The section offsets immediately follow the container header word. */
    section_offsets = &g_embedded_checkps_akao.section_count;
    section_offsets++;

    bank_data = AKAO_CONTAINER_DATA_AT(&g_embedded_checkps_akao, section_offsets[CHECKPS_AKAO_COPIED_SECTION]);
    bank_size = section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION] - section_offsets[CHECKPS_AKAO_COPIED_SECTION];
    bcopy(bank_data, (u8*)g_checkps_akao_bank, bank_size);
    akao_register_bank(g_checkps_akao_bank);
    upload_bank_data = AKAO_CONTAINER_DATA_AT(&g_embedded_checkps_akao, section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION]);
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
    AkaoContainerHeader* song_container;

    cdrom_queue_read(CD_RES_MUSIC_FILE(song_index), CHECKPS_AUDIO_WORK_ADDRESS);
    cdrom_wait_queue_empty();

    song_container = CHECKPS_AUDIO_WORK_ADDRESS;
    section_offsets = song_container->section_offsets;
    song_data = AKAO_CONTAINER_DATA_AT(song_container, section_offsets[CHECKPS_AKAO_COPIED_SECTION]);
    song_size = section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION] - section_offsets[CHECKPS_AKAO_COPIED_SECTION];
    bcopy(song_data, g_checkps_song_buffer, song_size);
    bank_data = AKAO_CONTAINER_DATA_AT(song_container, section_offsets[CHECKPS_AKAO_UPLOAD_BANK_SECTION]);
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
static void reset_fade_state(void)
{
    g_fade_current.red = 0;
    g_fade_current.green = 0;
    g_fade_current.blue = 0;
    g_fade_target.red = 0;
    g_fade_target.green = 0;
    g_fade_target.blue = 0;
    g_fade_target.steps_remaining = 0;
}

/**
 * @brief Advance the fade interpolation and emit its fullscreen GPU packets.
 * @param frame Frame receiving the fade primitives.
 */
static void update_and_draw_fade(CheckPSFrame* frame)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;
    CheckPSFadePrimitive* primitive;
    u_long* ordering_table_tag = frame->ordering_table;

    primitive = frame->primitive_cursor;
    if (g_fade_target.steps_remaining != 0)
    {
        red_step = (g_fade_target.red - g_fade_current.red) / g_fade_target.steps_remaining;
        green_step = (g_fade_target.green - g_fade_current.green) / g_fade_target.steps_remaining;
        blue_step = (g_fade_target.blue - g_fade_current.blue) / g_fade_target.steps_remaining;
        g_fade_target.steps_remaining--;
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
            setRGB0(&primitive->tile, g_fade_current.red - 1, g_fade_current.green - 1, g_fade_current.blue - 1);
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
 * @param step_count Number of interpolation steps.
 */
static void set_fade_target(s32 red, s32 green, s32 blue, s32 step_count)
{
    g_fade_target.red = red;
    g_fade_target.green = green;
    g_fade_target.blue = blue;
    g_fade_target.steps_remaining = step_count;
}

/**
 * @brief Advance the JP disc check or the US input and image timer.
 */
#if defined(VERSION_JP)
static void update_checkps_input_and_timeout(void)
{
    switch (g_checkps_startup_step)
    {
    case CHECKPS_STARTUP_ENTER_RECOVERY:
        if (cdrom_enter_recovery_mode() != 0)
        {
            g_checkps_startup_step = CHECKPS_STARTUP_BEGIN_CHECK;
        }
        break;
    case CHECKPS_STARTUP_BEGIN_CHECK:
        start_cd_integrity_check();
        run_cd_integrity_check(1);
        g_checkps_startup_step = CHECKPS_STARTUP_WAIT_CHECK;
        break;
    case CHECKPS_STARTUP_WAIT_CHECK:
        if (run_cd_integrity_check(1) == 0)
        {
            g_checkps_startup_step = CHECKPS_STARTUP_RESTORE_DRIVE;
        }
        break;
    case CHECKPS_STARTUP_RESTORE_DRIVE:
        if (cdrom_recover() != 0)
        {
            g_checkps_exit_reason = CHECKPS_EXIT_COMPLETE;
        }
        break;
    }
}
#else
static void update_checkps_input_and_timeout(void)
{
    process_controller_input();
    g_checkps_image_frames_remaining--;

    if (g_checkps_image_frames_remaining == 0)
    {
        g_checkps_exit_reason = CHECKPS_EXIT_COMPLETE;
    }
}
#endif

/**
 * @brief Emit the CHECKPS image primitives.
 * @note JP animates a swarm of mirrored images; US displays one centered sprite.
 * @param frame Frame receiving the image primitives.
 */
#if defined(VERSION_JP)
static void draw_checkps_image(CheckPSFrame* frame)
{
    POLY_FT4* primitive;
    u_long* ordering_table;
    CheckPSImage* image;
    s32 i;
    s32 delay;
    s32 volume;
    CheckPSImage* images;
    s32 timer;

    primitive = frame->primitive_cursor;
    ordering_table = frame->ordering_table;
    if (g_checkps_image_burst_active != 0 && g_checkps_images[CHECKPS_IMAGE_COUNT - 1].animation.bits.frame == CHECKPS_IMAGE_HIDDEN_FRAME)
    {
        for (i = 1; i < CHECKPS_IMAGE_COUNT; i++)
        {
            image = &g_checkps_images[i];
            image->red = (rand() & 0x7F) + 0x40;
            image->green = (rand() & 0x7F) + 0x40;
            image->blue = (rand() & 0x7F) + 0x40;
            image->x = -CHECKPS_IMAGE_WIDTH - (rand() >> 9);
            image->y = rand() & 0xFF;
            image->animation.bits.frame = 0;
            delay = (rand() & 1) + 1;
            image->frame_delay = delay;
            image->animation.bits.frame_timer = delay;
            image->animation.bits.moving_right = 1;
            image->animation.bits.speed = (rand() & 0xF) + 1;
        }
    }
    images = g_checkps_images;
    i = 0;
    do
    {
        image = &images[i];
        if ((image->animation.bytes[0] & 0xF) != CHECKPS_IMAGE_HIDDEN_FRAME)
        {
            timer = (image->animation.word >> 8) & 0x7F;
            if (timer != 0)
            {
                --image->animation.bits.frame_timer;
                timer = (image->animation.word >> 8) & 0x7F;
            }
            if (timer == 0)
            {
                image->animation.bits.frame_timer = image->frame_delay;
                if (image->animation.word >> 15)
                {
                    image->x += image->animation.bits.speed;
                    if (image->x >= SCREEN_WIDTH)
                    {
                        image->animation.bits.moving_right = 0;
                        image->animation.bits.frame = CHECKPS_IMAGE_ANIMATION_FRAMES - 1;
                    }
                }
                else
                {
                    image->x -= image->animation.bits.speed;
                    if (image->x < -(CHECKPS_IMAGE_WIDTH - 1))
                    {
                        g_checkps_image_burst_active = 1;
                        image->animation.bits.moving_right = 1;
                        image->animation.bits.frame = CHECKPS_IMAGE_ANIMATION_FRAMES - 1;
                    }
                }
                image->animation.bits.frame = (image->animation.bytes[0] & 0xF) + 1;
                if ((image->animation.bytes[0] & 0xF) == CHECKPS_IMAGE_ANIMATION_FRAMES)
                {
                    if ((s32)image < (s32)&images[CHECKPS_IMAGE_SOUND_COUNT])
                    {
                        volume = 0x7F;
                        if (g_checkps_image_burst_active != 0)
                        {
                            volume = 0x3F;
                        }
                        play_checkps_sfx(CHECKPS_IMAGE_SOUND, 0x80, volume);
                    }
                    image->animation.bits.frame = 0;
                }
            }
            setPolyFT4(primitive);
            setRGB0(primitive, image->red, image->green, image->blue);
            primitive->x2 = primitive->x0 = image->x;
            primitive->y1 = primitive->y0 = image->y - CHECKPS_IMAGE_WIDTH;
            primitive->x3 = primitive->x1 = image->x + CHECKPS_IMAGE_WIDTH - 1;
            primitive->y3 = primitive->y2 = image->y + CHECKPS_IMAGE_WIDTH - 1;
            if (image->animation.word >> 15)
            {
                primitive->u3 = primitive->u1 = (image->animation.bytes[0] & 0xF) * CHECKPS_IMAGE_WIDTH;
                primitive->u2 = primitive->u0 = (image->animation.bytes[0] & 0xF) * CHECKPS_IMAGE_WIDTH + CHECKPS_IMAGE_WIDTH - 1;
            }
            else
            {
                primitive->u2 = primitive->u0 = (image->animation.bytes[0] & 0xF) * CHECKPS_IMAGE_WIDTH;
                primitive->u3 = primitive->u1 = (image->animation.bytes[0] & 0xF) * CHECKPS_IMAGE_WIDTH + CHECKPS_IMAGE_WIDTH - 1;
            }
            primitive->v3 = primitive->v2 = CHECKPS_IMAGE_HEIGHT - 1;
            setClut(primitive, 0, CHECKPS_IMAGE_CLUT_Y);
            primitive->v1 = primitive->v0 = 0;
            primitive->tpage = CHECKPS_IMAGE_TPAGE;
            addPrim(ordering_table - (image->y - CHECKPS_ORDERING_TABLE_LENGTH), primitive);
            primitive++;
        }
        i++;
    } while ((s32)&images[i] < (s32)&images[CHECKPS_IMAGE_COUNT]);
    frame->primitive_cursor = primitive;
}
#else
static void draw_checkps_image(CheckPSFrame* frame)
{
    CheckPSImagePrimitive* primitive;
    s32 width_words;
    s32 height;
    u_long* ordering_table;
    s32 unused[2];

    primitive = frame->primitive_cursor;
    ordering_table = frame->ordering_table;

    SET_BGR0_PACKED(&primitive->sprite, GPU_TINT_NEUTRAL);
    setSprt(&primitive->sprite);
    width_words = g_checkps_image_width_words;
    height = g_checkps_image_height;
    setUV0(&primitive->sprite, 0, 0);
    setClut(&primitive->sprite, 0, CHECKPS_IMAGE_CLUT_Y);
    setXY0(&primitive->sprite, (SCREEN_WIDTH - width_words * 4) >> 1, (VRAM_DRAW_HEIGHT - height) / 2);
    setWH(&primitive->sprite, g_checkps_image_width_words * 4, g_checkps_image_height);
    addPrim(ordering_table, &primitive->sprite);

    primitive = CHECKPS_NEXT_IMAGE_PRIMITIVE(primitive, SPRT);
    setDrawTPage(&primitive->draw_mode, 0, 0, CHECKPS_IMAGE_TPAGE);
    addPrim(ordering_table, &primitive->draw_mode);

    primitive = CHECKPS_NEXT_IMAGE_PRIMITIVE(primitive, DR_TPAGE);
    frame->primitive_cursor = primitive;
}
#endif

/**
 * @brief Upload the embedded CHECKPS CLUT and image pixels to VRAM.
 */
#if defined(VERSION_JP)
static void load_checkps_image(void)
{
    CheckPSImageDestinations destinations;
    s32 i;

    g_checkps_startup_step = 0;
    g_checkps_image_burst_active = 0;
    for (i = 0; i < CHECKPS_IMAGE_COUNT; i++)
    {
        g_checkps_images[i].red = g_checkps_images[i].green = g_checkps_images[i].blue = CHECKPS_IMAGE_TINT;
        g_checkps_images[i].x = SCREEN_WIDTH;
        g_checkps_images[i].y = CHECKPS_IMAGE_INITIAL_Y;
        g_checkps_images[i].animation.bits.frame = CHECKPS_IMAGE_HIDDEN_FRAME;
        g_checkps_images[i].animation.bits.frame_timer = CHECKPS_IMAGE_FRAME_DELAY;
        g_checkps_images[i].animation.bits.moving_right = 0;
        g_checkps_images[i].frame_delay = CHECKPS_IMAGE_FRAME_DELAY;
        g_checkps_images[i].animation.bits.speed = CHECKPS_IMAGE_SPEED;
    }
    g_checkps_images[0].x = SCREEN_WIDTH;
    g_checkps_images[0].y = CHECKPS_IMAGE_INITIAL_Y;
    g_checkps_images[0].animation.bits.frame = 0;
    g_checkps_images[0].animation.bits.frame_timer = CHECKPS_IMAGE_FRAME_DELAY;
    g_checkps_images[0].animation.bits.moving_right = 0;
    g_checkps_images[0].frame_delay = CHECKPS_IMAGE_FRAME_DELAY;
    g_checkps_images[0].animation.bits.speed = CHECKPS_IMAGE_SPEED;
    destinations.pixel_x = SCREEN_WIDTH;
    destinations.pixel_y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = CHECKPS_IMAGE_CLUT_Y;
    checkps_upload_image_to_vram(&g_checkps_image_asset, &destinations);
}
#else
static void load_checkps_image(void)
{
    RECT image_destination;
    RECT upload_rect;
    TimPrefix* image_asset;
    u32 clut_block_size;
    TimBlock* pixel_block;
    TimDimensions* pixel_size;

    image_asset = &g_checkps_image_asset;

    g_checkps_image_frames_remaining = CHECKPS_IMAGE_DISPLAY_FRAMES;
    setRECT(&image_destination, SCREEN_WIDTH, 0, 0, CHECKPS_IMAGE_CLUT_Y);

    setRECT(&upload_rect, 0, CHECKPS_IMAGE_CLUT_Y, image_asset->clut_block.dimensions.width * image_asset->clut_block.dimensions.height, 1);

    clut_block_size = image_asset->clut_block.bnum;
    /* LoadImage transfers the packed 16-bit CLUT entries as words. */
    LoadImage(&upload_rect, (u_long*)image_asset->clut_data);

    pixel_block = TIM_PIXEL_BLOCK(image_asset, clut_block_size);
    pixel_size = &pixel_block->dimensions;
    setRECT(&upload_rect, image_destination.x, image_destination.y, pixel_size->width, pixel_size->height);

    g_checkps_image_width_words = pixel_size->width;
    g_checkps_image_height = pixel_size->height;

    /* The pixel payload follows the block dimensions. */
    pixel_size++;
    LoadImage(&upload_rect, (u_long*)pixel_size);
}
#endif

#if defined(VERSION_JP)
/**
 * @brief Upload a TIM palette and image to the supplied VRAM destinations.
 * @param tim Image resource with a palette block before its pixel block.
 * @param destinations VRAM coordinates for the pixels and palette.
 * @return Image width in VRAM words, rounded up to a 64-word boundary.
 */
static u32 checkps_upload_image_to_vram(TimPrefix* tim, CheckPSImageDestinations* destinations)
{
    RECT upload_rect;
    TimBlock* pixel_block;
    u32 clut_block_length = tim->clut_block.bnum;

    setRECT(&upload_rect, destinations->clut_x, destinations->clut_y, tim->clut_block.dimensions.width * tim->clut_block.dimensions.height, 1);
    LoadImage(&upload_rect, (u_long*)tim->clut_data);

    pixel_block = TIM_PIXEL_BLOCK(tim, clut_block_length);

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y, pixel_block->dimensions.width, pixel_block->dimensions.height);
    LoadImage(&upload_rect, (u_long*)(pixel_block + 1));

    return ALIGN64(pixel_block->dimensions.width);
}
#endif

/**
 * @brief Read and normalize the current controller sample.
 * @return Logical CHECKPS button mask, or zero when no device is available.
 */
s32 poll_input_device(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 input_mask;
    s16 axis_x;
    s16 axis_y;

    if (regs->device_type >= CHECKPS_CONTROLLER_UNAVAILABLE)
    {
        return 0;
    }

    /* Convert the controller protocol bits to the game's logical layout. */
    input_mask = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
#if !defined(VERSION_JP)
    input_mask = PAD_REMAP_FACE_BITS(input_mask);
#endif
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

#if !defined(VERSION_JP)
/**
 * @brief Apply CHECKPS debounce and key-repeat behavior to controller input.
 */
static void process_controller_input(void)
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
    if (((input_state == g_last_input_state) || ((g_last_input_state != 0) && ((input_state & (g_last_input_state | CHECKPS_NON_REPEAT_BUTTON_MASK))))) &&
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
static void update_controller_input(void)
{
    SCDRegs* regs;
    u32 buttons;
    s16 axis_x;
    s16 axis_y;
    s32 input_state;
    regs = SCD_REGS;

    g_debounced_input = 0;

    /* Unavailable controller types produce no input. */
    if (g_controller_device_type >= CHECKPS_CONTROLLER_UNAVAILABLE)
    {
        input_state = 0;
    }
    else
    {
        /* Convert the controller protocol bits to the game's logical layout. */
        buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (regs->device_type != 0)
        {
            axis_x = regs->axis_x.signed_value;

            if (axis_x < -1)
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }

            axis_y = regs->axis_y.signed_value;
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

    g_last_input_state = input_state;
    g_input_repeat_timer = CHECKPS_INITIAL_REPEAT_DELAY;
}
#endif
