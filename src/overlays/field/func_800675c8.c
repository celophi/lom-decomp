#include "common.h"

/**
 * @brief Render value right-aligned into `digits` columns at the entry's
 *        0x54 field, blanking leading zeros with spaces.
 * @param arg0   Array index (low 16 bits used).
 * @param value  Number to format.
 * @param digits Column count; digits >= 10 are clamped to '9'.
 * @note WIP - not yet byte-matching. The power-of-ten loop loses its guard
 *       to loop rotation. See working/func_800675C8/STATUS.md.
 * @see decomp.me (78.20%) TODO
 */
void func_800675C8(s32 arg0, u32 value, u8 digits)
{
    u8* dst = (u8*)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED054);
    u32 div;
    u32 digit;
    s32 leading;

    leading = 1;
    div = 1;
    digits = digits - 1;
    if (digits != 0)
    {
        do
        {
            div = div * 10;
            digits -= 1;
        } while (digits != 0);
    }
    if (div != 1)
    {
        do
        {
            digit = value / div;
            if ((digit == 0) && (leading != 0))
            {
                *dst = 0x20;
                dst += 1;
            }
            else
            {
                if (digit >= 10)
                {
                    digit = 9;
                }
                *dst = digit + 0x30;
                dst += 1;
                leading = 0;
            }
            value = value % div;
            div = div / 10;
        } while (div != 1);
    }
    *dst = value + 0x30;
    dst[1] = 0;
}

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
