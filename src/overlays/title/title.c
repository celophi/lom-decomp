#include "title.h"

/**
 * Top-level entry point of the TITLE.BIN overlay. Mirror of RunCheckPS in the
 * CHECKPS overlay: drives the title menu loop, dispatches on the user's
 * selection, and returns a state code consumed by the outer game-state
 * machine. The previously-decompiled symbol was title_func_8004FC74.
 *
 * decomp.me (100%) https://decomp.me/scratch/mEAXF
 */
s32 RunTitle(s32 arg0)
{
    s32 pad;
    S_801ED480* ptr = (S_801ED480*)0x801ED480;
    s32* base;
    u8* d8_base;
    u32 const_ff;
    s32 temp1, temp2;
    u8 d92;
    pad = arg0;

    LoadTitleAudioBank();
    LoadTitleSeq(0);
    StartTitleMusic();
    base = (s32*)0x80100000; /* base address for D_80102640 */
    const_ff = 0xFF;
    d8_base = D_80042FD8;
    while (1)
    {
        InitTitleDisplay(pad);
        ptr->unk0 = 0;
        ptr->unk2 = 0;
        ptr->unk4 = 0;
        ptr->unk8 = 0;
        ptr->unkC = 0;
        do
        {
            render_menu(pad);
        } while (base[0x990] == 0); /* g_titleMenuExitState (0x80102640): offset 0x2640/4 = 0x990 */

        D_80042FB4 = VSync(-1);
        d92 = g_titleSelectedItem;

        if (d92 == 0)
        {
            LoadMenuLayout(0);
            base[0x990] = 0; /* g_titleMenuExitState = 0 */
            if (RunSaveSlotMenu(pad) == 2)
            {
                GFX_Transition(0);
                continue;
            }
            return 3;
        }
        else if (d92 == 1)
        {
            return 7;
        }
        else if (d92 == const_ff)
        {
            StopTitleMusic();
            return 8;
        }
        else
        {
            func_800227D0(0, 0x3C, 0);
            LoadMenuLayout(-1);
            D_8003EC9C = const_ff;
            temp1 = rand();
            temp2 = rand();
            *(s16*)(d8_base + 0xD4) = (s16)(temp1 | (temp2 << 0xF));
            return 0;
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/bMLDn
 */
void render_menu(MenuContext* context)
{
    RECT rect;
    MenuContext* base = context;
    MenuContext* s0;
    u_long* s1;
    void* tmp;

    DrawSync(0);
    VSync(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 320;
    rect.h = 472;
    ClearImage(&rect, 0, 0, 0);

    s0 = base;
    ClearOTagR(s0->otag_buffer, 0x1000);
    ClearOTagR(s0->otag_buffer2, 0x1000);
    PutDispEnv(&s0->disp_env);
    func_800157DC();
    SetDispMask(1);

    while (1)
    {
        s1 = s0->otag_buffer;
        ClearOTagR(s1, 0x1000);
        s0->next_prim_ptr = s0->prim_buffer;
        rand();
        VSync(1);
        RenderFadeOverlay(s0);
        RenderTitleBackdrop(s0);
        RenderTitleMenuItems(s0);
        HandleTitleMenuInput();

        if (g_titleMenuExitState == 0)
        {
            DrawSync(0);
            func_800157B0(2);
            VSync(2);

            tmp = base;
            if (s0 == base)
            {
                tmp = s0->_pad4;
            }
            s0 = tmp;
            PutDispEnv(&s0->disp_env);
            PutDrawEnv(&s0->draw_env);
            DrawOTag((u_long*)(s1 + 4095));
            func_800157DC();
            cdrom_process_state();
            if (g_titleMenuExitState == 0)
            {
                continue;
            }
        }
        break;
    }

    func_800158E0();
    VSync(0);
    DrawSync(0);
}

/**
 * Save-slot picker: same loop shape as render_menu but drives the
 * sub-screen reached after the player selects "New Game" from the title.
 * Returns the menu exit state once the player confirms or cancels.
 *
 * decomp.me (100%) https://decomp.me/scratch/AKk7x
 */
s32 RunSaveSlotMenu(void* arg0)
{
    short rect[4];
    void* base;
    void* current;
    void* tmp;
    u_long* ot;

    base = arg0;

    InitSaveSlotMenu();
    GFX_Transition(0);
    SetFadeTarget(0x100, 0x100, 0x100, 0x14);
    DrawSync(0);
    VSync(0);
    rect[0] = 0;
    rect[1] = 0;
    rect[2] = 0x140;
    rect[3] = 0x1D8;
    ClearImage((RECT*)rect, 0, 0, 0);
    current = base;
    ClearOTagR((u_long*)((char*)current + 0x40), 0x1000);
    ClearOTagR((u_long*)((char*)current + 0xBD0C), 0x1000);
    VSync(0);
    PutDispEnv((DISPENV*)((char*)current + 0x4040));
    func_800157DC();
    SetDispMask(1);
    do
    {
        ot = (u_long*)((char*)current + 0x40);
        ClearOTagR(ot, 0x1000);
        *(void**)((char*)current + 0x80B8) = (void*)((char*)current + 0x40B8);
        VSync(1);
        RenderFadeOverlay(current);
        RenderSaveSlotMenu(current);
        DrawSync(0);
        func_800157B0(2);
        VSync(2);
        tmp = base;
        if (current == base)
            tmp = (char*)current + 0xBCCC;
        current = tmp;
        PutDispEnv((DISPENV*)((char*)current + 0x4040));
        PutDrawEnv((DRAWENV*)((char*)current + 0x4054));
        DrawOTag((u_long*)((char*)ot + 0x3FFC));
        func_800157DC();
        cdrom_process_state();
    } while (g_titleMenuExitState == 0);
    func_800158E0();
    VSync(0);
    return g_titleMenuExitState;
}

/**
 * Counterpart of InitCheckPSDisplay in the CHECKPS overlay.
 *
 * decomp.me (100%) https://decomp.me/scratch/evJur
 */
void InitTitleDisplay(void* arg0)
{
    RECT rect;
    unsigned char* base = (unsigned char*)arg0;
    unsigned char* hw = (unsigned char*)0x801ED600; /* hardware registers */
    unsigned char* base2;                           /* base + 0x8000 */

    /* Clear hardware register bytes */
    hw[0x13F] = 0;
    hw[0x91] = 0;
    hw[0x140] = 0;
    hw[0x92] = 0;

    func_8002237C(0);
    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);

    /* Write shorts at offsets 0x40B0..0x40B6 */
    *(short*)(base + 0x40B0) = 0;
    *(short*)(base + 0x40B2) = 0;
    *(short*)(base + 0x40B4) = 0x140;
    *(short*)(base + 0x40B6) = 0xF0;

    /* Secondary base (s0 = arg0 + 0x8000) */
    base2 = base + 0x8000;
    *(short*)(base2 + 0x7D7C) = 0;
    *(short*)(base2 + 0x7D7E) = 0xE8;
    *(short*)(base2 + 0x7D80) = 0x140;
    *(short*)(base2 + 0x7D82) = 0xF0;

    DrawSync(0);
    VSync(0);

    /* Clear a 0x400×0x200 rectangle */
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);

    /* Set display environments */
    SetDefDispEnv((DISPENV*)(base + 0x4040), 0, 0, 0x140, 0xF0);
    SetDefDispEnv((DISPENV*)(base + 0xFD0C), 0, 0xE8, 0x140, 0xF0);

    /* Set drawing environments */
    SetDefDrawEnv((DRAWENV*)(base + 0x4054), 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv((DRAWENV*)(base + 0xFD20), 0, 8, 0x140, 0xE0);

    /* Clear two more bytes */
    *(base + 0xFD36) = 0; /* was ctx->unkFD36 */
    *(base + 0x406A) = 0; /* was ctx->unk406A */

    ResetFadeState();
    SetFadeTarget(0x100, 0x100, 0x100, 0x14);
    InitTitleMenuState();

    g_titleMenuExitState = 0;
}

/**
 * Counterpart of CHECKPS func_800500FC: loads the title's audio bank from
 * CD-ROM into 0x8013C000 and registers it via func_80021FFC.
 *
 * decomp.me (100%) https://decomp.me/scratch/6zUZp
 */
void LoadTitleAudioBank(void)
{
    u8* base;
    u32* off;

    if (((u32)(g_previousGameState - 2) >= 2U) && (g_previousGameState != 6) && (g_previousGameState != 7) &&
        (g_previousGameState != 5))
    {

        D_80102668 = 0x8013C000;
        cdrom_queue_read(0x15, (void*)0x80180000);
        cdrom_wait_queue_empty();

        base = (u8*)0x80180000;
        off = (u32*)0x80180004;

        bcopy(base + off[0], (u8*)D_80102668, (int)(off[1] - off[0]));

        func_80021FFC(D_80102668);
        func_80022AE8(base + off[1], 1);
    }
}

/**
 * Counterpart of CHECKPS func_80050138: loads a SEQ resource from CD-ROM
 * into D_8003ECA0.
 *
 * decomp.me (100%) https://decomp.me/scratch/mBQ6i
 */
void LoadTitleSeq(s32 seqVariant)
{
    u32* off;
    u8* base;

    cdrom_queue_read((seqVariant + 0x17) & 0xFFFF, (void*)0x80180000);
    cdrom_wait_queue_empty();

    off = (u32*)0x80180004;
    base = (u8*)0x80180000;

    bcopy(base + off[0], (unsigned char*)&D_8003ECA0, (int)(off[1] - off[0]));
    func_80022AE8((s32)(base + off[1]), 1);
}

/**
 * Counterpart of CHECKPS func_800501AC.
 *
 * decomp.me (100%) https://decomp.me/scratch/1cta3
 */
void StopTitleMusic(void)
{
    func_80022068(0);
}

/**
 * Counterpart of CHECKPS func_800501CC.
 *
 * decomp.me (100%) https://decomp.me/scratch/xYPkq
 */
void StartTitleMusic(void)
{
    func_80022040(&D_8003ECA0);
    FUN_8002279c(0, 0x7F);
}

/**
 * Counterpart of CHECKPS func_800501FC (with the first parameter folded
 * away to a constant 0). soundId values observed: 0x3C (selection chime),
 * 0x7C..0x7F (cursor / cancel / confirm beeps).
 *
 * decomp.me (100%) https://decomp.me/scratch/ZuKeL
 */
void PlayTitleSfx(s32 soundId, s32 arg1)
{
    func_8002216C(soundId, 0, arg1, 0x7F);
}

/**
 * Counterpart of CHECKPS ResetFadeState.
 *
 * decomp.me (100%) https://decomp.me/scratch/m80gj
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
 * Counterpart of CHECKPS func_80050258: interpolates g_fadeCurrent toward
 * g_fadeTarget and emits the fade-overlay primitive into the active prim
 * buffer.
 *
 * decomp.me (100%) https://decomp.me/scratch/fBro2
 */
void RenderFadeOverlay(void* arg0)
{
    ArgStruct* arg = (ArgStruct*)arg0;
    u32* var_t4 = (u32*)arg->unk80B8;
    u32* unk40_ptr = (u32*)(((u8*)arg) + 0x40);
    s32 temp_a2;
    s32 temp_a0;
    s32 temp_v1;
    s32 var_a1;
    if (g_fadeTarget.steps != 0)
    {
        temp_a2 = ((s32)(g_fadeTarget.red - g_fadeCurrent.red)) / ((s32)g_fadeTarget.steps);
        temp_a0 = ((s32)(g_fadeTarget.green - g_fadeCurrent.green)) / ((s32)g_fadeTarget.steps);
        temp_v1 = ((s32)(g_fadeTarget.blue - g_fadeCurrent.blue)) / ((s32)g_fadeTarget.steps);
        g_fadeTarget.steps = g_fadeTarget.steps - 1;
        g_fadeCurrent.red = g_fadeCurrent.red + temp_a2;
        g_fadeCurrent.green = g_fadeCurrent.green + temp_a0;
        g_fadeCurrent.blue = g_fadeCurrent.blue + temp_v1;
    }
    else
    {
        g_fadeCurrent.red = g_fadeTarget.red;
        g_fadeCurrent.green = g_fadeTarget.green;
        g_fadeCurrent.blue = g_fadeTarget.blue;
    }
    if (!(((g_fadeCurrent.red == 0x100) && (g_fadeCurrent.green == 0x100)) && (g_fadeCurrent.blue == 0x100)))
    {
        if (((s32)g_fadeCurrent.red) >= 0x101)
        {
            ((u8*)var_t4)[4] = ((u8)g_fadeCurrent.red) - 1;
            ((u8*)var_t4)[5] = ((u8)g_fadeCurrent.green) - 1;
            ((u8*)var_t4)[6] = ((u8)g_fadeCurrent.blue) - 1;
        }
        else
        {
            if (g_fadeCurrent.red == 0x100)
            {
                ((u8*)var_t4)[4] = 0;
            }
            else
            {
                ((u8*)var_t4)[4] = ~((u8)g_fadeCurrent.red);
            }
            if (g_fadeCurrent.green == 0x100)
            {
                ((u8*)var_t4)[5] = 0;
            }
            else
            {
                ((u8*)var_t4)[5] = ~((u8)g_fadeCurrent.green);
            }
            if (g_fadeCurrent.blue == 0x100)
            {
                ((u8*)var_t4)[6] = 0;
            }
            else
            {
                ((u8*)var_t4)[6] = ~((u8)g_fadeCurrent.blue);
            }
        }
        ((u8*)var_t4)[3] = 3;
        ((u8*)var_t4)[7] = 0x62;
        *((u16*)(((u8*)var_t4) + 12)) = 0x140;
        *((u16*)(((u8*)var_t4) + 10)) = 0;
        *((u16*)(((u8*)var_t4) + 8)) = 0;
        *((u16*)(((u8*)var_t4) + 14)) = 0xF0;
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        ;
        var_a1 = 0x25;
        var_t4 = (u32*)(((u8*)var_t4) + 0x10);
        if (((s32)g_fadeCurrent.red) < 0x101)
        {
            var_a1 = 0x45;
        }
        ((u8*)var_t4)[3] = 1;
        *((u32*)(((u8*)var_t4) + 4)) = (s32)(var_a1 | 0xE1000000);
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        var_t4 = (u32*)(((u8*)var_t4) + 8);
    }
    arg->unk80B8 = var_t4;
}

/**
 * Counterpart of CHECKPS SetFadeTarget.
 *
 * decomp.me (100%) https://decomp.me/scratch/zxqdP
 */
void SetFadeTarget(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fadeTarget.red = red;
    g_fadeTarget.green = green;
    g_fadeTarget.blue = blue;
    g_fadeTarget.steps = steps;
}

/**
 * Emits the title screen's tiled backdrop strip (5 quads stepping 0x40 px).
 *
 * decomp.me (94.41%) https://decomp.me/scratch/Lluk6
 */
void RenderTitleBackdrop(void* arg0)
{
    u8* base = (u8*)arg0;
    unsigned int new_var;
    u8* t3;
    u32* a2;
    s32 t0;
    s32 t1;
    s32 a3;
    u8* a1;
    s32 temp_v0;
    s32 temp_v1;

    a2 = *((u32**)(base + 0x80B8));

    t3 = base + 0x40;

    t0 = 0;

    t1 = 0x140;

    a3 = 0x40;

    while (t0 < 5)
    {
        temp_v1 = t1 & 0x3FF;

        t1 += 0x40;

        *((short*)((((u8*)a2) + 0x0E) + 0x12)) = (short)a3;
        *((short*)((((u8*)a2) + 0x0E) + 2)) = (short)a3;
        a3 += 0x40;
        temp_v0 = t0 << 6;
        t0++;
        *((short*)((((u8*)a2) + 0x0E) + 0x0A)) = (short)temp_v0;
        *((short*)((((u8*)a2) + 0x0E) - 6)) = (short)temp_v0;
        (((u8*)a2) + 0x0E)[-0x0B] = 9;
        (((u8*)a2) + 0x0E)[-7] = 0x2C;
        (((u8*)a2) + 0x0E)[-8] = 0x80;
        (((u8*)a2) + 0x0E)[-9] = 0x80;
        (((u8*)a2) + 0x0E)[-0x0A] = 0x80;
        *((short*)((((u8*)a2) + 0x0E) + 4)) = 0;
        *((short*)((((u8*)a2) + 0x0E) - 4)) = 0;
        *((short*)((((u8*)a2) + 0x0E) + 0x14)) = 0xE0;
        *((short*)((((u8*)a2) + 0x0E) + 0x0C)) = 0xE0;
        (((u8*)a2) + 0x0E)[0x0E] = 0;
        (((u8*)a2) + 0x0E)[-2] = 0;
        (((u8*)a2) + 0x0E)[0x16] = 0x40;
        (((u8*)a2) + 0x0E)[6] = 0x40;
        (((u8*)a2) + 0x0E)[7] = 8;
        (((u8*)a2) + 0x0E)[-1] = 8;
        (((u8*)a2) + 0x0E)[0x17] = 0xE8;
        (((u8*)a2) + 0x0E)[0x0F] = 0xE8;
        *((short*)((((u8*)a2) + 0x0E) + 8)) = (short)((temp_v1 >> 6) | 0x110);
        *((short*)((((u8*)a2) + 0x0E) + 0)) = 0x7840;
        new_var = ((u32)a2) & 0xFFFFFF;
        a1 += 0x28;
        *a2 = ((*a2) & 0xFF000000) | ((*((u32*)(t3 + 0x3FFC))) & 0xFFFFFF);
        *((u32*)(t3 + 0x3FFC)) = ((*((u32*)(t3 + 0x3FFC))) & 0xFF000000) | new_var;
        a2 = (u32*)(((u8*)a2) + 0x28);
    }
    *((u32**)(base + 0x80B8)) = a2;
}

/**
 * Per-frame input dispatcher for the main title menu.
 *
 * decomp.me (100%) https://decomp.me/scratch/vmcmD
 */
void HandleTitleMenuInput(void)
{
    s32 op = 0x7C;
    UpdateMenuInput();
    if (g_titleIdleCountdown == 0)
    {
        g_titleMenuExitState = 1;
        g_titleSelectedItem = 0xFF;
        return;
    }
    g_titleIdleCountdown -= 1;
    if (g_debouncedInput & 0xA20)
    {
        PlayTitleSfx(op, 0x80);
        g_titleMenuExitState = 1;
        return;
    }
    if (g_debouncedInput & 0x9000)
    {
        MenuCursorUp();
        PlayTitleSfx(0x7D, 0x80);
    }
    else if (g_debouncedInput & 0x6100)
    {
        MenuCursorDown();
        PlayTitleSfx(0x7D, 0x80);
    }
}

/**
 * Linear-search D_80102670 forward for the next enabled menu slot. If none
 * remain, the cursor is reset to slot 0 with rank 0.
 *
 * decomp.me (100%) https://decomp.me/scratch/6k8uV
 */
void MenuCursorDown(void)
{
    s32 a1;
    u8* var_v1;
    const s32 LIMIT = 16;
    a1 = g_titleSelectedItem + 1;
    if (a1 < LIMIT)
    {
        u8* base = g_titleMenuItemFlags; // forces lui/addiu first
        var_v1 = base + a1 * 2;          // sll comes after
        while (1)
        {
            if (*var_v1 != 0)
            {
                break;
            }
            a1++;
            if (a1 < LIMIT)
            {
                var_v1 += 2;
                continue;
            }
            break;
        }
    }
    if (a1 == LIMIT)
    {
        g_titleVisibleItemRank = 0;
        g_titleSelectedItem = 0;
    }
    else
    {
        g_titleSelectedItem = (u8)a1;
        g_titleVisibleItemRank++;
    }
}

/**
 * Mirror of MenuCursorDown searching backward.
 *
 * decomp.me (100%) https://decomp.me/scratch/mmzjI
 */
void MenuCursorUp(void)
{
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_v1_2;
    u8* var_a0;
    u8* var_v1;
    u8* base;
    u8 temp;
    var_a1 = g_titleSelectedItem - 1;
    if (var_a1 >= 0)
    {
        base = &g_titleMenuItemFlags[0];
        var_v1 = base + (var_a1 * 2);
        while (var_a1 >= 0)
        {
            if ((*var_v1) != 0)
            {
                break;
            }
            var_a1--;
            var_v1 -= 2;
        }
    }
    if (var_a1 < 0)
    {
        var_v1_2 = 0;
        var_a1_2 = 0;
        do
        {
            var_a0 = &g_titleMenuItemFlags[0];
            while (var_a1_2 < 0x10)
            {
                if ((*var_a0) != 0)
                {
                    var_v1_2++;
                    var_a2 = var_a1_2;
                }
                var_a1_2++;
                var_a0 += 2;
            };
        } while (0);

        g_titleVisibleItemRank = var_v1_2 - 1;
        g_titleSelectedItem = (u8)var_a2;
        return;
    }
    temp = g_titleVisibleItemRank;
    g_titleSelectedItem = (u8)var_a1;
    g_titleVisibleItemRank = temp - 1;
    return;
}

/**
 * Walks D_80102670 and emits the cursor + each enabled menu item's quad.
 * The cursor uses D_8007FD2C[(g_titleAnimFrame >> 2) & 3] as a 4-frame
 * blink palette index.
 *
 * decomp.me (65.89%) https://decomp.me/scratch/K627m
 */
void RenderTitleMenuItems(void* arg0)
{
    s32 temp_s4;
    s32 var_a1;
    s32 var_s0;
    s32 var_s2;
    s32 var_s3;
    s32 var_s6;
    u8* var_s1;
    s32 unk80B8;
    s32 var_v0;
    s32 result;
    temp_s4 = (s32)(((u8*)arg0) + 0x40);
    var_s6 = 0x88;
    var_s3 = 0xA0;
    var_s2 = 0;
    var_s0 = 0;
    unk80B8 = *((s32*)(((u8*)arg0) + 0x80B8));
    var_a1 = EmitMenuItemQuad(temp_s4, unk80B8, 0, 0x64, 0xC8, 0, 0x80, 1);
    var_s1 = &g_titleMenuItemFlags[0];
    do
    {
        if ((*var_s1) != 0)
        {
            if (g_titleVisibleItemRank == var_s2)
            {
                var_v0 = 1;
            }
            else
            {
                var_v0 = 2;
            }
            var_a1 = EmitMenuItemQuad(temp_s4, var_a1, var_s0 + 1, var_s6, var_s3, 0, 0x80, var_v0) + 0x28;
            var_s2++;
            var_s3 += 0xC;
        }
        var_s0++;
        var_s1 += 2;
    } while (var_s0 < 0x10);
    result = EmitMenuItemQuad(temp_s4, var_a1, 7, 0x78, (6 * (2 * ((s32)g_titleVisibleItemRank))) + 0x9D,
                              (s32)D_8007FD2C[(g_titleAnimFrame >> 2) & 3], 0x10, 0);
    *((s32*)(((u8*)arg0) + 0x80B8)) = result;
    g_titleAnimFrame++;
}

/**
 * Builds a single menu-item sprite primitive (text quad + tex-page word)
 * at (x, y) and links it into the OT.
 *
 * decomp.me (100%) https://decomp.me/scratch/FcuOZ
 */
void* EmitMenuItemQuad(s32* otHead, void* prim, s32 slot, s16 x, s32 y, s32 colorY, s32 step, s32 paletteIdx)
{
    u8* ptr;
    u8 tmp1;
    u8 tmp2;
    u16 sum1;
    u16 sum2;
    u8 bsum;
    u8* new_var;
    u16 t1_val;
    u32 old_word;
    u32 new_word;
    u32 mask_lo;
    u32 mask_hi;
    ptr = (u8*)prim;
    mask_lo = 0x00FFFFFF;
    ptr[0x03] = 9;
    ptr[0x07] = 0x2C;
    tmp1 = (u8)(slot << 4);
    ptr[0x06] = 0x80;
    ptr[0x15] = tmp1;
    ptr[0x0D] = tmp1;
    tmp2 = (u8)((slot << 4) + 0x10);
    ptr[0x05] = 0x80;
    ptr[0x04] = 0x80;
    ptr[0x25] = tmp2;
    ptr[0x1D] = tmp2;
    *((u16*)(ptr + 0x18)) = (u16)x;
    *((u16*)(ptr + 0x08)) = (u16)x;
    *((u16*)(ptr + 0x16)) = 5;
    mask_hi = 0xFF000000;
    sum1 = (u16)(x + step);
    new_var = ptr + 0x12;
    *((u16*)new_var) = (u16)y;
    *((u16*)(ptr + 0x0A)) = (u16)y;
    sum2 = (u16)(y + 0x10);
    do
    {
    } while (0);
    ptr[0x1C] = (u8)colorY;
    ptr[0x0C] = (u8)colorY;
    bsum = (u8)(colorY + step);
    t1_val = (u16)((paletteIdx & 0x3F) | 0x7800);
    *((u16*)(ptr + 0x22)) = sum2;
    *((u16*)(ptr + 0x1A)) = sum2;
    old_word = *((u32*)ptr);
    *((u16*)(ptr + 0x20)) = sum1;
    *((u16*)(ptr + 0x10)) = sum1;
    ptr[0x24] = bsum;
    ptr[0x14] = bsum;
    *((u16*)(ptr + 0x0E)) = t1_val;
    new_word = (old_word & mask_hi) | (((u32)(*otHead)) & mask_lo);
    *((u32*)ptr) = new_word;
    *otHead = (s32)((((u32)(*otHead)) & mask_hi) | (((u32)ptr) & mask_lo));
    return (void*)(ptr + 0x28);
}

/**
 * Initialises menu-state globals: zeros the per-slot flag table, enables
 * the first 4 slots, sets the idle countdown to 0xE10 frames (≈60 s @ 60Hz),
 * and uploads the two menu TIMs from D_800522E8[1..2] to VRAM.
 *
 * decomp.me (100%) https://decomp.me/scratch/HW23j
 */
void InitTitleMenuState(void)
{
    u8* ptr;
    s32 i;
    s32 idx;
    u8* q;
    i = 0;
    ptr = g_titleMenuItemFlags;
    for (i = 0; i < 16; i++)
    {
        ptr[0] = 0;
        ptr[1] = 0;
        ptr += 2;
    }

    g_titleMenuItemFlags[0] = 1;
    g_titleMenuItemFlags[1] = 1;
    g_titleMenuItemFlags[2] = 1;
    g_titleMenuItemFlags[3] = 1;

    g_titleVisibleItemRank = 0;

    g_titleSelectedItem = 0;
    g_titleAnimFrame = 0;
    g_inputRepeatTimer = 0;
    g_lastInputState = 0;
    g_debouncedInput = 0;
    g_titleIdleCountdown = 0xE10;
    UploadTim((void*)(((u8*)&D_800522E8) + D_800522E8[1]), 0x140, 0, 0, 0x1E0);
    UploadTim((void*)(((u8*)&D_800522E8) + D_800522E8[2]), 0x140, 0x100, 0, 0x1E1);
    if (g_previousGameState == 0)
    {
        idx = g_titleSelectedItem + 1;
        if (idx < 16)
        {
            q = g_titleMenuItemFlags + (idx << 1);
            do
            {
                if ((D_800522E8 && D_800522E8) && D_800522E8)
                {
                }
                if ((*q) != 0)
                {
                    break;
                }
                idx++;
                q += 2;
            } while (idx < 16);
        }
        if (idx == 16)
        {
            g_titleVisibleItemRank = 0;
            g_titleSelectedItem = 0;
        }
        else
        {
            g_titleSelectedItem = (u8)idx;
            g_titleVisibleItemRank++;
        }
    }
}

/**
 * Standard TIM-style image uploader: reads the header flags at +4, then
 * conditionally LoadImages the CLUT followed by the pixel block.
 *
 * decomp.me (100%) https://decomp.me/scratch/fzh5x
 */
void UploadTim(void* tim, s16 x, s16 y, s16 clutX, s32 clutY)
{
    u8* p = (u8*)tim;
    int new_var;
    RECT rect;
    s32 temp_s1;
    u16 mul_h;
    int new_var2;
    int new_var3;
    u16 mul_l;
    new_var = 8;
    new_var2 = 1;
    if (p[4] & new_var)
    {
        mul_h = *((u16*)(p + 0x10));
        mul_l = *((u16*)(p + 0x12));
        temp_s1 = *((s32*)(p + new_var));
        new_var3 = 8;
        rect.x = clutX;
        rect.y = (s16)clutY;
        rect.w = mul_h * mul_l;
        rect.h = new_var2;
        LoadImage(&rect, (u_long*)(p + 0x14));
        p = (p + new_var3) + temp_s1;
    }
    else
    {
        p = p + 8;
    }
    rect.x = x;
    rect.y = y;
    rect.w = *((u16*)(p + 8));
    rect.h = *((u16*)(p + 0xA));
    LoadImage(&rect, (u_long*)(p + 0xC));
}

/**
 * Apparent dead/unused early variant of the pad-read routine — same shape
 * as read_pad_input but returns the bitmap rather than writing globals.
 *
 * decomp.me (100%) https://decomp.me/scratch/Z5swg
 */
s32 func_80050EE4(void)
{
    signed short new_var;
    unsigned char* ptr;
    unsigned char a2;
    unsigned short v1;
    unsigned short v0;
    unsigned long a0_val;
    unsigned int new_var2;
    signed short t;
    ptr = (unsigned char*)0x801ED600;
    a2 = ptr[0];
    if (a2 >= 0xFE)
    {
        return 0;
    }
    v1 = *((unsigned short*)(ptr + 2));
    new_var2 = *((unsigned short*)(ptr + 2));
    v0 = new_var2;
    a0_val = (v1 >> 8) | (v0 << 8);
    a0_val =
        (((((((((a0_val & 0x40) >> 1) | ((a0_val & 0x20) << 1)) | ((a0_val & 0x80) >> 3)) | ((a0_val & 0x10) << 3)) &
            0xFFFF) &
           0xFFFF) &
          0xFFFF) &
         0xFFFF) |
        (a0_val & (~0xF0));
    if (a2)
    {
        t = *((signed short*)(ptr + 0x2C));
        new_var = t;
        if (t < (-1))
        {
            a0_val |= 0x8000;
        }
        else if (new_var >= 2)
        {
            a0_val |= 0x2000;
        }
        t = *((signed short*)(ptr + 0x2E));
        if (t < (-1))
        {
            a0_val |= 0x1000;
        }
        else if (t >= 2)
        {
            a0_val |= 0x4000;
        }
    }
    return a0_val;
}

/**
 * Counterpart of CHECKPS UpdateInputDebounced: reads SCD pad state, builds
 * the remapped button bitmap, applies debounce + auto-repeat using
 * g_lastInputState / g_inputRepeatTimer, and stores the result in
 * g_debouncedInput.
 *
 * decomp.me (99.90%) https://decomp.me/scratch/yZqQJ
 */
void UpdateMenuInput(void)
{
    u8* ptr = (u8*)0x801ED600;
    u8 a2 = D_801ED600[0];
    u16 v1;
    u16 v0;
    u32 a1_val;
    s16 t;
    s32 var_v1;
    if (a2 >= 0xFE)
    {
        var_v1 = 0;
    }
    else
    {
        v1 = *((u16*)(ptr + 2));

        a1_val = (v1 >> 8) | (*((u16*)(2 + ptr)) << 8);
        a1_val = (((((((((a1_val & 0x40) >> 1) | ((a1_val & 0x20) << 1)) | ((a1_val & 0x80) >> 3)) |
                      ((a1_val & 0x10) << 3)) &
                     0xFFFF) &
                    0xFFFF) &
                   0xFFFF) &
                  0xFFFF) |
                 (a1_val & (~0xF0));
        if ((*ptr) != 0)
        {
            t = *((s16*)(ptr + 0x2C));
            if (t < (-1))
            {
                a1_val |= 0x8000;
            }
            else if (t >= 2)
            {
                a1_val |= 0x2000;
            }
            t = *((volatile s16*)(ptr + 0x2E));
            if (t < (-1))
            {
                a1_val |= 0x1000;
            }
            else if (t >= 2)
            {
                a1_val |= 0x4000;
            }
        }
        var_v1 = a1_val;
    }
    g_debouncedInput = 0;
    if ((var_v1 == g_lastInputState) || ((g_lastInputState != 0) && (var_v1 & (g_lastInputState | 0xB6F))))
    {
        if (var_v1 != 0)
        {
            u32 tmp = var_v1 & 0xF000;
            if (tmp != 0)
            {
                var_v1 = tmp;
            }
            if (g_inputRepeatTimer == 0)
            {
                g_debouncedInput = var_v1;
                g_inputRepeatTimer = 2;
            }
            else
            {
                g_inputRepeatTimer--;
                g_debouncedInput = 0;
            }
        }
        else
        {
            *((s32*)(&g_inputRepeatTimer)) = 0;
        }
        *((s32*)(&g_lastInputState)) = 0;
    }
    else if (var_v1 == 0)
    {
        (void)(&g_debouncedInput);
        *((s32*)(&g_inputRepeatTimer)) = 0;
        *((s32*)(&g_lastInputState)) = 0;
    }
    else
    {
        g_debouncedInput = var_v1;
        g_lastInputState = var_v1;
        g_inputRepeatTimer = 15;
    }
}

/**
 * Counterpart of CHECKPS UpdateControllerInput.
 *
 * decomp.me (100%) https://decomp.me/scratch/1dQbp
 */
static void read_pad_input(void)
{
    SCDRegs* base = (SCDRegs*)0x801ED600;
    s32 state;
    u32 buttons;
    s16 axis;

    g_debouncedInput = 0;
    if (D_801ED600[0] >= 254)
    {
        state = 0;
    }
    else
    {
        buttons = ((base->buttonData >> 8) & 0xFF) | (base->buttonData << 8);
        buttons = (((((buttons & PAD_BTN_CIRCLE) >> 1) | ((buttons & PAD_BTN_CROSS) << 1)) |
                    ((buttons & PAD_BTN_TRIANGLE) >> 3)) |
                   ((buttons & PAD_BTN_SQUARE) << 3)) |
                  (buttons & ~0xF0);
        if (base->deviceState != 0)
        {
            axis = base->axisX;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = base->axisY;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        state = buttons;
    }
    g_lastInputState = state;
    g_inputRepeatTimer = 15;
}

/**
 * Initialises the save-slot sub-menu state and uploads its sprite atlases.
 *
 * decomp.me (100%) https://decomp.me/scratch/t2lHt
 */
void InitSaveSlotMenu(void)
{
    read_pad_input();
    g_slotSlideFrames = 0;
    g_slotSlideYLerped = 0;
    g_slotSlideY = 0;
    g_slotSlideXLerped = 0;
    g_slotSlideX = 0;
    g_slotSelectedIndex = 0;
    g_slotHighlightX = 0;
    g_slotHighlightTargetX = 0;
    g_slotHighlightFrames = 0;
    UploadSaveLayoutTextures();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/so5cY
 */
void RenderSaveSlotMenu(void* arg0)
{
    InnerStruct* inner = (InnerStruct*)((char*)arg0 + 0x8000);
    inner->unk80B8 = RenderSaveLayoutPrims(inner->unk80B8, (char*)arg0 + 0x40);
    HandleSaveSlotInput();
}

/**
 * Per-frame input dispatcher for the save-slot sub-menu: drives the slide
 * lerper, decodes confirm/cancel/select, and reacts to L/R panel scrolls.
 *
 * decomp.me (99.41%) https://decomp.me/scratch/A1OF6
 */
void HandleSaveSlotInput(void)
{
    s32 temp_a1;
    s32 temp_s0;
    u8* new_var;
    s32* new_var4;
    s32 temp_v0_2;
    int new_var2;
    s32 temp_v0_3;
    s32 new_var3;
    s32 temp_v1;
    s32 var_a0_2;
    s32 var_v0;
    u32 var_a0;
    u8 temp_v0;
    u8* var_a1;
    u8* var_v1;
    u8* dest_ptr;
    s32 flag;
    if (g_slotSlideFrames != 0)
    {
        new_var4 = &g_slotSlideXLerped;
        temp_a1 = ((s32)(g_slotSlideX - (*new_var4))) / ((s32)g_slotSlideFrames);
        temp_v1 = ((s32)(g_slotSlideY - g_slotSlideYLerped)) / ((s32)g_slotSlideFrames);
        g_slotSlideFrames -= 1;
        g_slotSlideXLerped += temp_a1;
        g_slotSlideYLerped += temp_v1;
        return;
    }
    g_slotSlideXLerped = g_slotSlideX;
    g_slotSlideYLerped = g_slotSlideY;
    UpdateMenuInput();
    if (g_slotSlideX == 0)
    {
        if (g_debouncedInput & 0xA000)
        {
            PlayTitleSfx(0x7D, 0x80);
            if (D_800F993C[0x1B1] != 0)
            {
                D_800F993C[0x1B1] = 0;
                D_800F993C[0x1C9] = 1;
                return;
            }
            D_800F993C[0x1B1] = 1;
            D_800F993C[0x1C9] = g_slotSlideYLerped * 0;
            return;
        }
        if (g_debouncedInput & 0xA20)
        {
            PlayTitleSfx(0x7E, 0x80);
            if (D_800F9AED != 0)
            {
                ScrollSlotsRight();
                ResetSaveSlotPanel();
                return;
            }
            ScrollSlotsLeft();
            ResetSaveSlotPanel();
            return;
        }
        if (g_debouncedInput & 0x40)
        {
            PlayTitleSfx(0x7F, 0x80);
            g_titleMenuExitState = 2;
        }
    }
    else
    {
        if (g_debouncedInput & 0xA20)
        {
            if (g_slotSlideX > 0)
            {
                LoadSubMenuLayout(0);
                flag = ~0x7F;
                var_a1 = D_80042FD8;
                var_v0 = (*((s32*)(var_a1 + 0x608))) & flag;
            }
            else
            {
                LoadSubMenuLayout(1);
                flag = ~0x7F;
                var_a1 = D_80042FD8;
                var_v0 = ((*((s32*)(var_a1 + 0x608))) & flag) | 1;
            }
            *((s32*)(var_a1 + 0x608)) = var_v0;
            temp_s0 = rand();
            new_var2 = rand();
            temp_s0 |= new_var2 << 0xF;
            *((s16*)(var_a1 + 0xD4)) = (s16)temp_s0;
            dest_ptr = D_80043618;
            var_v1 = D_800F9BC4 + (g_slotSelectedIndex << 6);
            var_a0 = 0;
            while (var_a0 < 0x40U)
            {
                var_a0 += 1;
                temp_v0 = *var_v1;
                var_v1 += 1;
                *dest_ptr = temp_v0;
                dest_ptr += 1;
            }

            var_a0_2 = 0;
            new_var3 = g_slotSelectedIndex;
            var_v1 = D_80042FD8;
            var_a0_2 = 0;
            do
            {
                if (new_var3 != var_a0_2)
                {
                    *((s32*)(var_v1 + 0x34)) = 0;
                }
                var_a0_2 += 1;
                var_v1 += 4;
            } while (var_a0_2 < 0xB);
            PlayTitleSfx(0x7E, 0x80);
            g_titleMenuExitState = 1;
        }
        else if (g_debouncedInput & 0x40)
        {
            PlayTitleSfx(0x7F, 0x80);
            if (g_slotSlideX > 0)
            {
                ScrollSlotsLeft();
                ResetSaveSlotPanel();
            }
            else
            {
                ScrollSlotsRight();
                ResetSaveSlotPanel();
            }
        }
        else if (g_slotHighlightFrames == 0)
        {
            if ((g_debouncedInput & 0x1000) != 0U)
            {
                PlayTitleSfx(0x7D, 0x80);
                temp_v0_2 = g_slotSelectedIndex - 1;
                g_slotSelectedIndex = temp_v0_2;
                if (temp_v0_2 < 0)
                {
                    g_slotSelectedIndex = 0xA;
                }
            }
            if (g_debouncedInput & 0x4000)
            {
                PlayTitleSfx(0x7D, 0x80);
                temp_v0_3 = g_slotSelectedIndex + 1;
                g_slotSelectedIndex = temp_v0_3;
                if (temp_v0_3 >= 0xB)
                {
                    g_slotSelectedIndex = 0;
                }
            }
        }
        AnimateSaveSlotPanel();
    }
}

/**
 * Lerps g_slotHighlightX toward g_slotHighlightTargetX over
 * g_slotHighlightFrames frames and updates the panel-relative UI element
 * coordinates and visibility flags inside D_800F993C.
 *
 * decomp.me (99.09%) https://decomp.me/scratch/yKwnh
 */
void AnimateSaveSlotPanel(void)
{
    s16 temp_v0_2;
    s16 temp_v1;
    u8* new_var;
    s32 temp_a1;
    s32 temp_v0;
    s32 var_a0;
    s32 var_a1;
    s32 var_v0;
    u8* base;
    s32 var_v0_2;
    if (g_slotHighlightFrames != 0)
    {
        temp_v0 = (g_slotHighlightTargetX - g_slotHighlightX) / g_slotHighlightFrames;
        g_slotHighlightFrames -= 1;
        g_slotHighlightX += temp_v0;
    }
    else
    {
        g_slotHighlightX = g_slotHighlightTargetX;
    }
    var_a1 = g_slotHighlightTargetX;
    if (g_slotHighlightTargetX < 0)
    {
        var_a1 = 0xF;
        var_a1 = g_slotHighlightTargetX + var_a1;
    }
    temp_a1 = var_a1 >> 4;
    if (g_slotSelectedIndex < (var_a1 >> 4))
    {
        var_v0 = g_slotSelectedIndex * 0x10;
        g_slotHighlightTargetX = var_v0;
        g_slotHighlightFrames = 4;
    }
    else if ((temp_a1 + 6) < g_slotSelectedIndex)
    {
        var_a0 = 0x10;
        var_v0 = (g_slotSelectedIndex - 6) * var_a0;
        g_slotHighlightTargetX = var_v0;
        g_slotHighlightFrames = 4;
    }
    base = D_800F993C;
    temp_a1 = ((u16)g_slotHighlightX) + 0x20;
    temp_v1 = temp_a1;
    *((u16*)(D_800F993C + 0x3E)) = (u16)g_slotHighlightX;
    *((u16*)(D_800F993C + 0x56)) = temp_v1;
    *((u16*)(D_800F993C + 0xE6)) = (u16)g_slotHighlightX;
    *((u16*)(D_800F993C - (-0xFE))) = temp_v1;
    if (g_slotHighlightX != 0)
    {
        D_800F993C[0xA9] = 1;
        D_800F993C[0xC1] = 1;
        D_800F993C[0x151] = 1;
        D_800F993C[0x169] = 1;
    }
    else
    {
        D_800F993C[0xA9] = 0;
        D_800F993C[0xC1] = 0;
        D_800F993C[0x151] = 0;
        D_800F993C[0x169] = 0;
    }
    if (g_slotHighlightX != 0x40)
    {
        D_800F993C[0x61] = 1;
        D_800F993C[0x79] = 1;
        D_800F993C[0x109] = 1;
        D_800F993C[0x121] = 1;
    }
    else
    {
        base[0x61] = 0;
        base[0x79] = 0;
        D_800F993C[0x109] = 0;
        D_800F993C[0x121] = 0;
    }
    var_a0 = (g_slotSelectedIndex * 0x10) - g_slotHighlightX;
    if (var_a0 < 0)
    {
        var_a0 = 0;
    }
    if (var_a0 > 0x60)
    {
        var_a0 = 0x60;
    }
    new_var = D_800F993C;
    temp_v0_2 = var_a0 + 0x40;
    *((u16*)(new_var + 0x96)) = temp_v0_2;
    *((u16*)(new_var + 0x9A)) = temp_v0_2;
    *((u16*)(new_var + 0x13E)) = temp_v0_2;
    *((u16*)(new_var + 0x142)) = temp_v0_2;
}

/**
 * Snaps the save-slot panel back to its home position and clears its
 * highlight/selection state.
 *
 * decomp.me (100%) https://decomp.me/scratch/0YgmZ
 */
void ResetSaveSlotPanel(void)
{
    s16 temp_v1;
    if (g_slotSlideX != 0)
    {
        u32 base = (u32)(&D_800F993C);
        *((u16*)(base + 0xE)) = 0x10;
        g_slotSelectedIndex = 0;
        g_slotHighlightX = 0;
        g_slotHighlightTargetX = 0;
        g_slotHighlightFrames = 0;
        *((u8*)(base + 0xA9)) = 0;
        base++;
        base--;
        *((u8*)(base + 0xC1)) = 0;
        *((u8*)(base + 0x151)) = 0;
        *((u8*)(base + 0x169)) = 0;
        temp_v1 = ((u16)g_slotHighlightX) + 0x20;
        *((u16*)(base + 0x96)) = 0x40;
        *((u16*)(base + 0x9A)) = 0x40;
        *((u16*)(base + 0x13E)) = 0x40;
        *((u16*)(base + 0x142)) = 0x40;
        *((u16*)(base + 0xC)) = 0;
        *((u8*)(base + 0x61)) = 1;
        *((u8*)(base + 0x79)) = 1;
        *((u8*)(base + 0x109)) = 1;
        *((u8*)(base + 0x121)) = 1;

        *((u16*)(base + 0x3E)) = (u16)g_slotHighlightX;
        *((u16*)(base + 0x56)) = temp_v1;
        *((u16*)(base + 0xE6)) = (u16)g_slotHighlightX;
        *((u16*)(base + 0xFE)) = temp_v1;
        return;
    }
    {
        u32 low_addr = (u32)(&D_800F993C);
        u32 ptr = g_slotSlideX + low_addr;
        *((u16*)(ptr + 0xC)) = 0x10;
        *((u16*)(ptr + 0xE)) = 0;
    }
}

/**
 * Begins an 8-frame slide of the slot panel by +0xA0 (one panel right),
 * unless we're already at the right edge.
 *
 * decomp.me (100%) https://decomp.me/scratch/SRP9z
 */
void ScrollSlotsRight(void)
{
    if (g_slotSlideXLerped != 0xA0)
    {
        g_slotSlideX += 0xA0;
        g_slotSlideFrames = 8;
    }
}

/**
 * Mirror of ScrollSlotsRight going left.
 *
 * decomp.me (100%) https://decomp.me/scratch/W1iA5
 */
void ScrollSlotsLeft(void)
{
    if (g_slotSlideXLerped != -0xA0)
    {
        g_slotSlideX -= 0xA0;
        g_slotSlideFrames = 8;
    }
}

inline u16 inline_fn(unsigned char* arg0)
{
    return *((u16*)arg0);
}

/**
 * Walks the 0x1B-entry × 0x18-byte layout table at D_800F993C+2 and emits
 * one of four primitive shapes per entry (text quad, simple sprite,
 * solid-coloured POLY_F4, or chunked glyph strip). Returns the new prim
 * head.
 *
 * decomp.me (87.28%) https://decomp.me/scratch/UVLSm
 * (bad definitely not functional scratch 91.76% https://decomp.me/scratch/P2gt5)
 * NOTE THAT THIS MAY NOT BE FUNCTIONALLY EQUIVALENT YET!
 */
void* RenderSaveLayoutPrims(void* arg0, s32* arg1)
{
    unsigned char* t0;
    unsigned char* new_var;
    unsigned int new_var8;
    s32* t6;
    s32 s2;
    s32 s5;
    s32 new_var6;
    unsigned char* new_var4;
    s32 s4;
    s32 s3;
    unsigned char* s0;
    u32 t4;
    u32 t8;
    s32 s1;
    unsigned char* t2;
    int new_var3;
    unsigned char* a3;
    u32* new_var10;
    unsigned char* a1;
    unsigned char* a2;
    unsigned char* new_var7;
    s32 t1;
    s32 t3;
    u16 t5;
    unsigned char new_var2;
    u16 a3_16;
    s32 a1_16;
    s32 t9_16;
    int new_var5;
    u8 v0_8;
    int new_var9;
    u16 a2_16;
    unsigned long temp_ul;
    s32 v0;
    s32 v1;
    t0 = arg0;
    t6 = arg1;
    t2 = D_800F993C + 2;
    s2 = 0;
    s5 = 3;
    s4 = &g_slotSlideXLerped;
    s3 = (s32)(&g_slotSlideYLerped);
    s0 = D_800F97FC;
    t4 = 0x00FFFFFF;
    t8 = 0xFF000000;
    do
    {
        if (1)
        {
            s1 = 0x64;
        }
        v0 = t2[-1];
        if (v0 == s5)
        {
            if (g_slotSlideX > 0)
            {
                a3 = (unsigned char*)(g_slotSelectedIndex + 1);
            }
            else
            {
                a3 = 0;
            }
            a1 = (unsigned char*)(((unsigned long)D_800F98AC) + (((unsigned long)a3) * 6));
            *((u32*)(t0 + 4)) = 0x808080;
            new_var9 = 0;
            t0[3] = 9;
            a3 = t0;
            if (((new_var9, *((u32*)(t2 - 2)))) & 2)
            {
                a3[7] = 0x2E;
            }
            else
            {
                a3[7] = 0x2C;
            }
            v0 = 0x20;
            v0 = (*((u16*)(a3 + 8)) = (inline_fn(t2 + 2) + g_slotSlideXLerped) - ((a1[4] * 8) - v0));
            *((u16*)(a3 + 0x18)) = v0;
            v0 = (*((u16*)(a3 + 10)) = (inline_fn(t2 + 4) + g_slotSlideYLerped) - ((a1[5] * 8) - 0x28));
            *((u16*)(a3 + 0x12)) = v0;
            new_var5 = (inline_fn(a3 + 8) + (a1[2] * 8)) - 1;
            *((u16*)(a3 + 0x20)) = new_var5;
            *((u16*)(a3 + 0x10)) = new_var5;
            *((u16*)(a3 + 0x22)) = (v0 = (inline_fn(a3 + 10) + (a1[3] * 8)) - 1);
            *((u16*)(a3 + 0x1A)) = v0;
            v0_8 = a1[0] * 8;
            a3[0x14] = v0_8;
            a3[0x24] = v0_8;
            v0_8 = (new_var2 = a1[1]) * 8;
            a3[0x0D] = v0_8;
            a3[0x15] = v0_8;
            t3 = a3[0x14];
            v0 = (a3[0x1C] = (t3 + (a1[2] * 8)) - 1);
            a3[0x0C] = v0;
            v0 = (a3[0x25] = (a3[0x0D] + (a1[3] * 8)) - 1);
            a3[0x1D] = v0;
            t0 = a3 + 0x28;
            a3 = (unsigned char*)((t2[0] * 0x10) + ((unsigned long)s0));
            v0 = (inline_fn(a3 + 6) << 6) | ((inline_fn(a3 + 4) >> 4) & 0x3F);
            a2 = t2;
            new_var3 = 6;
            *((u16*)((t0 - 0x28) + 0x0E)) = v0;
            a1 = (unsigned char*)((a2[0] * 0x10) + ((unsigned long)s0));
            a2_16 = inline_fn(a1 + 2);
            v0 = ((((((*((u32*)(t2 - 2))) << 3) & 0x60) | ((a1[0x0C] & 3) << 7)) | ((a2_16 & 0x100) >> 4)) |
                  ((inline_fn(a1) & 0x3FF) >> new_var3)) |
                 ((a2_16 & 0x200) * 4);
            *((u16*)((t0 - 0x28) + 0x16)) = v0;
            ;
            ;
            *((u32*)((t0 + (-0x28)) + 0)) = ((*((u32*)((t0 - 0x28) + 0))) & t8) | ((*t6) & t4);
            v1 = (*t6) & t8;
            v0 = ((unsigned long)(t0 - 0x28)) & t4;
            *t6 = v1 | v0;
        }
        else if (v0 == 4)
        {
            if (g_slotSlideX < 0)
            {
                a3 = (unsigned char*)(g_slotSelectedIndex + 1);
            }
            else
            {
                a3 = 0;
            }
            *((u32*)(t0 + 4)) = 0x808080;
            t0[3] = 4;
            t0[7] = s1;
            a1 = (unsigned char*)(((unsigned long)D_800F98F4) + (((unsigned long)a3) * 6));
            if ((*((u32*)(t2 - 2))) & 2)
            {
                t0[7] = 0x66;
            }
            v1 = ((unsigned long)t0) & t4;
            *((u16*)(t0 + 8)) = (inline_fn(t2 - (-2)) + g_slotSlideXLerped) - ((a1[4] * 8) - 0x20);
            *((u16*)(t0 + 10)) = (inline_fn(t2 + 4) + g_slotSlideYLerped) - ((a1[5] * 8) - 0x28);
            t0[12] = a1[0] * 8;
            t0[13] = a1[1] * 8;
            *((u16*)(t0 + 0x10)) = a1[2] * 8;
            *((u16*)(t0 + 0x12)) = a1[3] * 8;
            a3 = s0 + ((t2[0] * 8) * 2);
            v0 = (inline_fn(a3 + 6) << 6) | ((inline_fn(a3 - (-4)) >> 4) & 0x3F);
            *((u16*)(t0 + 0x0E)) = v0;
            v1 = (*((u32*)t0)) & t8;
            ;
            *((u32*)t0) = v1 | ((*t6) & t4);
            v1 = ((unsigned long)t0) & t4;
            t0 += 0x14;
            v0 = (*t6) & t8;
            *t6 = v0 | v1;
            t0[3] = 1;
            a1 = s0 + (t2[0] * 0x10);
            a2_16 = inline_fn(a1 + 2);
            new_var6 = (*((u32*)(t2 - 2))) << 3;
            ;
            *((u32*)(t0 + 4)) = (((((((*((u32*)(a1 + 0x0C))) & 3) << 7) | (new_var6 & 0x60)) | ((a2_16 & 0x100) >> 4)) |
                                  ((inline_fn(a1) & 0x3FF) >> 6)) |
                                 ((a2_16 & 0x200) * 4)) |
                                0xE1000000;
            v1 = (*((u32*)t0)) & t8;
            ;
            new_var8 = v1 | ((*t6) & t4);
            *((u32*)t0) = new_var8;
            t0 += 8;
            v0 = (*t6) & t8;
            // FIX: use the old address of t0 (before the +8) instead of the old value
            *t6 = v0 | ((unsigned long)(t0 - 8) & t4);
        }
        else if (v0 == 2)
        {
            new_var10 = (u32*)t0;
            *((u32*)(t0 + 4)) = 0x40;
            v1 = ((unsigned long)t0) & t4;
            t0[3] = s5;
            t0[7] = 0x62;
            *((u16*)(t0 + 8)) = inline_fn(t2 + 6) + g_slotSlideXLerped;
            do
            {
                *((u16*)(t0 + 10)) = inline_fn(t2 + 8) + g_slotSlideYLerped;
                *((u16*)(t0 + 12)) = inline_fn(t2 + 14);
                *((u16*)(t0 + 14)) = inline_fn(t2 + 16);
                v1 = t8;
                v1 = (*((u32*)t0)) & v1;
                v0 = (*t6) & t4;
                *new_var10 = v1 | v0;
                v1 = (*t6) & t8;
                v0 = ((unsigned long)t0) & t4;
                *t6 = v1 | v0;
                a2 = t0 + 0x10;
                a2[3] = 1;
                *((u32*)(a2 + 4)) = 0xE1000025;
                v1 = (*((u32*)(t0 + 0x10))) & t8;
                v0 = (*t6) & t4;
                *((u32*)(t0 + 0x10)) = v1 | v0;
                // FIX: Use the address of the second sprite, not the lower bits of *t6
                *t6 = ((*t6) & t8) | ((unsigned long)a2 & t4);
                t0 += 0x18;
            } while (0);
        }
        else
        {
            new_var = t0;
            if (v0 != 0)
            {
                t3 = inline_fn(t2 + 14);
                t5 = inline_fn(t2 + 10);
                a1 = s0 + (t2[0] * 0x10);
                a3_16 = inline_fn(a1);
                s2 = (*((u32*)(t2 - 2))) & 1;
                if (s2)
                {
                    do
                    {
                        a1_16 = (*((s16*)(t2 + 2))) + g_slotSlideXLerped;
                        t9_16 = (*((s16*)(t2 + 4))) + g_slotSlideYLerped;
                    } while (0);
                }
                else
                {
                    a1_16 = *((s16*)(t2 + 2));
                    t9_16 = *((s16*)(t2 + 4));
                }
                t1 = 0x80;
                if (((t3 + 1) - 1) < 0x81)
                {
                    t1 = t3;
                }
                a2 = new_var + 4;
                while (1)
                {
                    *((u32*)(a2 + 0)) = 0x808080;
                    a2[-1] = 4;
                    a2[3] = 0x64;
                    if ((*((u32*)(t2 - 2))) & 2)
                    {
                        a2[3] = 0x66;
                    }
                    *((s16*)(a2 + 4)) = a1_16;
                    *((s16*)(a2 + 6)) = t9_16;
                    a2[8] = t5;
                    new_var7 = a2 + 14;
                    *((u16*)(a2 + 12)) = t1;
                    a2[9] = t2[12];
                    *((u16*)new_var7) = inline_fn(t2 + 16);
                    t3 -= t1;
                    a1 = s0 + (t2[0] * 0x10);
                    v0 = (inline_fn(a1 + 6) << 6) | ((inline_fn(a1 + 4) >> 4) & 0x3F);
                    *((u16*)(a2 + 10)) = v0;

                    a2 += 0x14;

                    v1 = (*((u32*)t0)) & t8;
                    v0 = (*t6) & t4;
                    *((u32*)t0) = v1 | v0;
                    v1 = ((unsigned long)t0) & t4;
                    t0 += 0x14;
                    v0 = (*t6) & t8;
                    *t6 = v0 | v1;
                    a2[-1] = 1;
                    a1 = s0 + (t2[0] * 0x10);
                    a2_16 = inline_fn(a1 + 2);
                    t9_16 = ((*((u32*)(t2 - 2))) << 3) & 0x60;
                    *((u32*)a2) = (((((((*((u32*)(a1 + 0x0C))) & 3) << 7) | t9_16) | ((a2_16 & 0x100) >> 4)) |
                                    (((s32)(a3_16 & 0x3FF)) >> 6)) |
                                   ((0, (a2_16 & 0x200) * 4))) |
                                  0xE1000000;
                    a2 += 8;
                    v1 = (*((u32*)t0)) & t8;
                    v0 = ((0, *t6)) & t4;
                    *((u32*)t0) = v1 | v0;
                    v1 = ((unsigned long)t0) & t4;
                    t0 += 8;
                    t9_16 = (*t6) & t8;
                    v0 = t9_16;
                    *t6 = v0 | v1;
                    if (t3 == 0)
                    {
                        break;
                    }
                    t5 ^= 0x80;
                    a1 = (new_var4 = s0 + (t2[0] * 0x10));
                    if (!((*((u32*)(a1 + 0x0C))) & 7))
                    {
                        a3_16 += 0x20;
                    }
                    else
                    {
                        a3_16 += 0x40;
                        t5 = 0;
                    }
                    t1 = 0x80;
                    if (t3 < 0x81)
                    {
                        t1 = t3;
                    }
                    a1_16 += 0x80;
                }
            }
        }
        s2++;
        t2 += 0x18;
    } while (s2 < 0x1B);
    return t0;
}

/**
 * For each of the 11 entries in D_800F97FC (stride 0x10), uploads the
 * CLUT and image data to VRAM and bit-packs the resulting tex-page info
 * back into the entry's control word.
 *
 * decomp.me (100%) https://decomp.me/scratch/lzJHa
 */
unsigned short UploadSaveLayoutTextures(void)
{
    unsigned char* entry_base;
    u32 new_var3;
    unsigned char* control_ptr;
    unsigned char* db;
    u32 offset8;
    unsigned char* secondary;
    int product;
    int new_var2;
    u32 new_var4;
    u32 control;
    RECT rect;
    int counter;
    unsigned char* new_var;
    entry_base = D_800F97FC;
    ;
    for (counter = 0; counter < 11; counter++)
    {
        control_ptr = entry_base;
        secondary = *((unsigned char**)(control_ptr + 8));
        new_var3 = *((u32*)(control_ptr + 0xc));
        control = new_var3;
        db = secondary;
        new_var = db + 0x12;
        control = (control & ((u32)(-8))) | (db[4] & 7);
        *((u32*)(control_ptr + 0xc)) = control;
        product = (*((u16*)(db + 0x10))) * (*((u16*)new_var));
        offset8 = *((u32*)(db + 8));
        db += 8;
        rect.x = *((s16*)((entry_base + 0xc) - 8));
        product++;
        product--;
        rect.y = *((s16*)((entry_base + 0xc) - 6));
        rect.h = 1;
        rect.w = product;

        LoadImage(&rect, (u_long*)(db + 0xc), product);
        secondary = db + offset8;
        new_var2 = 3;
        control = (new_var4 = *((u32*)(control_ptr + 0xc)));
        control = (control & ((u32)(-0x1ff9))) | (((*((u16*)(secondary + 8))) & 0x3ff) << new_var2);
        *((u32*)(entry_base + 0xc)) = control;
        control = control & 0xFF801FFF;
        control = control | (((*((u16*)(secondary + 0xa))) & 0x3ff) << 13);
        *((u32*)(entry_base + 0xc)) = control;
        rect.x = *((s16*)entry_base);
        rect.y = *((s16*)((entry_base + 0xc) - 0xa));
        rect.w = ((*((u32*)(entry_base + 0xc))) >> 3) & 0x3ff;
        rect.h = ((*((u32*)(entry_base + 0xc))) >> 13) & 0x3ff;
        LoadImage(&rect, (u_long*)(secondary + 0xc));
        entry_base += 0x10;
        control_ptr += 0x10;
    }
}

/**
 * Copies a ~13 KB menu-layout table (0xC9A s32s) from one of two source
 * tables into the working layout buffer at D_80042FD8, and updates the
 * companion mode field D_8003EC90.
 *   variant ==  0  -> "default" layout (D_800F9E84), D_8003EC90 = 0xD
 *   variant != 0   -> "alt"     layout (D_800FEF40), D_8003EC90 = 0
 *
 * decomp.me (100%) https://decomp.me/scratch/aPcbW
 */
void LoadMenuLayout(s32 variant)
{
    s32 temp_v0;
    s32* var_a1;
    s32* var_v1;
    u32 var_a0;
    if (variant == 0)
    {
        var_a1 = &D_800F9E84;
        D_8003EC90 = 0xD;
    }
    else
    {
        var_a1 = &D_800FEF40;
        D_8003EC90 = 0;
    }
    D_80046FDE = 0;
    D_80042FC4 = 0;
    do
    {
    } while (0);
    var_a0 = 0;
    var_v1 = &D_80042FD8;
    do
    {
        temp_v0 = *var_a1;
        var_a1++;
        var_a0++;
        *var_v1 = temp_v0;
        var_v1++;
    } while (var_a0 < 0xC9AU);
}

/**
 * Copies a smaller (0x94 s32s) sub-menu layout table into the game-data
 * base at g_gameDataBasePtr. When variant != 0, also OR's bit 0 into
 * (s32*)D_80042FD8[0xB8], marking this slot as "continue mode".
 *
 * decomp.me (100%) https://decomp.me/scratch/CU7Ml
 */
void LoadSubMenuLayout(s32 variant)
{
    s32 temp_v0;
    s32* var_a0;
    int new_var;
    s32* var_v1;
    u32 var_a1;
    if (variant != 0)
    {
        var_a0 = &D_801023F0;
        new_var = sizeof(s32);
        ((s32*)D_80042FD8)[0x2E0 / new_var] |= 1;
    }
    else
    {
        var_a0 = &D_801021A0;
    }
    var_a1 = 0;
    var_v1 = &g_gameDataBasePtr;
    do
    {
        temp_v0 = *var_a0;
        var_a0++;
        var_a1++;
        *var_v1 = temp_v0;
        var_v1++;
    } while (var_a1 < 0x94U);
}
