/** @file field_fade.c
 * @brief Field screen fade: target colors and the per-frame blend tile.
 */

#include "common.h"
#include "field_calls.h"
#include "display.h"
#include "field_runtime.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/** @brief Ordering-table entry the fade packets are linked into. */
#define FIELD_FADE_OT_INDEX 0x10
/** @brief Fade channel value that leaves the screen unchanged. */
#define FIELD_FADE_NEUTRAL 0x100
/** @brief Smallest red value drawn with additive blending. */
#define FIELD_FADE_ADDITIVE_THRESHOLD (FIELD_FADE_NEUTRAL + 1)
/** @brief Texture-page word for the additive blend pass. */
#define FIELD_FADE_ADDITIVE_DRAW_MODE getTPage(0, 1, 320, 0)
/** @brief Texture-page word for the subtractive blend pass. */
#define FIELD_FADE_SUBTRACTIVE_DRAW_MODE getTPage(0, 2, 320, 0)
/** @brief Frames the preset and restore fades take. */
#define FIELD_FADE_DEFAULT_FRAMES 5
/** @brief Channel level of the dimmed modal-overlay fade. */
#define FIELD_FADE_MODAL_LEVEL 0xC0
/** @brief Red level of the CD error tint (green and blue stay neutral). */
#define FIELD_FADE_CD_ERROR_RED 0xD0

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} FieldFadePrimitive;

/** @brief Advance a fade packet cursor by the concrete packet just emitted. */
#define FIELD_NEXT_FADE_PRIMITIVE(primitive, type) ((FieldFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/**
 * @brief Fade color triple plus its remaining step count.
 * @note @c duration is the transition length in frames when set as a target,
 *       and the number of remaining interpolation steps while the fade runs.
 */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFade;

/** @brief Saved fade color with no step/duration field. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
} FieldFadeColor;

extern FieldFade g_field_fade_current;
extern FieldFade g_field_fade_target;
extern FieldFadeColor g_field_fade_restore_color;

/**
 * @brief Reset the current and target field fade colors.
 */
void field_reset_fade_state(void)
{
    g_field_fade_current.red = 0;
    g_field_fade_current.green = 0;
    g_field_fade_current.blue = 0;
    g_field_fade_target.red = 0;
    g_field_fade_target.green = 0;
    g_field_fade_target.blue = 0;
    g_field_fade_target.duration = 0;
}

/**
 * @brief Advance the screen fade one step and emit its blend tile + draw mode.
 * @param render_half Render half whose ordering-table entry FIELD_FADE_OT_INDEX receives the packets.
 */
void field_update_and_render_fade(FieldRenderHalf* render_half)
{
    FieldFadePrimitive* primitive = (FieldFadePrimitive*)render_half->primitive_cursor;
    u_long* ordering_table_tag = &render_half->ordering_table[FIELD_FADE_OT_INDEX];
    s32 dr;
    s32 dg;
    s32 db;
    s32 draw_mode;

    if (g_field_fade_target.duration != 0)
    {
        dr = (g_field_fade_target.red - g_field_fade_current.red) / g_field_fade_target.duration;
        dg = (g_field_fade_target.green - g_field_fade_current.green) / g_field_fade_target.duration;
        db = (g_field_fade_target.blue - g_field_fade_current.blue) / g_field_fade_target.duration;
        g_field_fade_target.duration--;
        g_field_fade_current.red += dr;
        g_field_fade_current.green += dg;
        g_field_fade_current.blue += db;
    }
    else
    {
        g_field_fade_current.red = g_field_fade_target.red;
        g_field_fade_current.green = g_field_fade_target.green;
        g_field_fade_current.blue = g_field_fade_target.blue;
    }
    if ((g_field_fade_current.red != FIELD_FADE_NEUTRAL) || (g_field_fade_current.green != g_field_fade_current.red) ||
        (g_field_fade_current.blue != g_field_fade_current.green))
    {
        if (g_field_fade_current.red >= FIELD_FADE_ADDITIVE_THRESHOLD)
        {
            setRGB0(&primitive->tile, g_field_fade_current.red - 1, g_field_fade_current.green - 1, g_field_fade_current.blue - 1);
        }
        else
        {
            if (g_field_fade_current.red == FIELD_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_field_fade_current.red;
            }
            if (g_field_fade_current.green == FIELD_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_field_fade_current.green;
            }
            if (g_field_fade_current.blue == FIELD_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_field_fade_current.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        primitive->tile.w = SCREEN_WIDTH;
        draw_mode = FIELD_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0(&primitive->tile, 0, 0);
        primitive->tile.h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, &primitive->tile);

        primitive = FIELD_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_field_fade_current.red < FIELD_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = FIELD_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = FIELD_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    render_half->primitive_cursor = (u8*)primitive;
}

/**
 * @brief Set both the target and saved restore color for the field fade.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 */
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration)
{
    g_field_fade_target.red = red;
    g_field_fade_restore_color.red = red;
    g_field_fade_target.green = green;
    g_field_fade_restore_color.green = green;
    g_field_fade_target.blue = blue;
    g_field_fade_restore_color.blue = blue;
    g_field_fade_target.duration = duration;
}

/**
 * @brief Set the field fade target used by the CD error overlay.
 */
void field_set_cd_error_fade_target(void)
{
    g_field_fade_target.red = FIELD_FADE_CD_ERROR_RED;
    g_field_fade_target.green = FIELD_FADE_NEUTRAL;
    g_field_fade_target.blue = FIELD_FADE_NEUTRAL;
    g_field_fade_target.duration = FIELD_FADE_DEFAULT_FRAMES;
}

/**
 * @brief Set the field fade target without changing its saved restore color.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 */
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration)
{
    g_field_fade_target.red = red;
    g_field_fade_target.green = green;
    g_field_fade_target.blue = blue;
    g_field_fade_target.duration = duration;
}

/**
 * @brief Restore the saved field fade color over FIELD_FADE_DEFAULT_FRAMES frames.
 */
void field_restore_fade_target(void)
{
    g_field_fade_target.duration = FIELD_FADE_DEFAULT_FRAMES;
    g_field_fade_target.red = g_field_fade_restore_color.red;
    g_field_fade_target.green = g_field_fade_restore_color.green;
    g_field_fade_target.blue = g_field_fade_restore_color.blue;
}

/**
 * @brief Restore the saved field fade color over the requested duration.
 * @param duration Transition duration in frames.
 */
void field_restore_fade_target_with_duration(s16 duration)
{
    g_field_fade_target.duration = duration;
    g_field_fade_target.red = g_field_fade_restore_color.red;
    g_field_fade_target.green = g_field_fade_restore_color.green;
    g_field_fade_target.blue = g_field_fade_restore_color.blue;
}

/**
 * @brief Set the default modal-overlay fade target.
 */
void field_set_default_fade_target(void)
{
    g_field_fade_target.red = FIELD_FADE_MODAL_LEVEL;
    g_field_fade_target.green = FIELD_FADE_MODAL_LEVEL;
    g_field_fade_target.blue = FIELD_FADE_MODAL_LEVEL;
    g_field_fade_target.duration = FIELD_FADE_DEFAULT_FRAMES;
}
