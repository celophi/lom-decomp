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

/** @brief VRAM destinations for a TIM's pixel and palette blocks. */
typedef struct
{
    u16 pixel_x;
    u16 pixel_y;
    u16 clut_x;
    u16 clut_y;
} TimUploadDestinations;

/** @brief Stack storage reused for VRAM clearing and TIM upload destinations. */
typedef union
{
    RECT clear_rect;
    TimUploadDestinations upload_destinations;
} GoverVramTransfer;

/**
 * @brief Stores the active SFX offset table for the AKAO driver.
 *
 * @c active_table_offset is zero when empty; otherwise it contains the byte
 * offset from the buffer start to @c table_data.
 */
typedef struct
{
    u32 active_table_offset;
    u32 reserved_0;
    u32 reserved_1;
    u8 table_data[1];
} SfxTableBuffer;

/**
 * @brief Describes a self-relative table of resource offsets.
 *
 * The first word gives the number of entries; each following word is a byte
 * offset from the start of this table.
 */
typedef union
{
    struct
    {
        u32 entry_count;
        u32 entry_offsets[1];
    } header;
    u8 bytes[1];
} ResourceOffsetTable;

/**
 * @brief Returns the address stored in the offset table's final entry.
 *
 * @note The commuted pointer addition preserves the original instruction
 *       operand order.
 */
#define RESOURCE_TABLE_END(table) ((table)->bytes + *(((table)->header.entry_count - 1) + (table)->header.entry_offsets))

/** Number of entries in each Game Over ordering table. */
#define GOVER_OTAG_LENGTH 8

/** Number of alternating Game Over frame buffers. */
#define GOVER_FRAME_COUNT 2

/**
 * @brief Holds one Game Over display buffer and its GPU packet workspace.
 *
 * One frame is displayed while the other is prepared.
 */
typedef struct GoverFrameHalf
{
    u_long ordering_table[GOVER_OTAG_LENGTH];
    DISPENV display_environment;
    DRAWENV draw_environment;
    RECT vram_rect;
    u8 primitive_buffer[0x400];
    void* allocation_cursor;
} GoverFrameHalf;

/** @brief Provides typed views over a position in the GPU packet stream. */
typedef union
{
    SPRT sprite;
    DR_TPAGE draw_tpage;
    u8 bytes[1];
} GoverPrimitive;

/** @brief Advances a GPU packet cursor past a packet of @p type. */
#define NEXT_GOVER_PRIMITIVE(primitive, type) ((GoverPrimitive*)((primitive)->bytes + sizeof(type)))

/* Audio helpers used while presenting the Game Over screen. */
/** @brief Loads and registers a music sequence. */
extern void func_800A368C(s32 music_index, s32 destination_index);

/** @brief Starts the registered music sequence. */
extern void func_800A380C(void);

/** @brief Plays an SFX from the staged bank. */
extern void func_800A39A8(s32 sfx_index, s32 pan, s32 unused, s32 channel_group);

/** @brief AKAO music volume applied by func_800A380C. */
extern s32 g_akao_music_volume;

extern u32 g_scene_mode;
extern s32 g_pending_game_state;
extern SfxTableBuffer g_sfx_table_buffer;
extern void cdrom_queue_read(s32 resource_index, void* destination);

/**
 * @brief Byte offset of @c vram_rect within a Game Over frame.
 *
 * The linker exposes @c g_gover_frame_tail beginning at that member of frame
 * zero, so show-screen setup uses this offset to recover the complete frames.
 */
#define GOVER_FRAME_VRAM_RECT_OFFSET (sizeof(u_long[GOVER_OTAG_LENGTH]) + sizeof(DISPENV) + sizeof(DRAWENV))

/** Accesses a complete frame through the linker-exposed frame-tail symbol. */
#define GOVER_FRAME_FROM_TAIL(tail, index) (((GoverFrameHalf*)((tail) - GOVER_FRAME_VRAM_RECT_OFFSET))[index])

