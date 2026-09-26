#ifndef WMAP_SEQUENCE_RUNTIME_H
#define WMAP_SEQUENCE_RUNTIME_H

#include "common.h"
#include "sdk/libgte.h"

/** @brief Sequence callback: nonzero initializes; zero advances one update. */
typedef s32 (*WmapSequenceCallback)(s32 initialize);

s32 wmap_run_land_focus(s32 reset);
void wmap_update_sequences(void);
void wmap_start_sequence(WmapSequenceCallback callback);
void wmap_update_callbacks(void);
void wmap_install_callback(WmapSequenceCallback callback);
s32 wmap_step_actor_animation(void* actor_data, void* resource_data);
void wmap_init_sequences(void);
void wmap_draw_model_default(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale);
void wmap_project_focus_cell(void);
s32 wmap_scale_color(CVECTOR color, s32 scale);
void wmap_set_model_transform(VECTOR* translation, SVECTOR* rotation);
void func_8006CFE4(void* actor, void* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void func_8006D014(void* actor, void* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
s32 wmap_find_land_cell(s32 value, s32* row_out, s32* column_out);
void wmap_set_map_rotation(SVECTOR* rotation);
void wmap_reset_focus_screen_position(void);
s32 wmap_run_land_entry(s32 reset);

#endif
