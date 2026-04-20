
#include "gover.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/1qYnn
 */
void func_80140004(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    RECT rect;
    u8* base;
    u8* new_var;
    u16* second;
    u8(*new_var2)[];
    new_var2 = &D_801407A0;
    VSync(0);
    DrawSync(0);
    base = *new_var2;
    new_var = base + 0x49C;
    *((u16*)(base + 0)) = 0;
    *((u16*)(base + 2)) = 0;
    *((u16*)(base + 4)) = 0x140;
    *((u16*)(base + 6)) = 0xF0;
    second = (u16*)new_var;
    second[0] = 0;
    second[1] = 0xE8;
    second[2] = 0x140;
    second[3] = 0xF0;
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv((DISPENV*)(base - 0x70), 0, 0, 0x140, 0xF0);
    SetDefDispEnv((DISPENV*)(base + 0x42C), 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv((DRAWENV*)(base - 0x5C), 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv((DRAWENV*)(base + 0x440), 0, 8, 0x140, 0xE0);
    base = base - 0x90;
    *((u8*)(base + 0x4E6)) = 0;
    *((u8*)(base + 0x4A)) = 0;
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1E0;
    base = base - 0x90;
    LoadImageFromCd(arg1 + 0xFFC, (VramDstCoords*)(&rect), arg0);
    FUN_80022aa8();
    FUN_80022ac8();
    func_800224D8(0x7F);
    if (arg3 != (-1))
    {
        func_80140648(arg3);
        func_800A39A8(0, 0x80, 0, 0);
    }
    if (arg2 != (-1))
    {
        func_800A368C(arg2, 0);
        D_8011588C = 0x7F;
        func_800A380C();
        FUN_8002279c(0, 0x7F);
    }
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
            CD_UpdateAndProcessQueue();
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

/**
 * decomp.me link (97.86%%) https://decomp.me/scratch/q3LKi
 */
void BuildOTag(void* pOtBuf)
{
    unsigned char* base;
    unsigned char* node1;
    unsigned char* node3;
    unsigned char* new_var;
    unsigned char v0;
    base = (unsigned char*)pOtBuf;
    if (g_fadeStep != 0)
    {
        g_fadeLevel += g_fadeStep;
    }
    if (g_fadeLevel == 128)
    {
        g_fadeStep = 0;
    }
    // base+0x498 holds the primitive allocation cursor, reset to base+0x98 each
    // frame by the caller. The first 0x98 bytes of the buffer are occupied by the
    // OTag entries, DISPENV, DRAWENV, and display rect data.
    node1 = *((unsigned char**)(base + 0x498));

    // SPRT: left half (256x224), texture page 0xA5 (8bpp, VRAM X=320)
    setSprt(node1);
    
    node1++;
    node1--;
    v0 = (unsigned char)g_fadeLevel;
    *((unsigned short*)(node1 + 8)) = 0;           // x = 0
    *((unsigned short*)(node1 + 10)) = 0;          // y = 0
    *((unsigned short*)(node1 + 16)) = 256;        // w = 256
    *((unsigned short*)(node1 + 18)) = 224;        // h = 224
    *((unsigned short*)(node1 + 16)) = 256;        // w = 256 (redundant)
    node1[12] = 0;                                 // u = 0
    node1[13] = 0;                                 // v = 0
    *((unsigned short*)(node1 + 14)) = 0x7800;     // clut = VRAM (0, 480)
    node1[6] = v0;                                 // b = fade level
    node1[5] = v0;                                 // g = fade level
    node1[4] = v0;                                 // r = fade level
    node1[7] = 0x64;                               // code = SPRT (redundant)
    *((unsigned long*)node1) = ((*((unsigned long*)node1)) & 0xFF000000UL) | ((*((unsigned long*)base)) & 0x00FFFFFFUL);
    *((unsigned long*)base) = ((*((unsigned long*)base)) & 0xFF000000UL) | (((unsigned long)node1) & 0x00FFFFFFUL);
    node1 += 20;

    // DR_TPAGE: select texture page 0xA5 before drawing left SPRT (8bpp, VRAM X=320, ABR=add)
    *((unsigned long*)(new_var = node1 + 4)) = 0xE10000A5UL; // GPU draw mode cmd, tpage=0xA5
    node3 = node1 + 8;
    node1[3] = 1;                                  // TAG: len = 1 word
    *((unsigned long*)node1) = ((*((unsigned long*)node1)) & 0xFF000000UL) | ((*((unsigned long*)base)) & 0x00FFFFFFUL);
    *((unsigned long*)base) = ((*((unsigned long*)base)) & 0xFF000000UL) | (((unsigned long)node1) & 0x00FFFFFFUL);

    // SPRT: right half (64x224), texture page 0xA7 (8bpp, VRAM X=448)
    node3[3] = 4;                                  // TAG: len = 4 words
    node3[7] = 0x64;                               // TAG: code = SPRT
    v0 = (unsigned char)g_fadeLevel;
    node1 = node3;
    node1 = node1 + 20;
    *((unsigned short*)(node3 + 8)) = 0x100;       // x = 256
    node3[6] = v0;                                 // b = fade level
    node3[5] = v0;                                 // g = fade level
    node3[4] = v0;                                 // r = fade level
    *((unsigned short*)(node3 + 10)) = 0;          // y = 0
    *((unsigned short*)(node3 + 16)) = 64;         // w = 64
    *((unsigned short*)(node3 + 18)) = 224;        // h = 224
    node3[12] = 0;                                 // u = 0
    node3[13] = 0;                                 // v = 0
    *((unsigned short*)(node3 + 14)) = 0x7800;     // clut = VRAM (0, 480)
    *((unsigned long*)node3) = ((*((unsigned long*)node3)) & 0xFF000000UL) | ((*((unsigned long*)base)) & 0x00FFFFFFUL);
    *((unsigned long*)base) = ((*((unsigned long*)base)) & 0xFF000000UL) | (((unsigned long)node3) & 0x00FFFFFFUL);

    // DR_TPAGE: select texture page 0xA7 before drawing right SPRT (8bpp, VRAM X=448, ABR=add)
    node1[3] = 1;                                  // TAG: len = 1 word
    node3 = node1 + 8;
    *((unsigned long*)(node1 + 4)) = 0xE10000A7UL; // GPU draw mode cmd, tpage=0xA7
    *((unsigned long*)node1) = ((*((unsigned long*)node1)) & 0xFF000000UL) | ((*((unsigned long*)base)) & 0x00FFFFFFUL);
    *((unsigned char**)(base + 0x498)) = node3;    // advance allocation cursor
    *((unsigned long*)((unsigned char*)pOtBuf)) =
        ((*((unsigned long*)((unsigned char*)pOtBuf))) & 0xFF000000UL) | (((unsigned long)node1) & 0x00FFFFFFUL);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/OafFK
 */
void LoadImageFromCd(s32 arg0, VramDstCoords* coordinates, u32 address)
{
    volatile u8 dummy[8];
    CD_QueueRead(arg0 & 0xFFFF, address);
    CD_WaitForQueueEmpty();
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
 * decomp.me link (97.02%) https://decomp.me/scratch/KjjKf
 */
void func_80140648(s32 arg0)
{
    s32 offset;
    u8* var_v1;
    u8* temp_a0;
    u8* dest;
    u8* src;
    s32* ptr;
    offset = -2;
    if (arg0 != offset)
    {
        D_80119F00.unk8 = 0;
        D_80119F00.unk4 = 0;
        D_80119F00.unk0 = 0;
        if (arg0 != (-1))
        {
            CD_QueueRead((arg0 + 0x51) & 0xFFFF, 0x80180000UL);
            CD_WaitForQueueEmpty();
            D_80119F00.unk0 = 0xC;
            offset = D_80180004;

            var_v1 = ((u8*)D_80180000) + offset;
            ptr = (s32*)var_v1;
            temp_a0 = var_v1 + ((u32)ptr[*ptr]);
            dest = ((u8*)(&D_80119F00)) + 12;
            if (var_v1 != temp_a0)
            {
                src = var_v1;
                do
                {
                    *(dest++) = *(src++);
                } while (src != temp_a0);
            }
            func_80022AE8((void*)temp_a0, 1);
        }
    }
}