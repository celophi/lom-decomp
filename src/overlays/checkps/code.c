#include "checkps.h"

/* Nonzero ends the CHECKPS display loop; value 2 is used for image timeout. */
s32 g_checkpsExitReason;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkpsUnusedWord0;

FadeColor g_fadeTarget;

FadeColor g_fadeCurrent;

/* Sequence data copied from a CHECKPS disc asset before playback. */
u8 g_checkpsSongBuffer[16384];

/* Destination address used when registering the embedded AKAO bank. */
AkaoSeqHeader* g_checkpsAkaoBank;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkpsUnusedWord1;

s32 g_debouncedInput;

s32 g_checkpsImageHeight;

/* Width from the TIM-style image header, in VRAM words (4 pixels per word). */
s32 g_checkpsImageWidthWords;

/* Reserved word with no recovered CHECKPS references. */
s32 g_checkpsUnusedWord2;

s32 g_checkpsImageFramesRemaining;

s32 g_lastInputState;

s32 g_inputRepeatTimer;

/*
 * Unreferenced BSS extent between code.c and code7_data.c.  Keep the exact
 * element count: it preserves the linked address of the following CD state globals.
 */
s32 g_checkpsReservedBss[32769];

s32 RunCheckPS(s32 renderStateAddress)
{
    LoadEmbeddedCheckPSAudio();
    InitCheckPSDisplay((CheckPSRenderState*)renderStateAddress);
    do
    {
        RunCheckPSDisplayLoop((CheckPSRenderState*)renderStateAddress);
    } while (g_checkpsExitReason == 0);

    return 8;
}

