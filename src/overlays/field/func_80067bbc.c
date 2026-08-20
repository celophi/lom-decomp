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

/** @brief Fade colour triple plus its remaining step count. */
typedef struct
{
    s16 r;                  // 0x00
    s16 g;                  // 0x02
    s16 b;                  // 0x04
    s16 steps;              // 0x06
} FieldFade;

typedef struct
{
    u_long otag[0x1010];                 // 0x0000
    u8 _pad[0x40B8 - 0x4040];            // 0x4040
    FieldFadePrimitive* cursor;           // 0x40B8
} RenderHalf;

extern FieldFade g_field_fade_target;
extern FieldFade g_field_fade_current;

/**
 * @brief Advance the screen fade one step and emit its blend tile + draw mode.
 * @param ctx Render half whose OT slot 0x40 the packets are linked into.
 * @see decomp.me (100%) TODO
 */
void func_80067BBC(RenderHalf* ctx)
{
    FieldFadePrimitive* primitive = ctx->cursor;
    u_long* ordering_table_tag = &ctx->otag[FIELD_FADE_OT_INDEX];
    s32 dr;
    s32 dg;
    s32 db;
    s32 draw_mode;

    if (g_field_fade_target.steps != 0)
    {
        dr = (g_field_fade_target.r - g_field_fade_current.r) / g_field_fade_target.steps;
        dg = (g_field_fade_target.g - g_field_fade_current.g) / g_field_fade_target.steps;
        db = (g_field_fade_target.b - g_field_fade_current.b) / g_field_fade_target.steps;
        g_field_fade_target.steps = g_field_fade_target.steps - 1;
        g_field_fade_current.r = g_field_fade_current.r + dr;
        g_field_fade_current.g = g_field_fade_current.g + dg;
        g_field_fade_current.b = g_field_fade_current.b + db;
    }
    else
    {
        g_field_fade_current.r = g_field_fade_target.r;
        g_field_fade_current.g = g_field_fade_target.g;
        g_field_fade_current.b = g_field_fade_target.b;
    }
    if ((g_field_fade_current.r != FIELD_FADE_NEUTRAL) ||
        (g_field_fade_current.g != g_field_fade_current.r) ||
        (g_field_fade_current.b != g_field_fade_current.g))
    {
        if (g_field_fade_current.r >= FIELD_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = g_field_fade_current.r - 1;
            primitive->tile.g0 = g_field_fade_current.g - 1;
            primitive->tile.b0 = g_field_fade_current.b - 1;
        }
        else
        {
            if (g_field_fade_current.r == FIELD_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_field_fade_current.r;
            }
            if (g_field_fade_current.g == FIELD_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_field_fade_current.g;
            }
            if (g_field_fade_current.b == FIELD_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_field_fade_current.b;
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
        if (g_field_fade_current.r < FIELD_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = FIELD_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = FIELD_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    ctx->cursor = primitive;
}
