#ifndef CARDA_CARD_H
#define CARDA_CARD_H

#include "common.h"
#include "main/field_runtime.h"
#include "sdk/libgpu.h"

void carda_reset_entry_ranks(void);
s32 carda_advance_card_sequence(void);
void carda_reset_to_new_save_entry(void);
void carda_init_card_events(void);
void carda_commit_selected_entry(void);
s32 carda_card_lacks_free_blocks(void);
void carda_erase_placeholder_files(void);

#endif
