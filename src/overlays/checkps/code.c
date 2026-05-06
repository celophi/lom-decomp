#include "checkps.h"

s32 D_8005D060;

s32 D_8005D064;

FadeColor g_fadeTarget;

FadeColor g_fadeCurrent;

u8 D_8005D088[16384];

s32 D_80061088;

s32 D_8006108C;

s32 g_debouncedInput;

s32 D_80061094;

s32 D_80061098;

s32 D_8006109C;

s32 g_frameTimer;

s32 g_lastInputState;

s32 g_inputRepeatTimer;

s32 g_D_800610AC[32769];

/**
 * decomp.me link (100%) https://decomp.me/scratch/bzlSh
 */
s32 RunCheckPS(s32 baseAddress)
{
    func_80050080();
    InitCheckPSDisplay((CheckPSState*)baseAddress);

    do
    {
        func_8004FD68(baseAddress);
    } while (D_8005D060 == 0);

    return 8;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lBhMG
 */
void func_8004FD68(int baseAddress)
{
    RECT rect;
    u_long* temp_s1;
    void* var_s0;
    void* var_v0;

    DrawSync(0);
    VSync(0);

    var_s0 = baseAddress;
    func_800158E0();

    rect.w = SCREEN_WIDTH;
    rect.x = 0;
    rect.y = 0;
    rect.h = VRAM_BACK_DISP_Y + SCREEN_HEIGHT;

    ClearImage(&rect, 0, 0, 0);
    ClearOTagR(var_s0 + 0x40, 0x1000);
    ClearOTagR(var_s0 + 0xBD0C, 0x1000);
    PutDispEnv(var_s0 + 0x4040);
    func_800157DC();
    SetDispMask(1);

    do
    {
        temp_s1 = (u_long*)(var_s0 + 0x40);
        ClearOTagR(temp_s1, 0x1000);
        *(u32*)(var_s0 + 0x80B8) = (s32)(var_s0 + 0x40B8);
        InvalidateGlyphCache();
        func_80050258(var_s0);
        func_800505B4(var_s0);
        func_80050570();
        ClearInvalidGlyphs();
        DrawSync(0);
        func_800157B0(2);
        VSync(2);
        ClearImage(var_s0 + 0x40B0, 0, 0, 0);
        var_v0 = baseAddress;
        if (var_s0 == var_v0)
        {
            var_v0 = var_s0 + 0xBCCC;
        }
        var_s0 = var_v0;
        PutDispEnv(var_s0 + 0x4040);
        PutDrawEnv(var_s0 + 0x4054);

        temp_s1 = (u8*)temp_s1 + 0x3FFC;
        DrawOTag(temp_s1);

        func_800157DC();
        cdrom_process_state();
    } while (D_8005D060 == 0);

    func_800158E0();
    VSync(0);
}

/**
 * @brief Initialises the double-buffered display system for the CheckPS overlay.
 *
 * @details Sets up both render frames within the CheckPSState.
 *  - Configures the screen geometry offset (160, 120) for a 320x240 display.
 *  - Writes the screen-clear rects for each frame's display area.
 *  - Clears all of VRAM (1024x512).
 *  - Calls SetDefDispEnv / SetDefDrawEnv to configure the double-buffer swap chain.
 *  - Clears the texture cache region of VRAM.
 *  - Resets the text renderer, fade state, and input state.
 *  - Triggers a fade-in to full brightness over 20 frames.
 *
 * @param state  Pointer to the CheckPS render state containing both frame buffers.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/tlBGm
 */
void InitCheckPSDisplay(CheckPSState* state)
{
    RECT rect;

    func_8001D5AC(1500);
    func_8001D58C(160, 120);
    state->front.clearRect.x = 0;
    state->front.clearRect.y = 0;
    state->front.clearRect.w = SCREEN_WIDTH;
    state->front.clearRect.h = SCREEN_HEIGHT;
    state->back.clearRect.x = 0;
    state->back.clearRect.y = VRAM_BACK_DISP_Y;
    state->back.clearRect.w = SCREEN_WIDTH;
    state->back.clearRect.h = SCREEN_HEIGHT;
    DrawSync(0);
    VSync(0);

    rect.w = 1024;
    rect.x = 0;
    rect.y = 0;
    rect.h = 512;

    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv(&state->front.disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&state->back.disp, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&state->front.draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&state->back.draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    state->back.draw.dtd = 0;
    state->front.draw.dtd = 0;

    rect.x = 960;
    rect.w = 64;
    rect.y = 0;
    rect.h = 256;

    ClearImage(&rect, 0, 0, 0);
    func_8005239C();
    ResetFadeState();
    SetFadeTarget(256, 256, 256, 20);
    func_800506D0();
    D_8005D060 = 0;
    func_80050A0C();
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/4wbZH
 */
void func_80050080(void)
{
    u8* src;
    u8* dst;
    u32 count;
    s32* ref;
    u32* offs;

    if (((((g_previousGameState == 2) || (g_previousGameState == 3)) || (g_previousGameState == 0)) ||
         (g_previousGameState == 6)) ||
        (g_previousGameState == 7) || (g_previousGameState == 5))
    {
        return;
    }

    ref = &D_80061088;
    *ref = 0x8013C000;

    offs = &D_80052428;
    offs++;

    src = (u8*)&D_80052428 + offs[0];
    dst = (u8*)0x8013C000;
    count = offs[1] - offs[0];

    bcopy(src, dst, count);
    akao_register_bank(*ref);
    akao_play_sequence_blocking((u32)&D_80052428 + offs[1], 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Ptv27
 */
void func_80050138(s32 arg0)
{
    u32* offs;
    u8* ref;

    cdrom_queue_read((arg0 + 0x17) & 0xFFFF, 0x80180000);
    cdrom_wait_queue_empty();

    offs = (u32*)0x80180004;
    ref = (u8*)0x80180000;

    bcopy(ref + offs[0], &D_8005D088, offs[1] - offs[0]);
    akao_play_sequence_blocking(offs[1] + (u32)ref, 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/K0uKO
 */
void func_800501AC(void)
{
    func_80022068(0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/2R9zp
 */
void func_800501CC(void)
{
    func_80022040(&D_8005D088);
    FUN_8002279c(0, 0x7F);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Fklyd
 */
void func_800501FC(u32 arg1, u32 arg2, u32 arg3)
{
    func_8002216C(arg1, 0, arg2, arg3);
}

/**
 * @brief Resets the screen fade state to black.
 *
 * @details Zeroes both g_fadeCurrent and g_fadeTarget, setting all RGB channels
 * to 0 (fully black) and the step counter to 0. Typically called before
 * SetFadeTarget to ensure the fade starts from a known black state rather
 * than whatever colour was previously active.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/i9Kyk
 */
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

/**
 * decomp.me link (100%) https://decomp.me/scratch/o9VAE
 */
void func_80050258(s32* arg0)
{
    s32 temp_a2, temp_a0, temp_v1;
    s32 var_a1;
    u8 var_v0;
    u32* ref;
    u32* arg0_40 = (u32*)(arg0 + 16);

    ref = *(u32**)(arg0 + 8238);

    if (g_fadeTarget.steps != 0)
    {
        temp_a2 = (g_fadeTarget.red - g_fadeCurrent.red) / g_fadeTarget.steps;
        temp_a0 = (g_fadeTarget.green - g_fadeCurrent.green) / g_fadeTarget.steps;
        temp_v1 = (g_fadeTarget.blue - g_fadeCurrent.blue) / g_fadeTarget.steps;
        g_fadeTarget.steps--;
        g_fadeCurrent.red += temp_a2;
        g_fadeCurrent.green += temp_a0;
        g_fadeCurrent.blue += temp_v1;
    }
    else
    {
        g_fadeCurrent.red = g_fadeTarget.red;
        g_fadeCurrent.green = g_fadeTarget.green;
        g_fadeCurrent.blue = g_fadeTarget.blue;
    }

    if (g_fadeCurrent.red != 0x100 || g_fadeCurrent.green != g_fadeCurrent.red ||
        g_fadeCurrent.blue != g_fadeCurrent.green)
    {

        if (g_fadeCurrent.red >= 0x101)
        {
            ((u8*)ref)[4] = (u8)(g_fadeCurrent.red - 1);
            ((u8*)ref)[5] = (u8)(g_fadeCurrent.green - 1);
            ((u8*)ref)[6] = (u8)(g_fadeCurrent.blue - 1);
        }
        else
        {
            if (g_fadeCurrent.red == 0x100)
            {
                ((u8*)ref)[4] = 0;
            }
            else
            {
                ((u8*)ref)[4] = ~(u8)(g_fadeCurrent.red);
            }
            if (g_fadeCurrent.green == 0x100)
            {
                ((u8*)ref)[5] = 0;
            }
            else
            {
                ((u8*)ref)[5] = ~(u8)(g_fadeCurrent.green);
            }
            if (g_fadeCurrent.blue == 0x100)
            {
                ((u8*)ref)[6] = 0;
            }
            else
            {
                ((u8*)ref)[6] = ~(u8)(g_fadeCurrent.blue);
            }
        }

        ((u8*)ref)[3] = 3;
        ((u8*)ref)[7] = 0x62;
        *(u16*)((u8*)ref + 12) = 0x140;
        *(u16*)((u8*)ref + 10) = 0;
        *(u16*)((u8*)ref + 8) = 0;
        *(u16*)((u8*)ref + 14) = 0xF0;

        *ref = (*ref & 0xFF000000) | (*arg0_40 & 0x00FFFFFF);
        *arg0_40 = (*arg0_40 & 0xFF000000) | ((u32)ref & 0x00FFFFFF);

        var_a1 = 0x25;
        ref = (u32*)((u8*)ref + 0x10); // advance in-place; lands in delay slot
        if (g_fadeCurrent.red < 0x101)
        {
            var_a1 = 0x45;
        }
        ((u8*)ref)[3] = 1;
        *(u32*)((u8*)ref + 4) = (u32)var_a1 | 0xE1000000;

        *ref = (*ref & 0xFF000000) | (*arg0_40 & 0x00FFFFFF);
        *arg0_40 = (*arg0_40 & 0xFF000000) | ((u32)ref & 0x00FFFFFF);

        ref = (u32*)((u8*)ref + 8); // second advance (+8), total = +0x18
    }

    *(u32*)(arg0 + 8238) = (u32)ref;
}

/**
 * @brief Sets the target colour and duration for the screen fade.
 *
 * @details Writes directly to g_fadeTarget. The fade system in func_80050258
 * reads this each frame and interpolates g_fadeCurrent toward it over the
 * given number of steps. Colour values use an internal scale where 0 = fully
 * black and 0x100 = normal brightness (no colour modulation).
 *
 * @param red   Target red channel (0 = black, 0x100 = normal).
 * @param green Target green channel (0 = black, 0x100 = normal).
 * @param blue  Target blue channel (0 = black, 0x100 = normal).
 * @param steps Number of frames to interpolate over. 0 snaps immediately.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/A5HgV
 */
void SetFadeTarget(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fadeTarget.red = red;
    g_fadeTarget.green = green;
    g_fadeTarget.blue = blue;
    g_fadeTarget.steps = steps;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/jEnBn
 */
void func_80050570(void)
{
    s32 temp_v0;

    func_8005088C();
    temp_v0 = g_frameTimer - 1;
    g_frameTimer = temp_v0;

    if (temp_v0 == 0)
    {
        D_8005D060 = 2;
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/WIqbI
 */
void func_800505B4(s32 arg0)
{
    u8* base;
    u8* prim;
    s32 w;
    s32 new_var;
    s32 h;
    u_long* ot;
    u32 cmd;
    volatile u32 dummy;

    cmd = 0xE1000000;
    base = ((u8*)arg0) + 0x8000;
    prim = *((u8**)(base + 0xB8));

    // Set RGB
    *((u32*)(prim + 4)) = 0x808080;

    setSprt((SPRT*)prim);

    w = D_80061098;
    h = D_80061094;

    setUV0((SPRT*)prim, 0, 0);
    setClut((SPRT*)prim, 0, 480);
    setXY0((SPRT*)prim, (0x140 - (w * 4)) >> 1, (0xE0 - h) / 2);

    new_var = D_80061098;
    w = new_var;

    ot = (u_long*)(arg0 + 0x40);

    setWH((SPRT*)prim, w * 4, D_80061094);
    addPrim(ot, (SPRT*)prim);

    prim += 0x14;

    setDrawTPage((SPRT*)prim, 0, 0, 5);
    addPrim(ot, (SPRT*)prim);

    prim += 8;
    *((u8**)(base + 0xB8)) = prim;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/VRHxF
 */
void func_800506D0(void)
{
    RECT rectLoad;
    RECT rect;
    RECT* pRect;
    u8* gfxBase;
    u32 clutSize;
    u8* imageBlock;
    u16 loadX, loadY;
    register u16* pHeader asm("v1");

    pRect = &rect;
    gfxBase = D_8005B744;

    g_frameTimer = 120;

    rectLoad.x = 0x140;
    rectLoad.y = 0;
    rectLoad.w = 0;
    rectLoad.h = 0x1E0;

    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = *(u16*)(gfxBase + 0x10) * *(u16*)(gfxBase + 0x12);
    rect.h = 1;

    clutSize = *(u32*)(gfxBase + 8);
    LoadImage(pRect, (u32*)(gfxBase + 0x14));

    loadX = rectLoad.x;
    loadY = rectLoad.y;

    imageBlock = gfxBase + (clutSize + 8);

    pHeader = (u16*)(imageBlock + 8);

    rect.x = loadX;
    rect.y = loadY;
    rect.w = *(u16*)(imageBlock + 8);
    rect.h = pHeader[1];

    D_80061098 = *(u16*)(imageBlock + 8);
    D_80061094 = pHeader[1];

    LoadImage(pRect, (u32*)(imageBlock + 0xC));
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/JLAOc
 */
s32 PollInputDevice(void)
{
    SCDRegs* regs = (SCDRegs*)0x801ED600;

    u32 inputMask;
    u32 rawButtons;
    u32 remappedUpper;
    u32 workingBits;

    s32 axisX;
    s32 axisY;

    volatile u16* axisPtrX;
    u16* axisPtrY;
    u16 hiRead;
    u16 loRead;
    u16 axisRaw;

    if (regs->deviceState >= 0xFEU)
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
    workingBits = (rawButtons & 0x20) << 1;
    remappedUpper |= workingBits;

    workingBits = (rawButtons & 0x80) >> 3;
    remappedUpper |= workingBits;

    workingBits = (rawButtons & 0x10) << 3;
    remappedUpper |= workingBits;

    do
    {
        workingBits = rawButtons & (~0xF0U);
        inputMask = remappedUpper | workingBits;
    } while (0);

    if (regs->deviceState != 0)
    {
        // X axis → left/right flags
        axisPtrX = (volatile u16*)(((u8*)regs) + 0x2C);
        axisRaw = *axisPtrX;
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
        axisPtrY = (u16*)(((u8*)regs) + 0x2E);
        axisRaw = *axisPtrY;
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

/**
 * @brief Reads controller hardware state and updates the debounced input globals.
 *
 * @details Reads the SCD registers at 0x801ED600 and transforms the raw button data
 * into the game's internal format using the same byte-swap and face button remap as
 * UpdateControllerInput. If deviceState is >= 0xFE (no controller present), the button
 * state is forced to zero. Analog stick axes are thresholded and OR'd into the same
 * D-pad bit positions if an analog controller is connected (deviceState != 0).
 *
 * After building the button state, debounce and key-repeat logic is applied:
 * - New button press: written immediately to g_debouncedInput and g_lastInputState,
 *   repeat timer reset to 15.
 * - Held button: only directional bits (0xF000) are kept; g_debouncedInput is set once
 *   the repeat timer counts down to zero, then resets to 2.
 * - No input: g_debouncedInput, g_lastInputState, and g_inputRepeatTimer are all cleared.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/BkOv2
 */
void ProcessControllerInput(void)
{
    SCDRegs* controllerRegs;
    u32 processedButtons;
    s16 axisX;
    s16 axisY;
    s32 finalButtonState;

    controllerRegs = (SCDRegs*)0x801ED600;

    // 0xFF = no controller (High-Z, pins floating);
    // 0xFE = probably a defensive boundary just to be safe?
    if (((u8)D_801ED600) >= 0xFEU)
    {
        finalButtonState = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processedButtons = (controllerRegs->buttonData >> 8) | (controllerRegs->buttonData << 8);

        // Reorder face button bits 4-7 from hardware order (Triangle, Circle, Cross, Square)
        // to game order (Square, Cross, Circle, Triangle) by swapping Triangle<->Square and Circle<->Cross.
        // Keep D-pad and shoulder button bits (0-3, 8-15) unchanged.
        processedButtons = (((((processedButtons & PAD_BTN_CIRCLE) >> 1) | ((processedButtons & PAD_BTN_CROSS) << 1)) |
                             ((processedButtons & PAD_BTN_TRIANGLE) >> 3)) |
                            ((processedButtons & PAD_BTN_SQUARE) << 3)) |
                           (processedButtons & ~0xF0);

        if (controllerRegs->deviceState != 0)
        {
            axisX = (s16)controllerRegs->axisX;

            if (axisX < -1)
            {
                processedButtons |= PAD_BTN_LEFT;
            }
            else if (axisX >= 2)
            {
                processedButtons |= PAD_BTN_RIGHT;
            }

            axisY = (s16)controllerRegs->axisY;

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
    if (((finalButtonState == g_lastInputState) ||
         ((g_lastInputState != 0) && ((finalButtonState & (g_lastInputState | 0xB6F))))) &&
        (finalButtonState != 0))
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

/**
 * @brief Reads raw controller hardware state and stores it as the new input snapshot.
 *
 * @details Reads the SCD registers at 0x801ED600 and transforms the raw button data
 * into the game's internal format. If deviceState is >= 0xFE (no controller present),
 * the button state is forced to zero. Otherwise, the 16-bit buttonData is byte-swapped
 * to place the D-pad in bits 8-15 and face buttons in bits 0-7, then face button bits
 * 4-7 are remapped from PSX hardware order (Triangle, Circle, Cross, Square) to the
 * game's order (Square, Cross, Circle, Triangle). If an analog controller is connected
 * (deviceState != 0), axis values are thresholded and OR'd into the same D-pad bit
 * positions so the rest of the game can treat analog and digital input identically.
 *
 * The result is written to g_lastInputState and g_inputRepeatTimer is reset to 15.
 * This function does not perform debouncing or key-repeat; that is handled separately
 * by the sibling function that reads g_lastInputState each frame.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/qGA07
 */
void UpdateControllerInput(void)
{
    SCDRegs* regs;
    PadButton processedButtons;
    short axisX;
    short axisY;
    unsigned int finalButtonState;

    regs = (SCDRegs*)0x801ED600;

    g_debouncedInput = 0;

    // 0xFF = no controller (High-Z, pins floating);
    // 0xFE = probably a defensive boundary just to be safe?
    if (D_801ED600 >= 0xFEU)
    {
        finalButtonState = 0;
    }
    else
    {
        // PSX sends face buttons in the high byte and D-pad in the low byte;
        // swap them so D-pad ends up in bits 8-15 and face buttons in bits 0-7.
        processedButtons = (regs->buttonData >> 8) | (regs->buttonData << 8);

        // Reorder face button bits 4-7 from hardware order (Triangle, Circle, Cross, Square)
        // to game order (Square, Cross, Circle, Triangle) by swapping Triangle<->Square and Circle<->Cross.
        // Keep D-pad and shoulder button bits (0-3, 8-15) unchanged.
        processedButtons = (((((processedButtons & PAD_BTN_CIRCLE) >> 1) | ((processedButtons & PAD_BTN_CROSS) << 1)) |
                             ((processedButtons & PAD_BTN_TRIANGLE) >> 3)) |
                            ((processedButtons & PAD_BTN_SQUARE) << 3)) |
                           (processedButtons & ~0xF0);

        if (regs->deviceState != 0)
        {
            axisX = regs->axisX;

            if (axisX < -1)
            {
                processedButtons |= PAD_BTN_LEFT;
            }
            else if (axisX >= 2)
            {
                processedButtons |= PAD_BTN_RIGHT;
            }

            axisY = regs->axisY;

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