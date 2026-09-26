#ifndef WMAP_SEQUENCE_RUNTIME_H
#define WMAP_SEQUENCE_RUNTIME_H

#include "common.h"
#include "sdk/libgte.h"

/** @brief Sequence callback: nonzero initializes; zero advances one update. */
typedef s32 (*WmapSequenceCallback)(s32 initialize);

void func_8006C754(void);
s32 func_8006C81C(s32 arg0);
void func_8006C894(void);
void func_8006C8AC(void);
void func_8006C8E0(void);
void func_8006C930(void);
void func_8006C964(void);
void func_8006C984(void);
void func_8006C9B8(void);
void func_8006C9F4(void);
void func_8006CA28(void);
void func_8006CAC0(WmapSequenceCallback callback);
void func_8006CB60(void);
void wmap_install_callback(WmapSequenceCallback callback);
s32 wmap_step_actor_animation(void* actor_data, void* resource_data);
void func_8006CD18(void);
void func_8006CD98(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale);
void func_8006CDDC(void);
s32 func_8006CF40(CVECTOR color, s32 scale);
void func_8006CFA8(VECTOR* translation, SVECTOR* rotation);
void func_8006CFE4(void* arg0, void* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void func_8006D014(void* actor, void* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
s32 wmap_find_land_cell(s32 value, s32* row_out, s32* column_out);
void func_8006D150(SVECTOR* rotation);
void func_8006D190(void);
s32 func_8006D244(s32 arg0);
void func_8006D2BC(void);
void func_8006D2D4(void);
void func_8006D310(void);
void func_8006D3A0(void);
void func_8006D3B8(void);
void func_8006D3E4(void);
void func_8006D420(void);
void func_8006D4B0(void);
void func_8006D4F0(void);

#endif
