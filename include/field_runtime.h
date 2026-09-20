#ifndef _FIELD_RUNTIME_H
#define _FIELD_RUNTIME_H

#include "common.h"
#include "sdk/libgpu.h"

#define FIELD_ORDERING_TABLE_SIZE 4112
#define FIELD_PRIMITIVE_ARENA_SIZE 15360

/** @brief Ordering table, display setup, and primitive storage for one field frame. */
typedef struct FieldRenderHalf
{
    u_long ordering_table[FIELD_ORDERING_TABLE_SIZE];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT display_rect;
    u8* primitive_cursor;
    u8 primitive_arena[FIELD_PRIMITIVE_ARENA_SIZE];
    DR_TPAGE draw_mode;
} FieldRenderHalf;

s32 run_field_scene(void);
void field_init_text_renderer(FieldRenderHalf* render_buffers);

#endif
