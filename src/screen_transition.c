#include "screen_transition.h"
#include "cdrom.h"
#include "controller.h"
#include "display.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define TRANSITION_OT_SIZE 4
#define TRANSITION_PACKET_WORDS 64
#define TRANSITION_FRAME_COUNT 16
#define TRANSITION_FADE_LEVEL 32
#define TRANSITION_BLEND_SUBTRACT 2
#define TRANSITION_SUBTRACT_TPAGE getTPage(0, TRANSITION_BLEND_SUBTRACT, 0, 0)

/** @brief GPU command space used to darken one frame. */
typedef union
{
    u32 words[TRANSITION_PACKET_WORDS];
    struct
    {
        TILE tile;
        DR_TPAGE draw_page;
    } fade;
} TransitionPackets;

/** @brief Rendering workspace for one half of the screen transition. */
typedef struct
{
    u_long ot[TRANSITION_OT_SIZE];
    TransitionPackets packets;
    DISPENV display;
    DRAWENV draw;
    RECT display_rect;
} TransitionFrame;

static TransitionFrame g_transition_frames[2];

/**
 * @brief Fade out the screen while continuing controller and CD processing.
 * @param skip_fade Nonzero waits without fading or disabling the display.
 * @see decomp.me (100%) https://decomp.me/scratch/clAOi
 */
void screen_transition(s32 skip_fade)
{
    TransitionFrame* frames;
    TransitionFrame* current;
    TransitionFrame* next;
    RECT rect;
    RECT* front_rect;
    RECT* back_rect;
    s32 frame;
    TILE* tile;
    void* packet;
    DISPENV* display;
    u_long* ot;

    DrawSync(0);
    VSync(0);

    if (skip_fade == 0)
    {
        rect.y = SCREEN_HEIGHT;
        rect.w = SCREEN_WIDTH;
        rect.x = 0;
        rect.h = VRAM_DRAW_HEIGHT;
        MoveImage(&rect, 0, VRAM_BACK_DRAW_Y);
        DrawSync(0);
    }

    front_rect = &g_transition_frames[0].display_rect;
    display = &g_transition_frames[0].display;
    front_rect->x = 0;
    back_rect = &g_transition_frames[1].display_rect;
    front_rect->y = 0;
    setWH(front_rect, SCREEN_WIDTH, SCREEN_HEIGHT);
    setRECT(back_rect, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);

    SetDefDispEnv(display, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_transition_frames[1].display, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_transition_frames[0].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_transition_frames[1].draw, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    ot = g_transition_frames[1].ot;
    frames = g_transition_frames;
    frames[1].draw.dtd = 0;
    frames[0].draw.dtd = 0;
    ClearOTagR(ot, TRANSITION_OT_SIZE);
    PutDispEnv(display);
    current = frames;

    for (frame = 0; frame < TRANSITION_FRAME_COUNT; frame++)
    {
        ClearOTagR(current->ot, TRANSITION_OT_SIZE);

        packet = &current->packets.fade.tile;

        if (skip_fade == 0)
        {
            tile = packet;
            setTile(tile);
            tile->b0 = TRANSITION_FADE_LEVEL;
            tile->g0 = TRANSITION_FADE_LEVEL;
            tile->r0 = TRANSITION_FADE_LEVEL;
            setXY0(tile, 0, 0);
            setWH(tile, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
            setSemiTrans(tile, 1);
            addPrim(&current->ot[0], tile);

            packet = &current->packets.fade.draw_page;
            setDrawTPage(packet, 0, 0, TRANSITION_SUBTRACT_TPAGE);
            addPrim(&current->ot[0], packet);
        }

        DrawSync(0);
        VSync(0);
        next = &g_transition_frames[0];
        if (current == &g_transition_frames[0])
        {
            next = &current[1];
        }
        current = next;

        PutDispEnv(&current->display);
        PutDrawEnv(&current->draw);
        DrawOTag(&current->ot[TRANSITION_OT_SIZE - 1]);
        update_controllers();
        cdrom_process_state();
    }

    reset_controller_vsync_state();
    DrawSync(0);
    VSync(0);
    if (skip_fade == 0)
    {
        SetDispMask(0);
    }
}
