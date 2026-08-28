#include "screen_transition.h"
#include "cdrom.h"
#include "controller.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

typedef struct {
    u_long ot[4];
    u32 packetBuffer[64];
    DISPENV dispenv;
    DRAWENV drawenv;
} GfxBuffer;

typedef struct {
    GfxBuffer buf;
    RECT frame;
} FrameBuffer;

typedef struct {
    RECT frameA;
    GfxBuffer bufB;
    RECT frameB;
} FrameBufferOverlap;

typedef union {
    FrameBuffer fb;
    FrameBufferOverlap overlap;
} FrameBufferUnion;

extern FrameBufferUnion g_GfxDoubleBuffer;
extern FrameBufferUnion g_GfxPrimaryFrame;

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
        update_controllers();
        cdrom_process_state();
    }

    reset_controller_vsync_state();
    DrawSync(0);
    VSync(0);

    if (skipScreenClear == 0)
    {
        SetDispMask(0);
    }
}
