/* Use the Game Over overlay's scene-state declarations. */
#define GOVER_C
#include "gover.h"
#include "akao.h"
#include "akao_cmd.h"
#include "display.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libetc.h"

/**
 * @brief Describes the palette section of a staged image resource.
 *
 * The palette data is stored inline. @p size locates the following pixel-data
 * section relative to its own field.
 */
typedef struct
{
    u8 _pad0[8];
    u32 size;
    u8 _pad1[4];
    u16 width;
    u16 height;
    u_long clutData;
} ClutSectionHeader;

/**
 * @brief Describes the pixel section of a staged image resource.
 *
 * The image dimensions are followed by the inline pixel data.
 */
typedef struct
{
    u8 _pad0[8];
    u16 w;
    u16 h;
    u_long data;
} PixelDataHeader;

/**
 * @brief VRAM destinations for an image's pixel and palette data.
 */
typedef struct
{
    u16 pixelX;
    u16 pixelY;
    u16 clutX;
    u16 clutY;
} VramDstCoords;

/**
 * @brief Holds a staged sequence for the AKAO audio driver.
 *
 * @p payload_offset identifies populated data and locates the sequence payload.
 */
typedef struct
{
    u32 payload_offset;
    u32 unk4;
    u32 unk8;
    u8 payload[1];
} AudioDataBlock;

/**
 * @brief One half of the Game Over screen's double-buffered frame.
 *
 * Each half owns its display environments, ordering table, and primitive
 * workspace. One half is displayed while the other is prepared.
 */
typedef struct GoverFrameHalf
{
    u8 otag[0x20];
    DISPENV disp;
    DRAWENV draw;
    RECT vramRect;
    u8 primBuf[0x400];
    u8* allocCursor;
} GoverFrameHalf;

/* Audio helpers used while presenting the Game Over screen. */
extern s32 func_800A368C(s32, s32);             /* Starts music from a resource. */
extern s32 func_800A380C(void);                  /* Applies pending music state. */
extern s32 func_800A39A8(s32, s32, s32, s32);  /* Plays the staged audio clip. */

/** @brief AKAO music volume applied by func_800A380C. */
extern s32 g_akao_music_volume;

extern u32 g_scene_mode;
extern s32 g_pending_game_state;
extern AudioDataBlock g_audio_data;
extern void cdrom_queue_read(s32 resourceIndex, void* dstBuffer);

/** Accesses a half of the contiguous double-buffered frame. */
#define FRAME_HALF(i) (((GoverFrameHalf*)(frameTail - 0x90))[i])

/** VRAM Y-coordinate where the Game Over image's CLUT is uploaded and sampled from. */
#define GOVER_CLUT_Y 480

/** Base CD resource for Game Over audio clips. */
#define GOVER_AUDIO_RESOURCE_BASE 81

/** Leaves the staged audio block unchanged. */
#define GOVER_AUDIO_CLIP_NONE (-2)

/** Clears the staged audio block without loading a replacement. */
#define GOVER_AUDIO_CLIP_CLEAR (-1)

/** RAM staging address used to load audio clip data from CD. */
#define GOVER_AUDIO_LOAD_ADDR 0x80180000

/** Base CD resource for the Game Over image. */
#define GOVER_IMAGE_RESOURCE_BASE 0xFFC

/** Maximum AKAO volume level (7-bit MIDI-style volume). */
#define AKAO_VOLUME_MAX 0x7F

/** Fade level representing full brightness. */
#define GOVER_FADE_FULL 0x80

/** Per-frame fade increment. */
#define GOVER_FADE_STEP 4

/** Buttons that dismiss the screen after the fade-in completes. */
#define GOVER_DISMISS_BUTTON_MASK 0x260

/** Locates sequence data within the staged audio resource. */
#define GOVER_AUDIO_DATA_OFFSET (*(u32*)(GOVER_AUDIO_LOAD_ADDR + 4))

const s32 g_goverOverlayId = 10;
s32 D_80140704;
s32 g_fadeStep;
s32 D_8014070C;