void RunCheckPSDisplayLoop(CheckPSRenderState* renderState)
{
    RECT rect;
    u_long* orderingTableEnd;
    CheckPSFrame* frame;
    CheckPSFrame* nextFrame;

    DrawSync(0);
    VSync(0);

    frame = &renderState->frames[0];
    reset_controller_vsync_state();

    rect.w = SCREEN_WIDTH;
    rect.x = 0;
    rect.y = 0;
    rect.h = VRAM_BACK_DISP_Y + SCREEN_HEIGHT;
    ClearImage(&rect, 0, 0, 0);
    ClearOTagR(frame->orderingTable, CHECKPS_ORDERING_TABLE_LENGTH);
    ClearOTagR(renderState->frames[1].orderingTable, CHECKPS_ORDERING_TABLE_LENGTH);
    PutDispEnv(&frame->display.disp);
    update_controllers();
    SetDispMask(1);
    do
    {
        orderingTableEnd = frame->orderingTable;
        ClearOTagR(orderingTableEnd, CHECKPS_ORDERING_TABLE_LENGTH);
        frame->primitiveCursor = frame->primitiveBuffer;
        BeginGlyphCacheFrame();
        UpdateAndDrawFade(frame);
        DrawCheckPSImage(frame);
        UpdateCheckPSInputAndTimeout();
        EvictUnusedGlyphs();
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        ClearImage(&frame->display.clearRect, 0, 0, 0);
        nextFrame = &renderState->frames[0];
        if (frame == nextFrame)
        {
            nextFrame = &renderState->frames[1];
        }
        frame = nextFrame;
        PutDispEnv(&frame->display.disp);
        PutDrawEnv(&frame->display.draw);
        orderingTableEnd += CHECKPS_ORDERING_TABLE_LENGTH - 1;
        DrawOTag(orderingTableEnd);

        update_controllers();
        cdrom_process_state();
    } while (g_checkpsExitReason == 0);

    reset_controller_vsync_state();
    VSync(0);
}
/* Configure the two 320x240 frame environments and reset CHECKPS rendering state. */
void InitCheckPSDisplay(CheckPSRenderState* renderState)
{
    RECT rect;
    SetGeomScreen(1500);
    SetGeomOffset(160, 120);
    renderState->frames[0].display.clearRect.x = 0;
    renderState->frames[0].display.clearRect.y = 0;
    renderState->frames[0].display.clearRect.w = SCREEN_WIDTH;
    renderState->frames[0].display.clearRect.h = SCREEN_HEIGHT;
    renderState->frames[1].display.clearRect.x = 0;
    renderState->frames[1].display.clearRect.y = VRAM_BACK_DISP_Y;
    renderState->frames[1].display.clearRect.w = SCREEN_WIDTH;
    renderState->frames[1].display.clearRect.h = SCREEN_HEIGHT;
    DrawSync(0);
    VSync(0);

    rect.w = 1024;
    rect.x = 0;
    rect.y = 0;
    rect.h = 512;
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv(&renderState->frames[0].display.disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&renderState->frames[1].display.disp, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&renderState->frames[0].display.draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&renderState->frames[1].display.draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    renderState->frames[1].display.draw.dtd = 0;
    renderState->frames[0].display.draw.dtd = 0;

    rect.x = 960;
    rect.w = 64;
    rect.y = 0;
    rect.h = 256;
    ClearImage(&rect, 0, 0, 0);
    ResetGlyphRenderer();
    ResetFadeState();
    SetFadeTarget(256, 256, 256, 20);
    LoadCheckPSImage();
    g_checkpsExitReason = 0;
    UpdateControllerInput();
}

void LoadEmbeddedCheckPSAudio(void)
{
    u8* source;
    u8* bankDestination;
    u32 byteCount;
    AkaoSeqHeader** bankSlot;
    u32* sectionOffsets;
    if (((((g_previousGameState == 2) || (g_previousGameState == 3)) || (g_previousGameState == 0)) || (g_previousGameState == 6)) ||
        (g_previousGameState == 7) || (g_previousGameState == 5))
    {
        return;
    }

    bankSlot = &g_checkpsAkaoBank;
    *bankSlot = (AkaoSeqHeader*)0x8013C000;

    sectionOffsets = &g_embeddedCheckpsAkao;
    sectionOffsets++;

    source = (u8*)&g_embeddedCheckpsAkao + sectionOffsets[0];
    bankDestination = (u8*)*bankSlot;
    byteCount = sectionOffsets[1] - sectionOffsets[0];
    bcopy(source, bankDestination, byteCount);
    akao_register_bank(*bankSlot);
    akao_play_sequence_blocking((AkaoSeqHeader*)((u32)&g_embeddedCheckpsAkao + sectionOffsets[1]), 1);
}

void LoadCheckPSSongFromDisc(s32 songIndex)
{
    u32* sectionOffsets;
    u8* songContainer;

    cdrom_queue_read((songIndex + 0x17) & 0xFFFF, 0x80180000);
    cdrom_wait_queue_empty();

    sectionOffsets = (u32*)0x80180004;
    songContainer = (u8*)0x80180000;
    bcopy(songContainer + sectionOffsets[0], g_checkpsSongBuffer, sectionOffsets[1] - sectionOffsets[0]);
    akao_play_sequence_blocking((AkaoSeqHeader*)(sectionOffsets[1] + (u32)songContainer), 1);
}

void StopCheckPSSong(void)
{
    akao_stop_song(0);
}

void PlayLoadedCheckPSSong(void)
{
    akao_play_song(&g_checkpsSongBuffer);
    akao_cmd_c0(0, 0x7F);
}
void PlayCheckPSSfx(u32 soundId, u32 volume, u32 pan)
{
    akao_play_sfx(soundId, 0, volume, pan);
}
/* Reset both fade endpoints to black. */
void ResetFadeState(void)
{
    g_fadeCurrent.red = 0;
    g_fadeCurrent.green = 0;
    g_fadeCurrent.blue = 0;
    g_fadeTarget.red = 0;
    g_fadeTarget.green = 0;
    g_fadeTarget.blue = 0;
    g_fadeTarget.steps = 0;
}

void UpdateAndDrawFade(CheckPSFrame* frame)
{
    s32 redStep, greenStep, blueStep;
    s32 drawMode;
    u32* primitive;
    u32* orderingTableTag = (u32*)frame->orderingTable;

    primitive = (u32*)frame->primitiveCursor;
    if (g_fadeTarget.steps != 0)
    {
        redStep = (g_fadeTarget.red - g_fadeCurrent.red) / g_fadeTarget.steps;
        greenStep = (g_fadeTarget.green - g_fadeCurrent.green) / g_fadeTarget.steps;
        blueStep = (g_fadeTarget.blue - g_fadeCurrent.blue) / g_fadeTarget.steps;
        g_fadeTarget.steps--;
        g_fadeCurrent.red += redStep;
        g_fadeCurrent.green += greenStep;
        g_fadeCurrent.blue += blueStep;
    }
    else
    {
        g_fadeCurrent.red = g_fadeTarget.red;
        g_fadeCurrent.green = g_fadeTarget.green;
        g_fadeCurrent.blue = g_fadeTarget.blue;
    }
    if (g_fadeCurrent.red != 0x100 || g_fadeCurrent.green != g_fadeCurrent.red || g_fadeCurrent.blue != g_fadeCurrent.green)
    {
        if (g_fadeCurrent.red >= 0x101)
        {
            ((u8*)primitive)[4] = (u8)(g_fadeCurrent.red - 1);
            ((u8*)primitive)[5] = (u8)(g_fadeCurrent.green - 1);
            ((u8*)primitive)[6] = (u8)(g_fadeCurrent.blue - 1);
        }
        else
        {
            if (g_fadeCurrent.red == 0x100)
            {
                ((u8*)primitive)[4] = 0;
            }
            else
            {
                ((u8*)primitive)[4] = ~(u8)(g_fadeCurrent.red);
            }
            if (g_fadeCurrent.green == 0x100)
            {
                ((u8*)primitive)[5] = 0;
            }
            else
            {
                ((u8*)primitive)[5] = ~(u8)(g_fadeCurrent.green);
            }
            if (g_fadeCurrent.blue == 0x100)
            {
                ((u8*)primitive)[6] = 0;
            }
            else
            {
                ((u8*)primitive)[6] = ~(u8)(g_fadeCurrent.blue);
            }
        }
        ((u8*)primitive)[3] = 3;
        ((u8*)primitive)[7] = 0x62;
        *(u16*)((u8*)primitive + 12) = 0x140;
        *(u16*)((u8*)primitive + 10) = 0;
        *(u16*)((u8*)primitive + 8) = 0;
        *(u16*)((u8*)primitive + 14) = 0xF0;

        *primitive = (*primitive & 0xFF000000) | (*orderingTableTag & 0x00FFFFFF);
        *orderingTableTag = (*orderingTableTag & 0xFF000000) | ((u32)primitive & 0x00FFFFFF);
        drawMode = 0x25;
        primitive = (u32*)((u8*)primitive + 0x10); // advance in-place; lands in delay slot
        if (g_fadeCurrent.red < 0x101)
        {
            drawMode = 0x45;
        }
        ((u8*)primitive)[3] = 1;
        *(u32*)((u8*)primitive + 4) = (u32)drawMode | 0xE1000000;

        *primitive = (*primitive & 0xFF000000) | (*orderingTableTag & 0x00FFFFFF);
        *orderingTableTag = (*orderingTableTag & 0xFF000000) | ((u32)primitive & 0x00FFFFFF);

        primitive = (u32*)((u8*)primitive + 8); // second advance (+8), total = +0x18
    }
    frame->primitiveCursor = (u8*)primitive;
}
/* Set the RGB fade target; 0x100 is normal brightness. */
void SetFadeTarget(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fadeTarget.red = red;
    g_fadeTarget.green = green;
    g_fadeTarget.blue = blue;
    g_fadeTarget.steps = steps;
}
void UpdateCheckPSInputAndTimeout(void)
{
    s32 timer;

    ProcessControllerInput();
    timer = g_checkpsImageFramesRemaining - 1;
    g_checkpsImageFramesRemaining = timer;

    if (timer == 0)
    {
        g_checkpsExitReason = 2;
    }
}

void DrawCheckPSImage(CheckPSFrame* frame)
{
    u8* primitiveCursorPage;
    u8* primitive;
    s32 widthWords;
    s32 imageWidthWords;
    s32 imageHeight;
    u_long* orderingTable;
    u32 drawModeCommand;
    volatile u32 stackLayoutScratch; /* Required to preserve the original stack frame. */
    drawModeCommand = 0xE1000000;
    /* Keep the split +0x8000/+0xB8 addressing shape; GCC 2.7.2 emits the matched cursor load this way. */
    primitiveCursorPage = ((u8*)frame) + 0x8000;
    primitive = *((u8**)(primitiveCursorPage + 0xB8));

    // Set RGB
    *((u32*)(primitive + 4)) = 0x808080;

    setSprt((SPRT*)primitive);

    widthWords = g_checkpsImageWidthWords;
    imageHeight = g_checkpsImageHeight;

    setUV0((SPRT*)primitive, 0, 0);
    setClut((SPRT*)primitive, 0, 480);
    setXY0((SPRT*)primitive, (0x140 - (widthWords * 4)) >> 1, (0xE0 - imageHeight) / 2);

    imageWidthWords = g_checkpsImageWidthWords;
    widthWords = imageWidthWords;

    orderingTable = frame->orderingTable;

    setWH((SPRT*)primitive, widthWords * 4, g_checkpsImageHeight);
    addPrim(orderingTable, (SPRT*)primitive);
    primitive += 0x14;

    setDrawTPage((SPRT*)primitive, 0, 0, 5);
    addPrim(orderingTable, (SPRT*)primitive);

    primitive += 8;
    *((u8**)(primitiveCursorPage + 0xB8)) = primitive;
}

void LoadCheckPSImage(void)
{
    RECT imageDestination;
    RECT uploadRect;
    RECT* uploadRectPtr;
    u8* imageAsset;
    u32 clutBlockSize;
    u8* pixelBlock;
    u16 imageX, imageY;
    u16* imageHeader;

    uploadRectPtr = &uploadRect;
    imageAsset = g_checkpsImageAsset;

    g_checkpsImageFramesRemaining = 120;
    imageDestination.x = 0x140;
    imageDestination.y = 0;
    imageDestination.w = 0;
    imageDestination.h = 0x1E0;

    uploadRect.x = 0;
    uploadRect.y = 0x1E0;
    uploadRect.w = *(u16*)(imageAsset + 0x10) * *(u16*)(imageAsset + 0x12);
    uploadRect.h = 1;

    clutBlockSize = *(u32*)(imageAsset + 8);
    LoadImage(uploadRectPtr, (u_long*)(imageAsset + 0x14));

    imageX = imageDestination.x;
    imageY = imageDestination.y;

    pixelBlock = imageAsset + (clutBlockSize + 8);

    imageHeader = (u16*)(pixelBlock + 8);
    uploadRect.x = imageX;
    uploadRect.y = imageY;
    uploadRect.w = imageHeader[0];
    uploadRect.h = imageHeader[1];

    g_checkpsImageWidthWords = imageHeader[0];
    g_checkpsImageHeight = imageHeader[1];

    imageHeader += 2;
    LoadImage(uploadRectPtr, (u_long*)imageHeader);
}

s32 PollInputDevice(void)
{
    SCDRegs* regs = SCD_REGS;

    u32 inputMask;
    u32 rawButtons;
    u32 remappedUpper;
    u32 crossBit;
    u32 triangleBit;
    u32 squareBit;
    u32 nonFaceBits;

    s32 axisX;
    s32 axisY;
    u16 hiRead;
    u16 loRead;
    u16 axisRaw;

    if (regs->device_type >= 0xFEU)
    {
        return 0;
    }

    // Raw button read (byte-swapped via two reads)
    hiRead = *((volatile u16*)(((u8*)regs) + 2));
    loRead = *((volatile u16*)(((u8*)regs) + 2));
    inputMask = (((u32)hiRead) >> 8) | (((u32)loRead) << 8);

    rawButtons = inputMask;
    // Remap upper nibble bits (hardware → logical layout)
    remappedUpper = (rawButtons & 0x40) >> 1;
    crossBit = (rawButtons & 0x20) << 1;
    remappedUpper |= crossBit;

    triangleBit = (rawButtons & 0x80) >> 3;
    remappedUpper |= triangleBit;

    squareBit = (rawButtons & 0x10) << 3;
    remappedUpper |= squareBit;

    nonFaceBits = rawButtons & (~0xF0U);
    inputMask = remappedUpper | nonFaceBits;
    if (regs->device_type != 0)
    {
        // X axis → left/right flags
        axisRaw = *(volatile u16*)&regs->axis_x;
        axisX = (s32)((s16)axisRaw);

        if (axisX < -1)
        {
            inputMask |= 0x8000U;
        }
        else if (axisX >= 2)
        {
            inputMask |= 0x2000U;
        }

        // Y axis → up/down flags
        axisRaw = *(u16*)&regs->axis_y;
        axisY = (s32)((s16)axisRaw);
        if (axisY < -1)
        {
            inputMask |= 0x1000U;
        }
        else if (axisY >= 2)
        {
            inputMask |= 0x4000U;
        }
    }

    return (s32)inputMask;
}
/* Read the merged controller sample and apply CHECKPS key-repeat/debounce behavior. */
void ProcessControllerInput(void)
{
    SCDRegs* controllerRegs;
    u32 processedButtons;
    s16 axisX;
    s16 axisY;
    s32 finalButtonState;
    controllerRegs = SCD_REGS;

    /* Device types 0xFE/0xFF are unavailable controller states. */
    if (((u8)g_controllerDeviceType) >= 0xFEU)
    {
        finalButtonState = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processedButtons = (controllerRegs->held_buttons >> 8) | (controllerRegs->held_buttons << 8);
        // Remap face buttons from hardware order to game order.
        processedButtons = PAD_REMAP_FACE_BITS(processedButtons);

        if (controllerRegs->device_type != 0)
        {
            axisX = (s16)controllerRegs->axis_x;

            if (axisX < -1)
            {
                processedButtons |= PAD_BTN_LEFT;
            }
            else if (axisX >= 2)
            {
                processedButtons |= PAD_BTN_RIGHT;
            }

            axisY = (s16)controllerRegs->axis_y;
            if (axisY < -1)
            {
                processedButtons |= PAD_BTN_UP;
            }
            else if (axisY >= 2)
            {
                processedButtons |= PAD_BTN_DOWN;
            }
        }
        finalButtonState = processedButtons;
    }

    g_debouncedInput = 0; // current active input
    // 0x0B6F = L2 | R2 | L1 | R1 | Cross | Circle | Select | L3 | Start
    // = 0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0020 | 0x0040 | 0x0100 | 0x0200 | 0x0800
    if (((finalButtonState == g_lastInputState) || ((g_lastInputState != 0) && ((finalButtonState & (g_lastInputState | 0xB6F))))) && (finalButtonState != 0))
    {
        // Keep only directional bits
        if ((finalButtonState & 0xF000) != 0)
        {
            finalButtonState &= 0xF000;
        }
        if (g_inputRepeatTimer == 0)
        {
            g_debouncedInput = finalButtonState;
            g_inputRepeatTimer = 2;
        }
        else
        {
            g_inputRepeatTimer--;
            g_debouncedInput = 0;
        }
    }
    else if (finalButtonState == 0)
    {
        g_inputRepeatTimer = 0;
        g_lastInputState = 0;
    }
    else
    {
        g_debouncedInput = finalButtonState;
        g_lastInputState = finalButtonState;
        g_inputRepeatTimer = 15;
    }
}
/* Seed the CHECKPS input snapshot from the merged controller sample. */
void UpdateControllerInput(void)
{
    SCDRegs* regs;
    PadButton processedButtons;
    short axisX;
    short axisY;
    unsigned int finalButtonState;
    regs = SCD_REGS;

    g_debouncedInput = 0;

    /* Device types 0xFE/0xFF are unavailable controller states. */
    if (g_controllerDeviceType >= 0xFEU)
    {
        finalButtonState = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processedButtons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
        // Reorder face button bits 4-7 from hardware order (Triangle, Circle, Cross, Square)
        // to game order (Square, Cross, Circle, Triangle) by swapping Triangle<->Square and Circle<->Cross.
        // Keep D-pad and shoulder button bits (0-3, 8-15) unchanged.
        processedButtons =
            (((((processedButtons & PAD_BTN_CIRCLE) >> 1) | ((processedButtons & PAD_BTN_CROSS) << 1)) | ((processedButtons & PAD_BTN_TRIANGLE) >> 3)) |
             ((processedButtons & PAD_BTN_SQUARE) << 3)) |
            (processedButtons & ~0xF0);
        if (regs->device_type != 0)
        {
            axisX = regs->axis_x;

            if (axisX < -1)
            {
                processedButtons |= PAD_BTN_LEFT;
            }
            else if (axisX >= 2)
            {
                processedButtons |= PAD_BTN_RIGHT;
            }

            axisY = regs->axis_y;
            if (axisY < -1)
            {
                processedButtons |= PAD_BTN_UP;
            }
            else if (axisY >= 2)
            {
                processedButtons |= PAD_BTN_DOWN;
            }
        }
        finalButtonState = processedButtons;
    }

    g_lastInputState = finalButtonState;
    g_inputRepeatTimer = 15;
}
