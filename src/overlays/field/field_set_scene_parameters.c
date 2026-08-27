#include "common.h"

extern s32 D_80115888;
extern s32 D_80115898;
extern s32 D_8011589C;
extern s32 D_801178B0;
extern s32 D_801178BC;
extern s32 D_801178C0;
extern s32 g_field_scene_request_pending;

void field_set_scene_parameters(s32 arg0, s32 arg1, u32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    D_801178B0 = arg0;
    D_801178BC = arg1;
    D_80115888 = arg2;
    D_80115898 = arg3;
    g_field_scene_request_pending = 1;
    D_801178C0 = arg4;
    D_8011589C = arg5;
}
