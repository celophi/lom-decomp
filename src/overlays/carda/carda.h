#ifndef CARDA_H
#define CARDA_H

#include "common.h"
#include "field_runtime.h"
#include "sdk/libgpu.h"

struct CardaElement;

void carda_init(void* work, s32 mode);
s32 carda_update_frame(FieldRenderHalf* render);
void carda_scroll_to_selection(void);
void* carda_draw_cant_hold_more(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void carda_clear_elements(void);
struct CardaElement* carda_alloc_element(void);
void carda_deactivate_primary_element(void);

#endif
