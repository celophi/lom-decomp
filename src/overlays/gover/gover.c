/* Use the Game Over overlay's scene-state declarations. */
#define GOVER_C
#include "gover.h"
#include "akao.h"
#include "akao_cmd.h"
#include "display.h"
#include "pad.h"
#include "tim.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libetc.h"

/**
 * @brief VRAM destinations for a TIM's pixel and palette data.
 */
typedef struct
{
    u16 pixel_x;
    u16 pixel_y;
    u16 clut_x;
    u16 clut_y;
} TimUploadDestinations;

/**
 * @brief Holds SFX table data consumed by the AKAO audio driver.
 *
 * @c active_table_offset identifies populated data and locates the active SFX
 * table in @c table_data.
 */
typedef struct
{
    u32 active_table_offset;
    u32 reserved_0;
    u32 reserved_1;
    u8 table_data[1];
} SfxTableBuffer;

/**
 * @brief One half of the Game Over screen's double-buffered frame.
 *
 * Each half owns its display environments, ordering table, and primitive
 * workspace. One half is displayed while the other is prepared.
 */
typedef struct GoverFrameHalf
{
    u8 ordering_table[0x20];
    DISPENV display_environment;
    DRAWENV draw_environment;
    RECT vram_rect;
    u8 primitive_buffer[0x400];
    u8* allocation_cursor;
} GoverFrameHalf;

/* Audio helpers used while presenting the Game Over screen. */
extern s32 func_800A368C(s32 music_index, s32 destination_index); /* Loads and registers a music sequence. */
extern s32 func_800A380C(void);                                  /* Starts the registered music sequence. */
extern s32 func_800A39A8(s32 sfx_index, s32 volume, s32 unused, s32 channel_group); /* Plays a staged SFX. */

/** @brief AKAO music volume applied by func_800A380C. */
extern s32 g_akao_music_volume;

extern u32 g_scene_mode;
extern s32 g_pending_game_state;
extern SfxTableBuffer g_sfx_table_buffer;
extern void cdrom_queue_read(s32 resource_index, void* destination);

/** Accesses a half of the contiguous double-buffered frame. */
#define FRAME_HALF(index) (((GoverFrameHalf*)(frame_tail - 0x90))[index])

/** VRAM Y-coordinate where the Game Over image's CLUT is uploaded and sampled from. */
#define GOVER_CLUT_Y 480

/** Base CD resource for Game Over SFX banks. */
#define GOVER_SFX_RESOURCE_BASE 81

/** Reuses the currently staged SFX bank. */
#define GOVER_SFX_BANK_REUSE (-2)

/** Disables Game Over SFX playback. */
#define GOVER_SFX_DISABLED (-1)

/** RAM staging address used to load an SFX resource from CD. */
#define GOVER_SFX_LOAD_ADDR 0x80180000

/** Base CD resource for the Game Over image. */
#define GOVER_IMAGE_RESOURCE_BASE 0xFFC

/** Maximum 7-bit AKAO volume level. */
#define AKAO_VOLUME_MAX 0x7F

/** Fade level representing full brightness. */
#define GOVER_FADE_FULL 0x80

/** Per-frame fade increment. */
#define GOVER_FADE_STEP 4

/** Cross, Circle, and L3 dismiss the screen. */
#define GOVER_DISMISS_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_L3)

/** Locates the SFX table within the staged resource. */
#define GOVER_SFX_TABLE_OFFSET (*(u32*)(GOVER_SFX_LOAD_ADDR + 4))

const s32 g_gover_overlay_id = 10;
s32 D_80140704;
s32 g_fade_step;
s32 D_8014070C;

/* Adjacent storage for the Game Over screen's double-buffered frame. */
u8 g_gover_frame_header[0x90];
u8 g_gover_frame_tail[0x8A8];
s32 g_fade_level;

static void gover_load_sfx_bank(s32 sfx_bank_index);
static u32 gover_upload_image_to_vram(Tim* tim, TimUploadDestinations* destinations);
static void gover_load_image_from_cd(s32 cd_resource_index, TimUploadDestinations* destinations, u32 ram_buffer);
static void gover_build_otag(unsigned char* frame_buffer);
static void gover_run(void);