/** VRAM Y-coordinate where the Game Over image's CLUT is uploaded and sampled from. */
#define GOVER_CLUT_Y 480

/** Height of the displayed Game Over artwork. */
#define GOVER_IMAGE_HEIGHT 224

/** Number of 8bpp image pixels covered by the first texture page. */
#define GOVER_TEXTURE_PAGE_WIDTH 256

/** VRAM X-coordinate of the texture page holding the artwork's right edge. */
#define GOVER_SECOND_TPAGE_X (SCREEN_WIDTH + 128)

/** Base CD resource for Game Over SFX banks. */
#define GOVER_SFX_RESOURCE_BASE 81

/** Reuses the currently staged SFX bank. */
#define GOVER_SFX_BANK_REUSE (-2)

/** Disables Game Over SFX playback. */
#define GOVER_SFX_DISABLED (-1)

/** Disables Game Over music playback. */
#define GOVER_MUSIC_DISABLED (-1)

/** Center position for the staged Game Over SFX. */
#define AKAO_PAN_CENTER 0x80

/** RAM staging buffer used to load an SFX resource from CD. */
#define GOVER_SFX_LOAD_BUFFER ((u8*)0x80180000)

/** Offset from the SFX table-buffer header to its copied table data. */
#define GOVER_SFX_TABLE_DATA_OFFSET 0xC

/** Base CD resource for the Game Over image. */
#define GOVER_IMAGE_RESOURCE_BASE 0xFFC

/** Fade level representing full brightness. */
#define GOVER_FADE_FULL 0x80

/** Per-frame fade increment. */
#define GOVER_FADE_STEP 4

/** Duration passed to AKAO when fading out the Game Over music. */
#define GOVER_MUSIC_FADE_OUT_DURATION 0x20

/** Cross, Circle, and L3 dismiss the screen. */
#define GOVER_DISMISS_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_L3)

/** Locates the first nested offset table within the staged resource. */
#define GOVER_SFX_TABLE_OFFSET (*(u32*)0x80180004)

/** The SFX table in the resource currently held by the staging buffer. */
#define GOVER_LOADED_SFX_TABLE ((ResourceOffsetTable*)(GOVER_SFX_LOAD_BUFFER + GOVER_SFX_TABLE_OFFSET))

const s32 g_gover_overlay_id = 10;

/** Unreferenced BSS word retained for the original overlay layout. */
s32 D_80140704;

s32 g_fade_step;

/** Unreferenced BSS word retained for the original overlay layout. */
s32 D_8014070C;

/* Linker-split storage for the Game Over screen's contiguous frame pair. */
u8 g_gover_frame_header[GOVER_FRAME_VRAM_RECT_OFFSET];
u8 g_gover_frame_tail[sizeof(GoverFrameHalf) * GOVER_FRAME_COUNT - GOVER_FRAME_VRAM_RECT_OFFSET];
s32 g_fade_level;

/** Typed view of the contiguous Game Over frame buffers. */
#define GOVER_FRAMES ((GoverFrameHalf*)g_gover_frame_header)

static void gover_load_sfx_bank(s32 sfx_bank_index);
static u32 gover_upload_image_to_vram(Tim* tim, TimUploadDestinations* destinations);
static void gover_load_image_from_cd(s32 resource_index, TimUploadDestinations* destinations, Tim* image_buffer);
static void gover_build_otag(GoverFrameHalf* frame);
static void gover_run(void);

/**
 * @brief Initializes and presents the Game Over screen.
 *
 * Configures double-buffered rendering, loads the artwork and optional audio,
 * then runs the screen until fade-out completes.
 *
 * @param image_buffer Writable staging buffer for the TIM resource.
 * @param image_index Image index relative to @c GOVER_IMAGE_RESOURCE_BASE.
 * @param music_index Music index, or @c GOVER_MUSIC_DISABLED.
 * @param sfx_bank_index SFX bank index, @c GOVER_SFX_DISABLED, or
 *                       @c GOVER_SFX_BANK_REUSE.
 * @see https://decomp.me/scratch/1qYnn (100% match)
 */
