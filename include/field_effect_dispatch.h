#ifndef FIELD_EFFECT_DISPATCH_H
#define FIELD_EFFECT_DISPATCH_H

#include "common.h"

/** @brief Rendering state shared by field actor and effect packet builders. */
typedef struct
{
    u8 pad0[0x40];
    u32 ordering_table;
    u8 pad44[0x40B8 - 0x44];
    s32 *packet_cursor;
} FieldRenderContext;

void field_render_effects(FieldRenderContext *render_context);

#endif
