#ifndef CARDA_SAVE_H
#define CARDA_SAVE_H

#include "common.h"
#include "main/field_runtime.h"
#include "sdk/libgpu.h"

void* carda_draw_save_flow(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void carda_store_active_record(void);
void carda_open_save_status_dialog(s32 dialog_state);
void* carda_draw_item_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

#endif
