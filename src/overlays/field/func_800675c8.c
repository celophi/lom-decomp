#include "common.h"

/*
 * field_text_format_number (the right-align digit formatter at 0x800675C8)
 * lives in field_text_format_number.c: it needs the gcc272_cdk no-expand-div
 * toolchain, whereas field_build_frame_commands below uses the standard
 * gcc272_cdk (with --expand-div).
 */

void field_prepare_actor_render_commands(s32, s32);      /* extern */
void field_update_actor_animations(void);               /* extern */
void field_update_audio_timer(void);                       /* extern */
void field_update_gover_load(void);                        /* extern */
void field_update_return_to_title_prompt(s32);         /* extern */
void func_80067BBC(s32);                               /* extern */
void func_80067FB0(s32);                               /* extern */
void func_8006BC50(void);                                  /* extern */
void func_8006BFE8(s32);                               /* extern */
void func_800842E0(void);                                  /* extern */
void func_80084700(s32);                               /* extern */
void func_80086FB8(s32);                               /* extern */
void func_8008B73C(void);                                  /* extern */
void func_80096B54(void);                                  /* extern */
void func_80096E60(void);                                  /* extern */
void func_800A2E34(void);                                  /* extern */
void func_800A2E40(s32);                               /* extern */
void func_800A3FB0(void);                                  /* extern */
void func_800A4798(s32);                               /* extern */
void func_800A5794(s32);                               /* extern */
void func_800A64D0(s32);                               /* extern */
void func_800A9E78(u8);                                /* extern */
void func_800AA098(s32);                               /* extern */
void func_800AB214(s32);                               /* extern */
void func_800AD118(s32);                               /* extern */
void func_800AF8E8(s32);                               /* extern */
void func_800B0244(void);                                  /* extern */
void func_800B19FC(void);                                  /* extern */
extern s32 D_800473F8[];
extern s32 D_800F2288[];
extern s32 D_800F2298[];
extern s32 D_800F22C0[];
extern s32 D_800FE754[];
extern s32 D_80105764[];
extern s32 D_8010AE48[];
extern s32 D_8011F3AC[];
extern s32 D_8012269C[];
extern s32 D_801227C8[];
extern s32 g_field_scene_request_pending[];
extern s32 g_frame_counter[];



/**
 * @brief Build one frame's worth of draw commands for the given render half.
 * @param arg0 Render half being drawn.
 * @param arg1 Non-zero when drawing the alternate half.
 * @note WIP - not yet byte-matching. +10 insns; the D_800473F8[0] byte-read/
 *       word-write pair at the top is a guess. See
 *       working/field_build_frame_commands/STATUS.md.
 * @see decomp.me (100%) TODO
 */
void field_build_frame_commands(s32 arg0, s32 arg1)
{
    s32 temp;

    D_800F2288[0] = arg0;
    D_80105764[0] = 0;
    temp = *(u8*)&D_800473F8[0];
    D_800473F8[0] = temp;
    func_800A9E78(temp);
    func_800AA098(arg0);
    func_80067BBC(arg0);
    func_800B0244();
    if (D_800FE754[0] != 0)
    {
        if (D_8010AE48[0] == 0)
        {
            func_80084700(arg0);
        }
    }
    if ((D_800F2298[0] == 0) && (D_800F22C0[0] == 0) && (D_8012269C[0] == 0) && (D_801227C8[0] == 0))
    {
        func_800B19FC();
        if (g_field_scene_request_pending[0] != 0)
        {
            return;
        }
        if (D_8011F3AC[0] == 0)
        {
            func_8006BC50();
        }
    }
    func_800A4798(arg0);
    func_80096B54();
    if ((D_800F2298[0] == 0) && (D_800F22C0[0] == 0) && (D_8012269C[0] == 0) && (D_8011F3AC[0] == 0) && (D_801227C8[0] == 0))
    {
        field_update_actor_animations();
    }
    field_prepare_actor_render_commands(arg0, arg1);
    func_8006BFE8(arg0);
    func_80086FB8(arg0);
    func_800A2E40(arg0);
    func_800A2E34();
    func_800842E0();
    g_frame_counter[0] += 1;
    func_8008B73C();
    func_80067FB0(arg0);
    field_update_return_to_title_prompt(arg0);
    func_80096E60();
    func_800A64D0(arg0);
    func_800AB214(arg0);
    func_800AD118(arg0);
    func_800A5794(arg0);
    func_800AF8E8(arg0);
    func_800A3FB0();
    field_update_audio_timer();
    field_update_gover_load();
}
