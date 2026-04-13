#include "checkps.h"

s32 D_80061088 = 0;

s32 D_8006108C = 0;

s32 D_80061090 = 0;

s32 D_80061094 = 0;

s32 D_80061098 = 0;

s32 D_8006109C = 0;

s32 D_800610A0 = 0;

s32 D_800610A4 = 0;

s32 D_800610A8 = 0;

s32 g_D_800610AC[32769] = {0}; 

/**
 * decomp.me link (100%) https://decomp.me/scratch/bzlSh
 */
s32 FUN_8004fd14(s32 arg0)
{
    func_80050080();
    func_8004FEE8(arg0);

    do
    {
        func_8004FD68(arg0);
    } while (D_8005D060 == 0);

    return 8;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lBhMG
 */
void func_8004FD68(int arg0)
{
    RECT rect;
    u_long* temp_s1;
    void* var_s0;
    void* var_v0;

    DrawSync(0);
    VSync(0);

    var_s0 = arg0;
    func_800158E0();

    rect.w = 320;
    rect.x = 0;
    rect.y = 0;
    rect.h = 472;

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
        var_v0 = arg0;
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
        CD_UpdateAndProcessQueue();
    } while (D_8005D060 == 0);

    func_800158E0();
    VSync(0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/tlBGm
 */
void func_8004FEE8(int arg0)
{
    RECT rect;

    func_8001D5AC(0x5DC);
    func_8001D58C(0xA0, 0x78);
    *(u16*)(arg0 + 0x40B0) = 0;
    *(u16*)(arg0 + 0x40B2) = 0;
    *(u16*)(arg0 + 0x40B4) = 0x140;
    *(u16*)(arg0 + 0x40B6) = 0xF0;
    *(u16*)(arg0 + 0xFD7C) = 0;
    *(u16*)(arg0 + 0xFD7E) = 0xE8;
    *(u16*)(arg0 + 0xFD80) = 0x140;
    *(u16*)(arg0 + 0xFD82) = 0xF0;
    DrawSync(0);
    VSync(0);

    rect.w = 0x400;
    rect.x = 0;
    rect.y = 0;
    rect.h = 0x200;

    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv(arg0 + 0x4040, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(arg0 + 0xFD0C, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(arg0 + 0x4054, 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(arg0 + 0xFD20, 0, 8, 0x140, 0xE0);
    *(u8*)(arg0 + 0xFD36) = 0;
    *(u8*)(arg0 + 0x406A) = 0;

    rect.x = 0x3C0;
    rect.w = 0x40;
    rect.y = 0;
    rect.h = 0x100;

    ClearImage(&rect, 0, 0, 0);
    func_8005239C();
    func_80050228();
    func_80050554(0x100, 0x100, 0x100, 0x14);
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
    func_80021FFC(*ref);
    func_80022AE8((u32)&D_80052428 + offs[1], 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Ptv27
 */
void func_80050138(s32 arg0)
{
    u32* offs;
    u8* ref;

    CD_QueueRead((arg0 + 0x17) & 0xFFFF, 0x80180000);
    CD_WaitForQueueEmpty();

    offs = (u32*)0x80180004;
    ref = (u8*)0x80180000;

    bcopy(ref + offs[0], &D_8005D088, offs[1] - offs[0]);
    func_80022AE8(offs[1] + (u32)ref, 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/K0uKO
 */
void func_800501AC(void) { func_80022068(0); }

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
void func_800501FC(u32 arg1, u32 arg2, u32 arg3) { func_8002216C(arg1, 0, arg2, arg3); }

/**
 * decomp.me link (100%) https://decomp.me/scratch/i9Kyk
 */
void func_80050228(void)
{
    u32* addrA = &D_8005D078;
    u32* addrB = &D_8005D068;

    *addrA = 0;
    *(addrA + 1) = 0;
    *(addrA + 2) = 0;

    *addrB = 0;
    *(addrB + 1) = 0;
    *(addrB + 2) = 0;
    *(addrB + 3) = 0;
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

    if (D_8005D068[3] != 0)
    {
        temp_a2 = (D_8005D068[0] - D_8005D078[0]) / D_8005D068[3];
        temp_a0 = (D_8005D068[1] - D_8005D078[1]) / D_8005D068[3];
        temp_v1 = (D_8005D068[2] - D_8005D078[2]) / D_8005D068[3];
        D_8005D068[3]--;
        D_8005D078[0] += temp_a2;
        D_8005D078[1] += temp_a0;
        D_8005D078[2] += temp_v1;
    }
    else
    {
        D_8005D078[0] = D_8005D068[0];
        D_8005D078[1] = D_8005D068[1];
        D_8005D078[2] = D_8005D068[2];
    }

    if (D_8005D078[0] != 0x100 || D_8005D078[1] != D_8005D078[0] || D_8005D078[2] != D_8005D078[1])
    {

        if (D_8005D078[0] >= 0x101)
        {
            ((u8*)ref)[4] = (u8)(D_8005D078[0] - 1);
            ((u8*)ref)[5] = (u8)(D_8005D078[1] - 1);
            ((u8*)ref)[6] = (u8)(D_8005D078[2] - 1);
        }
        else
        {
            if (D_8005D078[0] == 0x100)
            {
                ((u8*)ref)[4] = 0;
            }
            else
            {
                ((u8*)ref)[4] = ~(u8)(D_8005D078[0]);
            }
            if (D_8005D078[1] == 0x100)
            {
                ((u8*)ref)[5] = 0;
            }
            else
            {
                ((u8*)ref)[5] = ~(u8)(D_8005D078[1]);
            }
            if (D_8005D078[2] == 0x100)
            {
                ((u8*)ref)[6] = 0;
            }
            else
            {
                ((u8*)ref)[6] = ~(u8)(D_8005D078[2]);
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
        if (D_8005D078[0] < 0x101)
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
 * decomp.me link (100%) https://decomp.me/scratch/A5HgV
 */
void func_80050554(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    D_8005D068[0] = arg0;
    D_8005D068[1] = arg1;
    D_8005D068[2] = arg2;
    D_8005D068[3] = arg3;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/jEnBn
 */
void func_80050570(void)
{
    s32 temp_v0;

    func_8005088C();
    temp_v0 = D_800610A0 - 1;
    D_800610A0 = temp_v0;

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

    D_800610A0 = 0x78;

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
 * decomp.me link (100%) https://decomp.me/scratch/BkOv2
 */
void ProcessControllerInput(void)
{
    SCDRegs* controllerRegs;
    u32 processedButtons;
    s16 axisX;
    s16 axisY;
    s32 finalButtonState;

    controllerRegs = (SCDRegs*)0x801ED600;

    if (((u8)D_801ED600) >= 0xFEU)
    {
        finalButtonState = 0;
    }
    else
    {
        // Swap high and low bytes of button data
        processedButtons = (controllerRegs->buttonData >> 8) | (controllerRegs->buttonData << 8);

        // Re-map some button bits
        processedButtons = (((((processedButtons & 0x40) >> 1) | ((processedButtons & 0x20) << 1)) |
                             ((processedButtons & 0x80) >> 3)) |
                            ((processedButtons & 0x10) << 3)) |
                           (processedButtons & (~0xF0));

        if (controllerRegs->deviceState != 0)
        {
            axisX = (s16)controllerRegs->axisX;

            if (axisX < -1)
            {
                processedButtons |= 0x8000; // left
            }
            else if (axisX >= 2)
            {
                processedButtons |= 0x2000; // right
            }

            axisY = (s16)controllerRegs->axisY;

            if (axisY < -1)
            {
                processedButtons |= 0x1000; // up
            }
            else if (axisY >= 2)
            {
                processedButtons |= 0x4000; // down
            }
        }
        finalButtonState = processedButtons;
    }

    D_80061090 = 0; // current active input

    if (((finalButtonState == D_800610A4) || ((D_800610A4 != 0) && ((finalButtonState & (D_800610A4 | 0xB6F))))) &&
        (finalButtonState != 0))
    {
        // Keep only directional bits
        if ((finalButtonState & 0xF000) != 0)
        {
            finalButtonState &= 0xF000;
        }

        if (D_800610A8 == 0)
        {
            D_80061090 = finalButtonState;
            D_800610A8 = 2; // input repeat timer
        }
        else
        {
            D_800610A8--;
            D_80061090 = 0;
        }
    }
    else if (finalButtonState == 0)
    {
        D_800610A8 = 0;
        D_800610A4 = 0;
    }
    else
    {
        D_80061090 = finalButtonState;
        D_800610A4 = finalButtonState; // last button state
        D_800610A8 = 0xF;              // input repeat timer max
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/qGA07
 */
void UpdateControllerInput(void)
{
    SCDRegs* regs;
    unsigned int processedButtons;
    short axisX, axisY;
    unsigned int finalButtonState;

    regs = (SCDRegs*)0x801ED600;

    D_80061090 = 0;

    if (D_801ED600 >= 0xFEU)
    {
        finalButtonState = 0;
    }
    else
    {
        /* Swap high and low bytes of button data */
        processedButtons = (regs->buttonData >> 8) | (regs->buttonData << 8);

        /* Re‑map button bits: 0x40→bit1, 0x20→bit5, 0x80→bit4, 0x10→bit3 */
        processedButtons = (((((processedButtons & 0x40) >> 1) | ((processedButtons & 0x20) << 1)) |
                             ((processedButtons & 0x80) >> 3)) |
                            ((processedButtons & 0x10) << 3)) |
                           (processedButtons & (~0xF0));

        if (regs->deviceState != 0)
        {
            axisX = regs->axisX;
            if (axisX < -1)
            {
                processedButtons |= 0x8000; /* left */
            }
            else if (axisX >= 2)
            {
                processedButtons |= 0x2000; /* right */
            }

            axisY = regs->axisY;
            if (axisY < -1)
            {
                processedButtons |= 0x1000; /* up */
            }
            else if (axisY >= 2)
            {
                processedButtons |= 0x4000; /* down */
            }
        }
        finalButtonState = processedButtons;
    }

    D_800610A4 = finalButtonState;
    D_800610A8 = 0xF;
}