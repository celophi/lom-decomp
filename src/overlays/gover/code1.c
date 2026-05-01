
#include "gover.h"

/**
 * FRAME_HALF(i) names the i-th GoverFrameHalf relative to the tail anchor. 
 * (frameTail - 0x90) == &halves[0]; 
 * this expression constant-folds back to frameTail-relative offsets in the emitted code.
 */
#define FRAME_HALF(i) (((GoverFrameHalf*)(frameTail - 0x90))[i])

const s32 g_goverOverlayId = 10;
s32 D_80140704;
s32 g_fadeStep;
s32 D_8014070C;

/*
 * The Game Over screen's double-buffered frame (2x GoverFrameHalf, 0x938 bytes
 * total) is split across two adjacent globals. They alias the same buffer:
 *
 *   g_goverFrameHeader  — &halves[0]              (struct start, 0x90 bytes)
 *   g_goverFrameTail    — &halves[0].vramRect     (= g_goverFrameHeader + 0x90)
 *
 * The asymmetric split is a fossil of incremental development: in the
 * single-buffered version, the first 0x90 bytes were the per-frame render
 * header (otag + DISPENV + DRAWENV) and the remaining bytes were a transient
 * frame-data buffer. When double-buffering was added, the second half was
 * appended onto the data buffer rather than refactoring into a struct array.
 *
 * As a result, gover_show_screen anchors most of its accesses on the tail
 * symbol (where the cluster of writes sits) and RunGameOver anchors on the
 * header symbol (where its loop starts). Merging them into one symbol breaks
 * the relocation entries in the original object file — keep them separate.
 */
u8 g_goverFrameHeader[0x90];
u8 g_goverFrameTail[0x8A8];
s32 g_fadeLevel;

/**
 * @brief Entry point for the Game Over screen overlay.
 *
 * @details Initializes a double-buffered 320x240 display, uploads the Game Over
 * image and palette from CD into VRAM, optionally starts background music and a
 * one-shot audio cue, primes the fade-in state, and hands control to the
 * per-frame loop in RunGameOver().
 *
 * The display structures live in one contiguous double-buffered frame; see
 * @p GoverFrameHalf for the field layout, and the comment block above
 * @p g_goverFrameHeader for the symbol-split rationale. This function anchors
 * its accesses on @p g_goverFrameTail (mid-buffer) while RunGameOver anchors
 * on @p g_goverFrameHeader (buffer start).
 *
 * The two halves are stacked vertically in VRAM (Y=0 and Y=232) for double
 * buffering; the CLUT lands at VRAM (0, 480) and the pixel data at VRAM (320, 0),
 * matching the SPRT/DR_TPAGE primitives produced by BuildOTag.
 *
 * @param cdLoadAddr           RAM staging address that receives the raw CD image
 *                             data before VRAM upload (e.g. 0x80160000).
 * @param imageResourceIndex   Logical resource index for the Game Over image; the
 *                             actual CD resource is @p imageResourceIndex + 0xFFC
 *                             after truncation to 16 bits.
 * @param musicResourceIndex   Background music resource to start, or -1 to skip.
 * @param audioClipIndex       One-shot audio clip (voice/SFX) to play, or -1 to skip.
 *
 * @note The implementation takes 4 arguments; the prototype in main.h exposes 7
 *       to preserve the call-site codegen in main.c. Only the first four are
 *       meaningful.
 *
 * @see decomp.me link (100%) https://decomp.me/scratch/1qYnn
 */
