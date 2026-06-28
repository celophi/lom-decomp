#include "decomp1.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/clAOi
 */
void GFX_Transition(s32 skipScreenClear)
{
    FrameBufferUnion* primary_fb;
    FrameBufferUnion* overlap_fb;
    FrameBufferUnion* cur_fb;
    FrameBufferUnion* swap;
    RECT rect;
    RECT* frame_b;
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

    primary_fb = &g_GfxPrimaryFrame;
    overlap_fb = primary_fb;

    dispenv = (DISPENV*)(((u8*)overlap_fb) - 0x70);

    overlap_fb->overlap.frameA.x = 0;
    frame_b = &overlap_fb->overlap.frameB;

    overlap_fb->overlap.frameA.y = 0;
    overlap_fb->overlap.frameA.w = 320;
    overlap_fb->overlap.frameA.h = 240;
    overlap_fb->overlap.frameB.x = 0;

    frame_b->y = 232;
    frame_b->w = 320;
    frame_b->h = 240;

    SetDefDispEnv(dispenv, 0, 0, 320, 240);
    SetDefDispEnv(&overlap_fb->overlap.bufB.dispenv, 0, 232, 320, 240);
    SetDefDrawEnv((DRAWENV*)(((u8*)overlap_fb) - 0x5C), 0, 240, 320, 224);
    SetDefDrawEnv(&overlap_fb->overlap.bufB.drawenv, 0, 8, 320, 224);

    ot = &overlap_fb->overlap.bufB.ot[0];
    overlap_fb = (FrameBufferUnion*)(((u8*)overlap_fb) - 0x180);
    ((&overlap_fb->fb) + 1)->buf.drawenv.dtd = 0;
    overlap_fb->fb.buf.drawenv.dtd = 0;

    ClearOTagR(ot, 4);
    PutDispEnv(dispenv);

    cur_fb = overlap_fb;

    for (count = 0; count < 16; count++)
    {
        ClearOTagR(cur_fb->fb.buf.ot, (double)4);

        packet = &cur_fb->fb.buf.packetBuffer[0];

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
            addPrim(&cur_fb->fb.buf.ot[0], tile);

            packet = &cur_fb->fb.buf.packetBuffer[4];

            setDrawTPage(packet, 0, 0, 0x40);
            addPrim(&cur_fb->fb.buf.ot[0], packet);
        }

        DrawSync(0);
        VSync(0);

        swap = &g_GfxDoubleBuffer;

        if (cur_fb == (&g_GfxDoubleBuffer))
        {
            swap = (FrameBufferUnion*)(&cur_fb->fb + 1);
        }

        cur_fb = swap;

        PutDispEnv(&cur_fb->fb.buf.dispenv);
        PutDrawEnv(&cur_fb->fb.buf.drawenv);
        DrawOTag(&cur_fb->fb.buf.ot[3]);
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
void load_and_play_song(s32 song_index)
{
    u32 cd_dest;
    u32 res_index;
    s32* seq_header;
    u8* seq_dest;
    s32 song_handle;

    if (song_index == 0xFF)
    {
        return;
    }

    res_index = (song_index + 0x93) & 0xFFFF;
    cd_dest = 0x80180000;
    cdrom_queue_read(res_index, cd_dest);
    cdrom_wait_queue_empty();
    cd_dest = 0x80180000;
    seq_header = (s32*)0x80180004;
    seq_dest = &g_music_data_buffer;

    bcopy((u_char*)((*seq_header) + cd_dest), seq_dest, seq_header[1] - (*seq_header));
    akao_play_sequence_blocking((AkaoSeqHeader*)(seq_header[1] + cd_dest), 1);

    song_handle = akao_play_song(seq_dest);
    g_current_song_handle = song_handle;
    akao_cmd_c0(song_handle, 0x7f);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/DRBPP
 */
void akao_set_song_params(int flags, s16 duration, s16 field_id, s16 sub_id)
{
    g_akao_song_cmd_arg0 = flags;

    if ((flags << 0x10) < 0)
    {
        g_akao_song_cmd_neg_arg0 = flags;
    }

    g_akao_song_cmd_arg1 = duration;
    g_akao_song_cmd_arg2 = field_id;
    g_akao_song_cmd_arg3 = sub_id;
}
