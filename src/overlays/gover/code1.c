
#include "gover.h"

const s32 g_goverOverlayId = 10;
s32 D_80140704;
s32 g_fadeStep;
s32 D_8014070C;
u8 D_80140710[144];
u8 D_801407A0[2216];
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
 * @p GoverFrameHalf for the field layout. The two halves are anchored by:
 *
 *   D_80140710 — &halves[0]              (struct start)
 *   D_801407A0 — &halves[0].vramRect     (i.e. D_80140710 + 0x90)
 *
 * Both symbols alias the same underlying buffer; this function uses the
 * @p D_801407A0 anchor while RunGameOver uses @p D_80140710.
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
    u8* buf;
    u8* buf2;
    u16* buf2Header;
    u8(*bufBasePtr)[];
    bufBasePtr = &D_801407A0;
    VSync(0);
    DrawSync(0);
    buf = *bufBasePtr;
    buf2 = buf + 0x49C;

    // halves[0].vramRect (GoverFrameHalf+0x90): VRAM (0, 0), 320x240
    *((u16*)(buf + 0)) = 0;
    *((u16*)(buf + 2)) = 0;
    *((u16*)(buf + 4)) = 0x140;
    *((u16*)(buf + 6)) = 0xF0;

    // halves[1].vramRect (offset 0x49C from halves[0]): VRAM (0, 232), 320x240
    buf2Header = (u16*)buf2;
    buf2Header[0] = 0;
    buf2Header[1] = 0xE8;
    buf2Header[2] = 0x140;
    buf2Header[3] = 0xF0;

    // Clear the entire VRAM frame area before uploading the new image.
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);

    // Configure halves[0].disp/draw and halves[1].disp/draw (struct offsets 0x20/0x34)
    // for vertical double-buffering at Y=0 / Y=232.
    SetDefDispEnv((DISPENV*)(buf - 0x70), 0, 0, 0x140, 0xF0);     // halves[0].disp
    SetDefDispEnv((DISPENV*)(buf + 0x42C), 0, 0xE8, 0x140, 0xF0); // halves[1].disp
    SetDefDrawEnv((DRAWENV*)(buf - 0x5C), 0, 0xF0, 0x140, 0xE0);  // halves[0].draw
    SetDefDrawEnv((DRAWENV*)(buf + 0x440), 0, 8, 0x140, 0xE0);    // halves[1].draw

    // Clear the dtd (dither) flag on both DRAWENVs (DRAWENV+0x16).
    // buf is now &halves[0] (i.e. D_80140710); 0x4A = halves[0].draw.dtd,
    // 0x4E6 = halves[1].draw.dtd (0x49C + 0x34 + 0x16).
    buf = buf - 0x90;
    *((u8*)(buf + 0x4E6)) = 0;
    *((u8*)(buf + 0x4A)) = 0;

    // VRAM destination coordinates for the Game Over image (overlaid on RECT):
    // pixelX/Y = (0x140, 0), clutX/Y = (0, 0x1E0).
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1E0;
    buf = buf - 0x90;
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
 * decomp.me link (100%) https://decomp.me/scratch/LOxbx
 */
void RunGameOver(void)
{
    u_char* var_s0;
    u_char* var_s1;
    int new_var;
    u_char* var_v0;
    u8 dummy[8];
    s32* p_d40708;
    func_800AA02C();
    var_s0 = (u_char*)D_80140710;
    ClearOTagR((u_long*)var_s0, 8);
    ClearOTagR((u_long*)(var_s0 - (-0x49C)), 8);
    VSync(0);
    PutDispEnv((DISPENV*)(var_s0 + 0x20));
    func_800157DC();
    SetDispMask(1);
    {
        var_s1 = var_s0;
        while (1)
        {
            var_s1 = var_s0;
            ClearOTagR((u_long*)var_s1, 8);
            *((void**)(var_s1 + 0x498)) = (void*)(var_s1 + 0x98);
            func_800A9E78();
            BuildOTag((s32*)var_s1);
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
            var_v0 = (u_char*)D_80140710;
            if (var_s0 == ((u_char*)D_80140710))
            {
                var_v0 = var_s0 + 0x49C;
            }
            var_s0 = var_v0;
            PutDispEnv((DISPENV*)(var_s0 + 0x20));
            new_var = 0x1C;
            PutDrawEnv((DRAWENV*)(var_s0 + 0x34));
            DrawOTag((u_long*)(var_s1 + new_var));
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