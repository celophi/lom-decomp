#ifndef CARDA_WIDGETS_H
#define CARDA_WIDGETS_H

#include "common.h"
#include "main/field_runtime.h"
#include <libgpu.h>

void carda_build_save_file(void);
s32 carda_test_option_flag_2(void);
void* carda_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* carda_draw_progress_bar(POLY_G4* quad, u_long* ot);
void* carda_draw_save_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* carda_draw_overwrite_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* carda_draw_format_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void carda_open_status_dialog(s32 dialog_state);
void* carda_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void carda_enable_choice_toggle(void);

#endif