/**
 * @brief Loads and presents the Game Over screen.
 *
 * Initializes double-buffered rendering, uploads the screen artwork, starts
 * the requested audio, and runs the fade sequence.
 *
 * @param cd_load_address      RAM staging address for the image resource.
 * @param image_index          Game Over image index.
 * @param music_index          Music index, or -1 to skip music.
 * @param sfx_bank_index       SFX bank index, -1 to skip playback, or -2 to
 *                             reuse the currently staged bank.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/1qYnn
 */
void gover_show_screen(
    s32 cd_load_address, s32 image_index, s32 music_index, s32 sfx_bank_index)
{
    RECT vram_rect;
    u8* frame_tail;
    RECT* back_vram_rect;
    GoverFrameHalf* frames;
    u8(*frame_tail_ptr)[];

    frame_tail_ptr = &g_gover_frame_tail;
    VSync(0);
    DrawSync(0);
    frame_tail = *frame_tail_ptr;

    // Place the display buffers in vertically adjacent VRAM regions.
    FRAME_HALF(0).vram_rect.x = 0;
    FRAME_HALF(0).vram_rect.y = 0;
    FRAME_HALF(0).vram_rect.w = SCREEN_WIDTH;
    FRAME_HALF(0).vram_rect.h = SCREEN_HEIGHT;

    back_vram_rect = (RECT*)(frame_tail + 0x49C);
    back_vram_rect->x = 0;
    back_vram_rect->y = VRAM_BACK_DISP_Y;
    back_vram_rect->w = SCREEN_WIDTH;
    back_vram_rect->h = SCREEN_HEIGHT;

    // Clear the entire VRAM frame area before uploading the new image.
    vram_rect.x = 0;
    vram_rect.y = 0;
    vram_rect.w = VRAM_WIDTH;
    vram_rect.h = VRAM_HEIGHT;
    ClearImage(&vram_rect, 0, 0, 0);

    // Configure alternating display and draw regions.
    SetDefDispEnv(&FRAME_HALF(0).display_environment, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&FRAME_HALF(1).display_environment, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&FRAME_HALF(0).draw_environment, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&FRAME_HALF(1).draw_environment, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    // Disable dithering for both frame buffers.
    frames = &FRAME_HALF(0);
    frames[1].draw_environment.dtd = 0;
    frames[0].draw_environment.dtd = 0;

    // Stage the texture beside the frame buffers and its palette below them.
    vram_rect.x = SCREEN_WIDTH;
    vram_rect.y = 0;
    vram_rect.w = 0;
    vram_rect.h = GOVER_CLUT_Y;

    gover_load_image_from_cd(
        image_index + GOVER_IMAGE_RESOURCE_BASE, (TimUploadDestinations*)(&vram_rect), cd_load_address);

    akao_cmd_f0();
    akao_cmd_f1();
    akao_cmd_a8(AKAO_VOLUME_MAX);

    if (sfx_bank_index != GOVER_SFX_DISABLED)
    {
        gover_load_sfx_bank(sfx_bank_index);
        func_800A39A8(0, 0x80, 0, 0);
    }

    if (music_index != -1)
    {
        func_800A368C(music_index, 0);
        g_akao_music_volume = AKAO_VOLUME_MAX;
        func_800A380C();
        akao_cmd_c0(0, AKAO_VOLUME_MAX);
    }

    // Begin the fade-in; player input reverses it after full brightness.
    g_fade_level = GOVER_FADE_STEP;
    g_fade_step = GOVER_FADE_STEP;
    gover_run();
}

/**
 * @brief Runs the Game Over screen until its fade-out completes.
 *
 * Builds and displays alternating frames while processing controller and CD
 * state. Dismissal begins the fade-out and returns control to the game state.
 *
 * @param void No parameters.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/IfwJm
 */
static void gover_run(void)
{
    GoverFrameHalf* current_frame;
    GoverFrameHalf* drawing_frame;
    GoverFrameHalf* next_frame;
    u8 padding[8];

    // Prime both ordering tables before enabling display output.
    func_800AA02C();
    current_frame = (GoverFrameHalf*)g_gover_frame_header;
    ClearOTagR((u_long*)current_frame->ordering_table, 8);
    ClearOTagR((u_long*)current_frame[1].ordering_table, 8);
    VSync(0);
    PutDispEnv(&current_frame->display_environment);
    update_controllers();
    SetDispMask(1);

    drawing_frame = current_frame;
    while (1)
    {
        // Rebuild the next frame and advance the fade.
        drawing_frame = current_frame;
        ClearOTagR((u_long*)drawing_frame->ordering_table, 8);
        drawing_frame->allocation_cursor = drawing_frame->primitive_buffer;
        func_800A9E78();
        gover_build_otag((unsigned char*)drawing_frame);
        DrawSync(0);
        set_controller_vsync_interval(2);

        VSync(2);

        if ((g_fade_level == GOVER_FADE_FULL) && (g_pad_input & GOVER_DISMISS_BUTTON_MASK))
        {
            akao_cmd_c1(0, 0x20, 0);
            g_fade_step = -GOVER_FADE_STEP;
        }

        if (g_fade_level == (0 & 0xFF))
        {
            break;
        }

        // Present the newly selected buffer and draw the frame just built.
        next_frame = (GoverFrameHalf*)g_gover_frame_header;
        if (current_frame == (GoverFrameHalf*)g_gover_frame_header)
        {
            next_frame = current_frame + 1;
        }
        current_frame = next_frame;
        PutDispEnv(&current_frame->display_environment);

        PutDrawEnv(&current_frame->draw_environment);
        // Ordering-table links are traversed from the final entry.
        DrawOTag((u_long*)((u_char*)drawing_frame + 0x1C));
        update_controllers();
        cdrom_process_state();
    }

    DrawSync(0);
    VSync(0);
    reset_controller_vsync_state();
    akao_cmd_f0();
    akao_cmd_f1();
    SetDispMask(0);
    g_scene_mode = 0;
    func_800AA02C();
    g_pending_game_state = 1;
}

/**
 * @brief Builds the textured primitives for one Game Over frame.
 *
 * Splits the artwork across two texture pages and modulates both sprites with
 * the current fade level.
 *
 * @param frame_buffer Frame buffer containing the ordering table and primitive pool.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/q3LKi
 */
static void gover_build_otag(unsigned char* frame_buffer)
{
    GoverFrameHalf* frame;
    unsigned char* primitive_a;
    unsigned char* primitive_b;
    unsigned char left_fade_level;
    unsigned char right_fade_level;

    if (g_fade_step != 0)
    {
        g_fade_level += g_fade_step;
    }

    if (g_fade_level == GOVER_FADE_FULL)
    {
        g_fade_step = 0;
    }

    // Append primitives at the frame's current allocation cursor.
    frame = (GoverFrameHalf*)frame_buffer;
    primitive_a = frame->allocation_cursor;

    // Draw the left image region from its texture page.
    setSprt(primitive_a);

    left_fade_level = (unsigned char)g_fade_level;

    setXY0((SPRT*)primitive_a, 0, 0);
    setWH((SPRT*)primitive_a, 256, 224);
    setUV0((SPRT*)primitive_a, 0, 0);
    setClut((SPRT*)primitive_a, 0, GOVER_CLUT_Y);
    SET_BGR0((SPRT*)primitive_a, left_fade_level, left_fade_level, left_fade_level);
    addPrim(frame_buffer, primitive_a);

    primitive_a += sizeof(SPRT);

    setDrawTPage((DR_TPAGE*)primitive_a, 0, 0, getTPage(1, 1, SCREEN_WIDTH, 0));
    addPrim(frame_buffer, primitive_a);

    primitive_b = primitive_a + sizeof(DR_TPAGE);
    primitive_a = primitive_b;

    // Draw the remaining image region from the adjacent texture page.
    setSprt(primitive_b);

    right_fade_level = (unsigned char)g_fade_level;

    SET_BGR0((SPRT*)primitive_b, right_fade_level, right_fade_level, right_fade_level);
    setXY0((SPRT*)primitive_b, 256, 0);
    setWH((SPRT*)primitive_b, 64, 224);
    setUV0((SPRT*)primitive_b, 0, 0);
    setClut((SPRT*)primitive_b, 0, GOVER_CLUT_Y);

    addPrim(frame_buffer, primitive_b);

    primitive_a += sizeof(SPRT);

    setDrawTPage((DR_TPAGE*)primitive_a, 0, 0, getTPage(1, 1, SCREEN_WIDTH + 128, 0));
    primitive_b = primitive_a;
    primitive_b += sizeof(DR_TPAGE);

    addPrim(frame_buffer, primitive_a);

    frame->allocation_cursor = primitive_b;
}

/**
 * @brief Reads a CD image resource into RAM, then uploads it to VRAM.
 *
 * @param cd_resource_index CD resource index.
 * @param destinations      VRAM destinations for the TIM blocks.
 * @param ram_buffer        RAM staging address for the resource.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/OafFK
 */
static void gover_load_image_from_cd(s32 cd_resource_index, TimUploadDestinations* destinations, u32 ram_buffer)
{
    volatile u8 padding[8];

    cdrom_queue_read(cd_resource_index & 0xFFFF, (void*)ram_buffer);
    cdrom_wait_queue_empty();
    gover_upload_image_to_vram((Tim*)ram_buffer, destinations);
}

/**
 * @brief Uploads a staged 8bpp TIM to VRAM.
 *
 * @param tim          Staged TIM resource.
 * @param destinations VRAM destinations for the palette and pixel data.
 * @return Pixel-block width in 16-bit VRAM words, rounded up to a 64-word
 *         texture-page boundary.
 * @see decomp.me (100%) https://decomp.me/scratch/BEM7D
 */
static u32 gover_upload_image_to_vram(Tim* tim, TimUploadDestinations* destinations)
{
    RECT upload_rect;
    TimBlock* pixel_block;
    u32 clut_block_length = tim->clut_block.bnum;

    upload_rect.x = destinations->clut_x;
    upload_rect.y = destinations->clut_y;
    upload_rect.w = tim->clut_block.w * tim->clut_block.h;
    upload_rect.h = 1;
    LoadImage(&upload_rect, tim->clut_data);

    // Locate the pixel block that follows the variable-length CLUT block.
    pixel_block = TIM_PIXEL_BLOCK(tim, clut_block_length);

    upload_rect.x = destinations->pixel_x;
    upload_rect.y = destinations->pixel_y;
    upload_rect.w = pixel_block->w;
    upload_rect.h = pixel_block->h;
    LoadImage(&upload_rect, pixel_block + 1);

    return ALIGN64(pixel_block->w);
}

/**
 * @brief Loads and registers a Game Over SFX bank.
 *
 * Clears the staged bank, reads the selected resource, copies its SFX table,
 * and submits the accompanying AKAO program to the driver.
 *
 * @param sfx_bank_index Bank index, GOVER_SFX_DISABLED to clear the staged
 *                       bank, or GOVER_SFX_BANK_REUSE to leave it unchanged.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/G5r92
 */
static void gover_load_sfx_bank(s32 sfx_bank_index)
{
    AkaoSeqHeader* akao_program;
    u8* table_destination;
    u8* table_source;
    s32* sfx_table;

    if (sfx_bank_index == GOVER_SFX_BANK_REUSE)
    {
        return;
    }

    g_sfx_table_buffer.reserved_1 = 0;
    g_sfx_table_buffer.reserved_0 = 0;
    g_sfx_table_buffer.active_table_offset = 0;

    if (sfx_bank_index == GOVER_SFX_DISABLED)
    {
        return;
    }

    // Load the resource and locate its first SFX table.
    cdrom_queue_read((sfx_bank_index + GOVER_SFX_RESOURCE_BASE) & 0xFFFF, (void*)GOVER_SFX_LOAD_ADDR);
    cdrom_wait_queue_empty();
    g_sfx_table_buffer.active_table_offset = 0xC;

    table_source = (u8*)(GOVER_SFX_LOAD_ADDR + GOVER_SFX_TABLE_OFFSET);
    sfx_table = (s32*)table_source;
    akao_program = (AkaoSeqHeader*)(table_source + ((u32)sfx_table[*sfx_table]));
    table_destination = g_sfx_table_buffer.table_data;

    // Preserve the SFX table that precedes the AKAO program.
    while (table_source != (u8*)akao_program)
    {
        *(table_destination++) = *(table_source++);
    }

    akao_play_sequence_blocking(akao_program, 1);
}
