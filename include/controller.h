#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "common.h"
extern void initialize_controllers(s8 enable_actuators);
extern void initialize_controller_vsync(void);
extern void set_controller_vsync_interval(unsigned long interval);
extern void update_controllers(void);
extern void reset_controller_vsync_state(void);

#endif
