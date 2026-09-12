#include "common.h"

extern void cdrom_stream(s32, u32), cdrom_wait_queue_empty(void), field_text_reset_windows(void),
    func_80084240(void), func_800A3938(s32, s32), func_800AA7A4(void), func_800AA90C(s32),
    func_800C3BB0(void);
extern void func_80140004(u32, void *, s32, s32, s32, void *, s32), func_80140024(u32, s32);
extern s32 func_801405B0(s32);
extern s32 D_80105880[];
extern u8 D_801226B8[], D_801226F0[];
extern s32 D_8011F424, D_801227D4, D_8012291C, D_80122984, D_801229F4, g_active_script,
    g_script_repeat_count;
/**
 * @brief Run the menu overlay and dispatch its requested follow-up screens.
 * @param render_buffer_addr Address of the pair of MENU render buffers.
 * @note Fixed entry addresses are reused by the overlays loaded before each call.
 */
void func_800AA570(s32 render_buffer_addr)
{
    s32 var_a0;
    u8 *screen;
    s32 *repeat, *active, *name_id, *history;
    u8 *initial, *custom;
    s32 temp_v0;
    s32 var_a3;

    screen = (u8 *)0x801ED600;
    var_a0 = D_8012291C;
    D_80122984 = 0;
    screen[0x91] = 0;
    screen[0x92] = 0;
    screen[0x13F] = 0;
    screen[0x140] = 0;
    if (var_a0 != 0 || D_80105880[0] != 0 || D_80105880[7] != 0 || D_80105880[14] != 0)
    {
        func_800AA7A4();
        return;
    }
    goto start;
finished:
    func_800AA90C(1);
    goto cleanup;
start:
    func_800A3938(0x80, 0x80);
    func_80084240();

    g_active_script = 0;

    for (;;)
    {
        cdrom_stream(6, 0x80140000);
        cdrom_wait_queue_empty();
        temp_v0 = func_801405B0(render_buffer_addr);
        if (temp_v0 == 0)
        {
            goto finished;
        }
        {
            if (temp_v0 == 0xA)
            {
                func_800AA90C(1);
                cdrom_stream(9, 0x80140000);
                cdrom_wait_queue_empty();
                func_80140024(0x80150000, 1);
                func_800C3BB0();
                func_80084240();
                field_text_reset_windows();
                g_active_script = temp_v0;
                g_script_repeat_count = 0;
            }
            else
            {
                func_800AA90C(1);
                cdrom_stream(5, 0x80140000);
                cdrom_wait_queue_empty();
                if ((u32)(temp_v0 - 0xB) < 2U)
                {
                    func_80140004(0x80160000, D_801226F0, D_801227D4, 1, D_801229F4, D_801226B8, 0);
                }
                else
                {
                    func_80140004(0x80160000, D_801226F0, D_801227D4, temp_v0, D_801229F4,
                                  D_801226B8, 0);
                }
                field_text_reset_windows();
                g_script_repeat_count = D_801229F4;
                if ((u32)(temp_v0 - 0xB) < 2U)
                {
                    g_script_repeat_count = 0;
                    g_active_script = temp_v0;
                }
                else
                {
                    g_active_script = D_8011F424 + 1;
                }
            }
            continue;
        }
        break;
    }
cleanup:
    func_80084240();
}
