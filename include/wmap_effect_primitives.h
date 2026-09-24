#ifndef WMAP_EFFECT_PRIMITIVES_H
#define WMAP_EFFECT_PRIMITIVES_H

#include "common.h"
#include "sdk/libgte.h"

void func_8006A2FC(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4_value, s32 arg5_value, u32 arg6, void* arg7);
void func_8006A9C4(s32 actor_address, s32 resource_address, s32 first, s32 end, s32 scale,
                   s32 velocity_min, s32 velocity_range, s32 lifetime_min, s32 lifetime_range, s32 initial_z,
                   s32 frame, s32 spawn_interval);
void func_8006ADD0(VECTOR* translation, SVECTOR* rotation);
void func_8006AEE0(void);
void func_8006AFAC(s32 first, s32 end, s32 frame, s32 depth,
                   s32 velocity_base, s32 velocity_divisor, s32 lifetime_base, s32 sequence);
void func_8006B328(s32 first, s32 end, s32 spawn_interval, s32 scale_override, s32 velocity_min, s32 velocity_range, s32 initial_z, s32 frame, s32 x_min, s32 x_range, s32 y_min, s32 y_range, s32 fade_z, s32 initial_scale_target, s32 initial_scale, s32 scale_step, s32 group);
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth);
void func_8006B998(s32 first, s32 end, void* point_data, s32 frame, s32 depth);
void func_8006BC44(s32 arg0, s32 arg1, void* arg2, s32 arg3);
s32 func_8006C0EC(void);
void func_8006C448(void* arg0);

#endif