/* Adjacent storage for the Game Over screen's double-buffered frame. */
u8 g_goverFrameHeader[0x90];
u8 g_goverFrameTail[0x8A8];
s32 g_fadeLevel;

static void gover_load_audio_clip(s32 audio_clip_index);
static u32 gover_upload_image_to_vram(ClutSectionHeader* header, VramDstCoords* coordinates);
static void gover_load_image_from_cd(s32 cdResourceIndex, VramDstCoords* coordinates, u32 ramBuffer);
static void gover_build_otag(unsigned char* pOtBuf);
static void gover_run(void);

/**
 * @brief Loads and presents the Game Over screen.
 *
 * Initializes double-buffered rendering, uploads the screen artwork, starts
 * the requested audio, and runs the fade sequence.
 *
 * @param cdLoadAddr         RAM staging address for the image resource.
 * @param imageResourceIndex Game Over image resource index.
 * @param musicResourceIndex Music resource index, or -1 to skip music.
 * @param audioClipIndex     Audio clip index, or -1 to skip playback.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/1qYnn
 */
void gover_show_screen(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex)
{
    RECT rect;
    u8* frameTail;
    RECT* half1VramRect;
    GoverFrameHalf* halves;
    u8(*frameTailPtr)[];

    frameTailPtr = &g_goverFrameTail;
    VSync(0);
    DrawSync(0);
    frameTail = *frameTailPtr;

    // Place the display buffers in vertically adjacent VRAM regions.
    FRAME_HALF(0).vramRect.x = 0;
    FRAME_HALF(0).vramRect.y = 0;
    FRAME_HALF(0).vramRect.w = SCREEN_WIDTH;
    FRAME_HALF(0).vramRect.h = SCREEN_HEIGHT;

    half1VramRect = (RECT*)(frameTail + 0x49C);
    half1VramRect->x = 0;
    half1VramRect->y = VRAM_BACK_DISP_Y;
    half1VramRect->w = SCREEN_WIDTH;
    half1VramRect->h = SCREEN_HEIGHT;

    // Clear the entire VRAM frame area before uploading the new image.
    rect.x = 0;
    rect.y = 0;
    rect.w = VRAM_WIDTH;
    rect.h = VRAM_HEIGHT;
    ClearImage(&rect, 0, 0, 0);

    // Configure alternating display and draw regions.
    SetDefDispEnv(&FRAME_HALF(0).disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&FRAME_HALF(1).disp, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&FRAME_HALF(0).draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&FRAME_HALF(1).draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    // Disable dithering for both frame buffers.
    halves = &FRAME_HALF(0);
    halves[1].draw.dtd = 0;
    halves[0].draw.dtd = 0;

    // Stage the texture beside the frame buffers and its palette below them.
    rect.x = SCREEN_WIDTH;
    rect.y = 0;
    rect.w = 0;
    rect.h = GOVER_CLUT_Y;

    gover_load_image_from_cd(imageResourceIndex + GOVER_IMAGE_RESOURCE_BASE, (VramDstCoords*)(&rect), cdLoadAddr);

    akao_cmd_f0();
    akao_cmd_f1();
    akao_cmd_a8(AKAO_VOLUME_MAX);

    if (audioClipIndex != GOVER_AUDIO_CLIP_CLEAR)
    {
        gover_load_audio_clip(audioClipIndex);
        func_800A39A8(0, 0x80, 0, 0);
    }

    if (musicResourceIndex != -1)
    {
        func_800A368C(musicResourceIndex, 0);
        g_akao_music_volume = AKAO_VOLUME_MAX;
        func_800A380C();
        akao_cmd_c0(0, AKAO_VOLUME_MAX);
    }

    // Begin the fade-in; player input reverses it after full brightness.
    g_fadeLevel = GOVER_FADE_STEP;
    g_fadeStep = GOVER_FADE_STEP;
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
    GoverFrameHalf* current;
    GoverFrameHalf* drawing;
    GoverFrameHalf* next;
    u8 _match_pad[8];

    // Prime both ordering tables before enabling display output.
    func_800AA02C();
    current = (GoverFrameHalf*)g_goverFrameHeader;
    ClearOTagR((u_long*)current->otag, 8);
    ClearOTagR((u_long*)current[1].otag, 8);
    VSync(0);
    PutDispEnv(&current->disp);
    update_controllers();
    SetDispMask(1);

    drawing = current;
    while (1)
    {
        // Rebuild the next frame and advance the fade.
        drawing = current;
        ClearOTagR((u_long*)drawing->otag, 8);
        drawing->allocCursor = drawing->primBuf;
        func_800A9E78();
        gover_build_otag((unsigned char*)drawing);
        DrawSync(0);
        set_controller_vsync_interval(2);

        VSync(2);

        if ((g_fadeLevel == GOVER_FADE_FULL) && (g_pad_input & GOVER_DISMISS_BUTTON_MASK))
        {
            akao_cmd_c1(0, 0x20, 0);
            g_fadeStep = -GOVER_FADE_STEP;
        }

        if (g_fadeLevel == (0 & 0xFF))
        {
            break;
        }

        // Present the newly selected buffer and draw the frame just built.
        next = (GoverFrameHalf*)g_goverFrameHeader;
        if (current == (GoverFrameHalf*)g_goverFrameHeader)
        {
            next = current + 1;
        }
        current = next;
        PutDispEnv(&current->disp);

        PutDrawEnv(&current->draw);
        // Ordering-table links are traversed from the final entry.
        DrawOTag((u_long*)((u_char*)drawing + 0x1C));
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
 * @param pOtBuf Frame buffer containing the ordering table and primitive pool.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/q3LKi
 */
static void gover_build_otag(unsigned char* pOtBuf)
{
    GoverFrameHalf* half;
    unsigned char* pPrimA;
    unsigned char* pPrimB;
    unsigned char leftFadeLevel;
    unsigned char rightFadeLevel;

    if (g_fadeStep != 0)
    {
        g_fadeLevel += g_fadeStep;
    }

    if (g_fadeLevel == GOVER_FADE_FULL)
    {
        g_fadeStep = 0;
    }

    // Append primitives at the frame's current allocation cursor.
    half = (GoverFrameHalf*)pOtBuf;
    pPrimA = half->allocCursor;

    // Draw the left image region from its texture page.
    setSprt(pPrimA);

    leftFadeLevel = (unsigned char)g_fadeLevel;

    setXY0((SPRT*)pPrimA, 0, 0);
    setWH((SPRT*)pPrimA, 256, 224);
    setUV0((SPRT*)pPrimA, 0, 0);
    setClut((SPRT*)pPrimA, 0, GOVER_CLUT_Y);
    SET_BGR0((SPRT*)pPrimA, leftFadeLevel, leftFadeLevel, leftFadeLevel);
    addPrim(pOtBuf, pPrimA);

    pPrimA += sizeof(SPRT);

    setDrawTPage((DR_TPAGE*)pPrimA, 0, 0, getTPage(1, 1, SCREEN_WIDTH, 0));
    addPrim(pOtBuf, pPrimA);

    pPrimB = pPrimA + sizeof(DR_TPAGE);
    pPrimA = pPrimB;

    // Draw the remaining image region from the adjacent texture page.
    setSprt(pPrimB);

    rightFadeLevel = (unsigned char)g_fadeLevel;

    SET_BGR0((SPRT*)pPrimB, rightFadeLevel, rightFadeLevel, rightFadeLevel);
    setXY0((SPRT*)pPrimB, 256, 0);
    setWH((SPRT*)pPrimB, 64, 224);
    setUV0((SPRT*)pPrimB, 0, 0);
    setClut((SPRT*)pPrimB, 0, GOVER_CLUT_Y);

    addPrim(pOtBuf, pPrimB);

    pPrimA += sizeof(SPRT);

    setDrawTPage((DR_TPAGE*)pPrimA, 0, 0, getTPage(1, 1, SCREEN_WIDTH + 128, 0));
    pPrimB = pPrimA;
    pPrimB += sizeof(DR_TPAGE);

    addPrim(pOtBuf, pPrimA);

    half->allocCursor = pPrimB;
}

/**
 * @brief Reads a CD image resource into RAM, then uploads it to VRAM.
 *
 * @param cdResourceIndex CD resource index.
 * @param coordinates     VRAM destinations for the palette and pixel data.
 * @param ramBuffer       RAM staging address for the resource.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/OafFK
 */
static void gover_load_image_from_cd(s32 cdResourceIndex, VramDstCoords* coordinates, u32 ramBuffer)
{
    volatile u8 _match_pad[8];

    cdrom_queue_read(cdResourceIndex & 0xFFFF, (void*)ramBuffer);
    cdrom_wait_queue_empty();
    gover_upload_image_to_vram((ClutSectionHeader*)ramBuffer, coordinates);
}

/**
 * @brief Uploads a staged image's palette and pixel data to VRAM.
 *
 * @param header      Staged image resource.
 * @param coordinates VRAM destinations for the palette and pixel data.
 * @return Pixel width rounded up to a texture-page boundary.
 * @see decomp.me (100%) https://decomp.me/scratch/BEM7D
 */
static u32 gover_upload_image_to_vram(ClutSectionHeader* header, VramDstCoords* coordinates)
{
    RECT rect;
    PixelDataHeader* pdh;
    u32 clutSectionSize = header->size;

    rect.x = coordinates->clutX;
    rect.y = coordinates->clutY;
    rect.w = header->width * header->height;
    rect.h = 1;
    LoadImage(&rect, &header->clutData);

    // Locate the pixel section that follows the variable-length palette.
    pdh = (PixelDataHeader*)((u8*)header + 8 + clutSectionSize);

    rect.x = coordinates->pixelX;
    rect.y = coordinates->pixelY;
    rect.w = pdh->w;
    rect.h = pdh->h;
    LoadImage(&rect, &pdh->data);

    return ALIGN64(pdh->w);
}

/**
 * @brief Loads and plays a Game Over audio clip.
 *
 * Clears the current staged block, reads the selected resource, copies its
 * driver data, and submits the contained AKAO sequence.
 *
 * @param audio_clip_index Clip index, GOVER_AUDIO_CLIP_CLEAR to clear the
 *                         staged block, or GOVER_AUDIO_CLIP_NONE to leave it.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/G5r92
 */
static void gover_load_audio_clip(s32 audio_clip_index)
{
    AkaoSeqHeader* akao_seq;
    u8* dst;
    u8* src;
    s32* offsets;

    if (audio_clip_index == GOVER_AUDIO_CLIP_NONE)
    {
        return;
    }

    g_audio_data.unk8 = 0;
    g_audio_data.unk4 = 0;
    g_audio_data.payload_offset = 0;

    if (audio_clip_index == GOVER_AUDIO_CLIP_CLEAR)
    {
        return;
    }

    // Load the clip and find its sequence through the resource's offset table.
    cdrom_queue_read((audio_clip_index + GOVER_AUDIO_RESOURCE_BASE) & 0xFFFF, (void*)GOVER_AUDIO_LOAD_ADDR);
    cdrom_wait_queue_empty();
    g_audio_data.payload_offset = 0xC;

    src = (u8*)(GOVER_AUDIO_LOAD_ADDR + GOVER_AUDIO_DATA_OFFSET);
    offsets = (s32*)src;
    akao_seq = (AkaoSeqHeader*)(src + ((u32)offsets[*offsets]));
    dst = ((u8*)(&g_audio_data)) + 12;

    // Preserve the driver data that precedes the sequence.
    while (src != (u8*)akao_seq)
    {
        *(dst++) = *(src++);
    }

    akao_play_sequence_blocking(akao_seq, 1);
}