void gover_show_screen(Tim* image_buffer, s32 image_index, s32 music_index, s32 sfx_bank_index)
{
    GoverVramTransfer vram_transfer;
    u8* frame_tail;
    RECT* back_vram_rect;
    GoverFrameHalf* frames;

    VSync(0);
    DrawSync(0);
    frame_tail = g_gover_frame_tail;

    // Place the display buffers in vertically adjacent VRAM regions.
    setRECT(&GOVER_FRAME_FROM_TAIL(frame_tail, 0).vram_rect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // The temporary preserves the original address calculation.
    back_vram_rect = &GOVER_FRAME_FROM_TAIL(frame_tail, 1).vram_rect;
    setRECT(back_vram_rect, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Clear the entire VRAM frame area before uploading the new image.
    setRECT(&vram_transfer.clear_rect, 0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    ClearImage(&vram_transfer.clear_rect, 0, 0, 0);

    // Configure alternating display and draw regions.
    SetDefDispEnv(&GOVER_FRAME_FROM_TAIL(frame_tail, 0).display_environment, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&GOVER_FRAME_FROM_TAIL(frame_tail, 1).display_environment, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&GOVER_FRAME_FROM_TAIL(frame_tail, 0).draw_environment, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&GOVER_FRAME_FROM_TAIL(frame_tail, 1).draw_environment, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    // Disable dithering for both frame buffers.
    frames = &GOVER_FRAME_FROM_TAIL(frame_tail, 0);
    frames[1].draw_environment.dtd = 0;
    frames[0].draw_environment.dtd = 0;

    // Stage the texture beside the frame buffers and its palette below them.
    vram_transfer.upload_destinations.pixel_x = SCREEN_WIDTH;
    vram_transfer.upload_destinations.pixel_y = 0;
    vram_transfer.upload_destinations.clut_x = 0;
    vram_transfer.upload_destinations.clut_y = GOVER_CLUT_Y;

    gover_load_image_from_cd(image_index + GOVER_IMAGE_RESOURCE_BASE, &vram_transfer.upload_destinations, image_buffer);

    akao_cmd_f0();
    akao_cmd_f1();
    akao_cmd_a8(AKAO_VOLUME_MAX);

    if (sfx_bank_index != GOVER_SFX_DISABLED)
    {
        gover_load_sfx_bank(sfx_bank_index);
        func_800A39A8(0, AKAO_PAN_CENTER, 0, 0);
    }

    if (music_index != GOVER_MUSIC_DISABLED)
    {
        func_800A368C(music_index, 0);
        g_akao_music_volume = AKAO_VOLUME_MAX;
        func_800A380C();
        akao_set_song_volume(0, AKAO_VOLUME_MAX);
    }

    // Begin the fade-in; player input reverses it after full brightness.
    g_fade_level = GOVER_FADE_STEP;
    g_fade_step = GOVER_FADE_STEP;
    gover_run();
}

/**
 * @brief Runs the Game Over rendering and input loop.
 *
 * Builds alternating frames until dismissal triggers a completed fade-out.
 *
 * @see https://decomp.me/scratch/IfwJm (100% match)
 */
static void gover_run(void)
{
    GoverFrameHalf* current_frame;
    GoverFrameHalf* drawing_frame;
    GoverFrameHalf* next_frame;
    // Required to preserve the original stack frame.
    u8 stack_padding[8];

    // Prime both ordering tables before enabling display output.
    func_800AA02C();
    current_frame = GOVER_FRAMES;
    ClearOTagR(current_frame->ordering_table, GOVER_OTAG_LENGTH);
    ClearOTagR(current_frame[1].ordering_table, GOVER_OTAG_LENGTH);
    VSync(0);
    PutDispEnv(&current_frame->display_environment);
    update_controllers();
    SetDispMask(1);

    while (1)
    {
        // Rebuild the next frame and advance the fade.
        drawing_frame = current_frame;
        ClearOTagR(drawing_frame->ordering_table, GOVER_OTAG_LENGTH);
        drawing_frame->allocation_cursor = drawing_frame->primitive_buffer;
        func_800A9E78();
        gover_build_otag(drawing_frame);
        DrawSync(0);
        set_controller_vsync_interval(2);

        VSync(2);

        if ((g_fade_level == GOVER_FADE_FULL) && (g_pad_input & GOVER_DISMISS_BUTTON_MASK))
        {
            // Fade out the music while reversing the screen fade.
            akao_cmd_c1(0, GOVER_MUSIC_FADE_OUT_DURATION, 0);
            g_fade_step = -GOVER_FADE_STEP;
        }

        if (g_fade_level == 0)
        {
            break;
        }

        // Present the newly selected buffer and draw the frame just built.
        next_frame = GOVER_FRAMES;
        if (current_frame == GOVER_FRAMES)
        {
            next_frame = current_frame + 1;
        }
        current_frame = next_frame;
        PutDispEnv(&current_frame->display_environment);

        PutDrawEnv(&current_frame->draw_environment);
        // Ordering-table links are traversed from the final entry.
        DrawOTag(&drawing_frame->ordering_table[GOVER_OTAG_LENGTH - 1]);
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
 * @brief Builds the GPU packets for one Game Over frame.
 *
 * Splits the artwork across two texture pages and modulates both sprites with
 * the current fade level.
 *
 * @param frame Frame whose ordering table and packet workspace are populated.
 * @see https://decomp.me/scratch/q3LKi (100% match)
 */
static void gover_build_otag(GoverFrameHalf* frame)
{
    GoverPrimitive* cursor;
    GoverPrimitive* next_cursor;
    u8 left_fade_level;
    u8 right_fade_level;

    if (g_fade_step != 0)
    {
        g_fade_level += g_fade_step;
    }

    if (g_fade_level == GOVER_FADE_FULL)
    {
        g_fade_step = 0;
    }

    // Append primitives at the frame's current allocation cursor.
    cursor = frame->allocation_cursor;

    // Draw the left image region from its texture page.
    setSprt(&cursor->sprite);

    left_fade_level = g_fade_level;

    setXY0(&cursor->sprite, 0, 0);
    setWH(&cursor->sprite, GOVER_TEXTURE_PAGE_WIDTH, GOVER_IMAGE_HEIGHT);
    setUV0(&cursor->sprite, 0, 0);
    setClut(&cursor->sprite, 0, GOVER_CLUT_Y);
    SET_BGR0(&cursor->sprite, left_fade_level, left_fade_level, left_fade_level);
    addPrim(frame->ordering_table, cursor);

    cursor = NEXT_GOVER_PRIMITIVE(cursor, SPRT);

    setDrawTPage(&cursor->draw_tpage, 0, 0, getTPage(1, 1, SCREEN_WIDTH, 0));
    addPrim(frame->ordering_table, cursor);

    next_cursor = NEXT_GOVER_PRIMITIVE(cursor, DR_TPAGE);
    cursor = next_cursor;

    // Draw the remaining image region from the adjacent texture page.
    setSprt(&next_cursor->sprite);

    right_fade_level = g_fade_level;

    SET_BGR0(&next_cursor->sprite, right_fade_level, right_fade_level, right_fade_level);
    setXY0(&next_cursor->sprite, GOVER_TEXTURE_PAGE_WIDTH, 0);
    setWH(&next_cursor->sprite, SCREEN_WIDTH - GOVER_TEXTURE_PAGE_WIDTH, GOVER_IMAGE_HEIGHT);
    setUV0(&next_cursor->sprite, 0, 0);
    setClut(&next_cursor->sprite, 0, GOVER_CLUT_Y);

    addPrim(frame->ordering_table, next_cursor);

    cursor = NEXT_GOVER_PRIMITIVE(cursor, SPRT);

    setDrawTPage(&cursor->draw_tpage, 0, 0, getTPage(1, 1, GOVER_SECOND_TPAGE_X, 0));
    // Separate assignments preserve the original register allocation.
    next_cursor = cursor;
    next_cursor = NEXT_GOVER_PRIMITIVE(next_cursor, DR_TPAGE);

    addPrim(frame->ordering_table, cursor);

    frame->allocation_cursor = next_cursor;
}

/**
 * @brief Loads a TIM resource from CD and uploads it to VRAM.
 *
 * @param resource_index CD resource identifier.
 * @param destinations VRAM destinations for the TIM blocks.
 * @param image_buffer Writable staging buffer for the TIM resource.
 * @see https://decomp.me/scratch/OafFK (100% match)
 */
static void gover_load_image_from_cd(s32 resource_index, TimUploadDestinations* destinations, Tim* image_buffer)
{
    // Required to preserve the original stack frame.
    volatile u8 padding[8];
    u16 resource_id;

    resource_id = resource_index;
    cdrom_queue_read(resource_id, image_buffer);
    cdrom_wait_queue_empty();
    gover_upload_image_to_vram(image_buffer, destinations);
}

/**
 * @brief Uploads the palette and pixel blocks of a staged 8bpp TIM.
 *
 * @param tim Staged TIM resource.
 * @param destinations VRAM destinations for the palette and pixel data.
 * @return Pixel-block width in 16-bit VRAM words, rounded up to a 64-word
 *         texture-page boundary.
 * @see https://decomp.me/scratch/BEM7D (100% match)
 */
static u32 gover_upload_image_to_vram(Tim* tim, TimUploadDestinations* destinations)
{
    RECT upload_rect;
    TimBlock* pixel_block;
    u32 clut_block_length = tim->clut_block.bnum;

    setRECT(&upload_rect, destinations->clut_x, destinations->clut_y,
            tim->clut_block.dimensions.width * tim->clut_block.dimensions.height, 1);
    LoadImage(&upload_rect, tim->clut_data);

    // Locate the pixel block that follows the variable-length CLUT block.
    pixel_block = TIM_PIXEL_BLOCK(tim, clut_block_length);

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y,
            pixel_block->dimensions.width, pixel_block->dimensions.height);
    LoadImage(&upload_rect, pixel_block + 1);

    return ALIGN64(pixel_block->dimensions.width);
}

/**
 * @brief Loads and registers a Game Over SFX bank with the AKAO driver.
 *
 * Clears the previous table, copies the selected table into driver storage,
 * and uploads the accompanying AKAO instrument bank.
 *
 * @param sfx_bank_index Bank index, @c GOVER_SFX_DISABLED to clear the staged
 *                       bank, or @c GOVER_SFX_BANK_REUSE to keep it unchanged.
 * @see https://decomp.me/scratch/G5r92 (100% match)
 */
static void gover_load_sfx_bank(s32 sfx_bank_index)
{
    ResourceOffsetTable* loaded_table;
    u8* copy_destination;
    u8* copy_source;
    void* akao_bank;
    u16 resource_index;

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
    resource_index = sfx_bank_index + GOVER_SFX_RESOURCE_BASE;
    cdrom_queue_read(resource_index, GOVER_SFX_LOAD_BUFFER);
    cdrom_wait_queue_empty();

    g_sfx_table_buffer.active_table_offset = GOVER_SFX_TABLE_DATA_OFFSET;
    loaded_table = GOVER_LOADED_SFX_TABLE;
    copy_source = loaded_table->bytes;
    akao_bank = RESOURCE_TABLE_END(loaded_table);
    copy_destination = g_sfx_table_buffer.table_data;

    // Preserve the SFX table that precedes the AKAO bank.
    while (copy_source != akao_bank)
    {
        *copy_destination++ = *copy_source++;
    }

    akao_upload_bank_blocking(akao_bank, 1);
}
