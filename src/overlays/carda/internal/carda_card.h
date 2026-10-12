#ifndef CARDA_CARD_H
#define CARDA_CARD_H

#include "common.h"
#include "main/field_runtime.h"
#include <libgpu.h>

void carda_reset_entry_ranks(void);
s32 carda_advance_card_sequence(void);
void carda_format_card(void);
void carda_init_card_events(void);
void carda_commit_selected_entry(void);

#endif
