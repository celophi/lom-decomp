#include "decomp1.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/clAOi
 */
void GFX_Transition(s32 skipScreenClear)
{
    FrameBufferUnion* primaryFrame;
    FrameBufferUnion* overlapFb;
    FrameBufferUnion* currentFb;
    FrameBufferUnion* swap;
    RECT rect;
    RECT* frameB;
    s32 count;
    TILE* tile;
    u32* packet;
    DISPENV* dispenv;
    u_long* ot;

    DrawSync(0);
    VSync(0);

    if (skipScreenClear == 0)
    {
        rect.y = 240;
        rect.w = 320;
        rect.x = 0;
        rect.h = 224;

        MoveImage(&rect, 0, 8);
        DrawSync(0);
    }

    primaryFrame = &g_GfxPrimaryFrame;
    overlapFb = primaryFrame;

    dispenv = (DISPENV*)(((u8*)overlapFb) - 0x70);

    overlapFb->overlap.frameA.x = 0;
    frameB = &overlapFb->overlap.frameB;

    overlapFb->overlap.frameA.y = 0;
    overlapFb->overlap.frameA.w = 320;
    overlapFb->overlap.frameA.h = 240;
    overlapFb->overlap.frameB.x = 0;

    frameB->y = 232;
    frameB->w = 320;
    frameB->h = 240;

    SetDefDispEnv(dispenv, 0, 0, 320, 240);
    SetDefDispEnv(&overlapFb->overlap.bufB.dispenv, 0, 232, 320, 240);
    SetDefDrawEnv((DRAWENV*)(((u8*)overlapFb) - 0x5C), 0, 240, 320, 224);
    SetDefDrawEnv(&overlapFb->overlap.bufB.drawenv, 0, 8, 320, 224);

    ot = &overlapFb->overlap.bufB.ot[0];
    overlapFb = (FrameBufferUnion*)(((u8*)overlapFb) - 0x180);
    ((&overlapFb->fb) + 1)->buf.drawenv.dtd = 0;
    overlapFb->fb.buf.drawenv.dtd = 0;

    ClearOTagR(ot, 4);
    PutDispEnv(dispenv);

    currentFb = overlapFb;

    for (count = 0; count < 16; count++)
    {
        ClearOTagR(currentFb->fb.buf.ot, (double)4);

        packet = &currentFb->fb.buf.packetBuffer[0];

        if (skipScreenClear == 0)
        {
            tile = (TILE*)(packet);
            setTile(tile);

            // setRGB0(tile, 32, 32, 32); //(but reversed?)
            tile->b0 = 32;
            tile->g0 = 32;
            tile->r0 = 32;

            setXY0(tile, 0, 0);
            setWH(tile, 320, 224);
            setSemiTrans(tile, 1);
            addPrim(&currentFb->fb.buf.ot[0], tile);

            packet = &currentFb->fb.buf.packetBuffer[4];

            setDrawTPage(packet, 0, 0, 0x40);
            addPrim(&currentFb->fb.buf.ot[0], packet);
        }

        DrawSync(0);
        VSync(0);

        swap = &g_GfxDoubleBuffer;

        if (currentFb == (&g_GfxDoubleBuffer))
        {
            swap = (FrameBufferUnion*)(&currentFb->fb + 1);
        }

        currentFb = swap;

        PutDispEnv(&currentFb->fb.buf.dispenv);
        PutDrawEnv(&currentFb->fb.buf.drawenv);
        DrawOTag(&currentFb->fb.buf.ot[3]);
        func_800157DC();
        cdrom_process_state();
    }

    func_800158E0();
    DrawSync(0);
    VSync(0);

    if (skipScreenClear == 0)
    {
        SetDispMask(0);
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/mXgky
 */
void akao_song_cmd_12c(void)
{
    akao_cmd_c1(g_current_song_handle, 0x12C, 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Oy5Dh
 */
void load_and_play_song(s32 arg0)
{
    u32 addr;
    u32 base;
    s32* info;
    u8* ptr;
    s32 temp;

    if (arg0 == 0xFF)
    {
        return;
    }

    base = (arg0 + 0x93) & 0xFFFF;
    addr = 0x80180000;
    cdrom_queue_read(base, addr);
    cdrom_wait_queue_empty();
    base = 0x80180000;
    info = (s32*)0x80180004;
    ptr = &g_music_data_buffer;

    bcopy((u_char*)((*info) + base), ptr, info[1] - (*info));
    akao_play_sequence_blocking((AkaoSeqHeader*)(info[1] + base), 1);

    temp = akao_play_song(ptr);
    g_current_song_handle = temp;
    akao_cmd_c0(temp, 0x7f);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/DRBPP
 */
void akao_set_song_params(int arg0, s16 arg1, s16 arg2, s16 arg3)
{
    g_akao_song_cmd_arg0 = arg0;

    if ((arg0 << 0x10) < 0)
    {
        g_akao_song_cmd_neg_arg0 = arg0;
    }

    g_akao_song_cmd_arg1 = arg1;
    g_akao_song_cmd_arg2 = arg2;
    g_akao_song_cmd_arg3 = arg3;
}
