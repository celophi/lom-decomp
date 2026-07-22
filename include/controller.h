#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "common.h"
#include "psyq/libetc.h"

extern s32 g_previous_controller_vsync_callback;
extern u8 g_controller_vsync_sample_count;
extern u8 g_controller_vsync_counter;

/**
 * @brief Per-port controller state, including the current sample and LIBPAD setup state.
 * @note The leading 0x90 bytes contain four 16-byte samples and unmapped history storage.
 */
typedef struct ControllerPortState
{
  u8 pad0[0x20];
  u8 device_type;
  u8 analog_direction_bits;
  u16 held_buttons;
  u16 pressed_buttons;
  u16 repeat_buttons;
  s16 right_stick_x;
  s16 right_stick_y;
  s16 left_stick_x;
  s16 left_stick_y;
  u8 pad1[0x90 - 0x30];
  u8 actuators_enabled;
  u8 small_motor_command;
  u16 actuator_config;
  u8 actuator_header;
  u8 small_motor_value;
  u8 large_motor_value;
  u8 actuator_value_2;
  u8 small_motor_index;
  u8 large_motor_index;
  u8 actuator_align_2;
  u8 actuator_align_3;
  u8 actuator_align_4;
  u8 actuator_align_5;
  u8 face_repeat_timer_0;
  u8 face_repeat_timer_1;
  u8 face_repeat_timer_2;
  u8 face_repeat_timer_3;
  u8 direction_repeat_timer_0;
  u8 direction_repeat_timer_1;
  u8 direction_repeat_timer_2;
  u8 direction_repeat_timer_3;
  u8 right_stick_center_x;
  u8 right_stick_center_y;
  u8 left_stick_center_x;
  u8 left_stick_center_y;
  u8 actuator_count;
  u8 small_motor_power;
  u8 large_motor_power;
  u8 port_id;
} ControllerPortState;

extern void controller_poll(void);
extern void clear_controller_sample(void *sample);
extern void PadStartCom();
extern void PadInitDirect(void *port1_buffer, void *port2_buffer);
extern u32 PadGetState(u8 port);
extern s32 PadInfoMode(u8 port, u8 info_mode, s32 index);
extern int PadInfoAct(u8 port, s32 actuator, u8 property);
extern void PadSetActAlign(u8 port, void *alignment);
extern void PadSetMainMode(u8 port, s32 mode, u8 lock);
extern void PadSetAct(u8 port, void *actuator_data, int length);
extern s32 PadChkVsync(void);
extern void PadStopCom(void);
extern void controller_vsync_callback(void);
extern void initialize_controllers(s8 enable_actuators);
extern void shutdown_controllers(void);
extern void initialize_controller_vsync(void);
extern void set_controller_vsync_interval(unsigned long interval);
extern void update_controllers(void);
extern void reset_controller_vsync_state(void);
extern void accumulate_controller_sample(void *port_state);
extern void merge_latest_controller_sample(void *port_state);
extern void copy_controller_sample(void *source, void *destination);
void poll_controller_port(ControllerPortState* port, s32* actuator_power_total);

#endif
