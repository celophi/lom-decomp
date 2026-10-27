#include "common/gpu_packet.h"
#include "common/screen_fade.h"

/**
 * @brief Step the current fade level toward the target, then emit the full-screen
 *        fade tile and its blend-mode packet unless the level is neutral.
 * @param tile Next free position in the packet buffer.
 * @param ordering_table Ordering-table entry receiving the fade.
 * @return First free primitive-buffer address after the emitted packets.
 */
void* render_screen_fade(TILE* tile, u_long* ordering_table)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;
    DR_TPAGE* tpage;

    if (g_screen_fade_target.steps_remaining != 0)
    {
        red_step = (g_screen_fade_target.red - g_screen_fade_current.red) / g_screen_fade_target.steps_remaining;
        green_step = (g_screen_fade_target.green - g_screen_fade_current.green) / g_screen_fade_target.steps_remaining;
        blue_step = (g_screen_fade_target.blue - g_screen_fade_current.blue) / g_screen_fade_target.steps_remaining;
        g_screen_fade_target.steps_remaining--;
        g_screen_fade_current.red += red_step;
        g_screen_fade_current.green += green_step;
        g_screen_fade_current.blue += blue_step;
    }
    else
    {
        g_screen_fade_current.red = g_screen_fade_target.red;
        g_screen_fade_current.green = g_screen_fade_target.green;
        g_screen_fade_current.blue = g_screen_fade_target.blue;
    }

    if ((g_screen_fade_current.red != FADE_NEUTRAL) || (g_screen_fade_current.green != g_screen_fade_current.red) ||
        (g_screen_fade_current.blue != g_screen_fade_current.green))
    {
        if (g_screen_fade_current.red >= FADE_ADDITIVE_THRESHOLD)
        {
            setRGB0(tile, g_screen_fade_current.red - 1, g_screen_fade_current.green - 1, g_screen_fade_current.blue - 1);
        }
        else
        {
            if (g_screen_fade_current.red == FADE_NEUTRAL)
            {
                tile->r0 = 0;
            }
            else
            {
                tile->r0 = ~g_screen_fade_current.red;
            }

            if (g_screen_fade_current.green == FADE_NEUTRAL)
            {
                tile->g0 = 0;
            }
            else
            {
                tile->g0 = ~g_screen_fade_current.green;
            }

            if (g_screen_fade_current.blue == FADE_NEUTRAL)
            {
                tile->b0 = 0;
            }
            else
            {
                tile->b0 = ~g_screen_fade_current.blue;
            }
        }

        setTile(tile);
        setSemiTrans(tile, 1);
        tile->w = SCREEN_WIDTH;
        draw_mode = BLEND_ADD_TPAGE;
        SET_YX0(tile, 0, 0);
        tile->h = SCREEN_HEIGHT;
        addPrim(ordering_table, tile);

        tile++;
        tpage = (DR_TPAGE*)tile;

        if (g_screen_fade_current.red < FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = BLEND_SUBTRACT_TPAGE;
        }

        setDrawTPage(tpage, 0, 0, draw_mode);
        addPrim(ordering_table, tpage);

        tile = (TILE*)(tpage + 1);
    }

    return tile;
}
