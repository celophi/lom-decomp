/** @file field_draw_state.c
 * @brief Field draw-state reset and the debug load bar.
 */

#include "common.h"
#include "field_calls.h"
#include "field_actor_runtime.h"
#include "field_runtime.h"
#include "sdk/libgpu.h"

extern s32 g_field_camera_offset_x;
extern s32 g_field_camera_offset_y;
extern s32 g_field_camera_offset_z;

/** @brief LINE_F2 with its color word addressable as one value. */
typedef struct
{
    u_long tag;
    u32 rgbc;
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
} FieldBarLine;

#define FIELD_LOAD_BAR_GREY 0x808080   /**< r = g = b = 0x80. */
#define FIELD_LOAD_BAR_YELLOW 0x8080   /**< r = g = 0x80. */
#define FIELD_LOAD_BAR_WARN 0x200      /**< Load from which the bar turns yellow. */
#define FIELD_LOAD_BAR_HIGH 0x300      /**< Load from which the bar turns red. */
#define FIELD_LOAD_BAR_Y 16

/**
 * @brief Clear the camera offsets, the effect pool and the global color scale.
 */
void field_reset_draw_state(void)
{
    g_field_camera_offset_z = 0;
    g_field_camera_offset_y = 0;
    g_field_camera_offset_x = 0;
    field_reset_effect_pool();
    field_reset_global_color_scale();
}

/**
 * @brief Draw a debug bar whose length and color show @p load.
 *
 * The bar is grey, yellow from FIELD_LOAD_BAR_WARN and red from
 * FIELD_LOAD_BAR_HIGH, and is @p load / 4 pixels long. Nothing calls it in
 * the retail build.
 *
 * @param ctx Render half whose first ordering-table entry receives the line.
 * @param load Value the bar represents.
 */
void field_draw_load_bar(FieldRenderHalf* ctx, s32 load)
{
    FieldBarLine* line = (FieldBarLine*)ctx->primitive_cursor;

    line->rgbc = FIELD_LOAD_BAR_GREY;
    if (load >= FIELD_LOAD_BAR_WARN)
    {
        line->rgbc = FIELD_LOAD_BAR_YELLOW;
    }
    if (load >= FIELD_LOAD_BAR_HIGH)
    {
        /* Yellow shifted down one byte leaves only red. */
        line->rgbc = line->rgbc >> 8;
    }
    setLineF2(line);
    line->y1 = FIELD_LOAD_BAR_Y;
    line->y0 = FIELD_LOAD_BAR_Y;
    line->x0 = 0;
    line->x1 = load >> 2;
    addPrim(&ctx->ordering_table[0], line);
    line += 1;
    ctx->primitive_cursor = (u8*)line;
}
