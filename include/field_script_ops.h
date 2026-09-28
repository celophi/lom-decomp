#ifndef FIELD_SCRIPT_OPS_H
#define FIELD_SCRIPT_OPS_H

#include "common.h"

void field_script_test_range(void);
void field_script_switch(void);
void field_script_record_bits(void);
void field_script_wait_text_window(void);
void field_script_wait_actor_idle(void);
void field_script_calculate(void);
void field_script_open_text_window(void);
void field_script_show_text(void);
void field_script_set_variable(s32 variable, s32 value);
void field_script_misc_command(u32 command, s32 operand);
void field_script_start_actor_script(s32 actor_id, s32 entry);
void field_script_start_animation(s32 actor_id, s32 resource);
void field_script_play_sound(s32 sound_id, s32 pan);

#endif