void gover_show_screen(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex)
{
    RECT rect;
    u8* frameTail;
    u16* half1VramRect;
    GoverFrameHalf* halves;
    u8(*frameTailPtr)[];
    frameTailPtr = &g_goverFrameTail; // matching: original used a pointer-to-array indirection
    VSync(0);
    DrawSync(0);
    frameTail = *frameTailPtr;

    // halves[0].vramRect: front buffer at VRAM (0, 0)
    FRAME_HALF(0).vramRect[0] = 0;
    FRAME_HALF(0).vramRect[1] = 0;
    FRAME_HALF(0).vramRect[2] = SCREEN_WIDTH;
    FRAME_HALF(0).vramRect[3] = SCREEN_HEIGHT;

    // halves[1].vramRect: back buffer at VRAM (0, VRAM_BACK_DISP_Y). Kept as a
    // typed alias to preserve the addiu+stores pattern in the original asm;
    // switching to FRAME_HALF(1).vramRect[i] reorders the instruction stream.
    half1VramRect = (u16*)(frameTail + 0x49C);
    half1VramRect[0] = 0;
    half1VramRect[1] = VRAM_BACK_DISP_Y;
    half1VramRect[2] = SCREEN_WIDTH;
    half1VramRect[3] = SCREEN_HEIGHT;

    // Clear the entire VRAM frame area before uploading the new image.
    rect.x = 0;
    rect.y = 0;
    rect.w = VRAM_WIDTH;
    rect.h = VRAM_HEIGHT;
    ClearImage(&rect, 0, 0, 0);

    // Configure halves[0]/halves[1] disp/draw for vertical double-buffering.
    // Front buffer draws at Y=SCREEN_HEIGHT (just below its display region);
    // back buffer draws at Y=VRAM_BACK_DRAW_Y (above its display region).
    SetDefDispEnv(&FRAME_HALF(0).disp, 0, 0,                SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&FRAME_HALF(1).disp, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&FRAME_HALF(0).draw, 0, SCREEN_HEIGHT,    SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&FRAME_HALF(1).draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    // Clear the dtd (dither) flag on both DRAWENVs.
    halves = &FRAME_HALF(0);
    halves[1].draw.dtd = 0;
    halves[0].draw.dtd = 0;

    // VRAM destination coordinates for the Game Over image (overlaid on RECT):
    // pixelX/Y = (SCREEN_WIDTH, 0)  — texture area, just past the framebuffers.
    // clutX/Y  = (0, 480)           — gover-specific CLUT slot in the bottom of VRAM.
    rect.x = SCREEN_WIDTH;
    rect.y = 0;
    rect.w = 0;
    rect.h = 480;
    
    LoadImageFromCd(imageResourceIndex + 0xFFC, (VramDstCoords*)(&rect), cdLoadAddr);

    FUN_80022aa8();
    FUN_80022ac8();
    func_800224D8(0x7F);

    if (audioClipIndex != (-1))
    {
        LoadAudioClip(audioClipIndex);
        func_800A39A8(0, 0x80, 0, 0);
    }
    if (musicResourceIndex != (-1))
    {
        func_800A368C(musicResourceIndex, 0);
        D_8011588C = 0x7F;
        func_800A380C();
        FUN_8002279c(0, 0x7F);
    }

    // Begin a 4-per-frame fade-in (0 -> 0x80); RunGameOver flips the sign on input.
    g_fadeLevel = 4;
    g_fadeStep = 4;
    RunGameOver();
}

/**
 * @brief Per-frame loop for the Game Over screen.
 *
 * @details Drives the fade-in / hold / fade-out cycle on the double-buffered
 * frame configured by @p gover_show_screen. The loop exits when @p g_fadeLevel
 * has been ramped back to 0 (fully black).
 *
 * Each iteration:
 *   1. Resets the drawing half's allocation cursor (@p allocCursor = @p primBuf)
 *      and clears its ordering table.
 *   2. Calls @p BuildOTag to emit the per-frame SPRT/DR_TPAGE primitives,
 *      which also advances @p g_fadeLevel by @p g_fadeStep.
 *   3. Waits for VSync, then checks user input (@p D_80122988 & 0x260) — once
 *      the fade has held at full brightness (0x80) and a button is pressed,
 *      flips @p g_fadeStep to -4 to begin the fade-out.
 *   4. Swaps display halves and queues @p PutDispEnv / @p PutDrawEnv /
 *      @p DrawOTag for the previously built half. The OT chain is drawn from
 *      its tail entry (@p otag[7], byte offset 0x1C).
 *   5. Pumps @p cdrom_process_state to keep CD-streaming alive during the loop.
 *
 * The fade-out break condition is @p g_fadeLevel == 0; on exit, audio is
 * stopped, the display is masked off, and @p D_8010D018 is set to signal the
 * caller that the Game Over sequence has completed.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/LOxbx
 */
void RunGameOver(void)
{
    GoverFrameHalf* current;
    GoverFrameHalf* drawing;
    int new_var;
    GoverFrameHalf* next;
    u8 dummy[8];
    s32* p_d40708;
    func_800AA02C();
    current = (GoverFrameHalf*)g_goverFrameHeader;
    ClearOTagR((u_long*)current, 8);
    ClearOTagR((u_long*)&current[1], 8);
    VSync(0);
    PutDispEnv(&current->disp);
    func_800157DC();
    SetDispMask(1);
    {
        drawing = current;
        while (1)
        {
            drawing = current;
            ClearOTagR((u_long*)drawing, 8);
            drawing->allocCursor = drawing->primBuf;
            func_800A9E78();
            BuildOTag((s32*)drawing);
            DrawSync(0);
            func_800157B0(2);
            if (!g_fadeLevel)
            {
            }
            VSync(2);
            p_d40708 = &g_fadeStep;
            if ((g_fadeLevel == 128) && (D_80122988 & 0x260))
            {
                func_800227D0(0, 0x20, 0);
                *p_d40708 = -4;
            }
            if (g_fadeLevel == (0 & 0xFF))
            {
                break;
            }
            next = (GoverFrameHalf*)g_goverFrameHeader;
            if (current == (GoverFrameHalf*)g_goverFrameHeader)
            {
                next = current + 1;
            }
            current = next;
            PutDispEnv(&current->disp);
            new_var = 0x1C;
            PutDrawEnv(&current->draw);
            // The OT linked list is built backward, so DrawOTag is invoked starting
            // at the last entry (otag[7] at offset 0x1C).
            DrawOTag((u_long*)((u_char*)drawing + new_var));
            func_800157DC();
            cdrom_process_state();
        }
    }
    DrawSync(0);
    VSync(0);
    func_800158E0();
    FUN_80022aa8();
    FUN_80022ac8();
    SetDispMask(0);
    D_8003EC90 = 0;
    func_800AA02C();
    D_8010D018 = 1;
}

void BuildOTag(unsigned char* pOtBuf)
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

    if (g_fadeLevel == 128)
    {
        g_fadeStep = 0;
    }

    // The primitive allocation cursor (allocCursor, struct offset 0x498) is reset to
    // &primBuf (offset 0x98) each frame by RunGameOver and advanced by this function.
    half = (GoverFrameHalf*)pOtBuf;
    pPrimA = half->allocCursor;

    // SPRT: left half (256x224), texture page 0xA5 (8bpp, VRAM X=320)
    setSprt(pPrimA);

    leftFadeLevel = (unsigned char)g_fadeLevel;

    setXY0((SPRT*)pPrimA, 0, 0);
    setWH((SPRT*)pPrimA, 256, 224);
    setUV0((SPRT*)pPrimA, 0, 0);
    setClut((SPRT*)pPrimA, 0, 480);
    setBGR0((SPRT*)pPrimA, leftFadeLevel, leftFadeLevel, leftFadeLevel);
    addPrim(pOtBuf, pPrimA);

    pPrimA += 20;

    // DR_TPAGE: select texture page 0xA5 before drawing left SPRT (8bpp, VRAM X=320, ABR=add)
    setDrawTPage((DR_TPAGE*)pPrimA, 0, 0, getTPage(1, 1, 320, 0));
    addPrim(pOtBuf, pPrimA);

    pPrimB = pPrimA + 8;
    pPrimA = pPrimB;

    // SPRT: right half (64x224), texture page 0xA7 (8bpp, VRAM X=448)
    setSprt(pPrimB);

    rightFadeLevel = (unsigned char)g_fadeLevel;

    setBGR0((SPRT*)pPrimB, rightFadeLevel, rightFadeLevel, rightFadeLevel);
    setXY0((SPRT*)pPrimB, 256, 0);
    setWH((SPRT*)pPrimB, 64, 224);
    setUV0((SPRT*)pPrimB, 0, 0);
    setClut((SPRT*)pPrimB, 0, 480);

    addPrim(pOtBuf, pPrimB);

    pPrimA += 20;

    // DR_TPAGE: select texture page 0xA7 before drawing right SPRT (8bpp, VRAM X=448, ABR=add)
    setDrawTPage((DR_TPAGE*)pPrimA, 0, 0, getTPage(1, 1, 448, 0));
    pPrimB = pPrimA;
    pPrimB += 8;

    addPrim(pOtBuf, pPrimA);

    half->allocCursor = pPrimB; // advance allocation cursor
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/OafFK
 */
void LoadImageFromCd(s32 arg0, VramDstCoords* coordinates, u32 address)
{
    volatile u8 dummy[8];
    cdrom_queue_read(arg0 & 0xFFFF, address);
    cdrom_wait_queue_empty();
    UploadImageDataToVram((ClutSectionHeader*)address, coordinates);
}

u32 UploadImageDataToVram(ClutSectionHeader* header, VramDstCoords* coordinates)
{
    RECT rect;
    PixelDataHeader* pdh;
    u32 clutSectionSize = header->size;

    rect.x = coordinates->clutX;
    rect.y = coordinates->clutY;
    rect.w = header->width * header->height;
    rect.h = 1;
    LoadImage(&rect, &header->clutData);

    // The pixel data header is located at a variable offset from the start of the CLUT section header,
    // so we have to calculate its address using the size field in the CLUT header.
    pdh = (PixelDataHeader*)((u8*)header + 8 + clutSectionSize);

    rect.x = coordinates->pixelX;
    rect.y = coordinates->pixelY;
    rect.w = pdh->w;
    rect.h = pdh->h;
    LoadImage(&rect, &pdh->data);

    return ALIGN64(pdh->w);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/At0Tp
 */
void LoadAudioClip(s32 arg0)
{
    s32 offset;
    u8* header;
    u8* end;
    u8* dest;
    u8* src;
    s32* ptr;
    offset = -2;
    if (arg0 != offset)
    {
        g_audioData.unk8 = 0;
        g_audioData.unk4 = 0;
        g_audioData.unk0 = 0;
        if (arg0 != (-1))
        {
            cdrom_queue_read((arg0 + 0x51) & 0xFFFF, (void*)0x80180000);
            cdrom_wait_queue_empty();
            g_audioData.unk0 = 0xC;

            header = (0x80180000 + *(u32*)0x80180004);
            ptr = (s32*)header;
            end = header + ((u32)ptr[*ptr]);
            dest = ((u8*)(&g_audioData)) + 12;
            if (header != end)
            {
                src = header;
                do
                {
                    *(dest++) = *(src++);
                } while (src != end);
            }
            func_80022AE8(end, 1);
        }
    }
}