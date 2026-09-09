#include "common.h"
extern void akao_stop_sfx_by_id(s32 id);
extern s32 cdrom_get_error_status(void);
extern void field_restore_fade_target(void);
extern void func_800AA02C(void);
extern u8 D_800FE3A0[], D_801226E0[], D_801228D0[], D_801228E0[];
extern u8 D_8011F3D2, D_801227D0;
extern s32 D_801227C8, D_80122984, g_field_draw_count, g_pad_input, g_pad_input_inject;
extern void *g_pad_ctx;
/**
 * @brief Update field actor selection or close the text session and restore saved part bytes.
 */
void func_800A9B88(void)
{
    s32 actor_index;
    u8 *part;
    if ((D_80122984 && !cdrom_get_error_status()) ||
        (!D_80122984 &&
         (g_pad_input == 0x800 || ((*(s32 *)((u8 *)g_pad_ctx + 0x858) & 0x80) &&
                                   *((u8 *)g_pad_ctx + 0x840) && g_pad_input_inject == 0x800))))
    {
        g_field_draw_count = 0;
        D_801227C8 = 0;
        field_restore_fade_target();
        akao_stop_sfx_by_id(0x7E);
        for (actor_index = 0; actor_index < D_801227D0; actor_index++)
        {
            part = D_800FE3A0 + D_801226E0[actor_index] * 0x48;
            part[0x2E] = D_801228D0[actor_index];
            part[0x33] = D_801228E0[actor_index];
        }
        func_800AA02C();
    }
    else
    {
        g_field_draw_count = 1;
        if (D_801227D0 >= 2U)
        {
            g_pad_input |= g_pad_input_inject;
            if (g_pad_input & 0x9000)
            {
                D_8011F3D2 = D_8011F3D2 ? D_8011F3D2 - 1 : D_801227D0 - 1;
                akao_stop_sfx_by_id(0x7D);
            }
            else if (g_pad_input & 0x6000)
            {
                D_8011F3D2 = D_8011F3D2 == D_801227D0 - 1 ? 0 : D_8011F3D2 + 1;
                akao_stop_sfx_by_id(0x7D);
            }
        }
    }
}
