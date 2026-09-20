#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "common.h"
void initialize_controllers(s8 enable_actuators);
void shutdown_controllers(void);
void initialize_controller_vsync(void);
void set_controller_vsync_interval(u32 interval);
void update_controllers(void);
void reset_controller_vsync_state(void);

#endif
