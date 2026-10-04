#ifndef FIELD_EFFECT_PRIMITIVES_H
#define FIELD_EFFECT_PRIMITIVES_H

#include "field_effect_types.h"

u8* field_render_effect_ring(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
u8* field_render_effect_fan(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
u8* field_render_effect_marker(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
u8* field_render_effect_trail(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
u8* field_render_effect_radial_fan(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
u8* field_render_effect_radial_lines(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);

#endif
