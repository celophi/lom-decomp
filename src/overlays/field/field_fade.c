#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

#define FIELD_FADE_OT_INDEX 0x10
#define FIELD_FADE_NEUTRAL 0x100
#define FIELD_FADE_ADDITIVE_THRESHOLD (FIELD_FADE_NEUTRAL + 1)
#define FIELD_FADE_ADDITIVE_DRAW_MODE 0x25
#define FIELD_FADE_SUBTRACTIVE_DRAW_MODE 0x45

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} FieldFadePrimitive;

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define FIELD_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((FieldFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/**
 * @brief Fade colour triple plus its remaining step count.
 * @note The 4th field is the transition duration in frames when set as a
 *       target, and the count of remaining interpolation steps while the fade
 *       is being advanced; they are the same 0x06 slot.
 */
typedef struct
{
    s16 red;                // 0x00
    s16 green;              // 0x02
    s16 blue;               // 0x04
    s16 duration;           // 0x06
} FieldFade;

/** @brief Saved fade colour with no step/duration field. */
typedef struct
{
    s16 red;                // 0x00
    s16 green;              // 0x02
    s16 blue;               // 0x04
} FieldFadeColor;

typedef struct
{
    u_long otag[0x1010];                 // 0x0000
    u8 _pad[0x40B8 - 0x4040];            // 0x4040
    FieldFadePrimitive* cursor;          // 0x40B8
} RenderHalf;

extern FieldFade g_field_fade_current;
extern FieldFade g_field_fade_target;
extern FieldFadeColor g_field_fade_restore_color;

/**
 * @brief Reset the current and target field fade colors.
 * @see decomp.me (100%) TODO
 */
void field_reset_fade_state(void) {
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
 * @param ctx Render half whose OT slot 0x40 the packets are linked into.
 * @see decomp.me (100%) TODO
 */
void field_update_and_render_fade(RenderHalf* ctx)
{
    FieldFadePrimitive* primitive = ctx->cursor;
    u_long* ordering_table_tag = &ctx->otag[FIELD_FADE_OT_INDEX];
    s32 dr;
    s32 dg;
    s32 db;
    s32 draw_mode;

    if (g_field_fade_target.duration != 0)
    {
        dr = (g_field_fade_target.red - g_field_fade_current.red) / g_field_fade_target.duration;
        dg = (g_field_fade_target.green - g_field_fade_current.green) / g_field_fade_target.duration;
        db = (g_field_fade_target.blue - g_field_fade_current.blue) / g_field_fade_target.duration;
        g_field_fade_target.duration = g_field_fade_target.duration - 1;
        g_field_fade_current.red = g_field_fade_current.red + dr;
        g_field_fade_current.green = g_field_fade_current.green + dg;
        g_field_fade_current.blue = g_field_fade_current.blue + db;
    }
    else
    {
        g_field_fade_current.red = g_field_fade_target.red;
        g_field_fade_current.green = g_field_fade_target.green;
        g_field_fade_current.blue = g_field_fade_target.blue;
    }
    if ((g_field_fade_current.red != FIELD_FADE_NEUTRAL) ||
        (g_field_fade_current.green != g_field_fade_current.red) ||
        (g_field_fade_current.blue != g_field_fade_current.green))
    {
        if (g_field_fade_current.red >= FIELD_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = g_field_fade_current.red - 1;
            primitive->tile.g0 = g_field_fade_current.green - 1;
            primitive->tile.b0 = g_field_fade_current.blue - 1;
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
    ctx->cursor = primitive;
}

/**
 * @brief Set both the target and saved restore color for the field fade.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration) {
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
 * @see decomp.me (100%) TODO
 */
void field_set_cd_error_fade_target(void) {
    g_field_fade_target.red = 0xD0;
    g_field_fade_target.green = 0x100;
    g_field_fade_target.blue = 0x100;
    g_field_fade_target.duration = 5;
}

/**
 * @brief Set the field fade target without changing its saved restore color.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration) {
    g_field_fade_target.red = red;
    g_field_fade_target.green = green;
    g_field_fade_target.blue = blue;
    g_field_fade_target.duration = duration;
}

/**
 * @brief Restore the saved field fade color over five frames.
 * @see decomp.me (100%) TODO
 */
void field_restore_fade_target(void) {
    g_field_fade_target.duration = 5;
    g_field_fade_target.red = (u16)g_field_fade_restore_color.red;
    g_field_fade_target.green = (u16)g_field_fade_restore_color.green;
    g_field_fade_target.blue = (u16)g_field_fade_restore_color.blue;
}

/**
 * @brief Restore the saved field fade color over the requested duration.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_restore_fade_target_with_duration(s16 duration) {
    g_field_fade_target.duration = duration;
    g_field_fade_target.red = (u16)g_field_fade_restore_color.red;
    g_field_fade_target.green = (u16)g_field_fade_restore_color.green;
    g_field_fade_target.blue = (u16)g_field_fade_restore_color.blue;
}

/**
 * @brief Set the default modal-overlay fade target.
 * @see decomp.me (100%) TODO
 */
void field_set_default_fade_target(void) {
    g_field_fade_target.red = 0xC0;
    g_field_fade_target.green = 0xC0;
    g_field_fade_target.blue = 0xC0;
    g_field_fade_target.duration = 5;
}
